#ifndef CHUNK_H
#define CHUNK_H

#include "_malloc.h"

chunk_t* chunk_alloc_metadata(void);
void chunk_free_metadata(chunk_t* chunk);
void chunk_list_push(chunk_t** head, chunk_t* chunk);
void chunk_list_remove(chunk_t** head, chunk_t* chunk);
chunk_t* chunk_list_pop(chunk_t** head);
void chunk_init_free_list(chunk_t* chunk, usize sizeclass_size);
chunk_t* chunk_create_slab(u16 bucket, u16 sizeclass);
chunk_t* find_chunk_for_ptr(const void* ptr);
b8 pointer_matches_chunk_block(const chunk_t* chunk, const void* ptr);

#endif
