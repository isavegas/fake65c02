#include "acutest.h"

void test_example(void)
{
    void* mem;

    mem = malloc(10);
    TEST_CHECK(mem != NULL);

    void* mem2 = realloc(mem, 20);
    TEST_CHECK(mem2 != NULL);

    free(mem2);
}

TEST_LIST = {
   { "example", test_example },
   { NULL, NULL }
};