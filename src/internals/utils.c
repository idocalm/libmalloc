#include "_malloc.h"

size_t sizeclass(size_t size) {
    for (size_t i = 0; i < NUM_SIZECLASSES; i++) {
        if (sizeclasses[i] >= size) 
            return i; 
    }

    return NULL;
}