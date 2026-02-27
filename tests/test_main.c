#include "tests.h"
#include <stdio.h>

int main(void)
{
    test_bounds();
    test_basic_alloc_free();
    test_realloc_behaviors();
    test_zero_on_free_reuse();
    test_magic_cookie_corruption();

    puts("tests passed");
    return 0;
}
