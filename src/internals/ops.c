#include "ops.h"
#include <string.h>

static sizeclass_allocator_t *sizeclass_for_request(allocator_t *state, usize size, void *caller, usize *out_sizeclass)
{
    usize sizeclass;
    u32 bucket;

    sizeclass = sizeclass_index_for_size(size);
    if (sizeclass >= ALLOC_SIZECLASS_COUNT)
        return NULL;

    bucket = bucket_for_caller(caller, state->seed);

    *out_sizeclass = sizeclass;
    return &state->sizeclasses[bucket][sizeclass];
}

static void *alloc_small_locked(allocator_t *state, usize size, void *caller)
{
    sizeclass_allocator_t *sizeclass_alloc;
    chunk_t *chunk;
    usize sizeclass;
    free_block_t *node;
    block_header_t *header;
    u32 before_free;

    sizeclass_alloc = sizeclass_for_request(state, size, caller, &sizeclass);
    if (sizeclass_alloc == NULL)
        return NULL;


    // start with current
    chunk = sizeclass_alloc->current;
    if (chunk == NULL || chunk->free_count == 0) // if full or no current, try partial or empty
    {
        if (sizeclass_alloc->partial != NULL)
            chunk = sizeclass_alloc->partial;
        else
        {
            // try empty
            chunk = chunk_list_pop(&sizeclass_alloc->empty);
            if (chunk == NULL)
            {
                // carve a new chunk, add to empty, and use it
                chunk = chunk_create_slab((u16)bucket_for_caller(caller, state->seed), (u16)sizeclass);
                if (chunk == NULL)
                    return NULL;

                chunk_list_push(&sizeclass_alloc->empty, chunk);
            }
        }

        // current is now the one we found
        sizeclass_alloc->current = chunk;
    }

    if (chunk->free_list == NULL)
        return NULL;

    // pop the free block from the chunk's free list
    node = chunk->free_list;
    chunk->free_list = node->next;

    // Decrement free count
    before_free = chunk->free_count;
    chunk->free_count -= 1;

    // Move chunk from empty to partial on first allocation
    if (before_free == chunk->capacity)
    {
        chunk_list_remove(&sizeclass_alloc->empty, chunk);
        chunk_list_push(&sizeclass_alloc->partial, chunk);
    }

    // Move chunk from partial to full if this allocation fills it up
    if (chunk->free_count == 0)
    {
        chunk_list_remove(&sizeclass_alloc->partial, chunk);
        chunk_list_push(&sizeclass_alloc->full, chunk);
        sizeclass_alloc->current = NULL;
    }

    // fill in block header
    header = ((block_header_t *)node) - 1;
    header->magic = BLOCK_MAGIC;
    header->sizeclass = (u16)sizeclass;
    header->flags = BLOCK_FLAG_IN_USE;
    header->requested_size = size;

    // return pointer to user data
    return (void *)node;
}

void *alloc_impl(usize size, void *caller)
{
    allocator_t *state = state_get();

    if (size == 0 || size > ALLOC_SMALL_MAX)
        return NULL;

    alloc_lock();

    alloc_init();
    void *out = alloc_small_locked(state, size, caller);

    alloc_unlock();
    return out;
}

void free_impl(void *ptr)
{
    allocator_t *state;
    chunk_t *chunk;
    block_header_t *header;

    if (ptr == NULL)
        return;

    alloc_lock();
    state = state_get();

    if (!state->initialized) // stop if allocator not initialized
    {
        DEBUG_LOG("allocator not initialized on free");
        alloc_unlock();
        return;
    }

    chunk = find_chunk_for_ptr(ptr);
    if (chunk == NULL || !pointer_matches_chunk_block(chunk, ptr))
    {
        alloc_unlock();
        return;
    }

    header = ((block_header_t *)ptr) - 1;
    if (header->magic != BLOCK_MAGIC || (header->flags & BLOCK_FLAG_IN_USE) == 0)
    {
        DEBUG_LOG("block header is corrupted or not in use! Rejected");
        alloc_unlock();
        return;
    }

    header->flags &= ~BLOCK_FLAG_IN_USE; // mark block as free
    memset(ptr, 0, header->requested_size); // zero on free the user data!

    sizeclass_allocator_t *sizeclass_alloc;
    free_block_t *node;
    u32 before_free;

    sizeclass_alloc = &state->lanes[chunk->bucket][chunk->sizeclass];
    node = (free_block_t *)ptr;

    // push the block back to the chunk's free list
    node->next = chunk->free_list;
    chunk->free_list = node;

    // inc free count
    before_free = chunk->free_count;
    chunk->free_count += 1;
    
    // if chunk was full, move it to partial (we are freeing a block)
    if (before_free == 0)
    {
        chunk_list_remove(&sizeclass_alloc->full, chunk);
        chunk_list_push(&sizeclass_alloc->partial, chunk);
    }

    // if chunk is now completely free, move it to empty
    if (chunk->free_count == chunk->capacity)
    {
        chunk_list_remove(&sizeclass_alloc->partial, chunk);
        chunk_list_push(&sizeclass_alloc->empty, chunk);
    }

    // If the current chunk is NULL, set it to this one
    if (sizeclass_alloc->current == NULL && chunk->free_count > 0)
        sizeclass_alloc->current = chunk;

    alloc_unlock();
}

void *realloc_impl(void *ptr, usize size)
{
    allocator_t *state;
    chunk_t *chunk;
    block_header_t *header;
    usize old_size;
    usize copy_size;
    usize inplace_capacity;
    void *new_ptr;

    if (ptr == NULL) // realloc with NULL ptr is like malloc
        return alloc_impl(size, NULL);

    if (size == 0) // realloc with size 0 is like free
    {
        free_impl(ptr);
        return NULL;
    }

    // Currently don't allow over small max
    if (size > ALLOC_SMALL_MAX)
        return NULL;

    alloc_lock();
    state = state_get();

    if (!state->initialized)
    {
        DEBUG_LOG("allocator not initialized on realloc");
        alloc_unlock();
        return NULL;
    }

    chunk = find_chunk_for_ptr(ptr);
    if (chunk == NULL || !pointer_matches_chunk_block(chunk, ptr))
    {
        DEBUG_LOG("pointer passed to realloc does not match any allocated block!");
        alloc_unlock();
        return NULL;
    }

    header = ((block_header_t *)ptr) - 1;
    if (header->magic != BLOCK_MAGIC || (header->flags & BLOCK_FLAG_IN_USE) == 0)
    {
        DEBUG_LOG("block header is corrupted or not in use! Realloc rejected");
        alloc_unlock();
        return NULL;
    }

    old_size = header->requested_size;
    inplace_capacity = (usize)sizeclasses[chunk->sizeclass];

    if (size <= inplace_capacity) // the new size does not require a bigger block
    {
        header->requested_size = size;
        alloc_unlock();
        return ptr;
    }

    alloc_unlock();

    // Need to allocate a new block and copy data over
    new_ptr = alloc_impl(size, NULL);
    if (new_ptr == NULL)
        return NULL;

    // copy the old data to the new block, up to the minimum of old size and new size
    copy_size = MIN(old_size, size);
    memcpy(new_ptr, ptr, copy_size);
    free_impl(ptr); // free the old block

    return new_ptr;
}
