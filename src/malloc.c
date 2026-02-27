#include "internals/_malloc.h"
#include <string.h>

void* malloc(size_t size)
{
    void* caller;

    if (size == 0u) {
        return NULL;
    }

#if defined(__GNUC__) || defined(__clang__)
    caller = __builtin_return_address(0);
#else
    caller = NULL;
#endif

    return alloc_impl((usize)size, caller);
}

void free(void* ptr)
{
    free_impl(ptr);
}

void* calloc(size_t count, size_t size)
{
    usize total;
    void* ptr;

    if (count == 0u || size == 0u) {
        return NULL;
    }

    if (mul_overflow_usize((usize)count, (usize)size, &total)) {
        return NULL;
    }

    ptr = alloc_impl(total, NULL);
    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, total);
    return ptr;
}

void* realloc(void* ptr, size_t size)
{
    return realloc_impl(ptr, (usize)size);
}
