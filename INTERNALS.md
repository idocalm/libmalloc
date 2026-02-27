# INTERNALS

This allocator is built as a small slab allocator with bucket isolation and fixed sizeclasses. At the moment the allocator intentionally handles only allocations up to 32KB; anything larger is rejected.

The runtime state lives in the global `allocator_t` object and a global chunk metadata pool. Because these structures are shared by every thread, all allocation and deallocation operations **run under a lock.**

## Core principles

A `block_header_t` lives directly before each user payload. It stores:
-  a magic value which we can use just like a stack canary cookie
- the sizeclass index
- some allocation flags
- the requested size. 

During `free` and `realloc`, this header is used to verify ownership and compute how many bytes should be treated as user data.

A `chunk_t` represents one slab region dedicated to one `(bucket, sizeclass)` pair. 
It tracks the mapped address range (`base`, `chunk_bytes`), block layout (`stride`, `capacity`), dynamic state (`free_count`, `free_list`), and linkage pointers for membership in state lists.

A `sizeclass_allocator_t` is a lane for one `(bucket, sizeclass)` combination. 
Each lane keeps four chunk lists named `current`, `partial`, `full`, and `empty`. 

- `current` is the preferred fast-path source
- `partial` contains chunks with free blocks
- `full` contains full chunks
- `empty` contains fully freed chunks, that we save for future use

The `allocator_t` contains all lanes indexed by bucket and sizeclass

## malloc

When a request comes in, `alloc_impl`:
1.  Reject invalid sizes
2.  Under lock, 
   - initialize state on first use if needed.
   - find the correct sizeclass for the allocation.
   - compute the bucket id from caller. 
3. At this point, we have a lane: a set of `(bucket, sizeclass)`. The allocator then chooses a chunk from the lane, with this priority: 
   - `current`, 
   - `partial`, 
   - `empty`, 
   - if none were useful, allocate a fresh chunk!
4. If the chunk was previously empty, it moves from `empty` to `partial`. 
5. If the allocation used the last free block, it moves from `partial` to `full` and the lane clears `current`.

## free

`free_impl` ignores null pointers.
For non-null pointers:
1. Lock 
2. Check that the allocator is initialized, 
3. find the owning chunk 
4. validates pointer shape
5. validates the block header using the `magic`.

If the validation succeeded, the allocator clears the in-use flag and zeroes the entire previously requested payload size.  **(zero-on-free)**

The block is then pushed back onto the chunk freelist. Chunk list state is updated - A first free from a full chunk moves it from `full` to `partial`; a fully freed chunk moves from `partial` to `empty`.

## realloc

For valid in-range pointers, the function checks ownership and header integrity.
- If the new size still fits the same sizeclass capacity, it updates only `requested_size` and returns the same pointer. 

- If it does not fit, it allocates a new block, copies `min(old_size, new_size)`, frees the old block, and returns the new pointer.