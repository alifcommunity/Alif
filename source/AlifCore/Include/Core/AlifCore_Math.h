#pragma once


static inline void _alif_adjustErange1(double _x) { // 33
	if (errno == 0) {
		if (_x == ALIF_INFINITY or _x == -ALIF_INFINITY) {
			errno = ERANGE;
		}
	}
	else if (errno == ERANGE and _x == 0.0) {
		errno = 0;
	}
}





// 86
#ifdef HAVE_GCC_ASM_FOR_X87
#define HAVE_ALIF_SET_53BIT_PRECISION 1

// Functions defined in alif/AlifMath.c
extern unsigned short _alifGet_387ControlWord(void);
extern void _alifSet_387ControlWord(unsigned short);

#define _ALIF_SET_53BIT_PRECISION_HEADER                                  \
    unsigned short old_387controlword, new_387controlword
#define _ALIF_SET_53BIT_PRECISION_START                                   \
    do {                                                                \
        old_387controlword = _alifGet_387ControlWord();                  \
        new_387controlword = (old_387controlword & ~0x0f00) | 0x0200;   \
        if (new_387controlword != old_387controlword) {                 \
            _alifSet_387ControlWord(new_387controlword);                 \
        }                                                               \
    } while (0)
#define _ALIF_SET_53BIT_PRECISION_END                                     \
    do {                                                                \
        if (new_387controlword != old_387controlword) {                 \
            _alifSet_387ControlWord(old_387controlword);                 \
        }                                                               \
    } while (0)
#endif

// Get and set x87 control word for VisualStudio/x86.
// x87 is not supported in 64-bit or ARM.
#if defined(_MSC_VER) && !defined(_WIN64) && !defined(_M_ARM)
#define HAVE_ALIF_SET_53BIT_PRECISION 1

#include <float.h>                // __control87_2()

#define _ALIF_SET_53BIT_PRECISION_HEADER \
    unsigned int old_387controlword, new_387controlword, out_387controlword
// We use the __control87_2 function to set only the x87 control word.
// The SSE control word is unaffected.
#define _ALIF_SET_53BIT_PRECISION_START                                   \
    do {                                                                \
        __control87_2(0, 0, &old_387controlword, NULL);                 \
        new_387controlword =                                            \
          (old_387controlword & ~(_MCW_PC | _MCW_RC)) | (_PC_53 | _RC_NEAR); \
        if (new_387controlword != old_387controlword) {                 \
            __control87_2(new_387controlword, _MCW_PC | _MCW_RC,        \
                          &out_387controlword, NULL);                   \
        }                                                               \
    } while (0)
#define _ALIF_SET_53BIT_PRECISION_END                                     \
    do {                                                                \
        if (new_387controlword != old_387controlword) {                 \
            __control87_2(old_387controlword, _MCW_PC | _MCW_RC,        \
                          &out_387controlword, NULL);                   \
        }                                                               \
    } while (0)
#endif


// MC68881
#ifdef HAVE_GCC_ASM_FOR_MC68881
#define HAVE_ALIF_SET_53BIT_PRECISION 1
#define _ALIF_SET_53BIT_PRECISION_HEADER \
    unsigned int old_fpcr, new_fpcr
#define _ALIF_SET_53BIT_PRECISION_START                                   \
    do {                                                                \
        __asm__ ("fmove.l %%fpcr,%0" : "=g" (old_fpcr));                \
        /* Set double precision / round to nearest.  */                 \
        new_fpcr = (old_fpcr & ~0xf0) | 0x80;                           \
        if (new_fpcr != old_fpcr) {                                     \
              __asm__ volatile ("fmove.l %0,%%fpcr" : : "g" (new_fpcr));\
        }                                                               \
    } while (0)
#define _ALIF_SET_53BIT_PRECISION_END                                     \
    do {                                                                \
        if (new_fpcr != old_fpcr) {                                     \
            __asm__ volatile ("fmove.l %0,%%fpcr" : : "g" (old_fpcr));  \
        }                                                               \
    } while (0)
#endif

// Default definitions are empty
#ifndef _ALIF_SET_53BIT_PRECISION_HEADER
#  define _ALIF_SET_53BIT_PRECISION_HEADER
#  define _ALIF_SET_53BIT_PRECISION_START
#  define _ALIF_SET_53BIT_PRECISION_END
#endif









// 197
#ifndef _ALIF_SHORT_FLOAT_REPR
	#define _ALIF_SHORT_FLOAT_REPR 1
#endif
