#include "../src/internals/_malloc.h"
#include "tests.h"
#include <assert.h>

void test_magic_cookie_corruption(void)
{
    void* p;
    block_header_t* h;
    u32 saved_magic;

    p = alloc_impl(64u, NULL);
    assert(p != NULL);

    h = ((block_header_t*)p) - 1;
    saved_magic = h->magic;
    assert((h->flags & BLOCK_FLAG_IN_USE) != 0u);

    /* Corrupt header cookie */
    h->magic = 0u;

    /* Free & realloc should reject */
    free_impl(p);
    assert((h->flags & BLOCK_FLAG_IN_USE) != 0u);
    assert(realloc_impl(p, 32u) == NULL);

    /* Restore header to avoid leaking this test allocation. */
    h->magic = saved_magic;
    free_impl(p);
}
