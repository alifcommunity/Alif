# include "alif.h"




#ifdef HAVE_GCC_ASM_FOR_X87
// Inline assembly for getting and setting the 387 FPU control word on
// GCC/x86.
#ifdef _ALIF_MEMORY_SANITIZER
__attribute__((no_sanitize_memory))
#endif
unsigned short _alifGet_387ControlWord(void) {
	unsigned short cw;
	__asm__ __volatile__ ("fnstcw %0" : "=m" (cw));
	return cw;
}

void _alifSet_387ControlWord(unsigned short cw) {
	__asm__ __volatile__ ("fldcw %0" : : "m" (cw));
}
#endif  // HAVE_GCC_ASM_FOR_X87
