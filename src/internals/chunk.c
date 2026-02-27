#include "chunk.h"
#include <string.h>

chunk_t* chunk_alloc_metadata(void)
{
    chunk_t* pool;
    usize i;

    // Scan the pool for a free chunk
    pool = state_chunk_pool();
    for (i = 0; i < state_chunk_pool_capacity(); ++i) {
        if (!pool[i].used) {
            // Reset the memory and mark it as used
            memset(&pool[i], 0, sizeof(pool[i]));
            pool[i].used = 1;
            return &pool[i];
        }
    }

    // No free chunks available
    return NULL;
}

void chunk_free_metadata(chunk_t* chunk)
{
    memset(chunk, 0, sizeof(*chunk));
}

void chunk_list_push(chunk_t** head, chunk_t* chunk)
{
    // Insert at list head
    chunk->prev = NULL;
    chunk->next = *head;

    if (*head != NULL) {
        (*head)->prev = chunk;
    }

    *head = chunk;
}

void chunk_list_remove(chunk_t** head, chunk_t* chunk)
{
    if (chunk->prev != NULL) {
        chunk->prev->next = chunk->next;
    } else {
        *head = chunk->next;
    }

    if (chunk->next != NULL) {
        chunk->next->prev = chunk->prev;
    }

    // Detach chunk from any list.
    chunk->prev = NULL;
    chunk->next = NULL;
}

chunk_t* chunk_list_pop(chunk_t** head)
{
    chunk_t* out;

    if (*head == NULL) {
        return NULL;
    }

    out = *head;
    chunk_list_remove(head, out);
    return out;
}

void chunk_init_free_list(chunk_t* chunk, usize sizeclass_size)
{
    usize i;
    u8* cursor;

    cursor = (u8*)chunk->base;
    chunk->free_list = NULL;

    for (i = 0; i < chunk->capacity; ++i) {
        block_header_t* header;
        free_block_t* node;

        // Setup the header for the block
        header = (block_header_t*)(void*)cursor;
        header->magic = BLOCK_MAGIC;
        header->sizeclass = chunk->sizeclass;
        header->flags = 0;
        header->requested_size = sizeclass_size;

        // Free list node starts after the header
        cursor += sizeof(block_header_t);
        node = (free_block_t*)(void*)(cursor);
        node->next = chunk->free_list;
        chunk->free_list = node;

        cursor += chunk->stride;
    }
}

chunk_t* chunk_create_slab(u16 bucket, u16 sizeclass)
{
    chunk_t* chunk;
    usize sizeclass_size;
    usize bytes;
    usize stride;
    usize capacity;

    chunk = chunk_alloc_metadata();
    if (chunk == NULL) {
        return NULL;
    }

    sizeclass_size = (usize) sizeclasses[sizeclass];
    bytes = chunk_bytes_for_sizeclass(sizeclass);
    stride = align_up_usize(sizeof(block_header_t) + sizeclass_size, sizeof(uptr));
    capacity = bytes / stride;

    if (bytes == 0 || capacity == 0) {
        chunk_free_metadata(chunk);
        return NULL;
    }

    // Allocate raw memory from the OS for this chunk
    chunk->base = os_alloc(bytes);
    if (chunk->base == NULL) {
        chunk_free_metadata(chunk);
        return NULL;
    }

    chunk->bucket = bucket;
    chunk->sizeclass = sizeclass;
    chunk->capacity = (u32)capacity;
    chunk->free_count = (u32)capacity;
    chunk->stride = stride;
    chunk->chunk_bytes = bytes;

    // Initialize all block headers and free lists
    chunk_init_free_list(chunk, sizeclass_size);
    return chunk;
}

chunk_t* find_chunk_for_ptr(const void* ptr)
{
    chunk_t* pool;
    const u8* p;
    usize i;

    if (ptr == NULL) {
        return NULL;
    }

    pool = state_chunk_pool();
    p = (const u8*)ptr;

    for (i = 0; i < state_chunk_pool_capacity(); ++i) {
        const u8* base;
        const u8* end;

        if (!pool[i].used || pool[i].base == NULL) {
            continue;
        }

        base = (const u8*)pool[i].base;
        end = base + pool[i].chunk_bytes;

        if (p < base || p >= end) {
            continue;
        }

        return &pool[i];
    }

    return NULL;
}

b8 pointer_matches_chunk_block(const chunk_t* chunk, const void* ptr)
{
    const u8* base;
    const u8* p;
    usize offset;

    base = (const u8*)chunk->base;
    p = (const u8*)ptr;

    if (p < base + sizeof(block_header_t)) {
        return 0;
    }

    offset = (usize)(p - base) - sizeof(block_header_t);

    if (offset % chunk->stride != 0u) {
        return 0;
    }

    if (offset / chunk->stride >= chunk->capacity) {
        return 0;
    }

    return 1;
}
