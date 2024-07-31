#include "alif.h"
#include "OpCode.h"

#include "AlifCore_Code.h"









void alifCode_quicken(AlifCodeObject* _code) { 
#if ENABLE_SPECIALIZATION
    int opcode = 0;
    AlifCodeUnit* instructions = ALIFCODE_CODE(_code);
    for (int i = 0; i < ALIF_SIZE(_code); i++) {
        opcode = alif_getBaseOpCode(_code, i);
        //int caches = alifOpCodeCaches[opcode];
        //if (caches) {
        //    switch (opcode) {
        //    case JUMP_BACKWARD:
        //        instructions[i + 1].counter = initial_jumpBackoffCounter();
        //        break;
        //    case POP_JUMPIF_FALSE:
        //    case POP_JUMPIF_TRUE:
        //    case POP_JUMPIF_NONE:
        //    case POP_JUMPIF_NOTNONE:
        //        instructions[i + 1].cache = 0x5555;  // Alternating 0, 1 bits
        //        break;
        //    default:
        //        instructions[i + 1].counter = adaptive_counterWarmup();
        //        break;
        //    }
        //    i += caches;
        //}
    }
#endif /* ENABLE_SPECIALIZATION */
}

