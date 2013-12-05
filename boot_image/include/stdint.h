#ifndef __STDINT_H__
#define __STDINT_H__

/* 8 bit */
#define INT8_MIN    SCHAR_MIN
#define INT8_MAX    SCHAR_MAX
#define UINT8_MAX   UCHAR_MAX
#define int8_t      signed char
#define uint8_t     unsigned char

/* 16 bit */

#define INT16_MIN   SHRT_MIN
#define INT16_MAX   SHRT_MAX
#define UINT16_MAX  USHRT_MAX
#define int16_t     short
#define uint16_t    unsigned short

/* 32 bit */

#define INT32_MIN   INT_MIN
#define INT32_MAX   INT_MAX
#define UINT32_MAX  UINT_MAX
#define int32_t     int
#define uint32_t    unsigned int


/* 64 bit */
#ifndef LLONG_MIN
#define LLONG_MIN   ((long long)9223372036854775808ull)
#endif

#ifndef LLONG_MAX
#define LLONG_MAX   9223372036854775807ll
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX  18446744073709551615ull
#endif

#define INT64_MIN   LONG_MIN
#define INT64_MAX   LONG_MAX
#define UINT64_MAX  ULONG_MAX
#define int64_t     long
#define uint64_t    unsigned long

#define INTPTR_MIN  LONG_MIN
#define INTPTR_MAX  LONG_MAX
#define UINTPTR_MAX ULONG_MAX
#define intptr_t    long
#define uintptr_t   unsigned long


#ifndef NULL
#define NULL ((void*)0)
#endif


#endif /* __STDINT_H__ */
