#include "alif.h"

#include "Opcode.h"
#include "AlifCore_AST.h"
#define NEED_OPCODE_TABLES
#include "AlifCore_OpcodeUtils.h"
#undef NEED_OPCODE_TABLES
#include "AlifCore_Code.h"
#include "AlifCore_Compile.h"
#include "AlifCore_FlowGraph.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_Intrinsics.h"
#include "AlifCore_State.h"
#include "AlifCore_SetObject.h"
#include "AlifCore_SymTable.h"



#define NEED_OPCODE_METADATA
#include "AlifCore_OpcodeMetaData.h"
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
static AlifSymTable* compiler_symtable(AlifCompiler* c); // 88
static AlifSTEntryObject* compiler_symtableEntry(AlifCompiler*); // 89

#define INSTR_SEQUENCE(_c) compiler_instrSequence(_c) // 91
#define SYMTABLE(_c) compiler_symtable(_c) // 93
#define SYMTABLE_ENTRY(_c) compiler_symtableEntry(_c) // 94

typedef AlifSourceLocation Location; // 99
typedef class AlifCFGBuilder CFGBuilder; // 100

static AlifObject* compiler_maybeMangle(AlifCompiler*, AlifObject*); // 103


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


static const AlifIntT _compareMasks_[] = { // 163
	COMPARISON_LESS_THAN,
	COMPARISON_LESS_THAN | COMPARISON_EQUALS,
	COMPARISON_EQUALS,
	COMPARISON_NOT_EQUALS,
	COMPARISON_GREATER_THAN,
	COMPARISON_GREATER_THAN | COMPARISON_EQUALS,
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
static AlifIntT compiler_visitKeyword(AlifCompiler*, KeywordTy); // 314
static AlifIntT compiler_visitExpr(AlifCompiler*, ExprTy); // 315


static AlifIntT codegen_addCompare(AlifCompiler*, Location, CmpOp_); // alif
static bool areAllItems_const(ASDLExprSeq*, AlifSizeT, AlifSizeT); // 321


static AlifIntT codegen_callSimpleKwHelper(AlifCompiler*, Location, ASDLKeywordSeq*, AlifSizeT); // 327
static AlifIntT codegen_callHelper(AlifCompiler*, Location, AlifIntT, ASDLExprSeq*, ASDLKeywordSeq*); // 331

static AlifCodeObject* optimize_andAssemble(AlifCompiler*, AlifIntT); // 358

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


static AlifObject* const_cacheInsert(AlifObject* _constCache,
	AlifObject* _o, bool _recursive) { // 765
	if (_o == ALIF_NONE or _o == ALIF_ELLIPSIS) {
		return _o;
	}

	AlifObject* key_ = alifCode_constantKey(_o);
	if (key_ == nullptr) {
		return nullptr;
	}

	AlifObject* t_{};
	AlifIntT res_ = alifDict_setDefaultRef(_constCache, key_, key_, &t_);
	if (res_ != 0) {
		ALIF_DECREF(key_);
		return t_;
	}
	ALIF_DECREF(t_);

	if (!_recursive) {
		return key_;
	}

	if (ALIFTUPLE_CHECKEXACT(_o)) {
		AlifSizeT len_ = ALIFTUPLE_GET_SIZE(_o);
		for (AlifSizeT i = 0; i < len_; i++) {
			AlifObject* item = ALIFTUPLE_GET_ITEM(_o, i);
			AlifObject* u_ = const_cacheInsert(_constCache, item, _recursive);
			if (u_ == nullptr) {
				ALIF_DECREF(key_);
				return nullptr;
			}

			AlifObject* v_{};
			if (ALIFTUPLE_CHECKEXACT(u_)) {
				v_ = ALIFTUPLE_GET_ITEM(u_, 1);
			}
			else {
				v_ = u_;
			}
			if (v_ != item) {
				ALIFTUPLE_SET_ITEM(_o, i, ALIF_NEWREF(v_));
				ALIF_DECREF(item);
			}

			ALIF_DECREF(u_);
		}
	}
	else if (ALIFFROZENSET_CHECKEXACT(_o)) {

		AlifSizeT len_ = ALIFSET_GET_SIZE(_o);
		if (len_ == 0) {  
			return key_;
		}
		AlifObject* tuple = alifTuple_new(len_);
		if (tuple == nullptr) {
			ALIF_DECREF(key_);
			return nullptr;
		}
		AlifSizeT i_ = 0, pos_ = 0;
		AlifObject* item{};
		AlifHashT hash{};
		while (alifSet_nextEntry(_o, &pos_, &item, &hash)) {
			AlifObject* k_ = const_cacheInsert(_constCache, item, _recursive);
			if (k_ == nullptr) {
				ALIF_DECREF(tuple);
				ALIF_DECREF(key_);
				return nullptr;
			}
			AlifObject* u_{};
			if (ALIFTUPLE_CHECKEXACT(k_)) {
				u_ = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(k_, 1));
				ALIF_DECREF(k_);
			}
			else {
				u_ = k_;
			}
			ALIFTUPLE_SET_ITEM(tuple, i_, u_);  // Steals reference of u.
			i_++;
		}
		AlifObject* new_ = alifFrozenSet_new(tuple);
		ALIF_DECREF(tuple);
		if (new_ == nullptr) {
			ALIF_DECREF(key_);
			return nullptr;
		}
		ALIF_DECREF(_o);
		ALIFTUPLE_SET_ITEM(key_, 1, new_);
	}

	return key_;
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


static AlifIntT codegen_addOpO(AlifCompiler* _c, Location _loc,
	AlifIntT _opcode, AlifObject* _dict, AlifObject* _o) { // 925
	AlifSizeT arg = dict_addO(_dict, _o);
	RETURN_IF_ERROR(arg);
	ADDOP_I(_c, _loc, _opcode, arg);
	return SUCCESS;
}

 // 935
#define ADDOP_N(_c, _loc, _op, _o, _type) { \
    AlifIntT ret = codegen_addOpO(_c, _loc, _op, _c->u_->metadata._type, _o); \
    ALIF_DECREF(_o); \
    RETURN_IF_ERROR(ret); \
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
    RETURN_IF_ERROR(codegen_addOpName(_c, _loc, _op, _c->u_->metadata._type, _o)) // 997




static AlifIntT codegen_addOpJ(InstrSequence* _seq, Location _loc,
	AlifIntT _opcode, JumpTargetLabel _target) { // 1000
	return _alifInstructionSequence_addOp(_seq, _opcode, _target.id, _loc);
}


#define ADDOP_JUMP(_c, _loc, _op, _o) \
    RETURN_IF_ERROR(codegen_addOpJ(INSTR_SEQUENCE(_c), _loc, _op, _o)) // 1010

#define ADDOP_COMPARE(_c, _loc, _cmp) \
    RETURN_IF_ERROR(codegen_addCompare(_c, _loc, (CmpOp_)_cmp)) // 1013

#define ADDOP_BINARY(_c, _loc, _binOp) \
    RETURN_IF_ERROR(addop_binary(_c, _loc, _binOp, false)) // 1016


 // 1035
#define VISIT(_c, _type, _v) \
    RETURN_IF_ERROR(compiler_visit ## _type(_c, _v));


 // 1041
#define VISIT_SEQ(_c, _type, _sequ) { \
    AlifIntT i_{}; \
    ASDL ## _type ## Seq *_seq = (_sequ); /* avoid variable capture */ \
    for (i_ = 0; i_ < ASDL_SEQ_LEN(_seq); i_++) { \
        _type ## Ty elt = (_type ## Ty)ASDL_SEQ_GET(_seq, i_); \
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



static bool check_isArg(ExprTy _e) { // 2626
	if (_e->type != ExprK_::ConstantK) {
		return true;
	}
	AlifObject* value = _e->V.constant.val;
	return (value == ALIF_NONE
		or value == ALIF_FALSE
		or value == ALIF_TRUE
		or value == ALIF_ELLIPSIS);
}

static AlifIntT codegen_checkCompare(AlifCompiler* _c, ExprTy _e) { // 2644
	AlifSizeT i{}, n{};
	bool left = check_isArg(_e->V.compare.left);
	ExprTy leftExpr = _e->V.compare.left;
	n = ASDL_SEQ_LEN(_e->V.compare.ops);
	for (i = 0; i < n; i++) {
		CmpOp_ op = (CmpOp_)ASDL_SEQ_GET(_e->V.compare.ops, i);
		ExprTy rightExpr = (ExprTy)ASDL_SEQ_GET(_e->V.compare.comparators, i);
		bool right = check_isArg(rightExpr);
		if (op == CmpOp_::Is or op == CmpOp_::IsNot) {
			if (!right or !left) {
				const char* msg = (op == CmpOp_::Is)
					? "\"is\" with '%.200s' literal. Did you mean \"==\"?"
					: "\"is not\" with '%.200s' literal. Did you mean \"!=\"?";
				ExprTy literal = !left ? leftExpr : rightExpr;
				//return compiler_warn(
				//	_c, LOC(_e), msg, infer_type(literal)->tp_name
				//);
			}
		}
		left = right;
		leftExpr = rightExpr;
	}
	return SUCCESS;
}


static AlifIntT codegen_jumpIf(AlifCompiler* _c, Location _loc,
	ExprTy _e, JumpTargetLabel _next, AlifIntT _cond) { // 2718
	switch (_e->type) {
	case ExprK_::UnaryOpK:
		if (_e->V.unaryOp.op == UnaryOp_::Not) {
			return codegen_jumpIf(_c, _loc, _e->V.unaryOp.operand, _next, !_cond);
		}
		/* fallback to general implementation */
		break;
	case ExprK_::BoolOpK: {
		ASDLExprSeq* s = _e->V.boolOp.vals;
		AlifSizeT i, n = ASDL_SEQ_LEN(s) - 1;
		AlifIntT cond2 = _e->V.boolOp.op == BoolOp_::Or;
		JumpTargetLabel next2 = _next;
		if (!cond2 != !_cond) {
			NEW_JUMP_TARGET_LABEL(_c, new_next2);
			next2 = new_next2;
		}
		for (i = 0; i < n; ++i) {
			RETURN_IF_ERROR(
				codegen_jumpIf(_c, _loc, (ExprTy)ASDL_SEQ_GET(s, i), next2, cond2));
		}
		RETURN_IF_ERROR(
			codegen_jumpIf(_c, _loc, (ExprTy)ASDL_SEQ_GET(s, n), _next, _cond));
		if (!SAME_LABEL(next2, _next)) {
			USE_LABEL(_c, next2);
		}
		return SUCCESS;
	}
	case ExprK_::IfExprK: {
		NEW_JUMP_TARGET_LABEL(_c, end);
		NEW_JUMP_TARGET_LABEL(_c, next2);
		RETURN_IF_ERROR(
			codegen_jumpIf(_c, _loc, _e->V.ifExpr.condition, next2, 0));
		RETURN_IF_ERROR(
			codegen_jumpIf(_c, _loc, _e->V.ifExpr.body, _next, _cond));
		ADDOP_JUMP(_c, _noLocation_, JUMP_NO_INTERRUPT, end);

		USE_LABEL(_c, next2);
		RETURN_IF_ERROR(
			codegen_jumpIf(_c, _loc, _e->V.ifExpr.else_, _next, _cond));

		USE_LABEL(_c, end);
		return SUCCESS;
	}
	case ExprK_::CompareK: {
		AlifSizeT n = ASDL_SEQ_LEN(_e->V.compare.ops) - 1;
		if (n > 0) {
			RETURN_IF_ERROR(codegen_checkCompare(_c, _e));
			NEW_JUMP_TARGET_LABEL(_c, cleanup);
			VISIT(_c, Expr, _e->V.compare.left);
			for (AlifSizeT i = 0; i < n; i++) {
				VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(_e->V.compare.comparators, i));
				ADDOP_I(_c, LOC(_e), SWAP, 2);
				ADDOP_I(_c, LOC(_e), COPY, 2);
				ADDOP_COMPARE(_c, LOC(_e), ASDL_SEQ_GET(_e->V.compare.ops, i));
				ADDOP(_c, LOC(_e), TO_BOOL);
				ADDOP_JUMP(_c, LOC(_e), POP_JUMP_IF_FALSE, cleanup);
			}
			VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(_e->V.compare.comparators, n));
			ADDOP_COMPARE(_c, LOC(_e), ASDL_SEQ_GET(_e->V.compare.ops, n));
			ADDOP(_c, LOC(_e), TO_BOOL);
			ADDOP_JUMP(_c, LOC(_e), _cond ? POP_JUMP_IF_TRUE : POP_JUMP_IF_FALSE, _next);
			NEW_JUMP_TARGET_LABEL(_c, end);
			ADDOP_JUMP(_c, _noLocation_, JUMP_NO_INTERRUPT, end);

			USE_LABEL(_c, cleanup);
			ADDOP(_c, LOC(_e), POP_TOP);
			if (!_cond) {
				ADDOP_JUMP(_c, _noLocation_, JUMP_NO_INTERRUPT, _next);
			}

			USE_LABEL(_c, end);
			return SUCCESS;
		}
		/* fallback to general implementation */
		break;
	}
	default:
		/* fallback to general implementation */
		break;
	}

	/* general implementation */
	VISIT(_c, Expr, _e);
	ADDOP(_c, LOC(_e), TO_BOOL);
	ADDOP_JUMP(_c, LOC(_e), _cond ? POP_JUMP_IF_TRUE : POP_JUMP_IF_FALSE, _next);
	return SUCCESS;
}



static AlifIntT codegen_addCompare(AlifCompiler* _c, Location _loc, CmpOp_ _op) { // 2672
	AlifIntT cmp;
	switch (_op) {
	case CmpOp_::Equal :
		cmp = ALIF_EQ;
		break;
	case CmpOp_::NotEq:
		cmp = ALIF_NE;
		break;
	case CmpOp_::LessThan:
		cmp = ALIF_LT;
		break;
	case CmpOp_::LessThanEq:
		cmp = ALIF_LE;
		break;
	case CmpOp_::GreaterThan:
		cmp = ALIF_GT;
		break;
	case CmpOp_::GreaterThanEq:
		cmp = ALIF_GE;
		break;
	case CmpOp_::Is:
		ADDOP_I(_c, _loc, IS_OP, 0);
		return SUCCESS;
	case CmpOp_::IsNot:
		ADDOP_I(_c, _loc, IS_OP, 1);
		return SUCCESS;
	case CmpOp_::In:
		ADDOP_I(_c, _loc, CONTAINS_OP, 0);
		return SUCCESS;
	case CmpOp_::NotIn:
		ADDOP_I(_c, _loc, CONTAINS_OP, 1);
		return SUCCESS;
	default:
		ALIF_UNREACHABLE();
	}
	ADDOP_I(_c, _loc, COMPARE_OP, (cmp << 5) | _compareMasks_[cmp]);
	return SUCCESS;
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
	//	AlifSizeT n = asdl_seq_LEN(_s->v.Assign.targets);
	//	VISIT(_c, expr, _s->v.Assign.value);
	//	for (AlifSizeT i = 0; i < n; i++) {
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
	//	AlifSizeT n = 0;
	//	if (_s->v.Raise.exc) {
	//		VISIT(_c, expr, _s->v.Raise.exc);
	//		n++;
	//		if (_s->v.Raise.cause) {
	//			VISIT(_c, expr, _s->v.Raise.cause);
	//			n++;
	//		}
	//	}
	//	ADDOP_I(_c, LOC(_s), RAISE_VARARGS, (AlifIntT)n);
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



static AlifIntT codegen_loadClassDictFreeVar(AlifCompiler* _c, Location _loc) { // 3997
	ADDOP_N(_c, _loc, LOAD_DEREF, &ALIF_ID(__classDict__), freevars);
	return SUCCESS;
}


static AlifIntT compiler_nameOp(AlifCompiler* _c, Location _loc,
	Identifier _name, ExprContext_ _ctx) { // 4004
	AlifIntT op{}, scope{};
	AlifSizeT arg{};
	enum OpType_ { OP_FAST, OP_GLOBAL, OP_DEREF, OP_NAME } optype;

	AlifObject* dict = _c->u_->metadata.names;
	AlifObject* mangled;

	mangled = compiler_maybeMangle(_c, _name);
	if (!mangled) {
		return ERROR;
	}

	op = 0;
	optype = OpType_::OP_NAME;
	scope = alifST_getScope(SYMTABLE_ENTRY(_c), mangled);
	switch (scope) {
	case FREE:
		dict = _c->u_->metadata.freevars;
		optype = OpType_::OP_DEREF;
		break;
	case CELL:
		dict = _c->u_->metadata.cellvars;
		optype = OpType_::OP_DEREF;
		break;
	case LOCAL:
		if (alifST_isFunctionLike(SYMTABLE_ENTRY(_c))) {
			optype = OpType_::OP_FAST;
		}
		else {
			AlifObject* item{};
			if (alifDict_getItemRef(_c->u_->metadata.fasthidden, mangled,
				&item) < 0) {
				goto error;
			}
			if (item == ALIF_TRUE) {
				optype = OpType_::OP_FAST;
			}
			ALIF_XDECREF(item);
		}
		break;
	case GLOBAL_IMPLICIT:
		if (alifST_isFunctionLike(SYMTABLE_ENTRY(_c)))
			optype = OpType_::OP_GLOBAL;
		break;
	case GLOBAL_EXPLICIT:
		optype = OpType_::OP_GLOBAL;
		break;
	case -1:
		goto error;
	default:
		/* scope can be 0 */
		break;
	}

	switch (optype) {
	case OpType_::OP_DEREF:
		switch (_ctx) {
		case Load:
			if (SYMTABLE_ENTRY(_c)->type == BlockType_::Class_Block
				and !_c->u_->inInlinedComp) {
				op = LOAD_FROM_DICT_OR_DEREF;
				// First load the locals
				if (codegen_addOpNoArg(INSTR_SEQUENCE(_c), LOAD_LOCALS, _loc) < 0) {
					goto error;
				}
			}
			else if (SYMTABLE_ENTRY(_c)->canSeeClassScope) {
				op = LOAD_FROM_DICT_OR_DEREF;
				// First load the classdict
				if (codegen_loadClassDictFreeVar(_c, _loc) < 0) {
					goto error;
				}
			}
			else {
				op = LOAD_DEREF;
			}
			break;
		case Store: op = STORE_DEREF; break;
		case Del: op = DELETE_DEREF; break;
		}
		break;
	case OpType_::OP_FAST:
		switch (_ctx) {
		case Load: op = LOAD_FAST; break;
		case Store: op = STORE_FAST; break;
		case Del: op = DELETE_FAST; break;
		}
		ADDOP_N(_c, _loc, op, mangled, varnames);
		return SUCCESS;
	case OpType_::OP_GLOBAL:
		switch (_ctx) {
		case Load:
			if (SYMTABLE_ENTRY(_c)->canSeeClassScope
				and scope == GLOBAL_IMPLICIT) {
				op = LOAD_FROM_DICT_OR_GLOBALS;
				// First load the classdict
				if (codegen_loadClassDictFreeVar(_c, _loc) < 0) {
					goto error;
				}
			}
			else {
				op = LOAD_GLOBAL;
			}
			break;
		case Store: op = STORE_GLOBAL; break;
		case Del: op = DELETE_GLOBAL; break;
		}
		break;
	case OpType_::OP_NAME:
		switch (_ctx) {
		case Load:
			op = (SYMTABLE_ENTRY(_c)->type == BlockType_::Class_Block
				and _c->u_->inInlinedComp)
				? LOAD_GLOBAL
				: LOAD_NAME;
			break;
		case Store: op = STORE_NAME; break;
		case Del: op = DELETE_NAME; break;
		}
		break;
	}

	arg = dict_addO(dict, mangled);
	ALIF_DECREF(mangled);
	if (arg < 0) {
		return ERROR;
	}
	if (op == LOAD_GLOBAL) {
		arg <<= 1;
	}
	ADDOP_I(_c, _loc, op, arg);
	return SUCCESS;

error:
	ALIF_DECREF(mangled);
	return ERROR;
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
		ADDOP_JUMP(_c, loc, jumpi, _end);
		ADDOP(_c, loc, POP_TOP);
	}
	VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(s, n));

	USE_LABEL(_c, _end);
	return SUCCESS;
}


static AlifIntT starUnpack_helper(AlifCompiler* _c, Location _loc,
	ASDLExprSeq* _elts, AlifIntT _pushed, AlifIntT _build,
	AlifIntT _add, AlifIntT _extend, AlifIntT _tuple) { // 4181
	AlifSizeT n = ASDL_SEQ_LEN(_elts);
	if (n > 2 and areAllItems_const(_elts, 0, n)) {
		AlifObject* folded = alifTuple_new(n);
		if (folded == nullptr) {
			return ERROR;
		}
		AlifObject* val{};
		for (AlifSizeT i = 0; i < n; i++) {
			val = ((ExprTy)ASDL_SEQ_GET(_elts, i))->V.constant.val;
			ALIFTUPLE_SET_ITEM(folded, i, ALIF_NEWREF(val));
		}
		if (_tuple and !_pushed) {
			ADDOP_LOAD_CONST_NEW(_c, _loc, folded);
		}
		else {
			if (_add == SET_ADD) {
				ALIF_SETREF(folded, alifFrozenSet_new(folded));
				if (folded == nullptr) {
					return ERROR;
				}
			}
			ADDOP_I(_c, _loc, _build, _pushed);
			ADDOP_LOAD_CONST_NEW(_c, _loc, folded);
			ADDOP_I(_c, _loc, _extend, 1);
			if (_tuple) {
				ADDOP_I(_c, _loc, CALL_INTRINSIC_1, INTRINSIC_LIST_TO_TUPLE);
			}
		}
		return SUCCESS;
	}

	AlifIntT big = n + _pushed > STACK_USE_GUIDELINE;
	AlifIntT seenStar = 0;
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy elt = ASDL_SEQ_GET(_elts, i);
		if (elt->type == ExprK_::StarK) {
			seenStar = 1;
			break;
		}
	}
	if (!seenStar and !big) {
		for (AlifSizeT i = 0; i < n; i++) {
			ExprTy elt = ASDL_SEQ_GET(_elts, i);
			VISIT(_c, Expr, elt);
		}
		if (_tuple) {
			ADDOP_I(_c, _loc, BUILD_TUPLE, n + _pushed);
		}
		else {
			ADDOP_I(_c, _loc, _build, n + _pushed);
		}
		return SUCCESS;
	}
	AlifIntT sequenceBuilt = 0;
	if (big) {
		ADDOP_I(_c, _loc, _build, _pushed);
		sequenceBuilt = 1;
	}
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy elt = ASDL_SEQ_GET(_elts, i);
		if (elt->type == ExprK_::StarK) {
			if (sequenceBuilt == 0) {
				ADDOP_I(_c, _loc, _build, i + _pushed);
				sequenceBuilt = 1;
			}
			VISIT(_c, Expr, elt->V.star.val);
			ADDOP_I(_c, _loc, _extend, 1);
		}
		else {
			VISIT(_c, Expr, elt);
			if (sequenceBuilt) {
				ADDOP_I(_c, _loc, _add, 1);
			}
		}
	}
	if (_tuple) {
		ADDOP_I(_c, _loc, CALL_INTRINSIC_1, INTRINSIC_LIST_TO_TUPLE);
	}
	return SUCCESS;
}



static AlifIntT unpack_helper(AlifCompiler* _c, Location _loc, ASDLExprSeq* _elts) { // 4266
	AlifSizeT n = ASDL_SEQ_LEN(_elts);
	AlifIntT seenStar = 0;
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy elt = ASDL_SEQ_GET(_elts, i);
		if (elt->type == ExprK_::StarK and !seenStar) {
			if ((i >= (1 << 8)) or
				(n - i - 1 >= (INT_MAX >> 8))) {
				//return compiler_error(_c, _loc,
				//	"too many expressions in "
				//	"star-unpacking assignment");
			}
			ADDOP_I(_c, _loc, UNPACK_EX, (i + ((n - i - 1) << 8)));
			seenStar = 1;
		}
		else if (elt->type == ExprK_::StarK) {
			//return compiler_error(_c, _loc,
			//	"multiple starred expressions in assignment");
		}
	}
	if (!seenStar) {
		ADDOP_I(_c, _loc, UNPACK_SEQUENCE, n);
	}
	return SUCCESS;
}


static AlifIntT assignment_helper(AlifCompiler* _c,
	Location _loc, ASDLExprSeq* _elts) { // 4294
	AlifSizeT n = ASDL_SEQ_LEN(_elts);
	RETURN_IF_ERROR(unpack_helper(_c, _loc, _elts));
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy elt = ASDL_SEQ_GET(_elts, i);
		VISIT(_c, Expr, elt->type != ExprK_::StarK ? elt : elt->V.star.val);
	}
	return SUCCESS;
}


static AlifIntT codegen_list(AlifCompiler* _c, ExprTy _e) { // 4306
	Location loc = LOC(_e);
	ASDLExprSeq* elts = _e->V.list.elts;
	if (_e->V.list.ctx == ExprContext_::Store) {
		return assignment_helper(_c, loc, elts);
	}
	else if (_e->V.list.ctx == ExprContext_::Load) {
		return starUnpack_helper(_c, loc, elts, 0,
			BUILD_LIST, LIST_APPEND, LIST_EXTEND, 0);
	}
	else {
		VISIT_SEQ(_c, Expr, elts);
	}
	return SUCCESS;
}

static AlifIntT codegen_tuple(AlifCompiler* _c, ExprTy _e) { // 4324
	Location loc = LOC(_e);
	ASDLExprSeq* elts = _e->V.tuple.elts;
	if (_e->V.tuple.ctx == ExprContext_::Store) {
		return assignment_helper(_c, loc, elts);
	}
	else if (_e->V.tuple.ctx == ExprContext_::Load) {
		return starUnpack_helper(_c, loc, elts, 0,
			BUILD_LIST, LIST_APPEND, LIST_EXTEND, 1);
	}
	else {
		VISIT_SEQ(_c, Expr, elts);
	}
	return SUCCESS;
}


static bool areAllItems_const(ASDLExprSeq* _seq, AlifSizeT _begin, AlifSizeT _end) { // 4350
	for (AlifSizeT i = _begin; i < _end; i++) {
		ExprTy key = (ExprTy)ASDL_SEQ_GET(_seq, i);
		if (key == nullptr or key->type != ExprK_::ConstantK) {
			return false;
		}
	}
	return true;
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

static AlifIntT check_caller(AlifCompiler* _c, ExprTy _e) { // 4509
	switch (_e->type) {
	case ExprK_::ConstantK:
	case ExprK_::TupleK:
	case ExprK_::ListK:
	case ExprK_::ListCompK:
	case ExprK_::DictK:
	case ExprK_::DictCompK:
	case ExprK_::SetK:
	case ExprK_::SetCompK:
	//case GeneratorExpK:
	case ExprK_::JoinStrK:
	case ExprK_::FormattedValK: {
		Location loc = LOC(_e);
		//return compiler_warn(_c, loc, "'%.200s' object is not callable; "
		//	"perhaps you missed a comma?",
		//	infer_type(_e)->name);
		return -1; // alif
	}
	default:
		return SUCCESS;
	}
}


static AlifIntT is_importOriginated(AlifCompiler* _c, ExprTy _e) { // 4599
	/* Check whether the global scope has an import named
	 e, if it is a Name object. For not traversing all the
	 scope stack every time this function is called, it will
	 only check the global scope to determine whether something
	 is imported or not. */

	if (_e->type != ExprK_::NameK) {
		return 0;
	}

	long flags = alifST_getSymbol(SYMTABLE(_c)->top, _e->V.name.name);
	RETURN_IF_ERROR(flags);
	return flags & DEF_IMPORT;
}




static Location updateStartLocation_toMatchAttr(AlifCompiler* _c,
	Location _loc, ExprTy _attr) { // 4704
	if (_loc.lineNo != _attr->endLineNo) {
		_loc.lineNo = _attr->endLineNo;
		AlifIntT len = (int)ALIFUSTR_GET_LENGTH(_attr->V.attribute.attr);
		if (len <= _attr->endColOffset) {
			_loc.colOffset = _attr->endColOffset - len;
		}
		else {
			_loc.colOffset = -1;
			_loc.endColOffset = -1;
		}
		// Make sure the end position still follows the start position, even for
		// weird ASTs:
		_loc.endLineNo = ALIF_MAX(_loc.lineNo, _loc.endLineNo);
		if (_loc.lineNo == _loc.endLineNo) {
			_loc.endColOffset = ALIF_MAX(_loc.colOffset, _loc.endColOffset);
		}
	}
	return _loc;
}


static AlifIntT maybeOptimize_methodCall(AlifCompiler* _c, ExprTy _e) { // 4731
	AlifSizeT argsl{}, i{}, kwdsl{};
	ExprTy meth = _e->V.call.func;
	ASDLExprSeq* args = _e->V.call.args;
	ASDLKeywordSeq* kwds = _e->V.call.keywords;

	/* Check that the call node is an attribute access */
	if (meth->type != ExprK_::AttributeK or meth->V.attribute.ctx != ExprContext_::Load) {
		return 0;
	}

	/* Check that the base object is not something that is imported */
	AlifIntT ret = is_importOriginated(_c, meth->V.attribute.val);
	RETURN_IF_ERROR(ret);
	if (ret) return 0;

	/* Check that there aren't too many arguments */
	argsl = ASDL_SEQ_LEN(args);
	kwdsl = ASDL_SEQ_LEN(kwds);
	if (argsl + kwdsl + (kwdsl != 0) >= STACK_USE_GUIDELINE) {
		return 0;
	}
	/* Check that there are no *varargs types of arguments. */
	for (i = 0; i < argsl; i++) {
		ExprTy elt = ASDL_SEQ_GET(args, i);
		if (elt->type == ExprK_::StarK) {
			return 0;
		}
	}

	for (i = 0; i < kwdsl; i++) {
		KeywordTy kw = ASDL_SEQ_GET(kwds, i);
		if (kw->arg == nullptr) {
			return 0;
		}
	}

	/* Alright, we can optimize the code. */
	Location loc = LOC(meth);

	//ret = canOptimize_superCall(_c, meth);
	RETURN_IF_ERROR(ret);
	if (ret) {
		//RETURN_IF_ERROR(loadArgs_forSuper(_c, meth->V.attribute.val));
		AlifIntT opcode = ASDL_SEQ_LEN(meth->V.attribute.val->V.call.args) ?
			LOAD_SUPER_METHOD : LOAD_ZERO_SUPER_METHOD;
		ADDOP_NAME(_c, loc, opcode, meth->V.attribute.attr, names);
		loc = updateStartLocation_toMatchAttr(_c, loc, meth);
		ADDOP(_c, loc, NOP);
	}
	else {
		VISIT(_c, Expr, meth->V.attribute.val);
		loc = updateStartLocation_toMatchAttr(_c, loc, meth);
		ADDOP_NAME(_c, loc, LOAD_METHOD, meth->V.attribute.attr, names);
	}

	VISIT_SEQ(_c, Expr, _e->V.call.args);

	if (kwdsl) {
		VISIT_SEQ(_c, Keyword, kwds);
		RETURN_IF_ERROR(
			codegen_callSimpleKwHelper(_c, loc, kwds, kwdsl));
		loc = updateStartLocation_toMatchAttr(_c, LOC(_e), meth);
		ADDOP_I(_c, loc, CALL_KW, argsl + kwdsl);
	}
	else {
		loc = updateStartLocation_toMatchAttr(_c, LOC(_e), meth);
		ADDOP_I(_c, loc, CALL, argsl);
	}
	return 1;
}

static AlifIntT codegen_validateKeywords(AlifCompiler* _c, ASDLKeywordSeq* _keywords) { // 4806
	AlifSizeT nkeywords = ASDL_SEQ_LEN(_keywords);
	for (AlifSizeT i = 0; i < nkeywords; i++) {
		KeywordTy key = ((KeywordTy)ASDL_SEQ_GET(_keywords, i));
		if (key->arg == nullptr) {
			continue;
		}
		for (AlifSizeT j = i + 1; j < nkeywords; j++) {
			KeywordTy other = ((KeywordTy)ASDL_SEQ_GET(_keywords, j));
			if (other->arg and !alifUStr_compare(key->arg, other->arg)) {
				//compiler_error(_c, LOC(other), "keyword argument repeated: %U", key->arg);
				return ERROR;
			}
		}
	}
	return SUCCESS;
}

static AlifIntT codegen_call(AlifCompiler* _c, ExprTy _e) { // 4826
	RETURN_IF_ERROR(codegen_validateKeywords(_c, _e->V.call.keywords));
	AlifIntT ret = maybeOptimize_methodCall(_c, _e);
	if (ret < 0) return ERROR;
	if (ret == 1) return SUCCESS;

	RETURN_IF_ERROR(check_caller(_c, _e->V.call.func));
	VISIT(_c, Expr, _e->V.call.func);
	Location loc = LOC(_e->V.call.func);
	ADDOP(_c, loc, PUSH_NULL);
	loc = LOC(_e);
	return codegen_callHelper(_c, loc, 0,
		_e->V.call.args,
		_e->V.call.keywords);
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
	AlifIntT oparg{};

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



static AlifIntT codegen_subKwArgs(AlifCompiler* _c, Location _loc,
	ASDLKeywordSeq* _keywords, AlifSizeT _begin, AlifSizeT _end) { // 4923
	AlifSizeT i{}, n = _end - _begin;
	KeywordTy kw{};
	AlifIntT big = n * 2 > STACK_USE_GUIDELINE;
	if (big) {
		ADDOP_I(_c, _noLocation_, BUILD_MAP, 0);
	}
	for (i = _begin; i < _end; i++) {
		kw = ASDL_SEQ_GET(_keywords, i);
		ADDOP_LOAD_CONST(_c, _loc, kw->arg);
		VISIT(_c, Expr, kw->val);
		if (big) {
			ADDOP_I(_c, _noLocation_, MAP_ADD, 1);
		}
	}
	if (!big) {
		ADDOP_I(_c, _loc, BUILD_MAP, n);
	}
	return SUCCESS;
}

static AlifIntT codegen_callSimpleKwHelper(AlifCompiler* _c, Location _loc,
	ASDLKeywordSeq* _keywords, AlifSizeT _nkwelts) { // 4952
	AlifObject* names{};
	names = alifTuple_new(_nkwelts);
	if (names == nullptr) {
		return ERROR;
	}
	for (AlifSizeT i = 0; i < _nkwelts; i++) {
		KeywordTy kw = ASDL_SEQ_GET(_keywords, i);
		ALIFTUPLE_SET_ITEM(names, i, ALIF_NEWREF(kw->arg));
	}
	ADDOP_LOAD_CONST_NEW(_c, _loc, names);
	return SUCCESS;
}

static AlifIntT codegen_callHelper(AlifCompiler* _c, Location _loc,
	AlifIntT _n, /* Args already pushed */ ASDLExprSeq* _args,
	ASDLKeywordSeq* _keywords) { // 4971
	AlifSizeT i{}, nseen{}, nelts{}, nkwelts{};

	RETURN_IF_ERROR(codegen_validateKeywords(_c, _keywords));

	nelts = ASDL_SEQ_LEN(_args);
	nkwelts = ASDL_SEQ_LEN(_keywords);

	if (nelts + nkwelts * 2 > STACK_USE_GUIDELINE) {
		goto ex_call;
	}
	for (i = 0; i < nelts; i++) {
		ExprTy elt = ASDL_SEQ_GET(_args, i);
		if (elt->type == ExprK_::StarK) {
			goto ex_call;
		}
	}
	for (i = 0; i < nkwelts; i++) {
		KeywordTy kw = ASDL_SEQ_GET(_keywords, i);
		if (kw->arg == nullptr) {
			goto ex_call;
		}
	}

	/* No * or ** args, so can use faster calling sequence */
	for (i = 0; i < nelts; i++) {
		ExprTy elt = ASDL_SEQ_GET(_args, i);
		VISIT(_c, Expr, elt);
	}
	if (nkwelts) {
		VISIT_SEQ(_c, Keyword, _keywords);
		RETURN_IF_ERROR(
			codegen_callSimpleKwHelper(_c, _loc, _keywords, nkwelts));
		ADDOP_I(_c, _loc, CALL_KW, _n + nelts + nkwelts);
	}
	else {
		ADDOP_I(_c, _loc, CALL, _n + nelts);
	}
	return SUCCESS;

ex_call:

	/* Do positional arguments. */
	if (_n == 0 and nelts == 1 and ((ExprTy)ASDL_SEQ_GET(_args, 0))->type == ExprK_::StarK) {
		VISIT(_c, Expr, ((ExprTy)ASDL_SEQ_GET(_args, 0))->V.star.val);
	}
	else {
		RETURN_IF_ERROR(starUnpack_helper(_c, _loc, _args, _n, BUILD_LIST,
			LIST_APPEND, LIST_EXTEND, 1));
	}
	/* Then keyword arguments */
	if (nkwelts) {
		/* Has a new dict been pushed */
		AlifIntT have_dict = 0;

		nseen = 0;  /* the number of keyword arguments on the stack following */
		for (i = 0; i < nkwelts; i++) {
			KeywordTy kw = ASDL_SEQ_GET(_keywords, i);
			if (kw->arg == nullptr) {
				/* A keyword argument unpacking. */
				if (nseen) {
					RETURN_IF_ERROR(codegen_subKwArgs(_c, _loc, _keywords, i - nseen, i));
					if (have_dict) {
						ADDOP_I(_c, _loc, DICT_MERGE, 1);
					}
					have_dict = 1;
					nseen = 0;
				}
				if (!have_dict) {
					ADDOP_I(_c, _loc, BUILD_MAP, 0);
					have_dict = 1;
				}
				VISIT(_c, Expr, kw->val);
				ADDOP_I(_c, _loc, DICT_MERGE, 1);
			}
			else {
				nseen++;
			}
		}
		if (nseen) {
			/* Pack up any trailing keyword arguments. */
			RETURN_IF_ERROR(codegen_subKwArgs(_c, _loc, _keywords, nkwelts - nseen, nkwelts));
			if (have_dict) {
				ADDOP_I(_c, _loc, DICT_MERGE, 1);
			}
			have_dict = 1;
		}
	}
	ADDOP_I(_c, _loc, CALL_FUNCTION_EX, nkwelts > 0);
	return SUCCESS;
}




static AlifIntT compiler_visitKeyword(AlifCompiler* _c, KeywordTy _k) { // 5769
	VISIT(_c, Expr, _k->val);
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
	case ExprK_::CallK:
		return codegen_call(_c, _e);
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




static AlifObject* constsDict_keysInorder(AlifObject* _dict) { // 7321
	AlifObject* consts{}, * k{}, * v{};
	AlifSizeT i{}, pos = 0, size = ALIFDICT_GET_SIZE(_dict);

	consts = alifList_new(size);
	if (consts == nullptr) return nullptr;
	while (alifDict_next(_dict, &pos, &k, &v)) {
		i = alifLong_asLong(v);
		if (ALIFTUPLE_CHECKEXACT(k)) {
			k = ALIFTUPLE_GET_ITEM(k, 1);
		}
		ALIFLIST_SET_ITEM(consts, i, ALIF_NEWREF(k));
	}
	return consts;
}




static AlifObject* compiler_maybeMangle(AlifCompiler* _c, AlifObject* _name) { // 7352
	return alif_maybeMangle(_c->u_->private_, _c->u_->ste, _name);
}

static InstrSequence* compiler_instrSequence(AlifCompiler* _c) { // 7358
	return _c->u_->instrSequence;
}



static AlifSymTable* compiler_symtable(AlifCompiler* _c) { // 7370
	return _c->st;
}

static AlifSTEntryObject* compiler_symtableEntry(AlifCompiler* _c) { // 7376
	return _c->u_->ste;
}





static AlifIntT compute_codeFlags(AlifCompiler* _c) { // 7402
	AlifSTEntryObject* ste = SYMTABLE_ENTRY(_c);
	AlifIntT flags = 0;
	if (alifST_isFunctionLike(ste)) {
		flags |= CO_NEWLOCALS | CO_OPTIMIZED;
		if (ste->nested)
			flags |= CO_NESTED;
		if (ste->generator and !ste->coroutine)
			flags |= CO_GENERATOR;
		if (ste->generator and ste->coroutine)
			flags |= CO_ASYNC_GENERATOR;
		if (ste->varArgs)
			flags |= CO_VARARGS;
		if (ste->varKeywords)
			flags |= CO_VARKEYWORDS;
	}

	if (ste->coroutine and !ste->generator) {
		flags |= CO_COROUTINE;
	}

	flags |= (_c->flags.flags & ALIFCF_MASK);

	return flags;
}

AlifIntT _alifCompile_constCacheMergeOne(AlifObject* _constCache,
	AlifObject** _obj) { // 7433

	AlifObject* key = const_cacheInsert(_constCache, *_obj, false);
	if (key == nullptr) {
		return ERROR;
	}
	if (ALIFTUPLE_CHECKEXACT(key)) {
		AlifObject* item = ALIFTUPLE_GET_ITEM(key, 1);
		ALIF_SETREF(*_obj, ALIF_NEWREF(item));
		ALIF_DECREF(key);
	}
	else {
		ALIF_SETREF(*_obj, key);
	}
	return SUCCESS;
}

static AlifIntT addReturn_atEnd(AlifCompiler* _c, AlifIntT _addNone) { // 7451
	/* Make sure every instruction stream that falls off the end returns None.
	 * This also ensures that no jump target offsets are out of bounds.
	 */
	if (_addNone) {
		ADDOP_LOAD_CONST(_c, _noLocation_, ALIF_NONE);
	}
	ADDOP(_c, _noLocation_, RETURN_VALUE);
	return SUCCESS;
}



static AlifCodeObject* optimizeAndAssemble_codeUnit(CompilerUnit* u, AlifObject* const_cache,
	AlifIntT _codeFlags, AlifObject* filename) { // 7464
	CFGBuilder* g = nullptr;
	InstrSequence optimizedInstrs{};
	memset(&optimizedInstrs, 0, sizeof(InstrSequence));

	AlifIntT nlocals{};
	AlifIntT nparams{};

	AlifIntT stackDepth{};
	AlifIntT nLocalsPlus{};

	AlifCodeObject* co = nullptr;
	AlifObject* consts = constsDict_keysInorder(u->metadata.consts);
	if (consts == nullptr) {
		goto error;
	}
	g = alifCFG_fromInstructionSequence(u->instrSequence);
	if (g == nullptr) {
		goto error;
	}
	nlocals = (AlifIntT)ALIFDICT_GET_SIZE(u->metadata.varnames);
	nparams = (AlifIntT)ALIFLIST_GET_SIZE(u->ste->varNames);

	if (alifCFG_optimizeCodeUnit(g, consts, const_cache, nlocals,
		nparams, u->metadata.firstLineno) < 0) {
		goto error;
	}


	if (alifCFG_optimizedCFGToInstructionSequence(g, &u->metadata, _codeFlags,
		&stackDepth, &nLocalsPlus, &optimizedInstrs) < 0) {
		goto error;
	}

	/** Assembly **/

	co = alifAssemble_makeCodeObject(&u->metadata, const_cache, consts,
		stackDepth, &optimizedInstrs, nLocalsPlus, _codeFlags, filename);

error:
	ALIF_XDECREF(consts);
	alifInstructionSequence_fini(&optimizedInstrs);
	alifCFGBuilder_free(g);
	return co;
}


static AlifCodeObject* optimize_andAssemble(AlifCompiler* _c, AlifIntT _addNone) { // 7511
	CompilerUnit* u = _c->u_;
	AlifObject* constCache = _c->constCache;
	AlifObject* filename = _c->filename;

	AlifIntT codeFlags = compute_codeFlags(_c);
	if (codeFlags < 0) {
		return nullptr;
	}

	if (addReturn_atEnd(_c, _addNone) < 0) {
		return nullptr;
	}

	return optimizeAndAssemble_codeUnit(u, constCache, codeFlags, filename);
}
