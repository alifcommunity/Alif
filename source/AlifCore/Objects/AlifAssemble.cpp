#include "alif.h"

#include "AlifCore_Code.h"
#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_OpcodeMetaData.h"
#include "AlifCore_SymTable.h"


 // 12
#define DEFAULT_CODE_SIZE 128
#define DEFAULT_LNOTAB_SIZE 16
#define DEFAULT_CNOTAB_SIZE 32


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


static inline bool same_location(Location _a, Location _b) { // 30
	return _a.lineNo == _b.lineNo and
		_a.endLineNo == _b.endLineNo and
		_a.colOffset == _b.colOffset and
		_a.endColOffset == _b.endColOffset;
}


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




static AlifIntT assemble_init(AlifAssembler* _a, AlifIntT _firstLineno) { // 62
	memset(_a, 0, sizeof(AlifAssembler));
	_a->lineno = _firstLineno;
	_a->lineTable = nullptr;
	_a->locationOff = 0;
	_a->exceptTable = nullptr;
	_a->bytecode = alifBytes_fromStringAndSize(nullptr, DEFAULT_CODE_SIZE);
	if (_a->bytecode == nullptr) {
		goto error;
	}
	_a->lineTable = alifBytes_fromStringAndSize(nullptr, DEFAULT_CNOTAB_SIZE);
	if (_a->lineTable == nullptr) {
		goto error;
	}
	_a->exceptTable = alifBytes_fromStringAndSize(nullptr, DEFAULT_LNOTAB_SIZE);
	if (_a->exceptTable == nullptr) {
		goto error;
	}
	return SUCCESS;
error:
	ALIF_XDECREF(_a->bytecode);
	ALIF_XDECREF(_a->lineTable);
	ALIF_XDECREF(_a->exceptTable);
	return ERROR;
}




static void assemble_free(AlifAssembler* _a) { // 90
	ALIF_XDECREF(_a->bytecode);
	ALIF_XDECREF(_a->lineTable);
	ALIF_XDECREF(_a->exceptTable);
}


static inline void write_exceptByte(AlifAssembler* _a, AlifIntT _byte) { // 99
	unsigned char* p_ = (unsigned char*)ALIFBYTES_AS_STRING(_a->exceptTable);
	p_[_a->exceptTableOff++] = _byte;
}

#define CONTINUATION_BIT 64 // 104

static void assemble_emitExceptionTableItem(AlifAssembler* _a, AlifIntT _value, AlifIntT _msb) { // 107
	if (_value >= 1 << 24) {
		write_exceptByte(_a, (_value >> 24) | CONTINUATION_BIT | _msb);
		_msb = 0;
	}
	if (_value >= 1 << 18) {
		write_exceptByte(_a, ((_value >> 18) & 0x3f) | CONTINUATION_BIT | _msb);
		_msb = 0;
	}
	if (_value >= 1 << 12) {
		write_exceptByte(_a, ((_value >> 12) & 0x3f) | CONTINUATION_BIT | _msb);
		_msb = 0;
	}
	if (_value >= 1 << 6) {
		write_exceptByte(_a, ((_value >> 6) & 0x3f) | CONTINUATION_BIT | _msb);
		_msb = 0;
	}
	write_exceptByte(_a, (_value & 0x3f) | _msb);
}

#define MAX_SIZE_OF_ENTRY 20 // 131

static AlifIntT assemble_emitExceptionTableEntry(AlifAssembler* _a, AlifIntT _start, AlifIntT _end,
	AlifIntT _handlerOffset,
	AlifExceptHandlerInfo* _handler) { // 134
	AlifSizeT len_ = ALIFBYTES_GET_SIZE(_a->exceptTable);
	if (_a->exceptTableOff + MAX_SIZE_OF_ENTRY >= len_) {
		RETURN_IF_ERROR(alifBytes_resize(&_a->exceptTable, len_ * 2));
	}
	AlifIntT size = _end - _start;
	AlifIntT target = _handlerOffset;
	AlifIntT depth = _handler->startDepth - 1;
	if (_handler->preserveLastI > 0) {
		depth -= 1;
	}
	AlifIntT depthLastI = (depth << 1) | _handler->preserveLastI;
	assemble_emitExceptionTableItem(_a, _start, (1 << 7));
	assemble_emitExceptionTableItem(_a, size, 0);
	assemble_emitExceptionTableItem(_a, target, 0);
	assemble_emitExceptionTableItem(_a, depthLastI, 0);
	return SUCCESS;
}

static AlifIntT assemble_exceptionTable(AlifAssembler* _a,
	InstrSequence* _instrs) { // 158
	AlifIntT ioffset = 0;
	AlifExceptHandlerInfo handler{};
	handler.label = -1;
	handler.startDepth = -1;
	handler.preserveLastI = -1;
	AlifIntT start = -1;
	for (AlifIntT i = 0; i < _instrs->used; i++) {
		Instruction* instr = &_instrs->instrs[i];
		if (instr->exceptHandlerInfo.label != handler.label) {
			if (handler.label >= 0) {
				AlifIntT handlerOffset = _instrs->instrs[handler.label].offset;
				RETURN_IF_ERROR(
					assemble_emitExceptionTableEntry(_a, start, ioffset,
						handlerOffset,
						&handler));
			}
			start = ioffset;
			handler = instr->exceptHandlerInfo;
		}
		ioffset += instr_size(instr);
	}
	if (handler.label >= 0) {
		AlifIntT handler_offset = _instrs->instrs[handler.label].offset;
		RETURN_IF_ERROR(assemble_emitExceptionTableEntry(_a, start, ioffset,
			handler_offset,
			&handler));
	}
	return SUCCESS;
}

#define MSB 0x80 // 194

static void write_locationByte(AlifAssembler* _a, AlifIntT _val) { // 197
	ALIFBYTES_AS_STRING(_a->lineTable)[_a->locationOff] = _val & 255;
	_a->locationOff++;
}


static uint8_t* location_pointer(AlifAssembler* _a) { // 205
	return (uint8_t*)ALIFBYTES_AS_STRING(_a->lineTable) +
		_a->locationOff;
}

static void write_locationFirstByte(AlifAssembler* _a, AlifIntT _code, AlifIntT _length) { // 212
	_a->locationOff += write_locationEntryStart(
		location_pointer(_a), _code, _length);
}

static void write_locationVarint(AlifAssembler* _a, AlifUIntT _val) { // 219
	uint8_t* ptr_ = location_pointer(_a);
	_a->locationOff += write_varint(ptr_, _val);
}


static void write_locationSignedVarint(AlifAssembler* _a, AlifIntT _val) { // 227
	uint8_t* ptr_ = location_pointer(_a);
	_a->locationOff += write_signedVarint(ptr_, _val);
}

static void write_locationInfoShortForm(AlifAssembler* _a, AlifIntT _length, AlifIntT _column, AlifIntT _endColumn) { // 234
	AlifIntT columnLowBits = _column & 7;
	AlifIntT columnGroup = _column >> 3;
	write_locationFirstByte(_a, AlifCode_Location_Info_Short + columnGroup, _length);
	write_locationByte(_a, (columnLowBits << 4) | (_endColumn - _column));
}

static void
write_locationInfoOneLineForm(AlifAssembler* _a, AlifIntT _length, AlifIntT _lineDelta, AlifIntT _column, AlifIntT _endColumn) { // 247
	write_locationFirstByte(_a, AlifCode_Location_Info_One_Line0 + _lineDelta, _length);
	write_locationByte(_a, _column);
	write_locationByte(_a, _endColumn);
}

static void write_locationInfoLongForm(AlifAssembler* _a, Location _loc, AlifIntT _length) { // 259
	write_locationFirstByte(_a, AlifCode_Location_Info_Long, _length);
	write_locationSignedVarint(_a, _loc.lineNo - _a->lineno);
	write_locationVarint(_a, _loc.endLineNo - _loc.lineNo);
	write_locationVarint(_a, _loc.colOffset + 1);
	write_locationVarint(_a, _loc.endColOffset + 1);
}

static void write_locationInfoNone(AlifAssembler* _a, AlifIntT _length) { // 271
	write_locationFirstByte(_a, AlifCode_Location_Info_None, _length);
}

static void write_locationInfoNoColumn(AlifAssembler* _a, AlifIntT _length, AlifIntT _lineDelta) { // 277
	write_locationFirstByte(_a, AlifCode_Location_Info_No_Columns, _length);
	write_locationSignedVarint(_a, _lineDelta);
}

#define THEORETICAL_MAX_ENTRY_SIZE 25 // 283

static AlifIntT write_locationInfoEntry(AlifAssembler* _a, Location _loc, AlifIntT _iSize) { // 287
	AlifSizeT len_ = ALIFBYTES_GET_SIZE(_a->lineTable);
	if (_a->locationOff + THEORETICAL_MAX_ENTRY_SIZE >= len_) {
		RETURN_IF_ERROR(alifBytes_resize(&_a->lineTable, len_ * 2));
	}
	if (_loc.lineNo < 0) {
		write_locationInfoNone(_a, _iSize);
		return SUCCESS;
	}
	AlifIntT lineDelta = _loc.lineNo - _a->lineno;
	AlifIntT column = _loc.colOffset;
	AlifIntT endColumn = _loc.endColOffset;
	if (column < 0 or endColumn < 0) {
		if (_loc.endLineNo == _loc.lineNo or _loc.endLineNo == -1) {
			write_locationInfoNoColumn(_a, _iSize, lineDelta);
			_a->lineno = _loc.lineNo;
			return SUCCESS;
		}
	}
	else if (_loc.endLineNo == _loc.lineNo) {
		if (lineDelta == 0 and column < 80 and endColumn - column < 16 and endColumn >= column) {
			write_locationInfoShortForm(_a, _iSize, column, endColumn);
			return SUCCESS;
		}
		if (lineDelta >= 0 and lineDelta < 3 and column < 128 and endColumn < 128) {
			write_locationInfoOneLineForm(_a, _iSize, lineDelta, column, endColumn);
			_a->lineno = _loc.lineNo;
			return SUCCESS;
		}
	}
	write_locationInfoLongForm(_a, _loc, _iSize);
	_a->lineno = _loc.lineNo;
	return SUCCESS;
}

static AlifIntT assemble_emitLocation(AlifAssembler* _a, Location _loc, AlifIntT _isize) { // 326
	if (_isize == 0) {
		return SUCCESS;
	}
	while (_isize > 8) {
		RETURN_IF_ERROR(write_locationInfoEntry(_a, _loc, 8));
		_isize -= 8;
	}
	return write_locationInfoEntry(_a, _loc, _isize);
}


static AlifIntT assemble_locationInfo(AlifAssembler* _a, InstrSequence* _instrs,
	AlifIntT _firstLineno) { // 339
	_a->lineno = _firstLineno;
	Location loc = _noLocation_;
	AlifIntT size = 0;
	for (AlifIntT i = 0; i < _instrs->used; i++) {
		Instruction* instr = &_instrs->instrs[i];
		if (!same_location(loc, instr->loc)) {
			RETURN_IF_ERROR(assemble_emitLocation(_a, loc, size));
			loc = instr->loc;
			size = 0;
		}
		size += instr_size(instr);
	}
	RETURN_IF_ERROR(assemble_emitLocation(_a, loc, size));
	return SUCCESS;
}





static void write_instr(AlifCodeUnit* codestr,
	Instruction* instr, AlifIntT ilen) { // 359

	AlifIntT opcode = instr->opcode;
	AlifIntT oparg = instr->oparg;
	AlifIntT caches = _alifOpcodeCaches_[opcode];
	switch (ilen - caches) {
	case 4:
		codestr->op.code = EXTENDED_ARG;
		codestr->op.arg = (oparg >> 24) & 0xFF;
		codestr++;
		ALIF_FALLTHROUGH;
	case 3:
		codestr->op.code = EXTENDED_ARG;
		codestr->op.arg = (oparg >> 16) & 0xFF;
		codestr++;
		ALIF_FALLTHROUGH;
	case 2:
		codestr->op.code = EXTENDED_ARG;
		codestr->op.arg = (oparg >> 8) & 0xFF;
		codestr++;
		ALIF_FALLTHROUGH;
	case 1:
		codestr->op.code = opcode;
		codestr->op.arg = oparg & 0xFF;
		codestr++;
		break;
	default:
		ALIF_UNREACHABLE();
	}
	while (caches--) {
		codestr->op.code = CACHE;
		codestr->op.arg = 0;
		codestr++;
	}
}





static AlifIntT assemble_emitInstr(AlifAssembler* a, Instruction* instr) { // 403
	AlifSizeT len = ALIFBYTES_GET_SIZE(a->bytecode);
	AlifCodeUnit* code{};

	AlifIntT size = instr_size(instr);
	if (a->offset + size >= len / (AlifIntT)sizeof(AlifCodeUnit)) {
		if (len > ALIF_SIZET_MAX / 2) {
			return ERROR;
		}
		RETURN_IF_ERROR(alifBytes_resize(&a->bytecode, len * 2));
	}
	code = (AlifCodeUnit*)ALIFBYTES_AS_STRING(a->bytecode) + a->offset;
	a->offset += size;
	write_instr(code, instr, size);
	return SUCCESS;
}


static AlifIntT assemble_emit(AlifAssembler* _a, InstrSequence* _instrs,
	AlifIntT _firstLineno, AlifObject* _constCache) { // 422
	RETURN_IF_ERROR(assemble_init(_a, _firstLineno));

	for (AlifIntT i = 0; i < _instrs->used; i++) {
		Instruction* instr = &_instrs->instrs[i];
		RETURN_IF_ERROR(assemble_emitInstr(_a, instr));
	}

	RETURN_IF_ERROR(assemble_locationInfo(_a, _instrs, _a->lineno));

	RETURN_IF_ERROR(assemble_exceptionTable(_a, _instrs));

	RETURN_IF_ERROR(alifBytes_resize(&_a->exceptTable, _a->exceptTableOff));
	RETURN_IF_ERROR(_alifCompiler_constCacheMergeOne(_constCache, &_a->exceptTable));

	RETURN_IF_ERROR(alifBytes_resize(&_a->lineTable, _a->locationOff));
	RETURN_IF_ERROR(_alifCompiler_constCacheMergeOne(_constCache, &_a->lineTable));

	RETURN_IF_ERROR(alifBytes_resize(&_a->bytecode, _a->offset * sizeof(AlifCodeUnit)));
	RETURN_IF_ERROR(_alifCompiler_constCacheMergeOne(_constCache, &_a->bytecode));
	return SUCCESS;
}

static AlifObject* dict_keysInorder(AlifObject* _dict, AlifSizeT _offset) { // 448
	AlifObject* tuple{}, * k{}, * v{};
	AlifSizeT pos = 0, size = ALIFDICT_GET_SIZE(_dict);

	tuple = alifTuple_new(size);
	if (tuple == nullptr)
		return nullptr;
	while (alifDict_next(_dict, &pos, &k, &v)) {
		AlifSizeT i = alifLong_asSizeT(v);
		if (i == -1 /*and alifErr_occurred()*/) {
			ALIF_DECREF(tuple);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(tuple, i - _offset, ALIF_NEWREF(k));
	}
	return tuple;
}


extern void alifSet_localsPlusInfo(AlifIntT, AlifObject*,
	unsigned char, AlifObject*, AlifObject*); // 471


static AlifIntT compute_localsPlusInfo(AlifCompileCodeUnitMetadata* _umd,
	AlifIntT _nLocalsPlus, AlifObject* _names, AlifObject* _kinds) { // 474
	AlifObject* k{}, * v{};
	AlifSizeT pos = 0;
	while (alifDict_next(_umd->varnames, &pos, &k, &v)) {
		AlifIntT offset = alifLong_asInt(v);
		if (offset == -1 /*and alifErr_occurred()*/) {
			return ERROR;
		}

		// For now we do not distinguish arg kinds.
		AlifLocalsKind kind = CO_FAST_LOCAL;
		AlifIntT has_key = alifDict_contains(_umd->fasthidden, k);
		RETURN_IF_ERROR(has_key);
		if (has_key) {
			kind |= CO_FAST_HIDDEN;
		}

		has_key = alifDict_contains(_umd->cellvars, k);
		RETURN_IF_ERROR(has_key);
		if (has_key) {
			kind |= CO_FAST_CELL;
		}

		alifSet_localsPlusInfo(offset, k, kind, _names, _kinds);
	}
	AlifIntT nlocals = (AlifIntT)ALIFDICT_GET_SIZE(_umd->varnames);

	// This counter mirrors the fix done in fix_cell_offsets().
	AlifIntT numdropped = 0;
	pos = 0;
	while (alifDict_next(_umd->cellvars, &pos, &k, &v)) {
		AlifIntT hasName = alifDict_contains(_umd->varnames, k);
		RETURN_IF_ERROR(hasName);
		if (hasName) {
			// Skip cells that are already covered by locals.
			numdropped += 1;
			continue;
		}

		AlifIntT offset = alifLong_asInt(v);
		if (offset == -1 /*and alifErr_occurred()*/) {
			return ERROR;
		}
		offset += nlocals - numdropped;
		alifSet_localsPlusInfo(offset, k, CO_FAST_CELL, _names, _kinds);
	}

	pos = 0;
	while (alifDict_next(_umd->freevars, &pos, &k, &v)) {
		AlifIntT offset = alifLong_asInt(v);
		if (offset == -1 /*and alifErr_occurred()*/) {
			return ERROR;
		}
		offset += nlocals - numdropped;
		alifSet_localsPlusInfo(offset, k, CO_FAST_FREE, _names, _kinds);
	}
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

	AlifIntT posOnlyArgCount{}; //* alif
	AlifIntT posOrKwArgCount{}; //* alif
	AlifIntT kwOnlyArgCount{}; //* alif

	class AlifCodeConstructor con; //* alif

	names = dict_keysInorder(_umd->names, 0);
	if (!names) {
		goto error;
	}
	if (_alifCompiler_constCacheMergeOne(_constCache, &names) < 0) {
		goto error;
	}

	consts = alifList_asTuple(_constsList); /* alifCode_new requires a tuple */
	if (consts == nullptr) {
		goto error;
	}
	if (_alifCompiler_constCacheMergeOne(_constCache, &consts) < 0) {
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

	if (_alifCompiler_constCacheMergeOne(_constCache, &localsPlusNames) < 0) {
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
