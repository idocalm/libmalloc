#ifndef OPS_H
#define OPS_H

#include "_malloc.h"

void* alloc_impl(usize size, void* caller);
void free_impl(void* ptr);
void* realloc_impl(void* ptr, usize size);

#endif
