#include "utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

const u32 sizeclasses[ALLOC_SIZECLASS_COUNT] = {
    16, 32, 64, 128, 256, 512,
    1024, 2048, 4096, 8192, 16384, 32768
};

/**
 * @brief  Return the first matching sizeclass index for a given size.
 * @param  size: Requested allocation size in bytes.
 * @retval Sizeclass index
 */
usize sizeclass_index_for_size(usize size)
{
    usize i;

    for (i = 0; i < ALLOC_SIZECLASS_COUNT; ++i) {
        usize sizeclass_size;

        sizeclass_size = (usize)sizeclasses[i];
        if (sizeclass_size >= size) {
            return i;
        }
    }

    return ALLOC_SIZECLASS_COUNT;
}

/**
 * @brief  Return slab chunk byte size for a given sizeclass.
 * @param  sizeclass_index: Index into sizeclasses.
 * @retval Chunk size in bytes, or 0 for invalid sizeclass indexes.
 */
usize chunk_bytes_for_sizeclass(usize sizeclass_index)
{
    if (sizeclass_index >= ALLOC_SIZECLASS_COUNT) {
        return 0;
    }

    // For tiny (<= 4 KB), use one slice = 16 KB;
    if ((usize)sizeclasses[sizeclass_index] <= ALLOC_TINY_MAX)
    {
        return ALLOC_SLICE_SIZE;
    }

    // For bigger sizes, use four slices = 64 KB
    return ALLOC_SLICE_SIZE * 4;
}

/**
 * @brief  Map a caller address to a bucket index.
 * @param  caller: Return address (or NULL) associated with the allocation site.
 * @param  seed: Per-process seed used to perturb the hash.
 * @retval Bucket index in range between 0 and ALLOC_BUCKET_COUNT.
 */
u32 bucket_for_caller(void* caller, u32 seed)
{
    uptr mixed;
    u32 bucket;

    mixed = (uptr)caller ^ (uptr)seed;
    bucket = (u32)(mixed % ALLOC_BUCKET_COUNT);

    return bucket;
}

/**
 * @brief  Reserve writable Vm from the OS.
 * @note   Uses VirtualAlloc on Windows and mmap on Linux.
 * @param  bytes: Number of bytes to allocate.
 * @retval Pointer to mapped memory, or NULL on failure.
 */
void* os_alloc(usize bytes)
{
    // On Windows, we use VirtualAlloc which allocates from the raw allocator
#ifdef _WIN32
    return (void*)VirtualAlloc(NULL, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    // On Linux, we use mmap to get memory from the OS. 
    // MAP_ANONYMOUS means the mapping is not backed by any file
    // MAP_PRIVATE means that changes to the mapped memory are not visible to other processes.

    void* region;
    region = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (region == MAP_FAILED) ? NULL : region;
#endif
}

/**
 * @brief  Release previously mapped virtual memory back to the OS.
 * @param  ptr: Pointer returned by os_alloc.
 * @param  bytes: Mapping size in bytes (used on POSIX).
 */
void os_free(void* ptr, usize bytes)
{
    if (ptr == NULL) {
        return;
    }

#ifdef _WIN32
    (void)VirtualFree(ptr, 0u, MEM_RELEASE);
#else
    (void)munmap(ptr, bytes);
#endif
}

/**
 * @brief  Multiply two usize values with overflow check
 * @param  a : first param
 * @param  b :  second param
 * @param  out: Output pointer (if not overflow)
 * @retval 1 if overflow is detected, 0 otherwise.
 */
b8 mul_overflow_usize(usize a, usize b, usize* out)
{
    if (a != 0u && b > ((usize)-1) / a) {
        return 1;
    }

    *out = a * b;
    return 0;
}
