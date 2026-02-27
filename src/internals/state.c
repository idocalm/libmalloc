#include "state.h"
#include <stdatomic.h>
#include <string.h>

static allocator_t g_state;
static chunk_t g_chunk_pool[ALLOC_MAX_CHUNKS];
static atomic_flag g_lock = ATOMIC_FLAG_INIT;

void alloc_lock(void)
{
    while (atomic_flag_test_and_set_explicit(&g_lock, memory_order_acquire)) {
    }
}

void alloc_unlock(void)
{
    atomic_flag_clear_explicit(&g_lock, memory_order_release);
}

allocator_t* state_get(void)
{
    return &g_state;
}

chunk_t* state_chunk_pool(void)
{
    return &g_chunk_pool[0];
}

usize state_chunk_pool_capacity(void)
{
    return ALLOC_MAX_CHUNKS;
}

void alloc_init(void)
{
    u32 bucket;
    u32 sizeclass;

    if (g_state.initialized)
    {
        DEBUG_LOG("allocator is already initialized.");
        return;
    }

    // Zero the state and the chunk pool
    memset(&g_state, 0, sizeof(g_state));
    memset(g_chunk_pool, 0, sizeof(g_chunk_pool));

    // Fun fact - this number (0x9E3779B9) is the fractional part of the golden ratio in hex, 
    // and is commonly used as a seed for hash functions.
    g_state.seed = (u32)((uptr)&g_state ^ 0x9E3779B9);

    // loop every (bucket, sizeclass) lane and set the sizeclass
    for (bucket = 0; bucket < ALLOC_BUCKET_COUNT; ++bucket) 
    {
        for (sizeclass = 0; sizeclass < ALLOC_SIZECLASS_COUNT; ++sizeclass)
        {
            g_state.lanes[bucket][sizeclass].sizeclass_size = sizeclasses[sizeclass];
        }
    }

    DEBUG_LOG("allocator initialized");
    g_state.initialized = 1; 
}
