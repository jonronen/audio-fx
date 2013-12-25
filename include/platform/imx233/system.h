#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "imx233.h"
#include "stdint.h"
#include "stdbool.h"


#ifndef NULL
#define NULL ((void*)0)
#endif


#include "system-target.h"

/* Declare this as HIGHEST_IRQ_LEVEL if they don't differ */
#ifndef DISABLE_INTERRUPTS
#define DISABLE_INTERRUPTS  HIGHEST_IRQ_LEVEL
#endif

/* Define this, if the CPU may take advantage of cache aligment. Is enabled
 * for all ARM CPUs. */
#define HAVE_CPU_CACHE_ALIGN

/* Calculate CACHEALIGN_SIZE from CACHEALIGN_BITS */
#ifdef CACHEALIGN_SIZE
    /* undefine, if defined. always calculate from CACHEALIGN_BITS */
    #undef CACHEALIGN_SIZE
#endif
#ifdef CACHEALIGN_BITS
    /* CACHEALIGN_SIZE = 2 ^ CACHEALIGN_BITS */
    #define CACHEALIGN_SIZE (1u << CACHEALIGN_BITS)
#else
    /* FIXME: set to maximum known cache alignment of supported CPUs */
    #define CACHEALIGN_BITS  5
    #define CACHEALIGN_SIZE 32
#endif

#ifdef HAVE_CPU_CACHE_ALIGN
    /* Cache alignment attributes and sizes are enabled */
    #define CACHEALIGN_ATTR __attribute__((aligned(CACHEALIGN_SIZE)))
    /* Aligns x up to a CACHEALIGN_SIZE boundary */
    #define CACHEALIGN_UP(x) \
        ((typeof (x))ALIGN_UP_P2((uintptr_t)(x), CACHEALIGN_BITS))
    /* Aligns x down to a CACHEALIGN_SIZE boundary */
    #define CACHEALIGN_DOWN(x) \
        ((typeof (x))ALIGN_DOWN_P2((uintptr_t)(x), CACHEALIGN_BITS))
    /* Aligns at least to the greater of size x or CACHEALIGN_SIZE */
    #define CACHEALIGN_AT_LEAST_ATTR(x) \
        __attribute__((aligned(CACHEALIGN_UP(x))))
    /* Aligns a buffer pointer and size to proper boundaries */
    #define CACHEALIGN_BUFFER(start, size) \
        ALIGN_BUFFER((start), (size), CACHEALIGN_SIZE)
#else
    /* Cache alignment attributes and sizes are not enabled */
    #define CACHEALIGN_ATTR
    #define CACHEALIGN_AT_LEAST_ATTR(x) __attribute__((aligned(x)))
    #define CACHEALIGN_UP(x) (x)
    #define CACHEALIGN_DOWN(x) (x)
    /* Make no adjustments */
    #define CACHEALIGN_BUFFER(start, size)
#endif

/* Define MEM_ALIGN_ATTR which may be used to align e.g. buffers for faster
 * access. */
#if defined(CPU_ARM)
    /* Use ARMs cache alignment. */
    #define MEM_ALIGN_ATTR CACHEALIGN_ATTR
    #define MEM_ALIGN_SIZE CACHEALIGN_SIZE
#elif defined(CPU_COLDFIRE)
    /* Use fixed alignment of 16 bytes. Speed up only for 'movem' in DRAM. */
    #define MEM_ALIGN_ATTR __attribute__((aligned(16)))
    #define MEM_ALIGN_SIZE 16
#else
    /* Align pointer size */
    #define MEM_ALIGN_ATTR __attribute__((aligned(sizeof(intptr_t))))
    #define MEM_ALIGN_SIZE sizeof(intptr_t)
#endif

#define MEM_ALIGN_UP(x) \
    ((typeof (x))ALIGN_UP((uintptr_t)(x), MEM_ALIGN_SIZE))
#define MEM_ALIGN_DOWN(x) \
    ((typeof (x))ALIGN_DOWN((uintptr_t)(x), MEM_ALIGN_SIZE))

#ifdef STORAGE_WANTS_ALIGN
    #define STORAGE_ALIGN_ATTR __attribute__((aligned(CACHEALIGN_SIZE)))
    #define STORAGE_ALIGN_DOWN(x) \
        ((typeof (x))ALIGN_DOWN_P2((uintptr_t)(x), CACHEALIGN_BITS))
    /* Pad a size so the buffer can be aligned later */
    #define STORAGE_PAD(x) ((x) + CACHEALIGN_SIZE - 1)
    /* Number of bytes in the last cacheline assuming buffer of size x is aligned */
    #define STORAGE_OVERLAP(x) ((x) & (CACHEALIGN_SIZE - 1))
    #define STORAGE_ALIGN_BUFFER(start, size) \
        ALIGN_BUFFER((start), (size), CACHEALIGN_SIZE)
#else
    #define STORAGE_ALIGN_ATTR
    #define STORAGE_ALIGN_DOWN(x) (x)
    #define STORAGE_PAD(x) (x)
    #define STORAGE_OVERLAP(x) 0
    #define STORAGE_ALIGN_BUFFER(start, size)
#endif


#endif /* __SYSTEM_H__ */
