#include "alif.h"
#include "OpCodeIDs.h"

#include "AlifCore_Code.h"

#include "AlifCore_OpCodeData.h"




static const uint8_t deInstrument[256] = { // 94
    RESUME,
    RETURN_VALUE,
    RETURN_CONST,
    //CALL,
    //CALL_KW,
    //CALL_FUNCTION_EX,
    //YIELD_VALUE,
    JUMP_FORWARD,
    JUMP_BACKWARD,
    POP_JUMPIF_FALSE,
    POP_JUMPIF_TRUE,
    POP_JUMPIF_NONE,
    POP_JUMPIF_NOTNONE,
    //FOR_ITER,
    //END_FOR,
    //END_SEND,
    //LOAD_SUPER_ATTR,
};



AlifIntT alif_getBaseOpCode(AlifCodeObject* _code, AlifIntT _i)
{
    AlifIntT opcode = ALIFCODE_CODE(_code)[_i].op.code;
    if (opcode == INSTRUMENTED_LINE) {
        //opcode = _code->monitoring->lines[_i].originalOpCode;
    }
    if (opcode == INSTRUMENTED_INSTRUCTION) {
        //opcode = _code->monitoring->perInstructionOpCodes[_i];
    }

    AlifIntT deinstrumented = deInstrument[opcode];
    if (deinstrumented) {
        return deinstrumented;
    }
    return alifOpCode_deOpt[opcode];
}

