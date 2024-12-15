#pragma once








 // 73
#if USE_COMPUTED_GOTOS
#  define TARGET(op) TARGET_##op:
#  define DISPATCH_GOTO() goto *opcode_targets[opcode]
#else
#  define TARGET(_op) case _op: TARGET_##op:
#  define DISPATCH_GOTO() goto dispatch_opcode
#endif


 // 109
#define DISPATCH() \
    { \
        NEXTOPARG(); \
        /*PRE_DISPATCH_GOTO();*/ \
        DISPATCH_GOTO(); \
    }










 // 154
#define NEXTOPARG()  do { \
        AlifCodeUnit word  = {.cache = alifAtomic_loadUint16Relaxed(&*(uint16_t*)nextInstr)}; \
        opcode = word.op.code; \
        oparg = word.op.arg; \
    } while (0)




static inline AlifIntT _alif_enterRecursiveAlif(AlifThread* _thread) { // 368
	return (_thread->alifRecursionRemaining-- <= 0) and
		_alif_checkRecursiveCallAlif(_thread);
}
