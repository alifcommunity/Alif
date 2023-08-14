#pragma once





















































































































#if defined(__GNUC__) || defined(__clang__)
#  define ALIF_UNUSED(name) _unused_ ## name __attribute__((unused))
#elif defined(_MSC_VER)
// Disable warning C4100: unreferenced formal parameter,
// declare the parameter,
// restore old compiler warnings.
#  define ALIF_UNUSED(name) \
        __pragma(warning(push)) \
        __pragma(warning(suppress: 4100)) \
        _unused_ ## name \
        __pragma(warning(pop))
#else
#  define ALIF_UNUSED(name) _unused_ ## name
#endif
