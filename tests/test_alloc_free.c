#include "../src/internals/_malloc.h"
#include "tests.h"
#include <assert.h>

void test_basic_alloc_free(void)
{
    u8* p;

    p = (u8*)alloc_impl(64u, NULL);
    assert(p != NULL);

    for (usize i = 0; i < 64u; ++i) {
        p[i] = (u8)i;
    }

    free_impl(p);
}
