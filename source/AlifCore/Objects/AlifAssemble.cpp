#include "alif.h"

#include "AlifCore_Code.h"
#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_OpcodeMetadata.h"
#include "AlifCore_SymTable.h"




 // 16
#undef SUCCESS
#undef ERROR
#define SUCCESS 0
#define ERROR -1

#define RETURN_IF_ERROR(X)  \
    if ((X) < 0) {          \
        return ERROR;       \
    }

 // 26
typedef AlifSourceLocation Location;
typedef AlifInstruction Instruction;
typedef AlifInstructionSequence InstrSequence;





static AlifIntT instr_size(Instruction * _instr) { // 39
	AlifIntT opcode = _instr->opcode;
	AlifIntT oparg = _instr->oparg;
	AlifIntT extended_args = (0xFFFFFF < oparg) + (0xFFFF < oparg) + (0xFF < oparg);
	AlifIntT caches = _alifOpcodeCaches_[opcode];
	return extended_args + 1 + caches;
}

class AlifAssembler { // 51
public:
	AlifObject* bytecode{};
	AlifIntT offset{};         
	AlifObject* exceptTable{}; 
	AlifIntT exceptTableOff{};
	/* Location Info */
	AlifIntT lineno{};
	AlifObject* lineTable{};
	AlifIntT locationOff{};
};





static void assemble_free(AlifAssembler* _a) { // 90
	ALIF_XDECREF(_a->bytecode);
	ALIF_XDECREF(_a->lineTable);
	ALIF_XDECREF(_a->exceptTable);
}
























static AlifIntT assemble_emit(AlifAssembler* _a, InstrSequence* _instrs,
	AlifIntT _firstLineno, AlifObject* _constCache) { // 422
	RETURN_IF_ERROR(assemble_init(_a, _firstLineno));

	for (int i = 0; i < _instrs->used; i++) {
		Instruction* instr = &_instrs->instrs[i];
		RETURN_IF_ERROR(assemble_emitInstr(_a, instr));
	}

	RETURN_IF_ERROR(assemble_locationInfo(_a, _instrs, _a->lineno));

	RETURN_IF_ERROR(assemble_exceptionTable(_a, _instrs));

	RETURN_IF_ERROR(alifBytes_resize(&_a->exceptTable, _a->exceptTableOff));
	RETURN_IF_ERROR(_alifCompile_constCacheMergeOne(_constCache, &_a->exceptTable));

	RETURN_IF_ERROR(alifBytes_resize(&_a->lineTable, _a->locationOff));
	RETURN_IF_ERROR(_alifCompile_constCacheMergeOne(_constCache, &_a->lineTable));

	RETURN_IF_ERROR(alifBytes_resize(&_a->bytecode, _a->offset * sizeof(AlifCodeUnit)));
	RETURN_IF_ERROR(_alifCompile_constCacheMergeOne(_constCache, &_a->byteCode));
	return SUCCESS;
}













static AlifCodeObject* makecode(AlifCompileCodeUnitMetadata* _umd,
	AlifAssembler* _a, AlifObject* _constCache, AlifObject* _constsList,
	AlifIntT _maxDepth, AlifIntT _nLocalsPlus, AlifIntT _codeFlags,
	AlifObject* _filename) { // 542
	AlifCodeObject* co = nullptr;
	AlifObject* names = nullptr;
	AlifObject* consts = nullptr;
	AlifObject* localsPlusNames = nullptr;
	AlifObject* localsPlusKinds = nullptr;

	AlifIntT posOnlyArgCount{}; // alif
	AlifIntT posOrKwArgCount{}; // alif
	AlifIntT kwOnlyArgCount{}; // alif

	class AlifCodeConstructor con {}; // alif

	names = dict_keysInorder(_umd->names, 0);
	if (!names) {
		goto error;
	}
	if (_alifCompile_constCacheMergeOne(_constCache, &names) < 0) {
		goto error;
	}

	consts = alifList_asTuple(_constsList); /* alifCode_new requires a tuple */
	if (consts == nullptr) {
		goto error;
	}
	if (_alifCompile_constCacheMergeOne(_constCache, &consts) < 0) {
		goto error;
	}

	posOnlyArgCount = (AlifIntT)_umd->posOnlyArgCount;
	posOrKwArgCount = (AlifIntT)_umd->argCount;
	kwOnlyArgCount = (AlifIntT)_umd->kwOnlyArgCount;

	localsPlusNames = alifTuple_new(_nLocalsPlus);
	if (localsPlusNames == nullptr) {
		goto error;
	}
	localsPlusKinds = alifBytes_fromStringAndSize(nullptr, _nLocalsPlus);
	if (localsPlusKinds == nullptr) {
		goto error;
	}
	if (compute_localsPlusInfo(_umd, _nLocalsPlus,
		localsPlusNames, localsPlusKinds) == ERROR) {
		goto error;
	}

	con = {
		.filename = _filename,
		.name = _umd->name,
		.qualname = _umd->qualname ? _umd->qualname : _umd->name,
		.flags = _codeFlags,

		.code = _a->bytecode,
		.firstLineno = _umd->firstLineno,
		.lineTable = _a->lineTable,

		.consts = consts,
		.names = names,

		.localsPlusNames = localsPlusNames,
		.localsPlusKinds = localsPlusKinds,

		.argCount = posOnlyArgCount + posOrKwArgCount,
		.posOnlyArgCount = posOnlyArgCount,
		.kwOnlyArgCount = kwOnlyArgCount,

		.stackSize = _maxDepth,

		.exceptionTable = _a->exceptTable,
	};

	if (_alifCode_validate(&con) < 0) {
		goto error;
	}

	if (_alifCompile_constCacheMergeOne(_constCache, &localsPlusNames) < 0) {
		goto error;
	}
	con.localsPlusNames = localsPlusNames;

	co = alifCode_new(&con);
	if (co == nullptr) {
		goto error;
	}

error:
	ALIF_XDECREF(names);
	ALIF_XDECREF(consts);
	ALIF_XDECREF(localsPlusNames);
	ALIF_XDECREF(localsPlusKinds);
	return co;
}


static AlifIntT resolve_jumpOffsets(InstrSequence* _instrs) { // 636
	for (AlifIntT i = 0; i < _instrs->used; i++) {
		Instruction* instr = &_instrs->instrs[i];
		if (OPCODE_HAS_JUMP(instr->opcode)) {
			instr->target = instr->oparg;
		}
	}

	AlifIntT extendedArgRecompile{};

	do {
		AlifIntT totsize = 0;
		for (AlifIntT i = 0; i < _instrs->used; i++) {
			Instruction* instr = &_instrs->instrs[i];
			instr->offset = totsize;
			AlifIntT isize = instr_size(instr);
			totsize += isize;
		}
		extendedArgRecompile = 0;

		AlifIntT offset = 0;
		for (AlifIntT i = 0; i < _instrs->used; i++) {
			Instruction* instr = &_instrs->instrs[i];
			AlifIntT isize = instr_size(instr);

			offset += isize;
			if (OPCODE_HAS_JUMP(instr->opcode)) {
				Instruction* target = &_instrs->instrs[instr->target];
				instr->oparg = target->offset;
				if (instr->oparg < offset) {
					instr->oparg = offset - instr->oparg;
				}
				else {
					instr->oparg = instr->oparg - offset;
				}
				if (instr_size(instr) != isize) {
					extendedArgRecompile = 1;
				}
			}
		}

	} while (extendedArgRecompile);
	return SUCCESS;
}








static AlifIntT resolve_unconditionalJumps(InstrSequence* _instrs) { // 705
	for (AlifIntT i = 0; i < _instrs->used; i++) {
		Instruction* instr = &_instrs->instrs[i];
		bool isForward = (instr->oparg > i);
		switch (instr->opcode) {
		case JUMP:
			instr->opcode = isForward ? JUMP_FORWARD : JUMP_BACKWARD;
			break;
		case JUMP_NO_INTERRUPT:
			instr->opcode = isForward ?
				JUMP_FORWARD : JUMP_BACKWARD_NO_INTERRUPT;
			break;
		default:
			if (OPCODE_HAS_JUMP(instr->opcode) and
				IS_PSEUDO_INSTR(instr->opcode)) {
				ALIF_UNREACHABLE();
			}
		}
	}
	return SUCCESS;
}

AlifCodeObject* alifAssemble_makeCodeObject(AlifCompileCodeUnitMetadata* _umd,
	AlifObject* _constCache, AlifObject* _consts, AlifIntT _maxDepth, InstrSequence* _instrs,
	AlifIntT _nLocalsPlus, AlifIntT _codeFlags, AlifObject* _filename) { // 735
	if (_alifInstructionSequence_applyLabelMap(_instrs) < 0) {
		return nullptr;
	}
	if (resolve_unconditionalJumps(_instrs) < 0) {
		return nullptr;
	}
	if (resolve_jumpOffsets(_instrs) < 0) {
		return nullptr;
	}
	AlifCodeObject* co = nullptr;

	AlifAssembler a{};
	AlifIntT res = assemble_emit(&a, _instrs, _umd->firstLineno, _constCache);
	if (res == SUCCESS) {
		co = makecode(_umd, &a, _constCache, _consts, _maxDepth, _nLocalsPlus,
			_codeFlags, _filename);
	}
	assemble_free(&a);
	return co;
}
