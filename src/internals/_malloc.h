#ifndef INTERNAL_MALLOC_H
#define INTERNAL_MALLOC_H

#include "../defs.h"

#define ALLOC_SLICE_SIZE ((usize)KiB(16))
#define ALLOC_TINY_MAX ((usize)KiB(4))
#define ALLOC_SMALL_MAX ((usize)KiB(32))

#define ALLOC_BUCKET_COUNT 4
#define ALLOC_SIZECLASS_COUNT 12
#define ALLOC_MAX_CHUNKS 4096

#define BLOCK_MAGIC 0xDEADBEEF
#define BLOCK_FLAG_IN_USE 0x1

typedef struct block_header {
    u32 magic;
    u16 sizeclass;
    u16 flags;
    usize requested_size;
} block_header_t;

typedef struct free_block {
    struct free_block* next;
} free_block_t;

typedef struct chunk {
    b8 used;
    u16 bucket;
    u16 sizeclass;
    u16 reserved;

    u32 capacity;
    u32 free_count;
    usize stride;
    usize chunk_bytes;

    void* base;
    free_block_t* free_list;

    struct chunk* next;
    struct chunk* prev;
} chunk_t;

typedef struct sizeclass_allocator {
    chunk_t* current;
    chunk_t* partial;
    chunk_t* full;
    chunk_t* empty;
    u32 sizeclass_size;
} sizeclass_allocator_t;

typedef struct allocator {
    b8 initialized;
    u8 reserved[3];
    u32 seed;

    sizeclass_allocator_t lanes[ALLOC_BUCKET_COUNT][ALLOC_SIZECLASS_COUNT];
} allocator_t;


#endif
