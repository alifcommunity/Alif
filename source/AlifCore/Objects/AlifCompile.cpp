#include "alif.h"

#include "Opcode.h"
#include "AlifCore_AST.h"
#define NEED_OPCODE_TABLES
#include "AlifCore_OpcodeUtils.h"
#undef NEED_OPCODE_TABLES
//#include "AlifCore_Code.h"
#include "AlifCore_Compile.h"
//#include "AlifCore_FlowGraph.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_Intrinsics.h"
#include "AlifCore_State.h"

#include "AlifCore_SymTable.h"


#define NEED_OPCODE_METADATA
//#include "AlifCore_OpcodeMetadata.h"
#undef NEED_OPCODE_METADATA



#define STACK_USE_GUIDELINE 30 // 58

 // 60
#undef SUCCESS
#undef ERROR
#define SUCCESS 0
#define ERROR -1
 // 65
#define RETURN_IF_ERROR(_x)  \
    if ((_x) == -1) {        \
        return ERROR;       \
    }


class AlifCompiler; // 81


typedef AlifInstructionSequence InstrSequence; // 84

static InstrSequence* compiler_instrSequence(AlifCompiler*); // 86

#define INSTR_SEQUENCE(_c) compiler_instrSequence(_c) // 91


typedef AlifSourceLocation Location; // 99


#define LOCATION(_lno, _endLno, _col, _endCol) {_lno, _endLno, _col, _endCol} // 108

#define LOC(_x) SRC_LOCATION_FROM_AST(_x)

typedef AlifJumpTargetLabel JumpTargetLabel; // 113

static JumpTargetLabel _noLabel_ = { -1 }; // 115

#define SAME_LABEL(_l1, _l2) ((_l1).id == (_l2).id) // 117
#define IS_LABEL(_l) (!SAME_LABEL((_l), (_noLabel_)))

 // 120
#define NEW_JUMP_TARGET_LABEL(_c, _name) \
    JumpTargetLabel _name = _alifInstructionSequence_newLabel(INSTR_SEQUENCE(_c)); \
    if (!IS_LABEL(_name)) { \
        return ERROR; \
    }

#define USE_LABEL(_c, _lbl) \
    RETURN_IF_ERROR(_alifInstructionSequence_useLabel(INSTR_SEQUENCE(_c), _lbl.id))


enum FBlockType_ { // 137
	While_Loop, For_Loop, Try_Except, Finally_Try, Finally_End,
	With, Async_With, Handler_Cleanup, Pop_Value, Exception_Handler,
	Exception_Group_Handler, Async_Comprehension_Generator,
	Stop_Iteration
};

class FBlockInfo { // 142
public:
	FBlockType_ type{};
	JumpTargetLabel block{};
	Location loc{};
	/* (optional) type-specific exit or cleanup block */
	JumpTargetLabel exit{};
	/* (optional) additional information required for unwinding */
	void* datum{};
};

enum ScopeType_ { // 152
	Compiler_Scope_Module,
	Compiler_Scope_Class,
	Compiler_Scope_Function,
	Compiler_Scope_Async_Function,
	Compiler_Scope_Lambda,
	Compiler_Scope_Comprehension,
	Compiler_Scope_Annotations,
};




AlifIntT _alifCompile_ensureArrayLargeEnough(AlifIntT _idx, void** _array,
	AlifIntT* _alloc, AlifIntT _defaultAlloc, AlifUSizeT _itemSize) { // 182
	void* arr = *_array;

	if (arr == nullptr) {
		AlifIntT newAlloc = _defaultAlloc;
		if (_idx >= newAlloc) {
			newAlloc = _idx + _defaultAlloc;
		}
		arr = alifMem_dataAlloc(newAlloc * _itemSize); // alif
		if (arr == nullptr) {
			//alifErr_noMemory();
			return ERROR;
		}
		*_alloc = newAlloc;
	}
	else if (_idx >= *_alloc) {
		AlifUSizeT oldsize = *_alloc * _itemSize;
		AlifIntT new_alloc = *_alloc << 1;
		if (_idx >= new_alloc) {
			new_alloc = _idx + _defaultAlloc;
		}
		AlifUSizeT newsize = new_alloc * _itemSize;

		if (oldsize > (SIZE_MAX >> 1)) {
			//alifErr_noMemory();
			return ERROR;
		}

		void* tmp = alifMem_dataRealloc(arr, newsize);
		if (tmp == nullptr) {
			//alifErr_noMemory();
			return ERROR;
		}
		*_alloc = new_alloc;
		arr = tmp;
		memset((char*)arr + oldsize, 0, newsize - oldsize);
	}

	*_array = arr;
	return SUCCESS;
}



/*
	The following items change on entry and exit of code blocks.
	They must be saved and restored when returning to a block.
*/
class CompilerUnit { // 231
public:
	AlifSTEntryObject* ste{};

	AlifIntT scopeType{};

	AlifObject* private_{};            /* for private name mangling */
	AlifObject* staticAttributes{};  /* for class: attributes accessed via self.X */
	AlifObject* deferredAnnotations{}; /* AnnAssign nodes deferred to the end of compilation */

	InstrSequence* instrSequence{}; /* codegen output */

	AlifIntT nfBlocks{};
	AlifIntT inInlinedComp{};

	FBlockInfo fBlock[MAXBLOCKS]{};

	AlifCompileCodeUnitMetadata metadata{};
};

class AlifCompiler { // 262
public:
	AlifObject* filename{};
	AlifSymTable* st{};
	AlifFutureFeatures future{};		/* module's __future__ */
	AlifCompilerFlags flags{};

	AlifIntT optimize{};				/* optimization level */
	AlifIntT interactive{};				/* true if in interactive mode */
	AlifObject* constCache{};			/* Alif dict holding all constants,
											including names tuple */
	CompilerUnit* u_{};					/* compiler state for current block */
	AlifObject* stack{};				/* Alif list holding compiler_unit ptrs */
	AlifASTMem* astMem{};				/* pointer to memory allocation astMem */

	bool saveNestedSeqs{};				/* if true, construct recursive instruction sequences
											* (including instructions for nested code objects)
										*/
};







static void compiler_free(AlifCompiler*); // 307
static AlifCodeObject* compiler_mod(AlifCompiler*, ModuleTy); // 312
static AlifIntT compiler_visitStmt(AlifCompiler*, StmtTy); // 313
static AlifIntT compiler_visitExpr(AlifCompiler*, ExprTy); // 315


#define CAPSULE_NAME "AlifCompile.cpp AlifCompiler unit" // 360

static AlifIntT compiler_setup(AlifCompiler* _c, ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _flags, AlifIntT _optimize, AlifASTMem* _astMem) { // 363
	AlifCompilerFlags localFlags = ALIFCOMPILERFLAGS_INIT;

	_c->constCache = alifDict_new();
	if (!_c->constCache) {
		return ERROR;
	}

	_c->stack = alifList_new(0);
	if (!_c->stack) {
		return ERROR;
	}

	_c->filename = ALIF_NEWREF(_filename);
	_c->astMem = _astMem;
	if (!alifFuture_fromAST(_mod, _filename, &_c->future)) {
		return ERROR;
	}
	if (!_flags) {
		_flags = &localFlags;
	}
	AlifIntT merged = _c->future.features | _flags->flags;
	_c->future.features = merged;
	_flags->flags = merged;
	_c->flags = *_flags;
	_c->optimize = (_optimize == -1) ? alif_getConfig()->optimizationLevel : _optimize;
	_c->saveNestedSeqs = false;

	if (!alifAST_optimize(_mod, _astMem, _c->optimize, merged)) {
		return ERROR;
	}
	_c->st = alifSymtable_build(_mod, _filename, &_c->future);
	if (_c->st == nullptr) {
		//if (!alifErr_occurred()) {
		//	alifErr_setString(_alifExcSystemError_, "no symtable");
		//}
		return ERROR;
	}
	return SUCCESS;
}




static AlifCompiler* new_compiler(ModuleTy mod, AlifObject* filename,
	AlifCompilerFlags* pflags, AlifIntT optimize, AlifASTMem* arena) { // 407
	AlifCompiler* compiler = (AlifCompiler*)alifMem_dataAlloc(sizeof(AlifCompiler));
	if (compiler == nullptr) {
		return nullptr;
	}
	if (compiler_setup(compiler, mod, filename, pflags, optimize, arena) < 0) {
		compiler_free(compiler);
		return nullptr;
	}
	return compiler;
}





AlifCodeObject* alifAST_compile(ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _pFlags, AlifIntT _optimize, AlifASTMem* _astMem) { // 422

	AlifCompiler* compiler = new_compiler(_mod, _filename, _pFlags, _optimize, _astMem);
	if (compiler == nullptr) {
		return nullptr;
	}

	AlifCodeObject* co = compiler_mod(compiler, _mod);
	compiler_free(compiler);
	return co;
}



static void compiler_free(AlifCompiler* _c) { // 456
	if (_c->st) alifSymtable_free(_c->st);
	ALIF_XDECREF(_c->filename);
	ALIF_XDECREF(_c->constCache);
	ALIF_XDECREF(_c->stack);
	alifMem_dataFree(_c);
}

static AlifObject* list2dict(AlifObject* _list) { // 467
	AlifSizeT i{}, n{};
	AlifObject* v{}, * k{};
	AlifObject* dict = alifDict_new();
	if (!dict) return nullptr;

	n = alifList_size(_list);
	for (i = 0; i < n; i++) {
		v = alifLong_fromSizeT(i);
		if (!v) {
			ALIF_DECREF(dict);
			return nullptr;
		}
		k = ALIFLIST_GET_ITEM(_list, i);
		if (alifDict_setItem(dict, k, v) < 0) {
			ALIF_DECREF(v);
			ALIF_DECREF(dict);
			return nullptr;
		}
		ALIF_DECREF(v);
	}
	return dict;
}

static AlifObject* dict_byType(AlifObject* _src, AlifIntT _scopeType,
	AlifIntT _flag, AlifSizeT _offset) { // 501
	AlifSizeT i = _offset, numKeys, keyI;
	AlifObject* k, * v, * dest = alifDict_new();
	AlifObject* sortedKeys;

	if (dest == nullptr) return nullptr;

	sortedKeys = alifDict_keys(_src);
	if (sortedKeys == nullptr) {
		ALIF_DECREF(dest);
		return nullptr;
	}
	if (alifList_sort(sortedKeys) != 0) {
		ALIF_DECREF(sortedKeys);
		ALIF_DECREF(dest);
		return nullptr;
	}
	numKeys = ALIFLIST_GET_SIZE(sortedKeys);

	for (keyI = 0; keyI < numKeys; keyI++) {
		k = ALIFLIST_GET_ITEM(sortedKeys, keyI);
		v = alifDict_getItemWithError(_src, k);
		if (!v) {
			//if (!alifErr_occurred()) {
			//	alifErr_SetObject(_alifExcKeyError_, k);
			//}
			ALIF_DECREF(sortedKeys);
			ALIF_DECREF(dest);
			return nullptr;
		}
		long vi = alifLong_asLong(v);
		if (vi == -1 /*and alifErr_occurred()*/) {
			ALIF_DECREF(sortedKeys);
			ALIF_DECREF(dest);
			return nullptr;
		}
		if (SYMBOL_TO_SCOPE(vi) == _scopeType or vi & _flag) {
			AlifObject* item = alifLong_fromSizeT(i);
			if (item == nullptr) {
				ALIF_DECREF(sortedKeys);
				ALIF_DECREF(dest);
				return nullptr;
			}
			i++;
			if (alifDict_setItem(dest, k, item) < 0) {
				ALIF_DECREF(sortedKeys);
				ALIF_DECREF(item);
				ALIF_DECREF(dest);
				return nullptr;
			}
			ALIF_DECREF(item);
		}
	}
	ALIF_DECREF(sortedKeys);
	return dest;
}

static void compiler_unitFree(CompilerUnit* _u) { // 567
	ALIF_CLEAR(_u->instrSequence);
	ALIF_CLEAR(_u->ste);
	ALIF_CLEAR(_u->metadata.name);
	ALIF_CLEAR(_u->metadata.qualname);
	ALIF_CLEAR(_u->metadata.consts);
	ALIF_CLEAR(_u->metadata.names);
	ALIF_CLEAR(_u->metadata.varnames);
	ALIF_CLEAR(_u->metadata.freevars);
	ALIF_CLEAR(_u->metadata.cellvars);
	ALIF_CLEAR(_u->metadata.fasthidden);
	ALIF_CLEAR(_u->private_);
	ALIF_CLEAR(_u->staticAttributes);
	ALIF_CLEAR(_u->deferredAnnotations);
	alifMem_dataFree(_u);
}


static AlifIntT compiler_setQualname(AlifCompiler* _c) { // 612
	AlifSizeT stackSize{};
	CompilerUnit* u = _c->u_;
	AlifObject* name{}, * base{};

	base = nullptr;
	stackSize = ALIFLIST_GET_SIZE(_c->stack);
	if (stackSize > 1) {
		AlifIntT scope, forceGlobal = 0;
		CompilerUnit* parent{};
		AlifObject* mangled{}, * capsule{};

		capsule = ALIFLIST_GET_ITEM(_c->stack, stackSize - 1);
		parent = (CompilerUnit*)alifCapsule_getPointer(capsule, CAPSULE_NAME);
		if (parent->scopeType == ScopeType_::Compiler_Scope_Annotations) {
			if (stackSize == 2) {
				u->metadata.qualname = ALIF_NEWREF(u->metadata.name);
				return SUCCESS;
			}
			capsule = ALIFLIST_GET_ITEM(_c->stack, stackSize - 2);
			parent = (CompilerUnit*)alifCapsule_getPointer(capsule, CAPSULE_NAME);
		}

		if (u->scopeType == ScopeType_::Compiler_Scope_Function
			or u->scopeType == ScopeType_::Compiler_Scope_Async_Function
			or u->scopeType == ScopeType_::Compiler_Scope_Class) {
			mangled = alif_mangle(parent->private_, u->metadata.name);
			if (!mangled) {
				return ERROR;
			}

			scope = alifST_getScope(parent->ste, mangled);
			ALIF_DECREF(mangled);
			RETURN_IF_ERROR(scope);
			if (scope == GLOBAL_EXPLICIT) forceGlobal = 1;
		}

		if (!forceGlobal) {
			if (parent->scopeType == ScopeType_::Compiler_Scope_Function
				or parent->scopeType == ScopeType_::Compiler_Scope_Async_Function
				or parent->scopeType == ScopeType_::Compiler_Scope_Lambda)
			{
				base = alifUStr_concat(parent->metadata.qualname,
					&ALIF_STR(DotLocals));
				if (base == nullptr) {
					return ERROR;
				}
			}
			else {
				base = ALIF_NEWREF(parent->metadata.qualname);
			}
		}
	}

	if (base != nullptr) {
		name = alifUStr_concat(base, ALIF_LATIN1_CHR('.'));
		ALIF_DECREF(base);
		if (name == nullptr) {
			return ERROR;
		}
		alifUStr_append(&name, u->metadata.name);
		if (name == nullptr) {
			return ERROR;
		}
	}
	else {
		name = ALIF_NEWREF(u->metadata.name);
	}
	u->metadata.qualname = name;

	return SUCCESS;
}


static AlifIntT codegen_addOpI(InstrSequence* seq,
	AlifIntT opcode, AlifSizeT oparg, Location loc) { // 699

	AlifIntT oparg_ = ALIF_SAFE_DOWNCAST(oparg, AlifSizeT, AlifIntT);
	return _alifInstructionSequence_addOp(seq, opcode, oparg_, loc);
}

#define ADDOP_I(_c, _loc, _op, _o) \
    RETURN_IF_ERROR(codegen_addOpI(INSTR_SEQUENCE(_c), _op, _o, _loc)) // 715

static AlifIntT codegen_addOpNoArg(InstrSequence* _seq,
	AlifIntT _opcode, Location _loc) { // 721
	return _alifInstructionSequence_addOp(_seq, _opcode, 0, _loc);
}

#define ADDOP(_c, _loc, _op) \
    RETURN_IF_ERROR(codegen_addOpNoArg(INSTR_SEQUENCE(_c), _op, _loc)) // 729

static AlifSizeT dict_addO(AlifObject* _dict, AlifObject* _o) { // 735
	AlifObject* v{};
	AlifSizeT arg{};

	if (alifDict_getItemRef(_dict, _o, &v) < 0) {
		return ERROR;
	}
	if (!v) {
		arg = ALIFDICT_GET_SIZE(_dict);
		v = alifLong_fromSizeT(arg);
		if (!v) {
			return ERROR;
		}
		if (alifDict_setItem(_dict, _o, v) < 0) {
			ALIF_DECREF(v);
			return ERROR;
		}
	}
	else arg = alifLong_asLong(v);

	ALIF_DECREF(v);
	return arg;
}





static AlifObject* merge_constsRecursive(AlifObject* _constCache, AlifObject* _o) { // 876
	return const_cacheInsert(_constCache, _o, true);
}

static AlifSizeT compiler_addConst(AlifCompiler* _c, AlifObject* _o) { // 882
	AlifObject* key = merge_constsRecursive(_c->constCache, _o);
	if (key == nullptr) {
		return ERROR;
	}

	AlifSizeT arg = dict_addO(_c->u_->metadata.consts, key);
	ALIF_DECREF(key);
	return arg;
}

static AlifIntT codegen_addOpLoadConst(AlifCompiler* _c,
	Location _loc, AlifObject* _o) { // 895
	AlifSizeT arg = compiler_addConst(_c, _o);
	if (arg < 0) {
		return ERROR;
	}
	ADDOP_I(_c, _loc, LOAD_CONST, arg);
	return SUCCESS;
}

#define ADDOP_LOAD_CONST(_c, _loc, _o) \
    RETURN_IF_ERROR(codegen_addOpLoadConst(_c, _loc, _o)) // 906



 // 913
#define ADDOP_LOAD_CONST_NEW(_c, _loc, _o) { \
    AlifObject *__new_const = _o; \
    if (__new_const == nullptr) { \
        return ERROR; \
    } \
    if (codegen_addOpLoadConst(_c, _loc, __new_const) < 0) { \
        ALIF_DECREF(__new_const); \
        return ERROR; \
    } \
    ALIF_DECREF(__new_const); \
}


 // 949
#define LOAD_METHOD -1
#define LOAD_SUPER_METHOD -2
#define LOAD_ZERO_SUPER_ATTR -3
#define LOAD_ZERO_SUPER_METHOD -4

static AlifIntT codegen_addOpName(AlifCompiler* _c, Location _loc,
	AlifIntT _opcode, AlifObject* _dict, AlifObject* _o) { // 954
	AlifObject* mangled = compiler_maybeMangle(_c, _o);
	if (!mangled) {
		return ERROR;
	}
	AlifSizeT arg = dict_addO(_dict, mangled);
	ALIF_DECREF(mangled);
	if (arg < 0) {
		return ERROR;
	}
	if (_opcode == LOAD_ATTR) {
		arg <<= 1;
	}
	if (_opcode == LOAD_METHOD) {
		_opcode = LOAD_ATTR;
		arg <<= 1;
		arg |= 1;
	}
	if (_opcode == LOAD_SUPER_ATTR) {
		arg <<= 2;
		arg |= 2;
	}
	if (_opcode == LOAD_SUPER_METHOD) {
		_opcode = LOAD_SUPER_ATTR;
		arg <<= 2;
		arg |= 3;
	}
	if (_opcode == LOAD_ZERO_SUPER_ATTR) {
		_opcode = LOAD_SUPER_ATTR;
		arg <<= 2;
	}
	if (_opcode == LOAD_ZERO_SUPER_METHOD) {
		_opcode = LOAD_SUPER_ATTR;
		arg <<= 2;
		arg |= 1;
	}
	ADDOP_I(_c, _loc, _opcode, arg);
	return SUCCESS;
}



#define ADDOP_NAME(_c, _loc, _op, _o, _type) \
    RETURN_IF_ERROR(codegen_addOpName(_c, _loc, _op, _c->u_->metadata. ## _type, _o)) // 997




static AlifIntT codegen_addOpJ(InstrSequence* _seq, Location _loc,
	AlifIntT _opcode, JumpTargetLabel _target) { // 1000
	return _alifInstructionSequence_addOp(_seq, _opcode, _target.id, _loc);
}


#define ADDOP_JUMP(_c, _loc, _op, _o) \
    RETURN_IF_ERROR(codegen_addOpJ(INSTR_SEQUENCE(_c), _loc, _op, _o)) // 1010

#define ADDOP_BINARY(_c, _loc, _binOp) \
    RETURN_IF_ERROR(addop_binary(_c, _loc, _binOp, false)) // 1016


 // 1035
#define VISIT(_c, _type, _v) \
    RETURN_IF_ERROR(compiler_visit ## _type(_c, _v));


 // 1041
#define VISIT_SEQ(_c, _type, _sequ) { \
    AlifIntT i_{}; \
    ASDL ## _type ## Seq *seq = (_sequ); /* avoid variable capture */ \
    for (i_ = 0; i_ < ASDL_SEQ_LEN(seq); i_++) { \
        _type ## Ty elt = (_type ## Ty)ASDL_SEQ_GET(seq, i_); \
        RETURN_IF_ERROR(compiler_visit ## _type(_c, elt)); \
    } \
}


static AlifIntT compiler_enterScope(AlifCompiler* _c, Identifier _name,
	AlifIntT _scopeType, void* _key, AlifIntT _lineno,
	AlifObject* _private, AlifCompileCodeUnitMetadata* _umd) { // 1063
	Location loc = LOCATION(_lineno, _lineno, 0, 0);

	CompilerUnit* u{};

	u = (CompilerUnit*)alifMem_dataAlloc(sizeof(CompilerUnit));
	if (!u) {
		//alifErr_noMemory();
		return ERROR;
	}
	u->scopeType = _scopeType;
	if (_umd != nullptr) {
		u->metadata = *_umd;
	}
	else {
		u->metadata.argCount = 0;
		u->metadata.posOnlyArgCount = 0;
		u->metadata.kwOnlyArgCount = 0;
	}
	u->ste = _alifSymtable_lookup(_c->st, _key);
	if (!u->ste) {
		compiler_unitFree(u);
		return ERROR;
	}
	u->metadata.name = ALIF_NEWREF(_name);
	u->metadata.varnames = list2dict(u->ste->varNames);
	if (!u->metadata.varnames) {
		compiler_unitFree(u);
		return ERROR;
	}
	u->metadata.cellvars = dict_byType(u->ste->symbols, CELL, DEF_COMP_CELL, 0);
	if (!u->metadata.cellvars) {
		compiler_unitFree(u);
		return ERROR;
	}
	if (u->ste->needsClassClosure) {
		AlifSizeT res{};
		res = dict_addO(u->metadata.cellvars, &ALIF_ID(__class__));
		if (res < 0) {
			compiler_unitFree(u);
			return ERROR;
		}
	}
	if (u->ste->needsClassDict) {
		AlifSizeT res{};
		res = dict_addO(u->metadata.cellvars, &ALIF_ID(__classDict__));
		if (res < 0) {
			compiler_unitFree(u);
			return ERROR;
		}
	}

	u->metadata.freevars = dict_byType(u->ste->symbols, FREE, DEF_FREE_CLASS,
		ALIFDICT_GET_SIZE(u->metadata.cellvars));
	if (!u->metadata.freevars) {
		compiler_unitFree(u);
		return ERROR;
	}

	u->metadata.fasthidden = alifDict_new();
	if (!u->metadata.fasthidden) {
		compiler_unitFree(u);
		return ERROR;
	}

	u->nfBlocks = 0;
	u->inInlinedComp = 0;
	u->metadata.firstLineno = _lineno;
	u->metadata.consts = alifDict_new();
	if (!u->metadata.consts) {
		compiler_unitFree(u);
		return ERROR;
	}
	u->metadata.names = alifDict_new();
	if (!u->metadata.names) {
		compiler_unitFree(u);
		return ERROR;
	}

	u->deferredAnnotations = nullptr;
	if (_scopeType == ScopeType_::Compiler_Scope_Class) {
		u->staticAttributes = alifSet_new(0);
		if (!u->staticAttributes) {
			compiler_unitFree(u);
			return ERROR;
		}
	}
	else {
		u->staticAttributes = nullptr;
	}

	u->instrSequence = (InstrSequence*)_alifInstructionSequence_new();
	if (!u->instrSequence) {
		compiler_unitFree(u);
		return ERROR;
	}

	/* Push the old compiler_unit on the stack. */
	if (_c->u_) {
		AlifObject* capsule = alifCapsule_new(_c->u_, CAPSULE_NAME, nullptr);
		if (!capsule or alifList_append(_c->stack, capsule) < 0) {
			ALIF_XDECREF(capsule);
			compiler_unitFree(u);
			return ERROR;
		}
		ALIF_DECREF(capsule);
		if (_private == nullptr) {
			_private = _c->u_->private_;
		}
	}

	u->private_ = ALIF_XNEWREF(_private);

	_c->u_ = u;

	if (u->scopeType == ScopeType_::Compiler_Scope_Module) {
		loc.lineNo = 0;
	}
	else {
		RETURN_IF_ERROR(compiler_setQualname(_c));
	}
	ADDOP_I(_c, loc, RESUME, RESUME_AT_FUNC_START);

	return SUCCESS;
}




static void compiler_exitScope(AlifCompiler* _c) { // 1197
	//AlifObject* exc = alifErr_getRaisedException();

	InstrSequence* nestedSeq = nullptr;
	if (_c->saveNestedSeqs) {
		nestedSeq = _c->u_->instrSequence;
		ALIF_INCREF(nestedSeq);
	}
	compiler_unitFree(_c->u_);
	AlifSizeT n = ALIFLIST_GET_SIZE(_c->stack) - 1;
	if (n >= 0) {
		AlifObject* capsule = ALIFLIST_GET_ITEM(_c->stack, n);
		_c->u_ = (CompilerUnit*)alifCapsule_getPointer(capsule, CAPSULE_NAME);
		if (alifSequence_delItem(_c->stack, n) < 0) {
			//alifErr_formatUnraisable("Exception ignored on removing "
			//	"the last compiler stack item");
		}
		if (nestedSeq != nullptr) {
			if (_alifInstructionSequence_addNested(_c->u_->instrSequence, nestedSeq) < 0) {
				//alifErr_formatUnraisable("Exception ignored on appending "
				//	"nested instruction sequence");
			}
		}
	}
	else {
		_c->u_ = nullptr;
	}
	ALIF_XDECREF(nestedSeq);

	//alifErr_setRaisedException(exc);
}








static AlifIntT compiler_body(AlifCompiler* _c,
	Location _loc, ASDLStmtSeq* _stmts) { // 1507
	//if ((FUTURE_FEATURES(_c) & CO_FUTURE_ANNOTATIONS) and SYMTABLE_ENTRY(_c)->annotationsUsed) {
	//	ADDOP(_c, _loc, SETUP_ANNOTATIONS);
	//}
	if (!ASDL_SEQ_LEN(_stmts)) {
		return SUCCESS;
	}
	AlifSizeT firstInstr = 0;
	//if (!IS_INTERACTIVE(_c)) {
	//	AlifObject* docString = alifAST_getDocString(_stmts);
	//	if (docString) {
	//		firstInstr = 1;
	//		/* if not -OO mode, set docstring */
	//		if (OPTIMIZATION_LEVEL(_c) < 2) {
	//			AlifObject* cleanDoc = alifCompile_cleanDoc(docString);
	//			if (cleanDoc == nullptr) {
	//				return ERROR;
	//			}
	//			StmtTy st = (StmtTy)ASDL_SEQ_GET(_stmts, 0);
	//			Location loc = LOC(st->V.expression.val);
	//			ADDOP_LOAD_CONST(_c, loc, cleanDoc);
	//			ALIF_DECREF(cleanDoc);
	//			RETURN_IF_ERROR(compiler_nameOp(_c, NO_LOCATION, &ALIF_ID(__doc__), ExprContext_::Store));
	//		}
	//	}
	//}
	for (AlifSizeT i = firstInstr; i < ASDL_SEQ_LEN(_stmts); i++) {
		VISIT(_c, Stmt, (StmtTy)ASDL_SEQ_GET(_stmts, i));
	}

	//if (!(FUTURE_FEATURES(_c) & CO_FUTURE_ANNOTATIONS) and
	//	_c->u_->deferredAnnotations != nullptr) {

	//	AlifSTEntryObject* ste = SYMTABLE_ENTRY(_c);
	//	AlifObject* deferredAnno = ALIF_NEWREF(_c->u_->deferredAnnotations);
	//	void* key = (void*)((uintptr_t)ste->id + 1);
	//	if (codegen_setupAnnotationsScope(_c, _loc, key,
	//		ste->annotationBlock->name) == -1) {
	//		ALIF_DECREF(deferredAnno);
	//		return ERROR;
	//	}
	//	AlifSizeT annotations_len = alifList_size(deferredAnno);
	//	for (AlifSizeT i = 0; i < annotations_len; i++) {
	//		AlifObject* ptr = ALIFLIST_GET_ITEM(deferredAnno, i);
	//		StmtTy st = (StmtTy)alifLong_asVoidPtr(ptr);
	//		if (st == NULL) {
	//			compiler_exitScope(_c);
	//			ALIF_DECREF(deferredAnno);
	//			return ERROR;
	//		}
	//		AlifObject* mangled = compiler_mangle(_c, st->V.annAssign.Target->V.name.id);
	//		ADDOP_LOAD_CONST_NEW(_c, LOC(st), mangled);
	//		VISIT(_c, expr, st->V.annAssign.annotation);
	//	}
	//	ALIF_DECREF(deferredAnno);

	//	RETURN_IF_ERROR(
	//		codegen_leaveAnnotationsScope(_c, _loc, annotations_len)
	//	);
	//	RETURN_IF_ERROR(
	//		compiler_nameOp(_c, _loc, &ALIF_ID(__annotate__), ExprContext_::Store)
	//	);
	//}
	return SUCCESS;
}



static Location start_location(ASDLStmtSeq* stmts) { // 1587
	if (ASDL_SEQ_LEN(stmts) > 0) {
		StmtTy st = (StmtTy)ASDL_SEQ_GET(stmts, 0);
		return LOC(st);
	}
	return LOCATION(1, 1, 0, 0);
}



static AlifIntT compiler_codegen(AlifCompiler* _c, ModuleTy _mod) { // 1602
	switch (_mod->type) {
	case ModK_::ModuleK: {
		ASDLStmtSeq* stmts = _mod->V.module.body;
		RETURN_IF_ERROR(compiler_body(_c, start_location(stmts), stmts));
		break;
	}
	case ModK_::InteractiveK: {
		_c->interactive = 1;
		ASDLStmtSeq* stmts = _mod->V.interactive.body;
		RETURN_IF_ERROR(compiler_body(_c, start_location(stmts), stmts));
		break;
	}
	case ModK_::ExpressionK: {
		VISIT(_c, Expr, _mod->V.expression.body);
		break;
	}
	default: {
		//alifErr_format(_alifExcSystemError_,
		//	"module kind %d should not be possible",
		//	_mod->type);
		return ERROR;
	}
	}
	return SUCCESS;
}


static AlifIntT codegenEnter_anonymousScope(AlifCompiler* _c, ModuleTy _mod) { // 1631
	RETURN_IF_ERROR(compiler_enterScope(_c, &ALIF_STR(AnonModule),
		ScopeType_::Compiler_Scope_Module, _mod, 1, nullptr, nullptr));

	return SUCCESS;
}


static AlifCodeObject* compiler_mod(AlifCompiler* _c, ModuleTy _mod) { // 1641
	AlifCodeObject* co = nullptr;
	AlifIntT addNone = _mod->type != ModK_::ExpressionK;
	if (codegenEnter_anonymousScope(_c, _mod) < 0) {
		return nullptr;
	}
	if (compiler_codegen(_c, _mod) < 0) {
		goto finally;
	}
	co = optimize_andAssemble(_c, addNone);
finally:
	compiler_exitScope(_c);
	return co;
}







static AlifIntT codegen_ifExpr(AlifCompiler* _c, ExprTy _e) { // 2812
	NEW_JUMP_TARGET_LABEL(_c, end);
	NEW_JUMP_TARGET_LABEL(_c, next);

	RETURN_IF_ERROR(
		codegen_jumpIf(_c, LOC(_e), _e->V.ifExpr.condition, next, 0));

	VISIT(_c, Expr, _e->V.ifExpr.body);
	ADDOP_JUMP(_c, _noLocation_, JUMP_NO_INTERRUPT, end);

	USE_LABEL(_c, next);
	VISIT(_c, Expr, _e->V.ifExpr.else_);

	USE_LABEL(_c, end);
	return SUCCESS;
}










static AlifIntT codegen_stmtExpr(AlifCompiler* _c, Location _loc, ExprTy _value) { // 3797
	//if (IS_INTERACTIVE(_c) and !IS_NESTED_SCOPE(_c)) {
	//	VISIT(_c, Expr, _value);
	//	ADDOP_I(_c, _loc, CALL_INTRINSIC_1, INTRINSIC_PRINT);
	//	ADDOP(_c, NO_LOCATION, POP_TOP);
	//	return SUCCESS;
	//}

	if (_value->type == ExprK_::ConstantK) {
		/* ignore constant statement */
		ADDOP(_c, _loc, NOP);
		return SUCCESS;
	}

	VISIT(_c, Expr, _value);
	ADDOP(_c, _noLocation_, POP_TOP); /* artificial */
	return SUCCESS;
}


static AlifIntT compiler_visitStmt(AlifCompiler* _c, StmtTy _s) { // 3818
	switch (_s->type) {
	//case StmtK_::FunctionDefK:
	//	return codegen_function(_c, _s, 0);
	//case StmtK_::ClassDefK:
	//	return codegen_class(_c, _s);
	//case StmtK_::TypeAliasK:
	//	return codegen_typealias(_c, _s);
	//case StmtK_::ReturnK:
	//	return codegen_return(_c, _s);
	//case StmtK_::DeleteK:
	//	VISIT_SEQ(_c, expr, _s->v.Delete.targets)
	//		break;
	//case StmtK_::AssignK:
	//{
	//	Py_ssize_t n = asdl_seq_LEN(_s->v.Assign.targets);
	//	VISIT(_c, expr, _s->v.Assign.value);
	//	for (Py_ssize_t i = 0; i < n; i++) {
	//		if (i < n - 1) {
	//			ADDOP_I(_c, LOC(_s), COPY, 1);
	//		}
	//		VISIT(_c, expr,
	//			(expr_ty)asdl_seq_GET(_s->v.Assign.targets, i));
	//	}
	//	break;
	//}
	//case StmtK_::AugAssignK:
	//	return codegen_augassign(_c, _s);
	//case StmtK_::AnnAssignK:
	//	return compiler_annassign(_c, _s);
	//case StmtK_::ForK:
	//	return codegen_for(_c, _s);
	//case StmtK_::WhileK:
	//	return codegen_while(_c, _s);
	//case StmtK_::IfK:
	//	return codegen_if(_c, _s);
	//case StmtK_::MatchK:
	//	return codegen_match(_c, _s);
	//case StmtK_::RaiseK:
	//{
	//	Py_ssize_t n = 0;
	//	if (_s->v.Raise.exc) {
	//		VISIT(_c, expr, _s->v.Raise.exc);
	//		n++;
	//		if (_s->v.Raise.cause) {
	//			VISIT(_c, expr, _s->v.Raise.cause);
	//			n++;
	//		}
	//	}
	//	ADDOP_I(_c, LOC(_s), RAISE_VARARGS, (int)n);
	//	break;
	//}
	//case StmtK_::TryK:
	//	return codegen_try(_c, _s);
	//case StmtK_::TryStarK:
	//	return codegen_try_star(_c, _s);
	//case StmtK_::AssertK:
	//	return codegen_assert(_c, _s);
	//case StmtK_::ImportK:
	//	return codegen_import(_c, _s);
	//case StmtK_::ImportFromK:
	//	return codegen_from_import(_c, _s);
	//case StmtK_::GlobalK:
	//case StmtK_::NonlocalK:
	//	break;
	case StmtK_::ExprK:
	{
		return codegen_stmtExpr(_c, LOC(_s), _s->V.expression.val);
	}
	//case StmtK_::PassK:
	//{
	//	ADDOP(_c, LOC(_s), NOP);
	//	break;
	//}
	//case StmtK_::BreakK:
	//{
	//	return codegen_break(_c, LOC(_s));
	//}
	//case StmtK_::ContinueK:
	//{
	//	return codegen_continue(_c, LOC(_s));
	//}
	//case StmtK_::WithK:
	//	return codegen_with(_c, _s, 0);
	//case StmtK_::AsyncFunctionDefK:
	//	return codegen_function(_c, _s, 1);
	//case StmtK_::AsyncWithK:
	//	return codegen_async_with(_c, _s, 0);
	//case StmtK_::AsyncForK:
	//	return codegen_async_for(_c, _s);
	}

	return SUCCESS;
}


static AlifIntT unaryop(UnaryOp_ _op) { // 3916
	switch (_op) {
	case UnaryOp_::Invert:
		return UNARY_INVERT;
	case UnaryOp_::USub:
		return UNARY_NEGATIVE;
	default:
		//alifErr_format(_alifExcSystemError_,
		//	"unary op %d should not be possible", _op);
		return 0;
	}
}

static AlifIntT addop_binary(AlifCompiler* _c, Location _loc,
	Operator_ _binop, bool _inplace) { // 3931
	AlifIntT oparg{};
	switch (_binop) {
	case Operator_::Add:
		oparg = _inplace ? NB_INPLACE_ADD : NB_ADD;
		break;
	case Operator_::Sub:
		oparg = _inplace ? NB_INPLACE_SUBTRACT : NB_SUBTRACT;
		break;
	case Operator_::Mult:
		oparg = _inplace ? NB_INPLACE_MULTIPLY : NB_MULTIPLY;
		break;
	//case MatMult:
	//	oparg = _inplace ? NB_INPLACE_MATRIX_MULTIPLY : NB_MATRIX_MULTIPLY;
	//	break;
	case Operator_::Div:
		oparg = _inplace ? NB_INPLACE_TRUE_DIVIDE : NB_TRUE_DIVIDE;
		break;
	case Operator_::Mod:
		oparg = _inplace ? NB_INPLACE_REMAINDER : NB_REMAINDER;
		break;
	case Operator_::Pow:
		oparg = _inplace ? NB_INPLACE_POWER : NB_POWER;
		break;
	case Operator_::LShift:
		oparg = _inplace ? NB_INPLACE_LSHIFT : NB_LSHIFT;
		break;
	case Operator_::RShift:
		oparg = _inplace ? NB_INPLACE_RSHIFT : NB_RSHIFT;
		break;
	case Operator_::BitOr:
		oparg = _inplace ? NB_INPLACE_OR : NB_OR;
		break;
	case Operator_::BitXor:
		oparg = _inplace ? NB_INPLACE_XOR : NB_XOR;
		break;
	case Operator_::BitAnd:
		oparg = _inplace ? NB_INPLACE_AND : NB_AND;
		break;
	case Operator_::FloorDiv:
		oparg = _inplace ? NB_INPLACE_FLOOR_DIVIDE : NB_FLOOR_DIVIDE;
		break;
	default:
		//alifErr_format(_alifExcSystemError_, "%s op %d should not be possible",
		//	_inplace ? "inplace" : "binary", _binop);
		return ERROR;
	}
	ADDOP_I(_c, _loc, BINARY_OP, oparg);
	return SUCCESS;
}








static AlifIntT codegen_boolOp(AlifCompiler* _c, ExprTy _e) { // 4151
	AlifIntT jumpi{};
	AlifSizeT i{}, n{};
	ASDLExprSeq* s{};

	Location loc = LOC(_e);
	if (_e->V.boolOp.op == BoolOp_::And)
		jumpi = POP_JUMP_IF_FALSE;
	else
		jumpi = POP_JUMP_IF_TRUE;
	NEW_JUMP_TARGET_LABEL(_c, _end);
	s = _e->V.boolOp.vals;
	n = ASDL_SEQ_LEN(s) - 1;
	for (i = 0; i < n; ++i) {
		VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(s, i));
		ADDOP_I(_c, loc, COPY, 1);
		ADDOP(_c, loc, TO_BOOL);
		ADDOP_JUMP(_c, loc, jumpi, end);
		ADDOP(_c, loc, POP_TOP);
	}
	VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(s, n));

	USE_LABEL(_c, end);
	return SUCCESS;
}





static AlifIntT codegen_subDict(AlifCompiler* _c,
	ExprTy _e, AlifSizeT _begin, AlifSizeT _end) { // 4362
	AlifSizeT i, n = _end - _begin;
	AlifIntT big = n * 2 > STACK_USE_GUIDELINE;
	Location loc = LOC(_e);
	if (big) {
		ADDOP_I(_c, loc, BUILD_MAP, 0);
	}
	for (i = _begin; i < _end; i++) {
		VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(_e->V.dict.keys, i));
		VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(_e->V.dict.vals, i));
		if (big) {
			ADDOP_I(_c, loc, MAP_ADD, 1);
		}
	}
	if (!big) {
		ADDOP_I(_c, loc, BUILD_MAP, n);
	}
	return SUCCESS;
}

static AlifIntT codegen_dict(AlifCompiler* _c, ExprTy _e) { // 4384
	Location loc = LOC(_e);
	AlifSizeT i{}, n{}, elements{};
	AlifIntT haveDict{};
	AlifIntT isUnpacking = 0;
	n = ASDL_SEQ_LEN(_e->V.dict.vals);
	haveDict = 0;
	elements = 0;
	for (i = 0; i < n; i++) {
		isUnpacking = (ExprTy)ASDL_SEQ_GET(_e->V.dict.keys, i) == nullptr;
		if (isUnpacking) {
			if (elements) {
				RETURN_IF_ERROR(codegen_subDict(_c, _e, i - elements, i));
				if (haveDict) {
					ADDOP_I(_c, loc, DICT_UPDATE, 1);
				}
				haveDict = 1;
				elements = 0;
			}
			if (haveDict == 0) {
				ADDOP_I(_c, loc, BUILD_MAP, 0);
				haveDict = 1;
			}
			VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(_e->V.dict.vals, i));
			ADDOP_I(_c, loc, DICT_UPDATE, 1);
		}
		else {
			if (elements * 2 > STACK_USE_GUIDELINE) {
				RETURN_IF_ERROR(codegen_subDict(_c, _e, i - elements, i + 1));
				if (haveDict) {
					ADDOP_I(_c, loc, DICT_UPDATE, 1);
				}
				haveDict = 1;
				elements = 0;
			}
			else {
				elements++;
			}
		}
	}
	if (elements) {
		RETURN_IF_ERROR(codegen_subDict(_c, _e, n - elements, n));
		if (haveDict) {
			ADDOP_I(_c, loc, DICT_UPDATE, 1);
		}
		haveDict = 1;
	}
	if (!haveDict) {
		ADDOP_I(_c, loc, BUILD_MAP, 0);
	}
	return SUCCESS;
}








static AlifIntT codegen_joinedStr(AlifCompiler* _c, ExprTy _e) { // 4847
	Location loc = LOC(_e);
	AlifSizeT valueCount = ASDL_SEQ_LEN(_e->V.joinStr.vals);
	if (valueCount > STACK_USE_GUIDELINE) {
		ADDOP_LOAD_CONST_NEW(_c, loc, ALIF_NEWREF(&ALIF_STR(Empty)));
		ADDOP_NAME(_c, loc, LOAD_METHOD, &ALIF_ID(Join), names);
		ADDOP_I(_c, loc, BUILD_LIST, 0);
		for (AlifSizeT i = 0; i < ASDL_SEQ_LEN(_e->V.joinStr.vals); i++) {
			VISIT(_c, Expr, ASDL_SEQ_GET(_e->V.joinStr.vals, i));
			ADDOP_I(_c, loc, LIST_APPEND, 1);
		}
		ADDOP_I(_c, loc, CALL, 1);
	}
	else {
		VISIT_SEQ(_c, Expr, _e->V.joinStr.vals);
		if (valueCount > 1) {
			ADDOP_I(_c, loc, BUILD_STRING, valueCount);
		}
		else if (valueCount == 0) {
			ADDOP_LOAD_CONST_NEW(_c, loc, ALIF_NEWREF(&ALIF_STR(Empty)));
		}
	}
	return SUCCESS;
}

static AlifIntT codegen_formattedValue(AlifCompiler* _c, ExprTy _e) { // 4877
	AlifIntT conversion = _e->V.formattedValue.conversion;
	AlifIntT oparg;

	/* The expression to be formatted. */
	VISIT(_c, Expr, _e->V.formattedValue.val);

	Location loc = LOC(_e);
	if (conversion != -1) {
		switch (conversion) {
		case 's': oparg = FVC_STR;   break;
		case 'r': oparg = FVC_REPR;  break;
		case 'a': oparg = FVC_ASCII; break;
		default:
			//alifErr_format(_alifExcSystemError_,
			//	"Unrecognized conversion character %d", conversion);
			return ERROR;
		}
		ADDOP_I(_c, loc, CONVERT_VALUE, oparg);
	}
	if (_e->V.formattedValue.formatSpec) {
		/* Evaluate the format spec, and update our opcode arg. */
		VISIT(_c, Expr, _e->V.formattedValue.formatSpec);
		ADDOP(_c, loc, FORMAT_WITH_SPEC);
	}
	else {
		ADDOP(_c, loc, FORMAT_SIMPLE);
	}
	return SUCCESS;
}






static AlifIntT compiler_visitExpr(AlifCompiler* _c, ExprTy _e) { // 5997
	Location loc = LOC(_e);
	switch (_e->type) {
	case ExprK_::NamedExprK:
		VISIT(_c, Expr, _e->V.namedExpr.val);
		ADDOP_I(_c, loc, COPY, 1);
		VISIT(_c, Expr, _e->V.namedExpr.target);
		break;
	case ExprK_::BoolOpK:
		return codegen_boolOp(_c, _e);
	case ExprK_::BinOpK:
		VISIT(_c, Expr, _e->V.binOp.left);
		VISIT(_c, Expr, _e->V.binOp.right);
		ADDOP_BINARY(_c, loc, _e->V.binOp.op);
		break;
	case ExprK_::UnaryOpK:
		VISIT(_c, Expr, _e->V.unaryOp.operand);
		if (_e->V.unaryOp.op == UnaryOp_::UAdd) {
			ADDOP_I(_c, loc, CALL_INTRINSIC_1, INTRINSIC_UNARY_POSITIVE);
		}
		else if (_e->V.unaryOp.op == UnaryOp_::Not) {
			ADDOP(_c, loc, TO_BOOL);
			ADDOP(_c, loc, UNARY_NOT);
		}
		else {
			ADDOP(_c, loc, unaryop(_e->V.unaryOp.op));
		}
		break;
	//case ExprK_::LambdaK:
	//	return codegen_lambda(_c, _e);
	case ExprK_::IfExprK:
		return codegen_ifExpr(_c, _e);
	case ExprK_::DictK:
		return codegen_dict(_c, _e);
	//case ExprK_::SetK:
	//	return codegen_set(_c, _e);
	//case ExprK_::GeneratorExpK:
	//	return codegen_genexp(_c, _e);
	//case ExprK_::ListCompK:
	//	return codegen_listComp(_c, _e);
	//case ExprK_::SetCompK:
	//	return codegen_setComp(_c, _e);
	//case ExprK_::DictCompK:
	//	return codegen_dictComp(_c, _e);
	//case ExprK_::YieldK:
	//	if (!alifST_isFunctionLike(SYMTABLE_ENTRY(_c))) {
	//		return compiler_error(_c, loc, "'yield' outside function");
	//	}
	//	if (_e->V.yield.val) {
	//		VISIT(_c, Expr, _e->V.yield.val);
	//	}
	//	else {
	//		ADDOP_LOAD_CONST(_c, loc, ALIF_NONE);
	//	}
	//	ADDOP_YIELD(_c, loc);
	//	break;
	//case ExprK_::YieldFromK:
	//	if (!alifST_isFunctionLike(SYMTABLE_ENTRY(_c))) {
	//		return compiler_error(_c, loc, "'yield from' outside function");
	//	}
	//	if (_c->u_->scopeType == COMPILER_SCOPE_ASYNC_FUNCTION) {
	//		return compiler_error(_c, loc, "'yield from' inside async function");
	//	}
	//	VISIT(_c, Expr, _e->V.yieldFrom.val);
	//	ADDOP(_c, loc, GET_YIELD_FROM_ITER);
	//	ADDOP_LOAD_CONST(_c, loc, ALIF_NONE);
	//	ADD_YIELD_FROM(_c, loc, 0);
	//	break;
	//case ExprK_::AwaitK:
	//	VISIT(_c, Expr, _e->V.await.val);
	//	ADDOP_I(_c, loc, GET_AWAITABLE, 0);
	//	ADDOP_LOAD_CONST(_c, loc, ALIF_NONE);
	//	ADD_YIELD_FROM(_c, loc, 1);
	//	break;
	//case ExprK_::CompareK:
	//	return codegen_compare(_c, _e);
	//case ExprK_::CallK:
	//	return codegen_call(_c, _e);
	case ExprK_::ConstantK:
		ADDOP_LOAD_CONST(_c, loc, _e->V.constant.val);
		break;
	case ExprK_::JoinStrK:
		return codegen_joinedStr(_c, _e);
	case ExprK_::FormattedValK:
		return codegen_formattedValue(_c, _e);
		/* The following exprs can be assignment targets. */
	//case ExprK_::AttributeK:
	//	if (_e->V.attribute.ctx == ExprContext_::Load) {
	//		AlifIntT ret = canOptimize_superCall(_c, _e);
	//		RETURN_IF_ERROR(ret);
	//		if (ret) {
	//			RETURN_IF_ERROR(loadArgs_forSuper(_c, _e->V.attribute.val));
	//			AlifIntT opcode = ASDL_SEQ_LEN(_e->V.attribute.val->V.call.args) ?
	//				LOAD_SUPER_ATTR : LOAD_ZERO_SUPER_ATTR;
	//			ADDOP_NAME(_c, loc, opcode, _e->V.attribute.attr, names);
	//			loc = updateStart_locationToMatchAttr(_c, loc, _e);
	//			ADDOP(_c, loc, NOP);
	//			return SUCCESS;
	//		}
	//	}
	//	RETURN_IF_ERROR(compiler_maybeAddStaticAttributeToClass(_c, _e));
	//	VISIT(_c, Expr, _e->V.attribute.val);
	//	loc = LOC(_e);
	//	loc = updateStart_locationToMatchAttr(_c, loc, _e);
	//	switch (_e->v.Attribute.ctx) {
	//	case ExprContext_::Load:
	//		ADDOP_NAME(_c, loc, LOAD_ATTR, _e->V.attribute.attr, names);
	//		break;
	//	case ExprContext_::Store:
	//		ADDOP_NAME(_c, loc, STORE_ATTR, _e->V.attribute.attr, names);
	//		break;
	//	case ExprContext_::Del:
	//		ADDOP_NAME(_c, loc, DELETE_ATTR, _e->V.attribute.attr, names);
	//		break;
	//	}
	//	break;
	//case ExprK_::SubScriptK:
	//	return codegen_subscript(_c, _e);
	//case ExprK_::StarK:
	//	switch (_e->V.star.ctx) {
	//	case Store:
	//		/* In all legitimate cases, the Starred node was already replaced
	//		 * by codegen_list/codegen_tuple. XXX: is that okay? */
	//		return compiler_error(_c, loc,
	//			"starred assignment target must be in a list or tuple");
	//	default:
	//		return compiler_error(_c, loc,
	//			"can't use starred expression here");
	//	}
	//	break;
	//case ExprK_::SliceK:
	//{
	//	AlifIntT n = codegen_slice(_c, _e);
	//	RETURN_IF_ERROR(n);
	//	ADDOP_I(_c, loc, BUILD_SLICE, n);
	//	break;
	//}
	case ExprK_::NameK:
		return compiler_nameOp(_c, loc, _e->V.name.name, _e->V.name.ctx);
		/* child nodes of List and Tuple will have expr_context set */
	case ExprK_::ListK:
		return codegen_list(_c, _e);
	case ExprK_::TupleK:
		return codegen_tuple(_c, _e);
	}
	return SUCCESS;
}






static AlifObject* compiler_maybeMangle(AlifCompiler* _c, AlifObject* _name) { // 7352
	return alif_maybeMangle(_c->u_->private_, _c->u_->ste, _name);
}

static InstrSequence* compiler_instrSequence(AlifCompiler* _c) { // 7358
	return _c->u_->instrSequence;
}
