#ifndef UTILS_H
#define UTILS_H

#include "_malloc.h"

usize sizeclass_index_for_size(usize size);
usize chunk_bytes_for_sizeclass(usize sizeclass_index);
u32 bucket_for_caller(void* caller, u32 seed);
void* os_alloc(usize bytes);
void os_free(void* ptr, usize bytes);
b8 mul_overflow_usize(usize a, usize b, usize* out);

#endif
