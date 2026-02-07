#include "internals/_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static allocator_t main_allocator = {0};

static void allocator_init()
{
    main_allocator.page_size = CHUNK_PAGE_SIZE;
    
    for (size_t i = 0; i < NUM_SIZECLASSES; i++)
    {
        main_allocator.buckets[i].chunks = NULL;
        main_allocator.buckets[i].sizeclass = sizeclasses[i];
    }
    
    main_allocator.initialized = 1;

#ifdef _DEBUG
    printf("Allocator init done\n");
#endif
}


static chunk_t* allocate_chunk(size_t sizeclass)
{
    chunk_t* chunk = (chunk_t*)calloc(1, sizeof(chunk_t));
    if (!chunk)
        return NULL;
    
    chunk->memory = calloc(1, CHUNK_PAGE_SIZE);
    if (!chunk->memory)
    {
        free(chunk);
        return NULL;
    }
    
    chunk->sizeclass = sizeclass;
    chunk->free_list = NULL;
    chunk->next = NULL;

    size_t block_size = sizeclass + sizeof(block_header_t);
    chunk->total_blocks = CHUNK_PAGE_SIZE / block_size;
    chunk->free_blocks = chunk->total_blocks;
    
    uint8_t* mem = (uint8_t*)chunk->memory; // TODO: What? 
    free_block_t* prev = NULL;
    
    for (size_t i = 0; i < chunk->total_blocks; i++)
    {
        free_block_t* block = (free_block_t*)(mem + i * block_size);

        block->header.size = sizeclass;
        block->header.free = 1;

        block->next = NULL;
        block->prev = prev;
        
        if (prev)
            prev->next = block;
        else
            chunk->free_list = block;  // First block
        
        prev = block;
    }
    
    return chunk;
}

static chunk_t* get_chunk_for_sizeclass(size_t alloc_size)
{
    // find the index 
    int sizeclass_index = -1;
    for (size_t i = 0; i < NUM_SIZECLASSES; sizeclass_index++) {
        if (sizeclasses[i] == alloc_size) {
            sizeclass_index = i;
            break;
        }
    }

    if (sizeclass_index == -1) {
#ifdef _DEBUG
        printf("get_chunk_for_sizeclass received an invalid allocation size\n");
#endif
        return NULL;

    }

    bucket_t* bucket = &main_allocator.buckets[sizeclass_index];    
    chunk_t* chunk = bucket->chunks;
    while (chunk) // search for free chunk
    {
        if (chunk->free_blocks > 0)
            return chunk;
        chunk = chunk->next;
    }
    
    // No available chunk found, allocate a new one
    chunk = allocate_chunk(bucket->sizeclass);
    if (!chunk)
        return NULL;
    
    chunk->next = bucket->chunks;
    bucket->chunks = chunk;
    
    return chunk;
}

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    if (!main_allocator.initialized)
    {
#ifdef _DEBUG
        printf("malloc was called, but the allocator was not initialized. Initializing...\n");
#endif
        allocator_init();
    }
    
    size_t alloc_size = sizeclass(size);
    chunk_t* chunk = get_chunk_for_sizeclass(alloc_size);
    if (!chunk)
        return NULL;
    
    free_block_t* block = chunk->free_list;
    if (!block)
#ifdef _DEBUG
        printf("Was not able to carve a block\n");
#endif
        return NULL;
    
    chunk->free_list = block->next;
    if (block->next)
        block->next->prev = NULL;
    
    block->header.free = 0;
    chunk->free_blocks--;
    
    return (void*)
        ((uint8_t*)block + sizeof(block_header_t));
}