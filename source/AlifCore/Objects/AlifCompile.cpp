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
#include "AlifCore_Long.h"
#include "AlifCore_State.h"
#include "AlifCore_SetObject.h"
#include "AlifCore_SymTable.h"



#define NEED_OPCODE_METADATA
#include "AlifCore_OpcodeMetaData.h"
#undef NEED_OPCODE_METADATA

 // 46
#define COMP_GENEXP   0
#define COMP_LISTCOMP 1
#define COMP_SETCOMP  2
#define COMP_DICTCOMP 3


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
 // 70
#define RETURN_IF_ERROR_IN_SCOPE(_c, _call) { \
    if (_call < 0) { \
        compiler_exitScope(_c); \
        return ERROR; \
    } \
}


class AlifCompiler; // 77


typedef AlifInstructionSequence InstrSequence; // 80

static InstrSequence* compiler_instrSequence(AlifCompiler*); // 82
static AlifSymTable* compiler_symtable(AlifCompiler* c); // 84
static SymTableEntry* compiler_symtableEntry(AlifCompiler*); // 85

#define INSTR_SEQUENCE(_c) compiler_instrSequence(_c) // 91
#define SYMTABLE(_c) compiler_symtable(_c) // 93
#define SYMTABLE_ENTRY(_c) compiler_symtableEntry(_c) // 94
#define OPTIMIZATION_LEVEL(_c) compiler_optimizationLevel(_c) // 95

#define QUALNAME(_c) compiler_qualname(_c) // 99
#define METADATA(_c) compiler_unitMetadata(_c)

typedef AlifSourceLocation Location; // 99
typedef class AlifCFGBuilder CFGBuilder; // 100
typedef AlifJumpTargetLabel JumpTargetLabel; // 101

enum FBlockType_; // 103

static AlifIntT compiler_isTopLevelAwait(AlifCompiler*); // 105
static AlifObject* compiler_maybeMangle(AlifCompiler*, AlifObject*); // 107
static AlifIntT compiler_optimizationLevel(AlifCompiler *); // 108
static AlifIntT compiler_isInInlinedComp(AlifCompiler*); // 110
static AlifObject* compiler_qualname(AlifCompiler*); // 111
static AlifObject* compiler_staticAttributesTuple(AlifCompiler*); // 114
static AlifIntT compiler_lookupArg(AlifCompiler*, AlifCodeObject*, AlifObject*); // 115
static AlifIntT compiler_getRefType(AlifCompiler*, AlifObject*); // 116
static AlifIntT compiler_lookupCellVar(AlifCompiler*, AlifObject*); // 117
static AlifIntT compiler_pushFBlock(AlifCompiler*, Location,
	FBlockType_, JumpTargetLabel, JumpTargetLabel, void*); // 119
static void compiler_popFBlock(AlifCompiler*, FBlockType_, JumpTargetLabel); // 122
static class FBlockInfo* compiler_topFBlock(AlifCompiler*); // 124
static AlifIntT compiler_enterScope(AlifCompiler*, Identifier, AlifIntT,
	void*, AlifIntT, AlifObject*, AlifCompileCodeUnitMetadata*); // 125
static AlifSizeT compiler_addConst(AlifCompiler*, AlifObject*); // 129
static void compiler_exitScope(AlifCompiler*);
static AlifIntT compiler_maybeAddStaticAttributeToClass(AlifCompiler*, ExprTy); // 130
static AlifCompileCodeUnitMetadata* compiler_unitMetadata(AlifCompiler*); // 131

#define LOCATION(_lno, _endLno, _col, _endCol) {_lno, _endLno, _col, _endCol} // 112

#define LOC(_x) SRC_LOCATION_FROM_AST(_x)


static JumpTargetLabel _noLabel_ = { -1 }; // 119

#define SAME_LABEL(_l1, _l2) ((_l1).id == (_l2).id) // 121
#define IS_LABEL(_l) (!SAME_LABEL((_l), (_noLabel_)))

 // 1224
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
		arr = alifMem_dataAlloc(newAlloc * _itemSize); //* alif
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
	SymTableEntry* ste{};

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
static AlifIntT codegen_nameOp(AlifCompiler*, Location, Identifier, ExprContext_); // 310
static AlifCodeObject* compiler_mod(AlifCompiler*, ModuleTy); // 312
static AlifIntT codegen_visitStmt(AlifCompiler*, StmtTy); // 313
static AlifIntT codegen_visitKeyword(AlifCompiler*, KeywordTy); // 314
static AlifIntT codegen_visitExpr(AlifCompiler*, ExprTy); // 315
static AlifIntT codegen_augAssign(AlifCompiler*, StmtTy); // 316
static AlifIntT codegen_subScript(AlifCompiler*, ExprTy); // 318
static AlifIntT codegen_slice(AlifCompiler*, ExprTy); // 319

static AlifIntT codegen_addCompare(AlifCompiler*, Location, CmpOp_); //* alif
static bool areAllItems_const(ASDLExprSeq*, AlifSizeT, AlifSizeT); // 321


static AlifIntT codegen_callSimpleKwHelper(AlifCompiler*, Location, ASDLKeywordSeq*, AlifSizeT); // 327
static AlifIntT codegen_callHelper(AlifCompiler*, Location, AlifIntT, ASDLExprSeq*, ASDLKeywordSeq*); // 331

static AlifIntT codegen_syncComprehensionGenerator(AlifCompiler*, Location, ASDLComprehensionSeq*,
	AlifIntT, AlifIntT, ExprTy, ExprTy, AlifIntT, AlifIntT); // 337
static AlifIntT codegen_asyncComprehensionGenerator(AlifCompiler*, Location, ASDLComprehensionSeq*,
	AlifIntT, AlifIntT, ExprTy, ExprTy, AlifIntT, AlifIntT); // 344

static AlifCodeObject* optimize_andAssemble(AlifCompiler*, AlifIntT); // 358






static AlifIntT codegen_addOpI(InstrSequence* seq,
	AlifIntT opcode, AlifSizeT oparg, Location loc) { // 699

	AlifIntT oparg_ = ALIF_SAFE_DOWNCAST(oparg, AlifSizeT, AlifIntT);
	return _alifInstructionSequence_addOp(seq, opcode, oparg_, loc);
}

#define ADDOP_I(_c, _loc, _op, _o) \
    RETURN_IF_ERROR(codegen_addOpI(INSTR_SEQUENCE(_c), _op, _o, _loc)) // 715

#define ADDOP_I_IN_SCOPE(_c, _loc, _op, _o) \
    RETURN_IF_ERROR_IN_SCOPE(_c, codegen_addOpI(INSTR_SEQUENCE(_c), _op, _o, _loc)); // 718

static AlifIntT codegen_addOpNoArg(InstrSequence* _seq,
	AlifIntT _opcode, Location _loc) { // 721
	return _alifInstructionSequence_addOp(_seq, _opcode, 0, _loc);
}

#define ADDOP(_c, _loc, _op) \
    RETURN_IF_ERROR(codegen_addOpNoArg(INSTR_SEQUENCE(_c), _op, _loc)) // 729

#define ADDOP_IN_SCOPE(_c, _loc, _op) \
    RETURN_IF_ERROR_IN_SCOPE(_c, codegen_addOpNoArg(INSTR_SEQUENCE(_c), _op, _loc)) // 732

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
    AlifIntT ret = codegen_addOpO(_c, _loc, _op, METADATA(_c)->_type, _o); \
    ALIF_DECREF(_o); \
    RETURN_IF_ERROR(ret); \
}

 // 942
#define ADDOP_N_IN_SCOPE(_c, _loc, _op, _o, _type) { \
    AlifIntT ret = codegen_addOpO(_c, _loc, _op, METADATA(_c)->_type, _o); \
    ALIF_DECREF(_o); \
    RETURN_IF_ERROR_IN_SCOPE(_c, ret); \
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
    RETURN_IF_ERROR(codegen_addOpName(_c, _loc, _op, METADATA(_c)->_type, _o)) // 997




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

#define ADDOP_INPLACE(_c, _loc, _binOp) \
    RETURN_IF_ERROR(addop_binary(_c, _loc, _binOp, true)) // 1019

#define ADD_YIELD_FROM(_c, _loc, _await) \
    RETURN_IF_ERROR(codegen_addYieldFrom(_c, _loc, _await)) // 1022

#define ADDOP_YIELD(_c, _loc) \
    RETURN_IF_ERROR(codegen_addOpYield(_c, _loc)) // 1028

 // 1035
#define VISIT(_c, _type, _v) \
    RETURN_IF_ERROR(codegen_visit ## _type(_c, _v));

#define VISIT_IN_SCOPE(_c, _type, _v) \
    RETURN_IF_ERROR_IN_SCOPE(_c, codegen_visit ## _type(_c, _v)) // 1038


 // 1041
#define VISIT_SEQ(_c, _type, _sequ) { \
    AlifIntT i_{}; \
    ASDL ## _type ## Seq *_seq = (_sequ); /* avoid variable capture */ \
    for (i_ = 0; i_ < ASDL_SEQ_LEN(_seq); i_++) { \
        _type ## Ty elt = (_type ## Ty)ASDL_SEQ_GET(_seq, i_); \
        RETURN_IF_ERROR(codegen_visit ## _type(_c, elt)); \
    } \
}


static AlifIntT codegen_callExitWithNones(AlifCompiler* _c, Location _loc) { // 1276
	ADDOP_LOAD_CONST(_c, _loc, ALIF_NONE);
	ADDOP_LOAD_CONST(_c, _loc, ALIF_NONE);
	ADDOP_LOAD_CONST(_c, _loc, ALIF_NONE);
	ADDOP_I(_c, _loc, CALL, 3);
	return SUCCESS;
}

static AlifIntT codegen_addYieldFrom(AlifCompiler* _c, Location _loc, AlifIntT _await) { // 1286
	NEW_JUMP_TARGET_LABEL(_c, send);
	NEW_JUMP_TARGET_LABEL(_c, fail);
	NEW_JUMP_TARGET_LABEL(_c, exit);

	USE_LABEL(_c, send);
	ADDOP_JUMP(_c, _loc, SEND, exit);
	// Set up a virtual try/except to handle when StopIteration is raised during
	// a close or throw call. The only way YIELD_VALUE raises if they do!
	ADDOP_JUMP(_c, _loc, SETUP_FINALLY, fail);
	ADDOP_I(_c, _loc, YIELD_VALUE, 1);
	ADDOP(_c, _noLocation_, POP_BLOCK);
	ADDOP_I(_c, _loc, RESUME, _await ? RESUME_AFTER_AWAIT : RESUME_AFTER_YIELD_FROM);
	ADDOP_JUMP(_c, _loc, JUMP_NO_INTERRUPT, send);

	USE_LABEL(_c, fail);
	ADDOP(_c, _loc, CLEANUP_THROW);

	USE_LABEL(_c, exit);
	ADDOP(_c, _loc, END_SEND);
	return SUCCESS;
}


static AlifIntT codegen_unwindFBlock(AlifCompiler* _c, Location* _ploc,
	FBlockInfo* _info, AlifIntT _preserveTos) { // 1332
	switch (_info->type) {
	case FBlockType_::While_Loop:
	case FBlockType_::Exception_Handler:
	case FBlockType_::Exception_Group_Handler:
	case FBlockType_::Async_Comprehension_Generator:
	case FBlockType_::Stop_Iteration:
		return SUCCESS;

	case FBlockType_::For_Loop:
		/* Pop the iterator */
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		ADDOP(_c, *_ploc, POP_TOP);
		return SUCCESS;

	case FBlockType_::Try_Except:
		ADDOP(_c, *_ploc, POP_BLOCK);
		return SUCCESS;

	case FBlockType_::Finally_Try:
		/* This POP_BLOCK gets the line number of the unwinding statement */
		ADDOP(_c, *_ploc, POP_BLOCK);
		if (_preserveTos) {
			RETURN_IF_ERROR(
				compiler_pushFBlock(_c, *_ploc, FBlockType_::Pop_Value, _noLabel_, _noLabel_, nullptr));
		}
		/* Emit the finally block */
		VISIT_SEQ(_c, Stmt, (ASDLStmtSeq*)_info->datum);
		if (_preserveTos) {
			compiler_popFBlock(_c, FBlockType_::Pop_Value, _noLabel_);
		}
		/* The finally block should appear to execute after the
		 * statement causing the unwinding, so make the unwinding
		 * instruction artificial */
		*_ploc = _noLocation_;
		return SUCCESS;

	case FBlockType_::Finally_End:
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		ADDOP(_c, *_ploc, POP_TOP); /* exc_value */
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		ADDOP(_c, *_ploc, POP_BLOCK);
		ADDOP(_c, *_ploc, POP_EXCEPT);
		return SUCCESS;

	case FBlockType_::With:
	case FBlockType_::Async_With:
		*_ploc = _info->loc;
		ADDOP(_c, *_ploc, POP_BLOCK);
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 3);
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		RETURN_IF_ERROR(codegen_callExitWithNones(_c, *_ploc));
		if (_info->type == FBlockType_::Async_With) {
			ADDOP_I(_c, *_ploc, GET_AWAITABLE, 2);
			ADDOP_LOAD_CONST(_c, *_ploc, ALIF_NONE);
			ADD_YIELD_FROM(_c, *_ploc, 1);
		}
		ADDOP(_c, *_ploc, POP_TOP);
		/* The exit block should appear to execute after the
		 * statement causing the unwinding, so make the unwinding
		 * instruction artificial */
		*_ploc = _noLocation_;
		return SUCCESS;

	case FBlockType_::Handler_Cleanup: {
		if (_info->datum) {
			ADDOP(_c, *_ploc, POP_BLOCK);
		}
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		ADDOP(_c, *_ploc, POP_BLOCK);
		ADDOP(_c, *_ploc, POP_EXCEPT);
		if (_info->datum) {
			ADDOP_LOAD_CONST(_c, *_ploc, ALIF_NONE);
			RETURN_IF_ERROR(codegen_nameOp(_c, *_ploc, (Identifier)_info->datum, ExprContext_::Store));
			RETURN_IF_ERROR(codegen_nameOp(_c, *_ploc, (Identifier)_info->datum, ExprContext_::Del));
		}
		return SUCCESS;
	}
	case FBlockType_::Pop_Value: {
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		ADDOP(_c, *_ploc, POP_TOP);
		return SUCCESS;
	}
	}
	ALIF_UNREACHABLE();
}

/** Unwind block stack. If loop is not nullptr, then stop when the first loop is encountered. */
static AlifIntT codegen_unwindFBlockStack(AlifCompiler* _c, Location* _ploc,
	AlifIntT _preserveTos, FBlockInfo** _loop) { // 1435
	FBlockInfo* top = compiler_topFBlock(_c);
	if (top == nullptr) {
		return SUCCESS;
	}
	if (top->type == FBlockType_::Exception_Group_Handler) {
		//return compiler_error(
		//	_c, *_ploc, "'break', 'continue' and 'return' cannot appear in an except* block");
	}
	if (_loop != nullptr
		and (top->type == FBlockType_::While_Loop or top->type == FBlockType_::For_Loop)) {
		*_loop = top;
		return SUCCESS;
	}
	FBlockInfo copy = *top;
	compiler_popFBlock(_c, top->type, top->block);
	RETURN_IF_ERROR(codegen_unwindFBlock(_c, _ploc, &copy, _preserveTos));
	RETURN_IF_ERROR(codegen_unwindFBlockStack(_c, _ploc, _preserveTos, _loop));
	compiler_pushFBlock(_c, copy.loc, copy.type, copy.block,
		copy.exit, copy.datum);
	return SUCCESS;
}





static AlifIntT codegen_body(AlifCompiler* _c,
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

	//if (!(FUTURE_FEATURES(c) & CO_FUTURE_ANNOTATIONS)) {
	//	RETURN_IF_ERROR(codegen_process_deferred_annotations(c, loc));
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


static AlifIntT codegenEnter_anonymousScope(AlifCompiler* _c, ModuleTy _mod) { // 1631
	RETURN_IF_ERROR(compiler_enterScope(_c, &ALIF_STR(AnonModule),
		ScopeType_::Compiler_Scope_Module, _mod, 1, nullptr, nullptr));

	return SUCCESS;
}



static AlifIntT codegen_makeClosure(AlifCompiler* _c, Location _loc,
	AlifCodeObject* _co, AlifSizeT _flags) { // 1731
	if (_co->nFreeVars) {
		AlifIntT i = alifUnstableCode_getFirstFree(_co);
		for (; i < _co->nLocalsPlus; ++i) {
			/* Bypass com_addop_varname because it will generate
			   LOAD_DEREF but LOAD_CLOSURE is needed.
			*/
			AlifObject* name = ALIFTUPLE_GET_ITEM(_co->localsPlusNames, i);
			AlifIntT arg = compiler_lookupArg(_c, _co, name);
			RETURN_IF_ERROR(arg);
			ADDOP_I(_c, _loc, LOAD_CLOSURE, arg);
		}
		_flags |= MAKE_FUNCTION_CLOSURE;
		ADDOP_I(_c, _loc, BUILD_TUPLE, _co->nFreeVars);
	}
	ADDOP_LOAD_CONST(_c, _loc, (AlifObject*)_co);

	ADDOP(_c, _loc, MAKE_FUNCTION);

	if (_flags & MAKE_FUNCTION_CLOSURE) {
		ADDOP_I(_c, _loc, SET_FUNCTION_ATTRIBUTE, MAKE_FUNCTION_CLOSURE);
	}
	if (_flags & MAKE_FUNCTION_ANNOTATIONS) {
		ADDOP_I(_c, _loc, SET_FUNCTION_ATTRIBUTE, MAKE_FUNCTION_ANNOTATIONS);
	}
	if (_flags & MAKE_FUNCTION_ANNOTATE) {
		ADDOP_I(_c, _loc, SET_FUNCTION_ATTRIBUTE, MAKE_FUNCTION_ANNOTATE);
	}
	if (_flags & MAKE_FUNCTION_KWDEFAULTS) {
		ADDOP_I(_c, _loc, SET_FUNCTION_ATTRIBUTE, MAKE_FUNCTION_KWDEFAULTS);
	}
	if (_flags & MAKE_FUNCTION_DEFAULTS) {
		ADDOP_I(_c, _loc, SET_FUNCTION_ATTRIBUTE, MAKE_FUNCTION_DEFAULTS);
	}
	return SUCCESS;
}



static AlifIntT codegen_kwOnlyDefaults(AlifCompiler* _c, Location _loc,
	ASDLArgSeq* _kwOnlyArgs, ASDLExprSeq* _kwDefaults) { // 1798
	/* Push a dict of keyword-only default values.

	   Return -1 on error, 0 if no dict pushed, 1 if a dict is pushed.
	   */

	AlifIntT defaultCount = 0;
	for (AlifIntT i = 0; i < ASDL_SEQ_LEN(_kwOnlyArgs); i++) {
		ArgTy arg = ASDL_SEQ_GET(_kwOnlyArgs, i);
		ExprTy default_ = ASDL_SEQ_GET(_kwDefaults, i);
		if (default_) {
			defaultCount++;
			AlifObject* mangled = compiler_maybeMangle(_c, arg->arg);
			if (!mangled) {
				return -1;
			}
			ADDOP_LOAD_CONST_NEW(_c, _loc, mangled);
			VISIT(_c, Expr, default_);
		}
	}
	if (defaultCount) {
		ADDOP_I(_c, _loc, BUILD_MAP, defaultCount);
		return 1;
	}
	else {
		return 0;
	}
}




static AlifIntT codegen_defaults(AlifCompiler* _c, ArgumentsTy _args,
	Location _loc) { // 1967
	VISIT_SEQ(_c, Expr, _args->defaults);
	ADDOP_I(_c, _loc, BUILD_TUPLE, ASDL_SEQ_LEN(_args->defaults));
	return SUCCESS;
}

static AlifSizeT codegen_defaultArguments(AlifCompiler* _c, Location _loc,
	ArgumentsTy _args) { // 1976
	AlifSizeT funcflags = 0;
	if (_args->defaults and ASDL_SEQ_LEN(_args->defaults) > 0) {
		RETURN_IF_ERROR(codegen_defaults(_c, _args, _loc));
		funcflags |= MAKE_FUNCTION_DEFAULTS;
	}
	if (_args->kwOnlyArgs) {
		AlifIntT res = codegen_kwOnlyDefaults(_c, _loc,
			_args->kwOnlyArgs, _args->kwDefaults);
		RETURN_IF_ERROR(res);
		if (res > 0) {
			funcflags |= MAKE_FUNCTION_KWDEFAULTS;
		}
	}
	return funcflags;
}


static AlifIntT codegen_wrapInStopIterationHandler(AlifCompiler* _c) { // 1997
	NEW_JUMP_TARGET_LABEL(_c, handler);

	/* Insert SETUP_CLEANUP at start */
	RETURN_IF_ERROR(
		_alifInstructionSequence_insertInstruction(
			INSTR_SEQUENCE(_c), 0,
			SETUP_CLEANUP, handler.id, _noLocation_));

	ADDOP_LOAD_CONST(_c, _noLocation_, ALIF_NONE);
	ADDOP(_c, _noLocation_, RETURN_VALUE);
	USE_LABEL(_c, handler);
	ADDOP_I(_c, _noLocation_, CALL_INTRINSIC_1, INTRINSIC_STOPITERATION_ERROR);
	ADDOP_I(_c, _noLocation_, RERAISE, 1);
	return SUCCESS;
}



static AlifIntT codegen_functionBody(AlifCompiler* _c, StmtTy _s,
	AlifIntT _isAsync, AlifSizeT _funcFlags, AlifIntT _firstLineNo) { // 2132
	ArgumentsTy args{};
	Identifier name{};
	ASDLStmtSeq* body{};
	AlifIntT scopeType{};

	if (_isAsync) {
		args = _s->V.asyncFunctionDef.args;
		name = _s->V.asyncFunctionDef.name;
		body = _s->V.asyncFunctionDef.body;

		scopeType = ScopeType_::Compiler_Scope_Async_Function;
	}
	else {
		args = _s->V.functionDef.args;
		name = _s->V.functionDef.name;
		body = _s->V.functionDef.body;

		scopeType = ScopeType_::Compiler_Scope_Function;
	}

	AlifCompileCodeUnitMetadata umd = {
		.argCount = ASDL_SEQ_LEN(args->args),
		.posOnlyArgCount = ASDL_SEQ_LEN(args->posOnlyArgs),
		.kwOnlyArgCount = ASDL_SEQ_LEN(args->kwOnlyArgs),
	};
	RETURN_IF_ERROR(
		compiler_enterScope(_c, name, scopeType, (void*)_s, _firstLineNo, nullptr, &umd));

	AlifSizeT first_instr = 0;
	AlifObject* docstring = alifAST_getDocString(body);
	if (docstring) {
		first_instr = 1;
		/* if not -OO mode, add docstring */
		if (OPTIMIZATION_LEVEL(_c) < 2) {
			//docstring = _alifCompile_cleanDoc(docstring);
			//if (docstring == nullptr) {
			//	compiler_exitScope(_c);
			//	return ERROR;
			//}
		}
		else {
			docstring = nullptr;
		}
	}
	AlifSizeT idx = compiler_addConst(_c, docstring ? docstring : ALIF_NONE);
	ALIF_XDECREF(docstring);
	RETURN_IF_ERROR_IN_SCOPE(_c, idx < 0 ? ERROR : SUCCESS);

	NEW_JUMP_TARGET_LABEL(_c, start);
	USE_LABEL(_c, start);
	SymTableEntry* ste = SYMTABLE_ENTRY(_c);
	bool add_stopiteration_handler = ste->coroutine or ste->generator;
	if (add_stopiteration_handler) {
		RETURN_IF_ERROR(
			compiler_pushFBlock(_c, _noLocation_, FBlockType_::Stop_Iteration,
				start, _noLabel_, nullptr));
	}

	for (AlifSizeT i = first_instr; i < ASDL_SEQ_LEN(body); i++) {
		VISIT_IN_SCOPE(_c, Stmt, (StmtTy)ASDL_SEQ_GET(body, i));
	}
	if (add_stopiteration_handler) {
		RETURN_IF_ERROR_IN_SCOPE(_c, codegen_wrapInStopIterationHandler(_c));
		compiler_popFBlock(_c, FBlockType_::Stop_Iteration, start);
	}
	AlifCodeObject* co = optimize_andAssemble(_c, 1);
	compiler_exitScope(_c);
	if (co == nullptr) {
		ALIF_XDECREF(co);
		return ERROR;
	}
	AlifIntT ret = codegen_makeClosure(_c, LOC(_s), co, _funcFlags);
	ALIF_DECREF(co);
	return ret;
}


static AlifIntT codegen_function(AlifCompiler* _c, StmtTy _s, AlifIntT _isAsync) { // 2220
	ArgumentsTy args{};
	ExprTy returns{};
	Identifier name{};
	//ASDLExprSeq* decos{};
	ASDLTypeParamSeq* typeParams{};
	AlifSizeT funcflags{};
	AlifIntT firstlineno{};

	if (_isAsync) {
		args = _s->V.asyncFunctionDef.args;
		//returns = _s->V.asyncFunctionDef.returns;
		//decos = _s->V.asyncFunctionDef.decoratorList;
		name = _s->V.asyncFunctionDef.name;
		typeParams = _s->V.asyncFunctionDef.typeParams;
	}
	else {
		args = _s->V.functionDef.args;
		//returns = _s->V.functionDef.returns;
		//decos = _s->V.functionDef.decoratorList;
		name = _s->V.functionDef.name;
		typeParams = _s->V.functionDef.typeParams;
	}

	//RETURN_IF_ERROR(codegen_decorators(_c, decos));

	firstlineno = _s->lineNo;
	//if (ASDL_SEQ_LEN(decos)) {
	//	firstlineno = ((ExprTy)ASDL_SEQ_GET(decos, 0))->lineNo;
	//}

	Location loc = LOC(_s);

	AlifIntT isGeneric = ASDL_SEQ_LEN(typeParams) > 0;

	funcflags = codegen_defaultArguments(_c, loc, args);
	RETURN_IF_ERROR(funcflags);

	AlifIntT numTypeParamArgs = 0;

	if (isGeneric) {
		if (funcflags & MAKE_FUNCTION_DEFAULTS) {
			numTypeParamArgs += 1;
		}
		if (funcflags & MAKE_FUNCTION_KWDEFAULTS) {
			numTypeParamArgs += 1;
		}
		if (numTypeParamArgs == 2) {
			ADDOP_I(_c, loc, SWAP, 2);
		}
		AlifObject* typeParamsName = alifUStr_fromFormat("<generic parameters of %U>", name);
		if (!typeParamsName) {
			return ERROR;
		}
		AlifCompileCodeUnitMetadata umd = {
			.argCount = numTypeParamArgs,
		};
		AlifIntT ret = compiler_enterScope(_c, typeParamsName, ScopeType_::Compiler_Scope_Annotations,
			(void*)typeParams, firstlineno, nullptr, &umd);
		ALIF_DECREF(typeParamsName);
		RETURN_IF_ERROR(ret);
		//RETURN_IF_ERROR_IN_SCOPE(_c, codegen_typeParams(_c, typeParams));
		for (AlifIntT i = 0; i < numTypeParamArgs; i++) {
			ADDOP_I_IN_SCOPE(_c, loc, LOAD_FAST, i);
		}
	}

	//AlifIntT annotationsFlag = codegen_annotations(_c, loc, args, returns);
	//if (annotationsFlag < 0) {
	//	if (isGeneric) {
	//		compiler_exitScope(_c);
	//	}
	//	return ERROR;
	//}
	//funcflags |= annotationsFlag;

	AlifIntT ret = codegen_functionBody(_c, _s, _isAsync, funcflags, firstlineno);
	if (isGeneric) {
		RETURN_IF_ERROR_IN_SCOPE(_c, ret);
	}
	else {
		RETURN_IF_ERROR(ret);
	}

	if (isGeneric) {
		ADDOP_I_IN_SCOPE(_c, loc, SWAP, 2);
		ADDOP_I_IN_SCOPE(_c, loc, CALL_INTRINSIC_2, INTRINSIC_SET_FUNCTION_TYPE_PARAMS);

		AlifCodeObject* co = optimize_andAssemble(_c, 0);
		compiler_exitScope(_c);
		if (co == nullptr) {
			return ERROR;
		}
		AlifIntT ret = codegen_makeClosure(_c, loc, co, 0);
		ALIF_DECREF(co);
		RETURN_IF_ERROR(ret);
		if (numTypeParamArgs > 0) {
			ADDOP_I(_c, loc, SWAP, numTypeParamArgs + 1);
			ADDOP_I(_c, loc, CALL, numTypeParamArgs - 1);
		}
		else {
			ADDOP(_c, loc, PUSH_NULL);
			ADDOP_I(_c, loc, CALL, 0);
		}
	}

	//RETURN_IF_ERROR(codegen_applyDecorators(_c, decos));
	return codegen_nameOp(_c, loc, name, ExprContext_::Store);
}


static AlifIntT codegen_setTypeParamsInClass(AlifCompiler* _c, Location _loc) { // 2337
	RETURN_IF_ERROR(codegen_nameOp(_c, _loc, &ALIF_STR(TypeParams), ExprContext_::Load));
	RETURN_IF_ERROR(codegen_nameOp(_c, _loc, &ALIF_ID(__typeParams__), ExprContext_::Store));
	return SUCCESS;
}

static AlifIntT codegen_classBody(AlifCompiler* _c, StmtTy _s, AlifIntT _firstLineNo) { // 2346

	/* 1. compile the class body into a code object */
	RETURN_IF_ERROR(
		compiler_enterScope(_c, _s->V.classDef.name, ScopeType_::Compiler_Scope_Class,
			(void*)_s, _firstLineNo, _s->V.classDef.name, nullptr));

	Location loc = LOCATION(_firstLineNo, _firstLineNo, 0, 0);
	/* load (global) __name__ ... */
	RETURN_IF_ERROR_IN_SCOPE(_c, codegen_nameOp(_c, loc, &ALIF_ID(__name__), ExprContext_::Load));
	/* ... and store it as __module__ */
	RETURN_IF_ERROR_IN_SCOPE(_c, codegen_nameOp(_c, loc, &ALIF_ID(__module__), ExprContext_::Store));
	ADDOP_LOAD_CONST(_c, loc, QUALNAME(_c));
	RETURN_IF_ERROR_IN_SCOPE(_c, codegen_nameOp(_c, loc, &ALIF_ID(__qualname__), ExprContext_::Store));
	ADDOP_LOAD_CONST_NEW(_c, loc, alifLong_fromLong(METADATA(_c)->firstLineno));
	RETURN_IF_ERROR_IN_SCOPE(_c, codegen_nameOp(_c, loc, &ALIF_ID(__firstLineno__), ExprContext_::Store));
	ASDLTypeParamSeq* typeParams = _s->V.classDef.typeParams;
	if (ASDL_SEQ_LEN(typeParams) > 0) {
		RETURN_IF_ERROR_IN_SCOPE(_c, codegen_setTypeParamsInClass(_c, loc));
	}
	if (SYMTABLE_ENTRY(_c)->needsClassDict) {
		ADDOP(_c, loc, LOAD_LOCALS);

		// We can't use codegen_nameOp here because we need to generate a
		// STORE_DEREF in a class namespace, and codegen_nameOp() won't do
		// that by default.
		ADDOP_N_IN_SCOPE(_c, loc, STORE_DEREF, &ALIF_ID(__classDict__), cellvars);
	}
	/* compile the body proper */
	RETURN_IF_ERROR_IN_SCOPE(_c, codegen_body(_c, loc, _s->V.classDef.body));
	AlifObject* staticAttributes = compiler_staticAttributesTuple(_c);
	if (staticAttributes == nullptr) {
		compiler_exitScope(_c);
		return ERROR;
	}
	ADDOP_LOAD_CONST(_c, _noLocation_, staticAttributes);
	ALIF_CLEAR(staticAttributes);
	RETURN_IF_ERROR_IN_SCOPE(
		_c, codegen_nameOp(_c, _noLocation_, &ALIF_ID(__staticAttributes__), ExprContext_::Store));
	/* The following code is artificial */
	/* Set __classdictcell__ if necessary */
	if (SYMTABLE_ENTRY(_c)->needsClassDict) {
		/* Store __classdictcell__ into class namespace */
		AlifIntT i = compiler_lookupCellVar(_c, &ALIF_ID(__classDict__));
		RETURN_IF_ERROR_IN_SCOPE(_c, i);
		ADDOP_I(_c, _noLocation_, LOAD_CLOSURE, i);
		RETURN_IF_ERROR_IN_SCOPE(
			_c, codegen_nameOp(_c, _noLocation_, &ALIF_ID(__classDictCell__), ExprContext_::Store));
	}
	/* Return __classcell__ if it is referenced, otherwise return None */
	if (SYMTABLE_ENTRY(_c)->needsClassClosure) {
		/* Store __classcell__ into class namespace & return it */
		AlifIntT i = compiler_lookupCellVar(_c, &ALIF_ID(__class__));
		RETURN_IF_ERROR_IN_SCOPE(_c, i);
		ADDOP_I(_c, _noLocation_, LOAD_CLOSURE, i);
		ADDOP_I(_c, _noLocation_, COPY, 1);
		RETURN_IF_ERROR_IN_SCOPE(
			_c, codegen_nameOp(_c, _noLocation_, &ALIF_ID(__classCell__), ExprContext_::Store));
	}
	else {
		/* No methods referenced __class__, so just return None */
		ADDOP_LOAD_CONST(_c, _noLocation_, ALIF_NONE);
	}
	ADDOP_IN_SCOPE(_c, _noLocation_, RETURN_VALUE);
	/* create the code object */
	AlifCodeObject* co = optimize_andAssemble(_c, 1);

	/* leave the new scope */
	compiler_exitScope(_c);
	if (co == nullptr) {
		return ERROR;
	}

	/* 2. load the 'build_class' function */

	// these instructions should be attributed to the class line,
	// not a decorator line
	loc = LOC(_s);
	ADDOP(_c, loc, LOAD_BUILD_CLASS);
	ADDOP(_c, loc, PUSH_NULL);

	/* 3. load a function (or closure) made from the code object */
	AlifIntT ret = codegen_makeClosure(_c, loc, co, 0);
	ALIF_DECREF(co);
	RETURN_IF_ERROR(ret);

	/* 4. load class name */
	ADDOP_LOAD_CONST(_c, loc, _s->V.classDef.name);

	return SUCCESS;
}

static AlifIntT codegen_class(AlifCompiler* _c, StmtTy _s) { // 2452
	//ASDLExprSeq* decos = _s->V.classDef.decoratorList;

	//RETURN_IF_ERROR(codegen_decorators(_c, decos));

	AlifIntT firstlineno = _s->lineNo;
	//if (ASDL_SEQ_LEN(decos)) {
	//	firstlineno = ((ExprTy)ASDL_SEQ_GET(decos, 0))->lineNo;
	//}
	Location loc = LOC(_s);

	ASDLTypeParamSeq* typeParams = _s->V.classDef.typeParams;
	AlifIntT isGeneric = ASDL_SEQ_LEN(typeParams) > 0;
	//if (isGeneric) {
	//	AlifObject* type_params_name = alifUStr_fromFormat("<generic parameters of %U>",
	//		_s->V.classDef.name);
	//	if (!type_params_name) {
	//		return ERROR;
	//	}
	//	AlifIntT ret = compiler_enterScope(_c, type_params_name, COMPILER_SCOPE_ANNOTATIONS,
	//		(void*)typeParams, firstlineno, _s->V.classDef.name, nullptr);
	//	ALIF_DECREF(type_params_name);
	//	RETURN_IF_ERROR(ret);
	//	RETURN_IF_ERROR_IN_SCOPE(_c, codegen_typeParams(_c, typeParams));
	//	RETURN_IF_ERROR_IN_SCOPE(_c, compiler_nameOp(_c, loc, &ALIF_STR(typeParams), ExprContext_::Store));
	//}

	AlifIntT ret = codegen_classBody(_c, _s, firstlineno);
	if (isGeneric) {
		RETURN_IF_ERROR_IN_SCOPE(_c, ret);
	}
	else {
		RETURN_IF_ERROR(ret);
	}

	/* generate the rest of the code for the call */

	if (isGeneric) {
		RETURN_IF_ERROR_IN_SCOPE(_c, codegen_nameOp(_c, loc, &ALIF_STR(TypeParams), ExprContext_::Load));
		ADDOP_I_IN_SCOPE(_c, loc, CALL_INTRINSIC_1, INTRINSIC_SUBSCRIPT_GENERIC);
		RETURN_IF_ERROR_IN_SCOPE(_c, codegen_nameOp(_c, loc, &ALIF_STR(GenericBase), ExprContext_::Store));

		AlifSizeT originalLen = ASDL_SEQ_LEN(_s->V.classDef.bases);
		ASDLExprSeq* bases = alifNew_exprSeq(
			originalLen + 1, _c->astMem);
		if (bases == nullptr) {
			compiler_exitScope(_c);
			return ERROR;
		}
		for (AlifSizeT i = 0; i < originalLen; i++) {
			ASDL_SEQ_SET(bases, i, ASDL_SEQ_GET(_s->V.classDef.bases, i));
		}
		ExprTy nameNode = alifAST_name(
			&ALIF_STR(GenericBase), ExprContext_::Load,
			loc.lineNo, loc.colOffset, loc.endLineNo, loc.endColOffset, _c->astMem
		);
		if (nameNode == nullptr) {
			compiler_exitScope(_c);
			return ERROR;
		}
		ASDL_SEQ_SET(bases, originalLen, nameNode);
		RETURN_IF_ERROR_IN_SCOPE(_c, codegen_callHelper(_c, loc, 2,
			bases,
			_s->V.classDef.keywords));

		AlifCodeObject* co = optimize_andAssemble(_c, 0);

		compiler_exitScope(_c);
		if (co == nullptr) {
			return ERROR;
		}
		AlifIntT ret = codegen_makeClosure(_c, loc, co, 0);
		ALIF_DECREF(co);
		RETURN_IF_ERROR(ret);
		ADDOP(_c, loc, PUSH_NULL);
		ADDOP_I(_c, loc, CALL, 0);
	}
	else {
		RETURN_IF_ERROR(codegen_callHelper(_c, loc, 2,
			_s->V.classDef.bases,
			_s->V.classDef.keywords));
	}

	/* 6. apply decorators */
	//RETURN_IF_ERROR(codegen_applyDecorators(_c, decos));

	/* 7. store into <name> */
	RETURN_IF_ERROR(codegen_nameOp(_c, loc, _s->V.classDef.name, ExprContext_::Store));
	return SUCCESS;
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

static AlifIntT codegen_addCompare(AlifCompiler* _c, Location _loc, CmpOp_ _op) { // 2672
	AlifIntT cmp;
	switch (_op) {
	case CmpOp_::Equal:
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


static AlifIntT codegen_if(AlifCompiler* _c, StmtTy _s) { // 2880
	JumpTargetLabel next{};
	NEW_JUMP_TARGET_LABEL(_c, end);
	if (ASDL_SEQ_LEN(_s->V.if_.else_)) {
		NEW_JUMP_TARGET_LABEL(_c, else_);
		next = else_;
	}
	else {
		next = end;
	}
	RETURN_IF_ERROR(
		codegen_jumpIf(_c, LOC(_s), _s->V.if_.condition, next, 0));

	VISIT_SEQ(_c, Stmt, _s->V.if_.body);
	if (ASDL_SEQ_LEN(_s->V.if_.else_)) {
		ADDOP_JUMP(_c, _noLocation_, JUMP_NO_INTERRUPT, end);

		USE_LABEL(_c, next);
		VISIT_SEQ(_c, Stmt, _s->V.if_.else_);
	}

	USE_LABEL(_c, end);
	return SUCCESS;
}

static AlifIntT codegen_for(AlifCompiler* _c, StmtTy _s) { // 2908
	Location loc = LOC(_s);
	NEW_JUMP_TARGET_LABEL(_c, start);
	NEW_JUMP_TARGET_LABEL(_c, body);
	NEW_JUMP_TARGET_LABEL(_c, cleanup);
	NEW_JUMP_TARGET_LABEL(_c, end);

	RETURN_IF_ERROR(compiler_pushFBlock(_c, loc, FBlockType_::For_Loop, start, end, nullptr));

	VISIT(_c, Expr, _s->V.for_.iter);

	loc = LOC(_s->V.for_.iter);
	ADDOP(_c, loc, GET_ITER);

	USE_LABEL(_c, start);
	ADDOP_JUMP(_c, loc, FOR_ITER, cleanup);

	/* Add NOP to ensure correct line tracing of multiline for statements.
	 * It will be removed later if redundant.
	 */
	ADDOP(_c, LOC(_s->V.for_.target), NOP);

	USE_LABEL(_c, body);
	VISIT(_c, Expr, _s->V.for_.target);
	VISIT_SEQ(_c, Stmt, _s->V.for_.body);
	/* Mark jump as artificial */
	ADDOP_JUMP(_c, _noLocation_, JUMP, start);

	USE_LABEL(_c, cleanup);
	/* It is important for instrumentation that the `END_FOR` comes first.
	* Iteration over a generator will jump to the first of these instructions,
	* but a non-generator will jump to a later instruction.
	*/
	ADDOP(_c, _noLocation_, END_FOR);
	ADDOP(_c, _noLocation_, POP_TOP);

	compiler_popFBlock(_c, FBlockType_::For_Loop, start);

	//VISIT_SEQ(_c, Stmt, _s->V.for_.else_);

	USE_LABEL(_c, end);
	return SUCCESS;
}


static AlifIntT codegen_while(AlifCompiler* _c, StmtTy _s) { // 3001
	NEW_JUMP_TARGET_LABEL(_c, loop);
	NEW_JUMP_TARGET_LABEL(_c, end);
	NEW_JUMP_TARGET_LABEL(_c, anchor);

	USE_LABEL(_c, loop);

	RETURN_IF_ERROR(compiler_pushFBlock(_c, LOC(_s), FBlockType_::While_Loop, loop, end, nullptr));
	RETURN_IF_ERROR(codegen_jumpIf(_c, LOC(_s), _s->V.while_.condition, anchor, 0));

	VISIT_SEQ(_c, Stmt, _s->V.while_.body);
	ADDOP_JUMP(_c, _noLocation_, JUMP, loop);

	compiler_popFBlock(_c, FBlockType_::While_Loop, loop);

	USE_LABEL(_c, anchor);
	//if (_s->V.while_.else_) {
	//	VISIT_SEQ(_c, Stmt, _s->V.while_.else_);
	//}

	USE_LABEL(_c, end);
	return SUCCESS;
}

static AlifIntT codegen_return(AlifCompiler* _c, StmtTy _s) { // 3027
	Location loc = LOC(_s);
	AlifIntT preserve_tos = ((_s->V.return_.val != nullptr) and
		(_s->V.return_.val->type != ExprK_::ConstantK));

	SymTableEntry* ste = SYMTABLE_ENTRY(_c);
	if (!alifST_isFunctionLike(ste)) {
		//return compiler_error(_c, loc, "'return' outside function");
	}
	if (_s->V.return_.val != nullptr and ste->coroutine and ste->generator) {
		//return compiler_error(_c, loc, "'return' with value in async generator");
	}

	if (preserve_tos) {
		VISIT(_c, Expr, _s->V.return_.val);
	}
	else {
		/* Emit instruction with line number for return value */
		if (_s->V.return_.val != nullptr) {
			loc = LOC(_s->V.return_.val);
			ADDOP(_c, loc, NOP);
		}
	}
	if (_s->V.return_.val == nullptr or _s->V.return_.val->lineNo != _s->lineNo) {
		loc = LOC(_s);
		ADDOP(_c, loc, NOP);
	}

	RETURN_IF_ERROR(codegen_unwindFBlockStack(_c, &loc, preserve_tos, nullptr));
	if (_s->V.return_.val == nullptr) {
		ADDOP_LOAD_CONST(_c, loc, ALIF_NONE);
	}
	else if (!preserve_tos) {
		ADDOP_LOAD_CONST(_c, loc, _s->V.return_.val->V.constant.val);
	}
	ADDOP(_c, loc, RETURN_VALUE);

	return SUCCESS;
}


static AlifIntT codegen_break(AlifCompiler* _c, Location _loc) { // 3067
	FBlockInfo* loop = nullptr;
	Location originLoc = _loc;
	/* Emit instruction with line number */
	ADDOP(_c, _loc, NOP);
	RETURN_IF_ERROR(codegen_unwindFBlockStack(_c, &_loc, 0, &loop));
	if (loop == nullptr) {
		//return compiler_error(_c, origin_loc, "'break' outside loop");
	}
	RETURN_IF_ERROR(codegen_unwindFBlock(_c, &_loc, loop, 0));
	ADDOP_JUMP(_c, _loc, JUMP, loop->exit);
	return SUCCESS;
}

static AlifIntT codegen_continue(AlifCompiler* _c, Location _loc) { // 3083
	FBlockInfo* loop = nullptr;
	Location origin_loc = _loc;
	/* Emit instruction with line number */
	ADDOP(_c, _loc, NOP);
	RETURN_IF_ERROR(codegen_unwindFBlockStack(_c, &_loc, 0, &loop));
	if (loop == nullptr) {
		//return compiler_error(_c, origin_loc, "'continue' not properly in loop");
	}
	ADDOP_JUMP(_c, _loc, JUMP, loop->block);
	return SUCCESS;
}




static AlifIntT codegen_import(AlifCompiler* c, StmtTy s) { // 3670
	Location loc = LOC(s);
	AlifSizeT i, n = ASDL_SEQ_LEN(s->V.import.names);

	AlifObject* zero = _alifLong_getZero();  // borrowed reference
	for (i = 0; i < n; i++) {
		AliasTy alias = (AliasTy)ASDL_SEQ_GET(s->V.import.names, i);
		AlifIntT r{};

		ADDOP_LOAD_CONST(c, loc, zero);
		ADDOP_LOAD_CONST(c, loc, ALIF_NONE);
		ADDOP_NAME(c, loc, IMPORT_NAME, alias->name, names);

		if (alias->asName) {
			//r = codegen_import_as(c, loc, alias->name, alias->asName);
			RETURN_IF_ERROR(r);
		}
		else {
			Identifier tmp = alias->name;
			AlifSizeT dot = alifUStr_findChar(
				alias->name, '.', 0, ALIFUSTR_GET_LENGTH(alias->name), 1);
			if (dot != -1) {
				tmp = alifUStr_subString(alias->name, 0, dot);
				if (tmp == nullptr) {
					return ERROR;
				}
			}
			r = codegen_nameOp(c, loc, tmp, ExprContext_::Store);
			if (dot != -1) {
				ALIF_DECREF(tmp);
			}
			RETURN_IF_ERROR(r);
		}
	}
	return SUCCESS;
}

static AlifIntT codegen_fromImport(AlifCompiler* c, StmtTy s) { // 3716
	AlifSizeT n = ASDL_SEQ_LEN(s->V.importFrom.names);

	ADDOP_LOAD_CONST_NEW(c, LOC(s), alifLong_fromLong(s->V.importFrom.level));

	AlifObject* names = alifTuple_new(n);
	if (!names) {
		return ERROR;
	}

	/* build up the names */
	for (AlifSizeT i = 0; i < n; i++) {
		AliasTy alias = (AliasTy)ASDL_SEQ_GET(s->V.importFrom.names, i);
		ALIFTUPLE_SET_ITEM(names, i, ALIF_NEWREF(alias->name));
	}

	ADDOP_LOAD_CONST_NEW(c, LOC(s), names);

	if (s->V.importFrom.module) {
		ADDOP_NAME(c, LOC(s), IMPORT_NAME, s->V.importFrom.module, names);
	}
	else {
		ADDOP_NAME(c, LOC(s), IMPORT_NAME, &ALIF_STR(Empty), names);
	}
	for (AlifSizeT i = 0; i < n; i++) {
		AliasTy alias = (AliasTy)ASDL_SEQ_GET(s->V.importFrom.names, i);
		Identifier store_name{};

		if (i == 0 and ALIFUSTR_READ_CHAR(alias->name, 0) == '*') {
			ADDOP_I(c, LOC(s), CALL_INTRINSIC_1, INTRINSIC_IMPORT_STAR);
			ADDOP(c, _noLocation_, POP_TOP);
			return SUCCESS;
		}

		ADDOP_NAME(c, LOC(s), IMPORT_FROM, alias->name, names);
		store_name = alias->name;
		if (alias->asName) {
			store_name = alias->asName;
		}

		RETURN_IF_ERROR(codegen_nameOp(c, LOC(s), store_name, ExprContext_::Store));
	}
	/* remove imported module */
	ADDOP(c, LOC(s), POP_TOP);
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


static AlifIntT codegen_visitStmt(AlifCompiler* _c, StmtTy _s) { // 3818
	switch (_s->type) {
	case StmtK_::FunctionDefK:
		return codegen_function(_c, _s, 0);
	case StmtK_::ClassDefK:
		return codegen_class(_c, _s);
	//case StmtK_::TypeAliasK:
	//	return codegen_typealias(_c, _s);
	case StmtK_::ReturnK:
		return codegen_return(_c, _s);
	case StmtK_::DeleteK:
		VISIT_SEQ(_c, Expr, _s->V.delete_.targets)
			break;
	case StmtK_::AssignK:
	{
		AlifSizeT n = ASDL_SEQ_LEN(_s->V.assign.targets);
		VISIT(_c, Expr, _s->V.assign.val);
		for (AlifSizeT i = 0; i < n; i++) {
			if (i < n - 1) {
				ADDOP_I(_c, LOC(_s), COPY, 1);
			}
			VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(_s->V.assign.targets, i));
		}
		break;
	}
	case StmtK_::AugAssignK:
		return codegen_augAssign(_c, _s);
	//case StmtK_::AnnAssignK:
	//	return compiler_annassign(_c, _s);
	case StmtK_::ForK:
		return codegen_for(_c, _s);
	case StmtK_::WhileK:
		return codegen_while(_c, _s);
	case StmtK_::IfK:
	{
		return codegen_if(_c, _s);
	}
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
	case StmtK_::ImportK:
		return codegen_import(_c, _s);
	case StmtK_::ImportFromK:
		return codegen_fromImport(_c, _s);
	//case StmtK_::GlobalK:
	//case StmtK_::NonlocalK:
	//	break;
	case StmtK_::ExprK:
	{
		return codegen_stmtExpr(_c, LOC(_s), _s->V.expression.val);
	}
	case StmtK_::PassK:
	{
		ADDOP(_c, LOC(_s), NOP);
		break;
	}
	case StmtK_::BreakK:
	{
		return codegen_break(_c, LOC(_s));
	}
	case StmtK_::ContinueK:
	{
		return codegen_continue(_c, LOC(_s));
	}
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
	case UnaryOp_::Sqrt: //* alif
		return UNARY_SQRT;
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



static AlifIntT codegen_addOpYield(AlifCompiler* _c, Location _loc) { // 3986
	SymTableEntry* ste = SYMTABLE_ENTRY(_c);
	if (ste->generator and ste->coroutine) {
		ADDOP_I(_c, _loc, CALL_INTRINSIC_1, INTRINSIC_ASYNC_GEN_WRAP);
	}
	ADDOP_I(_c, _loc, YIELD_VALUE, 0);
	ADDOP_I(_c, _loc, RESUME, RESUME_AFTER_YIELD);
	return SUCCESS;
}



static AlifIntT codegen_loadClassDictFreeVar(AlifCompiler* _c, Location _loc) { // 3997
	ADDOP_N(_c, _loc, LOAD_DEREF, &ALIF_ID(__classDict__), freevars);
	return SUCCESS;
}


enum CompilerOpType { OP_FAST, OP_GLOBAL, OP_DEREF, OP_NAME }; // 4031

static AlifIntT compiler_resolveNameOp(AlifCompiler*, AlifObject*,
	AlifIntT, CompilerOpType*, AlifSizeT*); // 4033


static AlifIntT codegen_nameOp(AlifCompiler* c, Location loc,
	Identifier name, ExprContext_ ctx) { // 4083

	AlifObject* mangled = compiler_maybeMangle(c, name);
	if (!mangled) {
		return ERROR;
	}

	AlifIntT scope = alifST_getScope(SYMTABLE_ENTRY(c), mangled);
	RETURN_IF_ERROR(scope);
	CompilerOpType optype{};
	AlifSizeT arg = 0;
	if (compiler_resolveNameOp(c, mangled, scope, &optype, &arg) < 0) {
		ALIF_DECREF(mangled);
		return ERROR;
	}


	AlifIntT op = 0;
	switch (optype) {
	case CompilerOpType::OP_DEREF:
		switch (ctx) {
		case ExprContext_::Load:
			if (SYMTABLE_ENTRY(c)->type == BlockType_::Class_Block
				and !compiler_isInInlinedComp(c)) {
				op = LOAD_FROM_DICT_OR_DEREF;
				// First load the locals
				if (codegen_addOpNoArg(INSTR_SEQUENCE(c), LOAD_LOCALS, loc) < 0) {
					goto error;
				}
			}
			else if (SYMTABLE_ENTRY(c)->canSeeClassScope) {
				op = LOAD_FROM_DICT_OR_DEREF;
				// First load the classdict
				if (codegen_loadClassDictFreeVar(c, loc) < 0) {
					goto error;
				}
			}
			else {
				op = LOAD_DEREF;
			}
			break;
		case ExprContext_::Store: op = STORE_DEREF; break;
		case ExprContext_::Del: op = DELETE_DEREF; break;
		}
		break;
	case CompilerOpType::OP_FAST:
		switch (ctx) {
		case ExprContext_::Load: op = LOAD_FAST; break;
		case ExprContext_::Store: op = STORE_FAST; break;
		case ExprContext_::Del: op = DELETE_FAST; break;
		}
		ADDOP_N(c, loc, op, mangled, varnames);
		return SUCCESS;
	case CompilerOpType::OP_GLOBAL:
		switch (ctx) {
		case ExprContext_::Load:
			if (SYMTABLE_ENTRY(c)->canSeeClassScope and scope == GLOBAL_IMPLICIT) {
				op = LOAD_FROM_DICT_OR_GLOBALS;
				// First load the classdict
				if (codegen_loadClassDictFreeVar(c, loc) < 0) {
					goto error;
				}
			}
			else {
				op = LOAD_GLOBAL;
			}
			break;
		case ExprContext_::Store: op = STORE_GLOBAL; break;
		case ExprContext_::Del: op = DELETE_GLOBAL; break;
		}
		break;
	case CompilerOpType::OP_NAME:
		switch (ctx) {
		case ExprContext_::Load:
			op = (SYMTABLE_ENTRY(c)->type == BlockType_::Class_Block
				and compiler_isInInlinedComp(c))
				? LOAD_GLOBAL
				: LOAD_NAME;
			break;
		case ExprContext_::Store: op = STORE_NAME; break;
		case ExprContext_::Del: op = DELETE_NAME; break;
		}
		break;
	}

	ALIF_DECREF(mangled);
	if (op == LOAD_GLOBAL) {
		arg <<= 1;
	}
	ADDOP_I(c, loc, op, arg);
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

static AlifIntT codegen_compare(AlifCompiler* _c, ExprTy _e) { // 4438
	Location loc = LOC(_e);
	AlifSizeT i{}, n{};

	RETURN_IF_ERROR(codegen_checkCompare(_c, _e));
	VISIT(_c, Expr, _e->V.compare.left);
	n = ASDL_SEQ_LEN(_e->V.compare.ops) - 1;
	if (n == 0) {
		VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(_e->V.compare.comparators, 0));
		ADDOP_COMPARE(_c, loc, ASDL_SEQ_GET(_e->V.compare.ops, 0));
	}
	else {
		NEW_JUMP_TARGET_LABEL(_c, cleanup);
		for (i = 0; i < n; i++) {
			VISIT(_c, Expr,
				(ExprTy)ASDL_SEQ_GET(_e->V.compare.comparators, i));
			ADDOP_I(_c, loc, SWAP, 2);
			ADDOP_I(_c, loc, COPY, 2);
			ADDOP_COMPARE(_c, loc, ASDL_SEQ_GET(_e->V.compare.ops, i));
			ADDOP_I(_c, loc, COPY, 1);
			ADDOP(_c, loc, TO_BOOL);
			ADDOP_JUMP(_c, loc, POP_JUMP_IF_FALSE, cleanup);
			ADDOP(_c, loc, POP_TOP);
		}
		VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(_e->V.compare.comparators, n));
		ADDOP_COMPARE(_c, loc, ASDL_SEQ_GET(_e->V.compare.ops, n));
		NEW_JUMP_TARGET_LABEL(_c, end);
		ADDOP_JUMP(_c, _noLocation_, JUMP_NO_INTERRUPT, end);

		USE_LABEL(_c, cleanup);
		ADDOP_I(_c, loc, SWAP, 2);
		ADDOP(_c, loc, POP_TOP);

		USE_LABEL(_c, end);
	}
	return SUCCESS;
}

static AlifTypeObject* infer_type(ExprTy _e) { // 4480
	switch (_e->type) {
	case ExprK_::TupleK:
		return &_alifTupleType_;
	case ExprK_::ListK:
	case ExprK_::ListCompK:
		return &_alifListType_;
	case ExprK_::DictK:
	case ExprK_::DictCompK:
		return &_alifDictType_;
	case ExprK_::SetK:
	case ExprK_::SetCompK:
		return &_alifSetType_;
	//case ExprK_::GeneratorExpK:
	//	return &_alifGenType_;
	//case ExprK_::LambdaK:
	//	return &_alifFunctionType_;
	case ExprK_::JoinStrK:
	case ExprK_::FormattedValK:
		return &_alifUStrType_;
	case ExprK_::ConstantK:
		return ALIF_TYPE(_e->V.constant.val);
	default:
		return nullptr;
	}
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
		return -1; //* alif
	}
	default:
		return SUCCESS;
	}
}

static AlifIntT check_subScripter(AlifCompiler* _c, ExprTy _e) { // 4534
	AlifObject* v{};

	switch (_e->type) {
	case ExprK_::ConstantK:
		v = _e->V.constant.val;
		if (!(v == ALIF_NONE or v == ALIF_ELLIPSIS or
			ALIFLONG_CHECK(v) or ALIFFLOAT_CHECK(v) or ALIFCOMPLEX_CHECK(v) or
			ALIFANYSET_CHECK(v)))
		{
			return SUCCESS;
		}
		ALIF_FALLTHROUGH;
	case ExprK_::SetK:
	case ExprK_::SetCompK: {
	//case ExprK_::GeneratorExpK:
	//case ExprK_::LambdaK: {
		Location loc = LOC(_e);
		//return compiler_warn(_c, loc, "'%.200s' object is not subscriptable; "
		//	"perhaps you missed a comma?",
		//	infer_type(_e)->name);
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

static AlifIntT check_index(AlifCompiler* _c, ExprTy _e, ExprTy _s) { // 4563
	AlifObject* v{};

	AlifTypeObject* index_type = infer_type(_s);
	if (index_type == nullptr
		or ALIFTYPE_FASTSUBCLASS(index_type, ALIF_TPFLAGS_LONG_SUBCLASS)
		or index_type == &_alifSliceType_) {
		return SUCCESS;
	}

	switch (_e->type) {
	case ExprK_::ConstantK:
		v = _e->V.constant.val;
		if (!(ALIFUSTR_CHECK(v) or ALIFBYTES_CHECK(v) or ALIFTUPLE_CHECK(v))) {
			return SUCCESS;
		}
		ALIF_FALLTHROUGH;
	case ExprK_::TupleK:
	case ExprK_::ListK:
	case ExprK_::ListCompK:
	case ExprK_::JoinStrK:
	case ExprK_::FormattedValK: {
		Location loc = LOC(_e);
		//return compiler_warn(_c, loc, "%.200s indices must be integers "
		//	"or slices, not %.200s; "
		//	"perhaps you missed a comma?",
		//	infer_type(_e)->name,
		//	index_type->name);
	}
	default:
		return SUCCESS;
	}
}

static AlifIntT canOptimize_superCall(AlifCompiler* _c, ExprTy _attr) { // 4617
	ExprTy e = _attr->V.attribute.val;
	if (e->type != ExprK_::CallK or
		e->V.call.func->type != ExprK_::NameK or
		!alifUStr_equalToASCIIString(e->V.call.func->V.name.name, "super") ||
		alifUStr_equalToASCIIString(_attr->V.attribute.attr, "__class__") ||
		ASDL_SEQ_LEN(e->V.call.keywords) != 0) {
		return 0;
	}
	AlifSizeT num_args = ASDL_SEQ_LEN(e->V.call.args);

	AlifObject* super_name = e->V.call.func->V.name.name;
	// detect statically-visible shadowing of 'super' name
	AlifIntT scope = alifST_getScope(SYMTABLE_ENTRY(_c), super_name);
	RETURN_IF_ERROR(scope);
	if (scope != GLOBAL_IMPLICIT) {
		return 0;
	}
	scope = alifST_getScope(SYMTABLE(_c)->top, super_name);
	RETURN_IF_ERROR(scope);
	if (scope != 0) {
		return 0;
	}

	if (num_args == 2) {
		for (AlifSizeT i = 0; i < num_args; i++) {
			ExprTy elt = ASDL_SEQ_GET(e->V.call.args, i);
			if (elt->type == ExprK_::StarK) {
				return 0;
			}
		}
		// exactly two non-starred args; we can just load
		// the provided args
		return 1;
	}

	if (num_args != 0) {
		return 0;
	}
	// we need the following for zero-arg super():

	// enclosing function should have at least one argument
	if (METADATA(_c)->argCount == 0 and
		METADATA(_c)->posOnlyArgCount == 0) {
		return 0;
	}
	// __class__ cell should be available
	if (compiler_getRefType(_c, &ALIF_ID(__class__)) == FREE) {
		return 1;
	}
	return 0;
}

static AlifIntT loadArgs_forSuper(AlifCompiler* _c, ExprTy _e) { // 4672
	Location loc = LOC(_e);

	// load super() global
	AlifObject* super_name = _e->V.call.func->V.name.name;
	RETURN_IF_ERROR(codegen_nameOp(_c, LOC(_e->V.call.func), super_name, ExprContext_::Load));

	if (ASDL_SEQ_LEN(_e->V.call.args) == 2) {
		VISIT(_c, Expr, ASDL_SEQ_GET(_e->V.call.args, 0));
		VISIT(_c, Expr, ASDL_SEQ_GET(_e->V.call.args, 1));
		return SUCCESS;
	}

	// load __class__ cell
	AlifObject* name = &ALIF_ID(__class__);
	RETURN_IF_ERROR(codegen_nameOp(_c, loc, name, ExprContext_::Load));

	// load self (first argument)
	AlifSizeT i = 0;
	AlifObject* key{}, * value{};
	if (!alifDict_next(METADATA(_c)->varnames, &i, &key, &value)) {
		return ERROR;
	}
	RETURN_IF_ERROR(codegen_nameOp(_c, loc, key, ExprContext_::Load));

	return SUCCESS;
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


static AlifIntT compiler_comprehensionGenerator(AlifCompiler* c, Location loc,
	ASDLComprehensionSeq* generators, AlifIntT gen_index, AlifIntT depth,
	ExprTy elt, ExprTy val, AlifIntT type, AlifIntT iter_on_stack) { // 5085
	ComprehensionTy gen{};
	gen = (ComprehensionTy)ASDL_SEQ_GET(generators, gen_index);
	if (gen->isAsync) {
		return codegen_asyncComprehensionGenerator(
			c, loc, generators, gen_index, depth, elt, val, type,
			iter_on_stack);
	}
	else {
		return codegen_syncComprehensionGenerator(
			c, loc, generators, gen_index, depth, elt, val, type,
			iter_on_stack);
	}
}

static AlifIntT codegen_syncComprehensionGenerator(AlifCompiler* _c, Location _loc,
	ASDLComprehensionSeq* _generators, AlifIntT _genIndex, AlifIntT _depth,
	ExprTy _elt, ExprTy _val, AlifIntT _type, AlifIntT _iterOnStack) { // 5105

	NEW_JUMP_TARGET_LABEL(_c, start);
	NEW_JUMP_TARGET_LABEL(_c, if_cleanup);
	NEW_JUMP_TARGET_LABEL(_c, anchor);

	ComprehensionTy gen = (ComprehensionTy)ASDL_SEQ_GET(_generators,
		_genIndex);

	if (!_iterOnStack) {
		if (_genIndex == 0) {
			ADDOP_I(_c, _loc, LOAD_FAST, 0);
		}
		else {
			ASDLExprSeq* elts{};
			switch (gen->iter->type) {
			case ExprK_::ListK:
				elts = gen->iter->V.list.elts;
				break;
			case ExprK_::TupleK:
				elts = gen->iter->V.tuple.elts;
				break;
			default:
				elts = nullptr;
			}
			if (ASDL_SEQ_LEN(elts) == 1) {
				ExprTy elt = ASDL_SEQ_GET(elts, 0);
				if (elt->type != ExprK_::StarK) {
					VISIT(_c, Expr, elt);
					start = _noLabel_;
				}
			}
			if (IS_LABEL(start)) {
				VISIT(_c, Expr, gen->iter);
				ADDOP(_c, LOC(gen->iter), GET_ITER);
			}
		}
	}

	if (IS_LABEL(start)) {
		_depth++;
		USE_LABEL(_c, start);
		ADDOP_JUMP(_c, LOC(gen->iter), FOR_ITER, anchor);
	}
	VISIT(_c, Expr, gen->target);

	AlifSizeT n = ASDL_SEQ_LEN(gen->ifs);
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy e = (ExprTy)ASDL_SEQ_GET(gen->ifs, i);
		RETURN_IF_ERROR(codegen_jumpIf(_c, _loc, e, if_cleanup, 0));
	}

	if (++_genIndex < ASDL_SEQ_LEN(_generators)) {
		RETURN_IF_ERROR(
			compiler_comprehensionGenerator(_c, _loc,
				_generators, _genIndex, _depth,
				_elt, _val, _type, 0));
	}

	Location elt_loc = LOC(_elt);

	if (_genIndex >= ASDL_SEQ_LEN(_generators)) {
		switch (_type) {
		case COMP_GENEXP:
			VISIT(_c, Expr, _elt);
			ADDOP_YIELD(_c, elt_loc);
			ADDOP(_c, elt_loc, POP_TOP);
			break;
		case COMP_LISTCOMP:
			VISIT(_c, Expr, _elt);
			ADDOP_I(_c, elt_loc, LIST_APPEND, _depth + 1);
			break;
		case COMP_SETCOMP:
			VISIT(_c, Expr, _elt);
			ADDOP_I(_c, elt_loc, SET_ADD, _depth + 1);
			break;
		case COMP_DICTCOMP:
			VISIT(_c, Expr, _elt);
			VISIT(_c, Expr, _val);
			elt_loc = LOCATION(_elt->lineNo,
				_val->endLineNo,
				_elt->colOffset,
				_val->endColOffset);
			ADDOP_I(_c, elt_loc, MAP_ADD, _depth + 1);
			break;
		default:
			return ERROR;
		}
	}

	USE_LABEL(_c, if_cleanup);
	if (IS_LABEL(start)) {
		ADDOP_JUMP(_c, elt_loc, JUMP, start);

		USE_LABEL(_c, anchor);
		/* It is important for instrumentation that the `END_FOR` comes first.
		* Iteration over a generator will jump to the first of these instructions,
		* but a non-generator will jump to a later instruction.
		*/
		ADDOP(_c, _noLocation_, END_FOR);
		ADDOP(_c, _noLocation_, POP_TOP);
	}

	return SUCCESS;
}


static AlifIntT codegen_asyncComprehensionGenerator(AlifCompiler* _c, Location _loc,
	ASDLComprehensionSeq* _generators, AlifIntT _genIndex, AlifIntT _depth,
	ExprTy _elt, ExprTy _val, AlifIntT _type, AlifIntT _iterOnStack) { // 5230
	NEW_JUMP_TARGET_LABEL(_c, start);
	NEW_JUMP_TARGET_LABEL(_c, except);
	NEW_JUMP_TARGET_LABEL(_c, if_cleanup);

	ComprehensionTy gen = (ComprehensionTy)ASDL_SEQ_GET(_generators,
		_genIndex);

	if (!_iterOnStack) {
		if (_genIndex == 0) {
			ADDOP_I(_c, _loc, LOAD_FAST, 0);
		}
		else {
			VISIT(_c, Expr, gen->iter);
			ADDOP(_c, LOC(gen->iter), GET_AITER);
		}
	}

	USE_LABEL(_c, start);
	RETURN_IF_ERROR(
		compiler_pushFBlock(_c, _loc, FBlockType_::Async_Comprehension_Generator,
			start, _noLabel_, nullptr));

	ADDOP_JUMP(_c, _loc, SETUP_FINALLY, except);
	ADDOP(_c, _loc, GET_ANEXT);
	ADDOP_LOAD_CONST(_c, _loc, ALIF_NONE);
	ADD_YIELD_FROM(_c, _loc, 1);
	ADDOP(_c, _loc, POP_BLOCK);
	VISIT(_c, Expr, gen->target);

	AlifSizeT n = ASDL_SEQ_LEN(gen->ifs);
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy e = (ExprTy)ASDL_SEQ_GET(gen->ifs, i);
		RETURN_IF_ERROR(codegen_jumpIf(_c, _loc, e, if_cleanup, 0));
	}

	_depth++;
	if (++_genIndex < ASDL_SEQ_LEN(_generators)) {
		RETURN_IF_ERROR(
			compiler_comprehensionGenerator(_c, _loc,
				_generators, _genIndex, _depth,
				_elt, _val, _type, 0));
	}

	Location elt_loc = LOC(_elt);
	if (_genIndex >= ASDL_SEQ_LEN(_generators)) {
		switch (_type) {
		case COMP_GENEXP:
			VISIT(_c, Expr, _elt);
			ADDOP_YIELD(_c, elt_loc);
			ADDOP(_c, elt_loc, POP_TOP);
			break;
		case COMP_LISTCOMP:
			VISIT(_c, Expr, _elt);
			ADDOP_I(_c, elt_loc, LIST_APPEND, _depth + 1);
			break;
		case COMP_SETCOMP:
			VISIT(_c, Expr, _elt);
			ADDOP_I(_c, elt_loc, SET_ADD, _depth + 1);
			break;
		case COMP_DICTCOMP:
			VISIT(_c, Expr, _elt);
			VISIT(_c, Expr, _val);
			elt_loc = LOCATION(_elt->lineNo,
				_val->endLineNo,
				_elt->colOffset,
				_val->endColOffset);
			ADDOP_I(_c, elt_loc, MAP_ADD, _depth + 1);
			break;
		default:
			return ERROR;
		}
	}

	USE_LABEL(_c, if_cleanup);
	ADDOP_JUMP(_c, elt_loc, JUMP, start);

	compiler_popFBlock(_c, FBlockType_::Async_Comprehension_Generator, start);

	USE_LABEL(_c, except);

	ADDOP(_c, _loc, END_ASYNC_FOR);

	return SUCCESS;
}


class InlinedComprehensionState { // 5330
public:
	AlifObject* pushedLocals{};
	AlifObject* tempSymbols{};
	AlifObject* fastHidden{};
	JumpTargetLabel cleanup{};
};


static AlifIntT codegen_pushInlinedComprehensionLocals(AlifCompiler* c, Location loc,
	SymTableEntry* comp, InlinedComprehensionState* state) { // 5417
	AlifIntT in_class_block = (SYMTABLE_ENTRY(c)->type == BlockType_::Class_Block)
		and !compiler_isInInlinedComp(c);
	SymTableEntry* outer = SYMTABLE_ENTRY(c);
	AlifObject* k{}, * v{};
	AlifSizeT pos = 0;
	while (alifDict_next(comp->symbols, &pos, &k, &v)) {
		long symbol = alifLong_asLong(v);
		RETURN_IF_ERROR(symbol);
		long scope = SYMBOL_TO_SCOPE(symbol);

		long outsymbol = alifST_getSymbol(outer, k);
		RETURN_IF_ERROR(outsymbol);
		long outsc = SYMBOL_TO_SCOPE(outsymbol);

		if ((symbol & DEF_LOCAL and !(symbol & DEF_NONLOCAL)) or in_class_block) {
			if (state->pushedLocals == nullptr) {
				state->pushedLocals = alifList_new(0);
				if (state->pushedLocals == nullptr) {
					return ERROR;
				}
			}
			ADDOP_NAME(c, loc, LOAD_FAST_AND_CLEAR, k, varnames);
			if (scope == CELL) {
				if (outsc == FREE) {
					ADDOP_NAME(c, loc, MAKE_CELL, k, freevars);
				}
				else {
					ADDOP_NAME(c, loc, MAKE_CELL, k, cellvars);
				}
			}
			if (alifList_append(state->pushedLocals, k) < 0) {
				return ERROR;
			}
		}
	}
	if (state->pushedLocals) {
		ADDOP_I(c, loc, SWAP, ALIFLIST_GET_SIZE(state->pushedLocals) + 1);

		NEW_JUMP_TARGET_LABEL(c, cleanup);
		state->cleanup = cleanup;

		ADDOP_JUMP(c, loc, SETUP_FINALLY, cleanup);
	}
	return SUCCESS;
}

static AlifIntT compiler_tweakInlinedComprehensionScopes(AlifCompiler*, Location,
	SymTableEntry*, InlinedComprehensionState*); // 4696

static AlifIntT pushInlined_comprehensionState(AlifCompiler* c, Location loc,
	SymTableEntry* comp, InlinedComprehensionState* state) { // 5484
	RETURN_IF_ERROR(
		compiler_tweakInlinedComprehensionScopes(c, loc, comp, state));
	RETURN_IF_ERROR(
		codegen_pushInlinedComprehensionLocals(c, loc, comp, state));
	return SUCCESS;
}

static AlifIntT restoreInlined_comprehensionLocals(AlifCompiler* c, Location loc,
	InlinedComprehensionState* state) { // 5496
	AlifObject* k{};
	AlifSizeT npops = ALIFLIST_GET_SIZE(state->pushedLocals);
	ADDOP_I(c, loc, SWAP, npops + 1);
	for (AlifSizeT i = npops - 1; i >= 0; --i) {
		k = alifList_getItem(state->pushedLocals, i);
		if (k == nullptr) {
			return ERROR;
		}
		ADDOP_NAME(c, loc, STORE_FAST_MAYBE_NULL, k, varnames);
	}
	return SUCCESS;
}

static AlifIntT codegen_popInlinedComprehensionLocals(AlifCompiler* c, Location loc,
	InlinedComprehensionState* state) { // 5518
	if (state->pushedLocals) {
		ADDOP(c, _noLocation_, POP_BLOCK);

		NEW_JUMP_TARGET_LABEL(c, end);
		ADDOP_JUMP(c, _noLocation_, JUMP_NO_INTERRUPT, end);

		// cleanup from an exception inside the comprehension
		USE_LABEL(c, state->cleanup);
		// discard incomplete comprehension result (beneath exc on stack)
		ADDOP_I(c, _noLocation_, SWAP, 2);
		ADDOP(c, _noLocation_, POP_TOP);
		RETURN_IF_ERROR(restoreInlined_comprehensionLocals(c, loc, state));
		ADDOP_I(c, _noLocation_, RERAISE, 0);

		USE_LABEL(c, end);
		RETURN_IF_ERROR(restoreInlined_comprehensionLocals(c, loc, state));
		ALIF_CLEAR(state->pushedLocals);
	}
	return SUCCESS;
}

static AlifIntT compiler_revertInlinedComprehensionScopes(AlifCompiler*, Location,
	InlinedComprehensionState*); // 5543

static AlifIntT popInlined_comprehensionState(AlifCompiler* c,
	Location loc, InlinedComprehensionState* state) { // 5576
	c->u_->inInlinedComp--;
	RETURN_IF_ERROR(codegen_popInlinedComprehensionLocals(c, loc, state));
	RETURN_IF_ERROR(compiler_revertInlinedComprehensionScopes(c, loc, state));
	return SUCCESS;
}


static inline AlifIntT codegen_comprehensionIter(AlifCompiler* c, Location loc,
	ComprehensionTy comp) { // 5586
	VISIT(c, Expr, comp->iter);
	if (comp->isAsync) {
		ADDOP(c, loc, GET_AITER);
	}
	else {
		ADDOP(c, loc, GET_ITER);
	}
	return SUCCESS;
}


static AlifIntT compiler_comprehension(AlifCompiler* _c, ExprTy _e, AlifIntT _type, Identifier _name,
	ASDLComprehensionSeq* _generators, ExprTy _elt, ExprTy _val) { // 5600

	AlifIntT isInlined{}; //* alif
	AlifIntT isAsyncComprehension{}; //* alif
	Location loc{}; //* alif

	AlifCodeObject* co = nullptr;
	InlinedComprehensionState inline_state = { nullptr, nullptr, nullptr, _noLabel_ };
	ComprehensionTy outermost{};
	SymTableEntry* entry = _alifSymtable_lookup(SYMTABLE(_c), (void*)_e);
	if (entry == nullptr) {
		goto error;
	}
	isInlined = entry->compInlined;
	isAsyncComprehension = entry->coroutine;

	loc = LOC(_e);

	outermost = (ComprehensionTy)ASDL_SEQ_GET(_generators, 0);
	if (isInlined) {
		if (codegen_comprehensionIter(_c, loc, outermost)) {
			goto error;
		}
		if (pushInlined_comprehensionState(_c, loc, entry, &inline_state)) {
			goto error;
		}
	}
	else {
		/* Receive outermost iter as an implicit argument */
		AlifCompileCodeUnitMetadata umd = {
			.argCount = 1,
		};
		if (compiler_enterScope(_c, _name, ScopeType_::Compiler_Scope_Comprehension,
			(void*)_e, _e->lineNo, nullptr, &umd) < 0) {
			goto error;
		}
	}
	ALIF_CLEAR(entry);

	if (_type != COMP_GENEXP) {
		AlifIntT op{};
		switch (_type) {
		case COMP_LISTCOMP:
			op = BUILD_LIST;
			break;
		case COMP_SETCOMP:
			op = BUILD_SET;
			break;
		case COMP_DICTCOMP:
			op = BUILD_MAP;
			break;
		default:
			//alifErr_format(_alifExcSystemError_,
			//	"unknown comprehension type %d", type);
			goto error_in_scope;
		}

		ADDOP_I(_c, loc, op, 0);
		if (isInlined) {
			ADDOP_I(_c, loc, SWAP, 2);
		}
	}

	if (compiler_comprehensionGenerator(_c, loc, _generators, 0, 0,
		_elt, _val, _type, isInlined) < 0) {
		goto error_in_scope;
	}

	if (isInlined) {
		if (popInlined_comprehensionState(_c, loc, &inline_state)) {
			goto error;
		}
		return SUCCESS;
	}

	if (_type != COMP_GENEXP) {
		ADDOP(_c, LOC(_e), RETURN_VALUE);
	}
	if (_type == COMP_GENEXP) {
		if (codegen_wrapInStopIterationHandler(_c) < 0) {
			goto error_in_scope;
		}
	}

	co = optimize_andAssemble(_c, 1);
	compiler_exitScope(_c);
	if (co == nullptr) {
		goto error;
	}

	loc = LOC(_e);
	if (codegen_makeClosure(_c, loc, co, 0) < 0) {
		goto error;
	}
	ALIF_CLEAR(co);

	if (codegen_comprehensionIter(_c, loc, outermost)) {
		goto error;
	}

	ADDOP_I(_c, loc, CALL, 0);

	if (isAsyncComprehension && _type != COMP_GENEXP) {
		ADDOP_I(_c, loc, GET_AWAITABLE, 0);
		ADDOP_LOAD_CONST(_c, loc, ALIF_NONE);
		ADD_YIELD_FROM(_c, loc, 1);
	}

	return SUCCESS;
error_in_scope:
	if (!isInlined) {
		compiler_exitScope(_c);
	}
error:
	ALIF_XDECREF(co);
	ALIF_XDECREF(entry);
	ALIF_XDECREF(inline_state.pushedLocals);
	ALIF_XDECREF(inline_state.tempSymbols);
	ALIF_XDECREF(inline_state.fastHidden);
	return ERROR;
}


static AlifIntT codegen_listComp(AlifCompiler* _c, ExprTy _e) { // 5737
	return compiler_comprehension(_c, _e, COMP_LISTCOMP, &ALIF_STR(AnonListComp),
		_e->V.listComp.generators,
		_e->V.listComp.elt, nullptr);
}


static AlifIntT codegen_visitKeyword(AlifCompiler* _c, KeywordTy _k) { // 5769
	VISIT(_c, Expr, _k->val);
	return SUCCESS;
}


static AlifIntT codegen_visitExpr(AlifCompiler* _c, ExprTy _e) { // 5997
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
	case ExprK_::ListCompK:
		return codegen_listComp(_c, _e);
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
	//	if (SCOPE_TYPE(_c) == COMPILER_SCOPE_ASYNC_FUNCTION) {
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
	case ExprK_::CompareK:
		return codegen_compare(_c, _e);
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
	case ExprK_::AttributeK:
		if (_e->V.attribute.ctx == ExprContext_::Load) {
			AlifIntT ret = canOptimize_superCall(_c, _e);
			RETURN_IF_ERROR(ret);
			if (ret) {
				RETURN_IF_ERROR(loadArgs_forSuper(_c, _e->V.attribute.val));
				AlifIntT opcode = ASDL_SEQ_LEN(_e->V.attribute.val->V.call.args) ?
					LOAD_SUPER_ATTR : LOAD_ZERO_SUPER_ATTR;
				ADDOP_NAME(_c, loc, opcode, _e->V.attribute.attr, names);
				loc = updateStartLocation_toMatchAttr(_c, loc, _e);
				ADDOP(_c, loc, NOP);
				return SUCCESS;
			}
		}
		RETURN_IF_ERROR(compiler_maybeAddStaticAttributeToClass(_c, _e));
		VISIT(_c, Expr, _e->V.attribute.val);
		loc = LOC(_e);
		loc = updateStartLocation_toMatchAttr(_c, loc, _e);
		switch (_e->V.attribute.ctx) {
		case ExprContext_::Load:
			ADDOP_NAME(_c, loc, LOAD_ATTR, _e->V.attribute.attr, names);
			break;
		case ExprContext_::Store:
			ADDOP_NAME(_c, loc, STORE_ATTR, _e->V.attribute.attr, names);
			break;
		case ExprContext_::Del:
			ADDOP_NAME(_c, loc, DELETE_ATTR, _e->V.attribute.attr, names);
			break;
		}
		break;
	case ExprK_::SubScriptK:
		return codegen_subScript(_c, _e);
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
	case ExprK_::SliceK:
	{
		AlifIntT n = codegen_slice(_c, _e);
		RETURN_IF_ERROR(n);
		ADDOP_I(_c, loc, BUILD_SLICE, n);
		break;
	}
	case ExprK_::NameK:
		return codegen_nameOp(_c, loc, _e->V.name.name, _e->V.name.ctx);
		/* child nodes of List and Tuple will have expr_context set */
	case ExprK_::ListK:
		return codegen_list(_c, _e);
	case ExprK_::TupleK:
		return codegen_tuple(_c, _e);
	}
	return SUCCESS;
}

static bool is_twoElementSlice(ExprTy _s) { // 6152
	return _s->type == ExprK_::SliceK and
		_s->V.slice.step == nullptr;
}

static AlifIntT codegen_augAssign(AlifCompiler* _c, StmtTy _s) { // 6159
	ExprTy e = _s->V.augAssign.target;

	Location loc = LOC(e);

	switch (e->type) {
	case ExprK_::AttributeK:
		VISIT(_c, Expr, e->V.attribute.val);
		ADDOP_I(_c, loc, COPY, 1);
		loc = updateStartLocation_toMatchAttr(_c, loc, e);
		ADDOP_NAME(_c, loc, LOAD_ATTR, e->V.attribute.attr, names);
		break;
	case ExprK_::SubScriptK:
		VISIT(_c, Expr, e->V.subScript.val);
		if (is_twoElementSlice(e->V.subScript.slice)) {
			RETURN_IF_ERROR(codegen_slice(_c, e->V.subScript.slice));
			ADDOP_I(_c, loc, COPY, 3);
			ADDOP_I(_c, loc, COPY, 3);
			ADDOP_I(_c, loc, COPY, 3);
			ADDOP(_c, loc, BINARY_SLICE);
		}
		else {
			VISIT(_c, Expr, e->V.subScript.slice);
			ADDOP_I(_c, loc, COPY, 2);
			ADDOP_I(_c, loc, COPY, 2);
			ADDOP(_c, loc, BINARY_SUBSCR);
		}
		break;
	case ExprK_::NameK:
		RETURN_IF_ERROR(codegen_nameOp(_c, loc, e->V.name.name, ExprContext_::Load));
		break;
	default:
		//alifErr_format(_alifExcSystemError_,
		//	"invalid node type (%d) for augmented assignment",
		//	e->type);
		return ERROR;
	}

	loc = LOC(_s);

	VISIT(_c, Expr, _s->V.augAssign.val);
	ADDOP_INPLACE(_c, loc, _s->V.augAssign.op);

	loc = LOC(e);

	switch (e->type) {
	case ExprK_::AttributeK:
		loc = updateStartLocation_toMatchAttr(_c, loc, e);
		ADDOP_I(_c, loc, SWAP, 2);
		ADDOP_NAME(_c, loc, STORE_ATTR, e->V.attribute.attr, names);
		break;
	case ExprK_::SubScriptK:
		if (is_twoElementSlice(e->V.subScript.slice)) {
			ADDOP_I(_c, loc, SWAP, 4);
			ADDOP_I(_c, loc, SWAP, 3);
			ADDOP_I(_c, loc, SWAP, 2);
			ADDOP(_c, loc, STORE_SLICE);
		}
		else {
			ADDOP_I(_c, loc, SWAP, 3);
			ADDOP_I(_c, loc, SWAP, 2);
			ADDOP(_c, loc, STORE_SUBSCR);
		}
		break;
	case ExprK_::NameK:
		return codegen_nameOp(_c, loc, e->V.name.name, ExprContext_::Store);
	default:
		ALIF_UNREACHABLE();
	}
	return SUCCESS;
}

static AlifIntT codegen_subScript(AlifCompiler* _c, ExprTy _e) { // 6427
	Location loc = LOC(_e);
	ExprContext_ ctx = _e->V.subScript.ctx;
	AlifIntT op = 0;

	if (ctx == ExprContext_::Load) {
		RETURN_IF_ERROR(check_subScripter(_c, _e->V.subScript.val));
		RETURN_IF_ERROR(check_index(_c, _e->V.subScript.val, _e->V.subScript.slice));
	}

	VISIT(_c, Expr, _e->V.subScript.val);
	if (is_twoElementSlice(_e->V.subScript.slice) and ctx != ExprContext_::Del) {
		RETURN_IF_ERROR(codegen_slice(_c, _e->V.subScript.slice));
		if (ctx == ExprContext_::Load) {
			ADDOP(_c, loc, BINARY_SLICE);
		}
		else {
			ADDOP(_c, loc, STORE_SLICE);
		}
	}
	else {
		VISIT(_c, Expr, _e->V.subScript.slice);
		switch (ctx) {
		case ExprContext_::Load:    op = BINARY_SUBSCR; break;
		case ExprContext_::Store:   op = STORE_SUBSCR; break;
		case ExprContext_::Del:     op = DELETE_SUBSCR; break;
		}
		ADDOP(_c, loc, op);
	}
	return SUCCESS;
}

static AlifIntT codegen_slice(AlifCompiler* _c, ExprTy _s) { // 6465
	AlifIntT n = 2;

	if (_s->V.slice.lower) {
		VISIT(_c, Expr, _s->V.slice.lower);
	}
	else {
		ADDOP_LOAD_CONST(_c, LOC(_s), ALIF_NONE);
	}

	if (_s->V.slice.upper) {
		VISIT(_c, Expr, _s->V.slice.upper);
	}
	else {
		ADDOP_LOAD_CONST(_c, LOC(_s), ALIF_NONE);
	}

	if (_s->V.slice.step) {
		n++;
		VISIT(_c, Expr, _s->V.slice.step);
	}
	return n;
}

static AlifIntT codegen_addReturnAtEnd(AlifCompiler* _c, AlifIntT _addNone) { // 6429
	/* Make sure every instruction stream that falls off the end returns None.
	 * This also ensures that no jump target offsets are out of bounds.
	 */
	if (_addNone) {
		ADDOP_LOAD_CONST(_c, _noLocation_, ALIF_NONE);
	}
	ADDOP(_c, _noLocation_, RETURN_VALUE);
	return SUCCESS;
}


/*** end of CODEGEN, start of compiler implementation ***/



static AlifIntT compiler_setup(AlifCompiler* _c, ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _flags, AlifIntT _optimize, AlifASTMem* _astMem) { // 6445
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
		if (!alifErr_occurred()) {
			//alifErr_setString(_alifExcSystemError_, "no symtable");
		}
		return ERROR;
	}
	return SUCCESS;
}

static AlifCompiler* new_compiler(ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _pflags, AlifIntT _optimize, AlifASTMem* _astMem) { // 6489
	AlifCompiler* compiler = (AlifCompiler*)alifMem_dataAlloc(sizeof(AlifCompiler));
	if (compiler == nullptr) {
		return nullptr;
	}
	if (compiler_setup(compiler, _mod, _filename, _pflags, _optimize, _astMem) < 0) {
		compiler_free(compiler);
		return nullptr;
	}
	return compiler;
}


AlifCodeObject* alifAST_compile(ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _pFlags, AlifIntT _optimize, AlifASTMem* _astMem) { // 6505

	AlifCompiler* compiler = new_compiler(_mod, _filename, _pFlags, _optimize, _astMem);
	if (compiler == nullptr) {
		return nullptr;
	}

	AlifCodeObject* co = compiler_mod(compiler, _mod);
	compiler_free(compiler);
	return co;
}



static void compiler_free(AlifCompiler* _c) { // 6540
	if (_c->st) alifSymtable_free(_c->st);
	ALIF_XDECREF(_c->filename);
	ALIF_XDECREF(_c->constCache);
	ALIF_XDECREF(_c->stack);
	alifMem_dataFree(_c);
}

static void compiler_unitFree(CompilerUnit* _u) { // 6551
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

#define CAPSULE_NAME "AlifCompile.cpp AlifCompiler unit" // 6570

static AlifIntT compiler_maybeAddStaticAttributeToClass(AlifCompiler* _c, ExprTy _e) { // 6572
	ExprTy attr_value = _e->V.attribute.val;
	if (attr_value->type != ExprK_::NameK or
		_e->V.attribute.ctx != ExprContext_::Store or
		!alifUStr_equalToASCIIString(attr_value->V.name.name, "self"))
	{
		return SUCCESS;
	}
	AlifSizeT stackSize = ALIFLIST_GET_SIZE(_c->stack);
	for (AlifSizeT i = stackSize - 1; i >= 0; i--) {
		AlifObject* capsule = ALIFLIST_GET_ITEM(_c->stack, i);
		CompilerUnit* u = (CompilerUnit*)alifCapsule_getPointer(
			capsule, CAPSULE_NAME);
		if (u->scopeType == ScopeType_::Compiler_Scope_Class) {
			RETURN_IF_ERROR(alifSet_add(u->staticAttributes, _e->V.attribute.attr));
			break;
		}
	}
	return SUCCESS;
}

static AlifIntT compiler_setQualname(AlifCompiler* _c) { // 6598
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


static AlifSizeT compiler_addConst(AlifCompiler* _c, AlifObject* _o) { // 6684
	AlifObject* key = merge_constsRecursive(_c->constCache, _o);
	if (key == nullptr) {
		return ERROR;
	}

	AlifSizeT arg = dict_addO(_c->u_->metadata.consts, key);
	ALIF_DECREF(key);
	return arg;
}

static AlifObject* list2dict(AlifObject* _list) { // 6697
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
	AlifIntT _flag, AlifSizeT _offset) { // 6731
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

static AlifIntT compiler_enterScope(AlifCompiler* _c, Identifier _name,
	AlifIntT _scopeType, void* _key, AlifIntT _lineno,
	AlifObject* _private, AlifCompileCodeUnitMetadata* _umd) { // 6797
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


static void compiler_exitScope(AlifCompiler* _c) { // 6931
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


/*
 * Frame block handling functions
 */

 static AlifIntT compiler_pushFBlock(AlifCompiler* _c, Location _loc,
 	FBlockType_ _t, JumpTargetLabel _blockLabel,
 	JumpTargetLabel _exit, void* _datum) { // 6974
 	FBlockInfo* f{};
 	if (_c->u_->nfBlocks >= MAXBLOCKS) {
 		//return compiler_error(_c, _loc, "too many statically nested blocks");
 	}
 	f = &_c->u_->fBlock[_c->u_->nfBlocks++];
 	f->type = _t;
 	f->block = _blockLabel;
 	f->loc = _loc;
 	f->exit = _exit;
 	f->datum = _datum;
 	return SUCCESS;
 }

static void compiler_popFBlock(AlifCompiler* _c,
	FBlockType_ _t, JumpTargetLabel _blockLabel) { // 6991
	CompilerUnit* u = _c->u_;
	u->nfBlocks--;
}

static FBlockInfo* compiler_topFBlock(AlifCompiler* _c) { // 7001
	if (_c->u_->nfBlocks == 0) {
		return nullptr;
	}
	return &_c->u_->fBlock[_c->u_->nfBlocks - 1];
}



static AlifIntT compiler_codegen(AlifCompiler* _c, ModuleTy _mod) { // 7017
	switch (_mod->type) {
	case ModK_::ModuleK: {
		ASDLStmtSeq* stmts = _mod->V.module.body;
		RETURN_IF_ERROR(codegen_body(_c, start_location(stmts), stmts));
		break;
	}
	case ModK_::InteractiveK: {
		_c->interactive = 1;
		ASDLStmtSeq* stmts = _mod->V.interactive.body;
		RETURN_IF_ERROR(codegen_body(_c, start_location(stmts), stmts));
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

static AlifCodeObject* compiler_mod(AlifCompiler* _c, ModuleTy _mod) { // 7046
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

static AlifIntT compiler_getRefType(AlifCompiler* _c, AlifObject* _name) { // 7062
	if (_c->u_->scopeType == ScopeType_::Compiler_Scope_Class and
		(alifUStr_equalToASCIIString(_name, "__class__") or
			alifUStr_equalToASCIIString(_name, "__classDict__"))) {
		return CELL;
	}
	SymTableEntry* ste = SYMTABLE_ENTRY(_c);
	AlifIntT scope = alifST_getScope(ste, _name);
	if (scope == 0) {
		//alifErr_format(_alifExcSystemError_,
		//	"alifST_getScope(name=%R) failed: "
		//	"unknown scope in unit %S (%R); "
		//	"symbols: %R; locals: %R; "
		//	"globals: %R",
		//	_name, _c->u_->metadata.name, ste->id,
		//	ste->symbols, _c->u_->metadata.varnames,
		//	_c->u_->metadata.names);
		return ERROR;
	}
	return scope;
}

static AlifIntT dict_lookupArg(AlifObject* _dict, AlifObject* _name) { // 7088
	AlifObject* v = alifDict_getItemWithError(_dict, _name);
	if (v == nullptr) {
		return ERROR;
	}
	return alifLong_asLong(v);
}

static AlifIntT compiler_lookupCellVar(AlifCompiler* _c, AlifObject* _name) { // 7098
	return dict_lookupArg(_c->u_->metadata.cellvars, _name);
}

static AlifIntT compiler_lookupArg(AlifCompiler* _c,
	AlifCodeObject* _co, AlifObject* _name) { // 7104
	AlifIntT reftype = compiler_getRefType(_c, _name);
	if (reftype == -1) {
		return ERROR;
	}
	AlifIntT arg{};
	if (reftype == CELL) {
		arg = dict_lookupArg(_c->u_->metadata.cellvars, _name);
	}
	else {
		arg = dict_lookupArg(_c->u_->metadata.freevars, _name);
	}
	if (arg == -1 and !alifErr_occurred()) {
		//AlifObject* freevars = _alifCode_getFreeVars(_co);
		//if (freevars == nullptr) {
		//	alifErr_clear();
		//}
		//alifErr_format(_alifExcSystemError_,
		//	"compiler_lookupArg(name=%R) with reftype=%d failed in %S; "
		//	"freevars of code %S: %R",
		//	_name, reftype, _c->u_->metadata.name, _co->name, freevars);
		//ALIF_DECREF(freevars);
		return ERROR;
	}
	return arg;
}

static AlifObject* compiler_staticAttributesTuple(AlifCompiler* _c) { // 7144
	return alifSequence_tuple(_c->u_->staticAttributes);
}

static AlifIntT compiler_resolveNameOp(AlifCompiler* _c, AlifObject* _mangled,
	AlifIntT _scope, CompilerOpType* _optype, AlifSizeT* _arg) { // 7150
	AlifObject* dict = _c->u_->metadata.names;
	*_optype = CompilerOpType::OP_NAME;

	switch (_scope) {
	case FREE:
		dict = _c->u_->metadata.freevars;
		*_optype = CompilerOpType::OP_DEREF;
		break;
	case CELL:
		dict = _c->u_->metadata.cellvars;
		*_optype = CompilerOpType::OP_DEREF;
		break;
	case LOCAL:
		if (alifST_isFunctionLike(SYMTABLE_ENTRY(_c))) {
			*_optype = CompilerOpType::OP_FAST;
		}
		else {
			AlifObject* item{};
			RETURN_IF_ERROR(alifDict_getItemRef(_c->u_->metadata.fasthidden, _mangled,
				&item));
			if (item == ALIF_TRUE) {
				*_optype = CompilerOpType::OP_FAST;
			}
			ALIF_XDECREF(item);
		}
		break;
	case GLOBAL_IMPLICIT:
		if (alifST_isFunctionLike(SYMTABLE_ENTRY(_c))) {
			*_optype = CompilerOpType::OP_GLOBAL;
		}
		break;
	case GLOBAL_EXPLICIT:
		*_optype = CompilerOpType::OP_GLOBAL;
		break;
	default:
		/* scope can be 0 */
		break;
	}
	if (*_optype != CompilerOpType::OP_FAST) {
		*_arg = dict_addO(dict, _mangled);
		RETURN_IF_ERROR(*_arg);
	}
	return SUCCESS;
}

static AlifIntT compiler_tweakInlinedComprehensionScopes(AlifCompiler* c, Location loc,
	SymTableEntry* entry, InlinedComprehensionState* state) { // 7200
	AlifIntT in_class_block = (SYMTABLE_ENTRY(c)->type == BlockType_::Class_Block)
		and !c->u_->inInlinedComp;
	c->u_->inInlinedComp++;

	AlifObject* k{}, * v{};
	AlifSizeT pos = 0;
	while (alifDict_next(entry->symbols, &pos, &k, &v)) {
		long symbol = alifLong_asLong(v);
		RETURN_IF_ERROR(symbol);
		long scope = SYMBOL_TO_SCOPE(symbol);

		long outsymbol = alifST_getSymbol(SYMTABLE_ENTRY(c), k);
		RETURN_IF_ERROR(outsymbol);
		long outsc = SYMBOL_TO_SCOPE(outsymbol);

		if ((scope != outsc && scope != FREE && !(scope == CELL && outsc == FREE))
			|| in_class_block) {
			if (state->tempSymbols == nullptr) {
				state->tempSymbols = alifDict_new();
				if (state->tempSymbols == nullptr) {
					return ERROR;
				}
			}

			if (alifDict_setItem(SYMTABLE_ENTRY(c)->symbols, k, v) < 0) {
				return ERROR;
			}
			AlifObject* outv = alifLong_fromLong(outsymbol);
			if (outv == nullptr) {
				return ERROR;
			}
			AlifIntT res = alifDict_setItem(state->tempSymbols, k, outv);
			ALIF_DECREF(outv);
			RETURN_IF_ERROR(res);
		}
		if ((symbol & DEF_LOCAL and !(symbol & DEF_NONLOCAL)) or in_class_block) {
			if (!alifST_isFunctionLike(SYMTABLE_ENTRY(c))) {
				AlifObject* orig{};
				if (alifDict_getItemRef(c->u_->metadata.fasthidden, k, &orig) < 0) {
					return ERROR;
				}
				if (orig != ALIF_TRUE) {
					if (alifDict_setItem(c->u_->metadata.fasthidden, k, ALIF_TRUE) < 0) {
						return ERROR;
					}
					if (state->fastHidden == nullptr) {
						state->fastHidden = alifSet_new(nullptr);
						if (state->fastHidden == nullptr) {
							return ERROR;
						}
					}
					if (alifSet_add(state->fastHidden, k) < 0) {
						return ERROR;
					}
				}
			}
		}
	}
	return SUCCESS;
}

static AlifIntT compiler_revertInlinedComprehensionScopes(AlifCompiler* _c, Location _loc,
	InlinedComprehensionState* _state) { // 7280
	if (_state->tempSymbols) {
		AlifObject* k{}, * v{};
		AlifSizeT pos = 0;
		while (alifDict_next(_state->tempSymbols, &pos, &k, &v)) {
			if (alifDict_setItem(SYMTABLE_ENTRY(_c)->symbols, k, v)) {
				return ERROR;
			}
		}
		ALIF_CLEAR(_state->tempSymbols);
	}
	if (_state->fastHidden) {
		while (alifSet_size(_state->fastHidden) > 0) {
			AlifObject* k = alifSet_pop(_state->fastHidden);
			if (k == nullptr) {
				return ERROR;
			}
			if (alifDict_setItem(_c->u_->metadata.fasthidden, k, ALIF_FALSE)) {
				ALIF_DECREF(k);
				return ERROR;
			}
			ALIF_DECREF(k);
		}
		ALIF_CLEAR(_state->fastHidden);
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

static SymTableEntry* compiler_symtableEntry(AlifCompiler* _c) { // 7376
	return _c->u_->ste;
}

static AlifIntT compiler_optimizationLevel(AlifCompiler* _c) { // 7381
	return _c->optimize;
}

static AlifIntT compiler_isInInlinedComp(AlifCompiler* _c) { // 7449
	return _c->u_->inInlinedComp;
}

static AlifObject* compiler_qualname(AlifCompiler* _c) { // 7455
	return _c->u_->metadata.qualname;
}


static AlifCompileCodeUnitMetadata* compiler_unitMetadata(AlifCompiler* _c) { // 7473
	return &_c->u_->metadata;
}

AlifIntT _alifCompile_constCacheMergeOne(AlifObject* _constCache,
	AlifObject** _obj) { // 7493

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


static AlifCodeObject* optimizeAndAssemble_codeUnit(CompilerUnit* u, AlifObject* const_cache,
	AlifIntT _codeFlags, AlifObject* filename) { // 7530
	CFGBuilder* g = nullptr;
	InstrSequence optimizedInstrs{};
	memset(&optimizedInstrs, 0, sizeof(InstrSequence)); //* review //* delete

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


static AlifIntT compute_codeFlags(AlifCompiler* _c) { // 7577
	SymTableEntry* ste = SYMTABLE_ENTRY(_c);
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

static AlifCodeObject* optimize_andAssemble(AlifCompiler* _c, AlifIntT _addNone) { // 7608
	CompilerUnit* u = _c->u_;
	AlifObject* constCache = _c->constCache;
	AlifObject* filename = _c->filename;

	AlifIntT codeFlags = compute_codeFlags(_c);
	if (codeFlags < 0) {
		return nullptr;
	}

	if (codegen_addReturnAtEnd(_c, _addNone) < 0) {
		return nullptr;
	}

	return optimizeAndAssemble_codeUnit(u, constCache, codeFlags, filename);
}
