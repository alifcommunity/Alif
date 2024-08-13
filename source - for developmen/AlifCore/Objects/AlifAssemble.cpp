#include "alif.h"

#include "AlifCore_Code.h"
#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSeq.h"
#include "AlifCore_OpCodeData.h"
#include "AlifCore_OpCodeData.h"
#include "AlifCore_SymTable.h"



#define DEFAULT_CODE_SIZE 128
#define DEFAULT_LNOTAB_SIZE 16
#define DEFAULT_CNOTAB_SIZE 32




class AlifAssembler {
public:
	AlifObject* wByteCode{};
	AlifIntT offset{};

	AlifIntT lineNo{};
	AlifObject* lineTable{};
	AlifIntT locationOff{};
};

static AlifIntT instr_size(AlifInstruction* _instr) { 

	AlifIntT opCode = _instr->opCode;
	AlifIntT opArg = _instr->opArg;
	AlifIntT extendedArgs = (0xFFFFFF < opArg) + (0xFFFF < opArg) + (0xFF < opArg);

	return extendedArgs + 1;
}

static AlifIntT assemble_init(AlifAssembler* _assembler, AlifIntT _firstLineNo) { 

	_assembler->lineNo = _firstLineNo;
	_assembler->lineTable = nullptr;
	_assembler->locationOff = 0;
	
	_assembler->wByteCode = alifBytes_fromStringAndSize(nullptr, DEFAULT_CODE_SIZE);
	if (_assembler->wByteCode == nullptr) goto error;

	_assembler->lineTable = alifBytes_fromStringAndSize(nullptr, DEFAULT_CNOTAB_SIZE);
	if (_assembler->lineTable == nullptr) goto error;


	return 1;

error:
	ALIF_XDECREF(_assembler->wByteCode);
	ALIF_XDECREF(_assembler->lineTable);
	return -1;
}

static void write_instr(AlifCodeUnit* _codeStr, AlifInstruction* _instr, AlifIntT _iLen) { 

	AlifIntT opCode = _instr->opCode;
	AlifIntT opArg = _instr->opArg;
	switch (_iLen) {
	case 4:
		_codeStr->op.code = EXTENDED_ARG;
		_codeStr->op.arg = (opArg >> 24) & 0xFF;
		_codeStr++;
	case 3:
		_codeStr->op.code = EXTENDED_ARG;
		_codeStr->op.arg = (opArg >> 16) & 0xFF;
		_codeStr++;
	case 2:
		_codeStr->op.code = EXTENDED_ARG;
		_codeStr->op.arg = (opArg >> 8) & 0xFF;
		_codeStr++;
	case 1:
		_codeStr->op.code = opCode;
		_codeStr->op.arg = opArg & 0xFF;
		_codeStr++;
		break;
	//default:
		//ALIF_UNREACHABLE(); // error
	}
	//while (caches--) {
	//	_codeStr->op.code = CACHE;
	//	_codeStr->op.arg = 0;
	//	_codeStr++;
	//}
}



static AlifIntT assemble_emitInstr(AlifAssembler* _assembler, AlifInstruction* _instr) { 

	AlifSizeT len = ALIFWBYTES_GET_SIZE(_assembler->wByteCode);
	AlifCodeUnit* code{};

	AlifIntT size = instr_size(_instr);
	if (_assembler->offset + size >= len / (AlifIntT)sizeof(AlifCodeUnit)) {
		if (len > ALIF_SIZET_MAX / 2) {
			return - 1;
		}
		if (alifSubBytes_resize(&_assembler->wByteCode, len * 2) == -1) return -1;
	}

	code = (AlifCodeUnit*)ALIFWBYTES_AS_STRING(_assembler->wByteCode) + _assembler->offset;
	_assembler->offset += size;
	write_instr(code, _instr, size);

	return 1;
}

static AlifIntT assemble_emit(AlifAssembler* _assembler, InstructionSequence* _instrs, AlifIntT _firstLineNo) { 

	if (assemble_init(_assembler, _firstLineNo) == -1) return -1;

	for (AlifIntT i = 0; i < _instrs->used; i++) {
		AlifInstruction* instr = &_instrs->instructions[i];
		if (assemble_emitInstr(_assembler, instr) == -1) return -1;
	}

	/* code here */

	return 1;
}

static AlifObject* dictKeys_inorder(AlifObject* _dict, AlifSizeT _offset) { 

	AlifObject* tuple{}, * k{}, * v{};
	AlifSizeT pos = 0, size = ALIFDICT_GET_SIZE(_dict);

	tuple = alifNew_tuple(size);
	if (tuple == nullptr) return nullptr;

	while (alifDict_next(_dict, &pos, &k, &v)) {
		AlifSizeT i = alifInteger_asSizeT(v);
		if (i == -1
			//and
			// error
			) {
			ALIF_DECREF(tuple);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(tuple, i - _offset, ALIF_NEWREF(k));
	}

	return tuple;
}

static AlifIntT compute_localsPlusInfo(AlifCompileCodeUnitData* _cud, AlifIntT nLocalsPlus,
	AlifObject* _names, AlifObject* _kinds) { 

	/* code here */

	return 1;
}

static AlifCodeObject* makeCode(AlifCompileCodeUnitData* _cud, AlifAssembler* _assembler, 
	AlifObject* _constsList, AlifIntT _maxDepth, AlifIntT _nLocalsPlus, AlifObject* _fn) { 

	AlifCodeObject* co = nullptr;
	AlifObject* names = nullptr;
	AlifObject* consts = nullptr;
	AlifObject* localsPlusNames = nullptr;
	AlifObject* localsPlusKinds = nullptr;
	AlifIntT posOnlyArgCount;
	AlifIntT posOrKwargCount;
	AlifIntT kwOnlyArgCount;
	AlifCodeConstructor cons {};


	names = dictKeys_inorder(_cud->names, 0);
	if (!names) goto error;

	consts = alifList_asTuple(_constsList);
	if (consts == nullptr) goto error;

	posOnlyArgCount = (AlifIntT)_cud->posOnlyArgCount;
	posOrKwargCount = (AlifIntT)_cud->argCount;
	kwOnlyArgCount = (AlifIntT)_cud->kwOnlyArgCount;

	localsPlusNames = alifNew_tuple(_nLocalsPlus);
	if (localsPlusNames == nullptr) goto error;

	localsPlusKinds = alifBytes_fromStringAndSize(nullptr, _nLocalsPlus);
	if (localsPlusKinds == nullptr) goto error;

	if (compute_localsPlusInfo(_cud, _nLocalsPlus,
		localsPlusNames, localsPlusKinds) == -1) {
		goto error;
	}

	cons = {
		_fn,
		_cud->name,
		_cud->qualName ? _cud->qualName : _cud->name,

		_assembler->wByteCode,
		_cud->firstLineNo,
		_assembler->lineTable,

		consts,
		names,

		localsPlusNames,
		localsPlusKinds,

		posOnlyArgCount + posOrKwargCount,
		posOnlyArgCount,
		posOrKwargCount,

		_maxDepth,
	};

	//if (alifCode_validate(&cons) < 0) goto error;

	cons.localsPlusNames = localsPlusNames;

	co = alifCode_new(&cons);
	if (co == nullptr) goto error;

error:
	//ALIF_XDECREF(names);
	//ALIF_XDECREF(consts);
	ALIF_XDECREF(localsPlusNames);
	ALIF_XDECREF(localsPlusKinds);
	return co;
}

AlifCodeObject* alifAssemble_makeCodeObject(AlifCompileCodeUnitData* _cud, AlifObject* _consts,
	AlifIntT _maxDepth, InstructionSequence* _instrs, AlifIntT _nLocalsPlus, AlifObject* _fn) { 

	if (alifInstructionSeq_applyLableMap(_instrs) < 0) {
		return nullptr;
	}
	//if (resolve_unconditionalJumps(_instrs) < 0) {
	//	return nullptr;
	//}
	//if (resolve_jumpOffsets(_instrs) < 0) {
	//	return nullptr;
	//}
	AlifCodeObject* co = nullptr;

	AlifAssembler assembler{};
	AlifIntT res = assemble_emit(&assembler, _instrs, _cud->firstLineNo);
	if (res == 1) {
		co = makeCode(_cud, &assembler, _consts, _maxDepth, _nLocalsPlus, _fn);
	}

	//assemble_free(&assembler);
	return co;
}
