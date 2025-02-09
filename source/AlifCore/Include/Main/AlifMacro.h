#pragma once

#define ALIF_ABS(_x)	((_x) < 0 ? -(_x) : (_x))
#define ALIF_MAX(_a,_b)	(((_a) > (_b)) ? (_a) : (_b))
#define ALIF_MIN(_a,_b) (((_a) < (_b)) ? (_a) : (_b))

#define ALIF_XSTRINGIFY(_x) #_x
/* Convert the argument to a string. For example, ALIF_STRINGIFY(123) is replaced
   with "123" by the preprocessor. Defines are also replaced by their value.
   For example ALIF_STRINGIFY(__ALIF__) is replaced by the line number, not
   by "__ALIF__". */
#define ALIF_STRINGIFY(_x) ALIF_XSTRINGIFY(_x)

/* Get the size of a structure member in bytes */
#define ALIF_MEMBER_SIZE(_type, _member) sizeof(((_type *)0)->_member)

/* Argument must be a char or an int in [-128, 127] or [0, 255]. */
#define ALIF_CHARMASK(_c) ((unsigned char)((_c) & 0xff))

#define ALIF_ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

#define ALIFDOC_VAR(name) static const char name[] // 110
#define ALIFDOC_STRVAR(name,str) ALIFDOC_VAR(name) = ALIFDOC_STR(str) // 111
//#ifdef WITH_DOC_STRINGS
#define ALIFDOC_STR(str) str
//#else
//#define ALIFDOC_STR(str) ""
//#endif


/* Below "a" is a power of 2. */
/* Round down size "n" to be a multiple of "a". */
#define ALIF_SIZE_ROUND_DOWN(_n, _a) ((size_t)(_n) & ~(size_t)((_a) - 1))
/* Round up size "n" to be a multiple of "a". */
#define ALIF_SIZE_ROUND_UP(_n, _a) (((size_t)(_n) + \
        (size_t)((_a) - 1)) & ~(size_t)((_a) - 1))
/* Round pointer "p" down to the closest "a"-aligned address <= "p". */
#define ALIF_ALIGN_DOWN(_p, _a) ((void *)((uintptr_t)(_p) & ~(uintptr_t)((_a) - 1)))
/* Round pointer "p" up to the closest "a"-aligned address >= "p". */
#define ALIF_ALIGN_UP(_p, _a) ((void *)(((uintptr_t)(_p) + (uintptr_t)((_a) - 1)) & ~(uintptr_t)((_a) - 1)))
/* Check if pointer "p" is aligned to "a"-bytes boundary. */
#define ALIF_IS_ALIGNED(_p, _a) (!((uintptr_t)(_p) & (uintptr_t)((_a) - 1)))


/* Use this for unused arguments in a function definition to silence compiler
 * warnings. Example:
 *
 * int func(int a, int ALIF_UNUSED(b)) { return a; }
 */
#if defined(__GNUC__) || defined(__clang__)
#  define ALIF_UNUSED(_name) _unused_ ## _name __attribute__((unused))
#elif defined(_MSC_VER)
// Disable warning C4100: unreferenced formal parameter,
// declare the parameter,
// restore old compiler warnings.
#  define ALIF_UNUSED(_name) \
        __pragma(warning(push)) \
        __pragma(warning(suppress: 4100)) \
        _unused_ ## _name \
        __pragma(warning(pop))
#else
#  define ALIF_UNUSED(_name) _unused_ ## _name
#endif


#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
#  define ALIF_UNREACHABLE() __builtin_unreachable()
#elif defined(__clang__) || defined(__INTEL_COMPILER)
#  define ALIF_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#  define ALIF_UNREACHABLE() __assume(0)
#endif


#define ALIF_CONTAINER_OF(_ptr, _type, _member) \
    (_type*)((char*)_ptr - offsetof(_type, _member))

#define ALIF_RVALUE(_expr) ((void)0, (_expr))

 // Return non-zero if the type is signed, return zero if it's unsigned.
 // Use "<= 0" rather than "< 0" to prevent the compiler warning:
 // "comparison of unsigned expression in '< 0' is always false".
#define ALIF_IS_TYPE_SIGNED(_type) ((_type)(-1) <= 0)
