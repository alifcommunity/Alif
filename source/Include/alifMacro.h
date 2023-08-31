#pragma once

























#define ALIF_MIN(x, y) (((x) > (y)) ? (y) : (x))


#define ALIF_MAX(x, y) (((x) > (y)) ? (x) : (y))


#define ALIF_ABS(x) ((x) < 0 ? -(x) : (x))











































#if (defined(__GNUC__) && !defined(__STRICT_ANSI__) && \
    (((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)) || (__GNUC__ >= 4)))
#define ALIF_ARRAY_LENGTH(array) \
    (sizeof(array) / sizeof((array)[0]) \
     + ALIF_BUILD_ASSERT_EXPR(!__builtin_types_compatible_p(typeof(array), \
                                                          typeof(&(array)[0]))))
#else
#define ALIF_ARRAY_LENGTH(array) \
    (sizeof(array) / sizeof((array)[0]))
#endif















#define ALIFSIZE_ROUND_DOWN(n, a) ((size_t)(n) & ~(size_t)((a) - 1))

#define ALIFSIZE_ROUND_UP(n, a) (((size_t)(n) + \
        (size_t)((a) - 1)) & ~(size_t)((a) - 1))

#define ALIFALIGN_DOWN(p, a) ((void *)((uintptr_t)(p) & ~(uintptr_t)((a) - 1)))

#define ALIFALIGN_UP(p, a) ((void *)(((uintptr_t)(p) + \
        (uintptr_t)((a) - 1)) & ~(uintptr_t)((a) - 1)))

#define ALIFIS_ALIGNED(p, a) (!((uintptr_t)(p) & (uintptr_t)((a) - 1)))






#if defined(__GNUC__) || defined(__clang__)
#  define ALIF_UNUSED(name) unused ## name __attribute__((unused))
#elif defined(_MSC_VER)



#  define ALIF_UNUSED(name) \
        __pragma(warning(push)) \
        __pragma(warning(suppress: 4100)) \
        _unused_ ## name \
        __pragma(warning(pop))
#else
#  define ALIF_UNUSED(name) unused ## name
#endif
