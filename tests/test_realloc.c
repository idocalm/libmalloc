#include "../src/internals/_malloc.h"
#include "tests.h"
#include <assert.h>

void test_realloc_behaviors(void)
{
    u8* p;
    u8* q;
    void* r;

    p = (u8*)alloc_impl(24u, NULL);
    assert(p != NULL);

    for (usize i = 0; i < 24u; ++i) {
        p[i] = (u8)(0xA0u + i);
    }

    q = (u8*)realloc_impl(p, 200u);
    assert(q != NULL);

    for (usize i = 0; i < 24u; ++i) {
        assert(q[i] == (u8)(0xA0u + i));
    }

    free_impl(q);

    r = realloc_impl(NULL, 80u);
    assert(r != NULL);
    assert(realloc_impl(r, 0u) == NULL);
}
