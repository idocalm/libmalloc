#include "../src/internals/_malloc.h"
#include "tests.h"
#include <assert.h>
#include <string.h>

/* After free+reuse, payload bytes (except freelist pointer area) are zeroed. */
void test_zero_on_free_reuse(void)
{
    u8* p;
    u8* q;

    p = (u8*)alloc_impl(32u, NULL);
    assert(p != NULL);
    memset(p, 0x5Au, 32u);
    free_impl(p);

    q = (u8*)alloc_impl(32u, NULL);
    assert(q != NULL);
    assert(q == p);

    for (usize i = sizeof(free_block_t); i < 32u; ++i) {
        assert(q[i] == 0u);
    }

    free_impl(q);
}
