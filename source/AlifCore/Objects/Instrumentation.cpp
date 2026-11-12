#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_Frame.h"
#include "AlifCore_Interpreter.h"

#include "AlifCore_OpcodeMetaData.h"



// 41
#define LOCK_CODE(_code)                                             \
    ALIF_BEGIN_CRITICAL_SECTION(_code)

#define UNLOCK_CODE()   ALIF_END_CRITICAL_SECTION()

#define MODIFY_BYTECODE(code, func, ...)                       \
    do {                                                       \
        AlifCodeObject* co = (code);                             \
        for (AlifSizeT i = 0; i < code->coTlbc->size; i++) { \
            char *bc = co->coTlbc->entries[i];                \
            if (bc == nullptr) {                                  \
                continue;                                      \
            }                                                  \
            (func)((AlifCodeUnit*)bc, __VA_ARGS__);           \
        }                                                      \
    } while (0)


AlifObject _alifInstrumentationDisable_ = ALIFOBJECT_HEAD_INIT(&_alifBaseObjectType_); // 54

AlifObject _alifInstrumentationMissing_ = ALIFOBJECT_HEAD_INIT(&_alifBaseObjectType_); // 56





static const uint8_t _deInstrument_[256] = { // 94
	0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	CALL_KW, // INSTRUMENTED_CALL_KW // 241
	CALL_FUNCTION_EX, // INSTRUMENTED_CALL_FUNCTION_EX // 242
	0,0,0,0,0,0,0,0,0,
	CALL, // INSTRUMENTED_CALL // 252
};



AlifIntT _alifInstruction_getLength(AlifCodeObject* code,
	AlifIntT offset) { // 321
	//ASSERT_WORLD_STOPPED_OR_LOCKED(code);

	AlifIntT opcode = alifAtomic_loadUint8Relaxed(&ALIFCODE_CODE(code)[offset].op.code);
	if (opcode == INSTRUMENTED_LINE) {
		opcode = code->monitoring->lines[offset].originalOpcode;
	}
	if (opcode == INSTRUMENTED_INSTRUCTION) {
		opcode = code->monitoring->perInstructionOpcodes[offset];
	}
	int deinstrumented = _deInstrument_[opcode];
	if (deinstrumented) {
		opcode = deinstrumented;
	}
	else {
		opcode = _alifOpcodeDeopt_[opcode];
	}
	if (opcode == ENTER_EXECUTOR) {
		AlifIntT exec_index = ALIFCODE_CODE(code)[offset].op.arg;
		AlifExecutorObject* exec = code->executors->executors[exec_index];
		opcode = _alifOpcodeDeopt_[exec->data.opcode];
	}
	return 1 + _alifOpcodeCaches_[opcode];
}



AlifCodeUnit _alif_getBaseCodeUnit(AlifCodeObject* _code, AlifIntT _i) { // 583
	AlifCodeUnit* src_instr = ALIFCODE_CODE(_code) + _i;
	AlifCodeUnit inst = {
		.cache = alifAtomic_loadUint16Relaxed(&(*(uint16_t*)src_instr))};
	AlifIntT opcode = inst.op.code;
	if (opcode < MIN_INSTRUMENTED_OPCODE) {
		inst.op.code = _alifOpcodeDeopt_[opcode];
		return inst;
	}
	if (opcode == ENTER_EXECUTOR) {
		AlifExecutorObject* exec = _code->executors->executors[inst.op.arg];
		opcode = _alifOpcodeDeopt_[exec->data.opcode];
		inst.op.code = opcode;
		inst.op.arg = exec->data.oparg;
		return inst;
	}
	if (opcode == INSTRUMENTED_LINE) {
		opcode = _code->monitoring->lines[_i].originalOpcode;
	}
	if (opcode == INSTRUMENTED_INSTRUCTION) {
		opcode = _code->monitoring->perInstructionOpcodes[_i];
	}

	AlifIntT deinstrumented = _deInstrument_[opcode];
	if (deinstrumented) {
		inst.op.code = deinstrumented;
	}
	else {
		inst.op.code = _alifOpcodeDeopt_[opcode];
	}
	return inst;
}


//AlifIntT _alif_callInstrumentation2args( AlifThread* _thread, AlifIntT _event, AlifInterpreterFrame* _frame,
//	AlifCodeUnit* _instr, AlifObject* _arg0, AlifObject* _arg1) { // 1164
//	AlifObject* args[5] = { nullptr, nullptr, nullptr, _arg0, _arg1 };
//	return call_instrumentationVector(_thread, _event, _frame, _instr, 4, args);
//}


static AlifIntT instrument_lockHeld(AlifCodeObject* _code, AlifInterpreter* _interp) { // 1852
	//if (isVersion_upToDate(_code, _interp)) {
	//	return 0;
	//}

	//return forceInstrument_lockHeld(_code, _interp);
	return 1; //* alif
}

AlifIntT _alif_instrument(AlifCodeObject* _code, AlifInterpreter* _interp) { // 1868
	AlifIntT res{};
	LOCK_CODE(_code);
	res = instrument_lockHeld(_code, _interp);
	UNLOCK_CODE();
	return res;
}
