#ifndef _MALLOC_H
#define _MALLOC_H

#include <stdint.h>
#include <stddef.h>

#define _DEBUG 1

// The chunk size for now is 16KB (0x4000)
#define CHUNK_PAGE_SIZE 0x4000

#define NUM_SIZECLASSES 12
static const size_t sizeclasses[NUM_SIZECLASSES] = {
    16,    // 0x10
    32,    // 0x20
    64,    // 0x40
    128,   // 0x80
    256,   // 0x100
    512,   // 0x200
    1024,  // 0x400
    2048,  // 0x800
    4096,  // 0x1000
    8192,  // 0x2000
    12288, // 0x3000
    16384  // 0x4000 = CHUNK_PAGE_SIZE
};


typedef struct block_header {
    size_t size;
    uint8_t free; 
} block_header_t;

typedef struct free_block {
    block_header_t header; 
    struct free_block* next;
    struct free_block* prev;
} free_block_t;

// A chunk is tied to a sizeclass: every allocation in that chunk is of the same sizeclass.
typedef struct chunk {
    size_t sizeclass;               // The sizeclass of the chunk
    void* memory;                   // Pointer to the chunk start
    free_block_t* free_list;
    struct chunk* next;             // Next chunk (same sizeclass)
    size_t total_blocks;            // Total blocks = CHUNK_PAGE_SIZE / sizeclass
    size_t free_blocks;             // Free blocks remaining
} chunk_t;

// A bucket has a linked list of chunks of the same sizeclass.
typedef struct {
    chunk_t* chunks;
    size_t sizeclass;
} bucket_t;

typedef struct {
    bucket_t buckets[NUM_SIZECLASSES];  // One bucket per sizeclass
    size_t page_size; 
    int initialized;
} allocator_t; 

/*
    Terms:
        1. Sizeclass: The allocator works with predefined sizes called sizeclasses. Memory blocks are only allowed ot be of size 
            that is a sizeclass.

        2. Chunk: A chunk is a constant sized memory pile of 0x4000 bytes, which is 16KB. That's the page size we use for now. 
            Every chunk is tied to a sizeclass, and every allocation in that chunk is of the same sizeclass. That means allocations
            of different sizeclasses will NEVER land in the same chunk.
        
        3. Bucket: A bucket is a linked list of chunks of the same sizeclass. We'll use the bucket to find a chunk with free blocks
            later when doing allocations. 

    The alloctor is going to be a static global var, that should be initialized by the OS with the function allocator_init().
    When given a alloc request of a certain size, we do:
        1. Round up the size to the nearest sizeclass ()
        2. Find the correct bucket for that sizeclass
        3. Find a chunk with free blocks in the matching bucket. 
        4. If found - carve a block from the freelist of that chunk and give it to the caller.
        5. If not found - allocate a new chunk (of that sizeclass) and give the first block to the caller.
        6. Return the pointer to the user data (after the block header).

    TODO: Documenting free, realloc, calloc. 

*/

size_t sizeclass(size_t size);
static void allocator_init();

#endif