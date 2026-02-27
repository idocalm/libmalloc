#ifndef TESTS_H
#define TESTS_H

void test_bounds(void);
void test_basic_alloc_free(void);
void test_realloc_behaviors(void);
void test_zero_on_free_reuse(void);
void test_magic_cookie_corruption(void);

#endif
