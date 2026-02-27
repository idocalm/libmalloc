#ifndef DEFS_H
#define DEFS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;
typedef uintptr_t uptr;

typedef i8 b8;
typedef i32 b32;

#define KiB(n) ((u64)(n) << 10)
#define MiB(n) ((u64)(n) << 20)
#define GiB(n) ((u64)(n) << 30)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static inline usize align_up_usize(usize value, usize alignment)
{
    return (value + alignment - 1u) & ~(alignment - 1u);
}

static inline uptr align_down_uptr(uptr value, uptr alignment)
{
    return value & ~(alignment - 1u);
}

#endif
