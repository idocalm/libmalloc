#include "../src/internals/_malloc.h"
#include "tests.h"
#include <assert.h>

void test_bounds(void)
{
    assert(alloc_impl(0u, NULL) == NULL);
    assert(alloc_impl(ALLOC_SMALL_MAX + 1u, NULL) == NULL);
}
