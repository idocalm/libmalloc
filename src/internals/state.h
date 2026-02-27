#ifndef STATE_H
#define STATE_H

#include "_malloc.h"

void alloc_lock(void);
void alloc_unlock(void);
void alloc_init(void);
chunk_t* state_chunk_pool(void);
usize state_chunk_pool_capacity(void);

#endif
