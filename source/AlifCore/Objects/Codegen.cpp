#include "alif.h"

#include "Opcode.h"
#include "AlifCore_AST.h"
#define NEED_OPCODE_TABLES
#include "AlifCore_OpcodeUtils.h"
#undef NEED_OPCODE_TABLES
#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_Intrinsics.h"
#include "AlifCore_Long.h"
#include "AlifCore_State.h"
#include "AlifCore_SymTable.h"

#define NEED_OPCODE_METADATA
#include "AlifCore_OpcodeMetaData.h"
#undef NEED_OPCODE_METADATA
















#define COMP_GENEXP   0
#define COMP_LISTCOMP 1
#define COMP_SETCOMP  2
#define COMP_DICTCOMP 3








#define STACK_USE_GUIDELINE 30

#undef SUCCESS
#undef ERROR
#define SUCCESS 0
#define ERROR -1

#define RETURN_IF_ERROR(_x)  \
    if ((_x) == -1) {        \
        return ERROR;       \
    }

#define RETURN_IF_ERROR_IN_SCOPE(_c, _call) { \
    if (_call < 0) { \
        _alifCompiler_exitScope(_c); \
        return ERROR; \
    } \
}

class AlifCompiler;



#define INSTR_SEQUENCE(_c) _alifCompiler_instrSequence(_c)

#define SYMTABLE(_c) _alifCompiler_symTable(_c) 
#define SYMTABLE_ENTRY(_c) _alifCompiler_symTableEntry(_c)
#define OPTIMIZATION_LEVEL(_c) _alifCompiler_optimizationLevel(_c)



#define QUALNAME(_c) _alifCompiler_qualname(_c)
#define METADATA(_c) _alifCompiler_metadata(_c)


typedef AlifInstructionSequence InstrSequence;
typedef AlifSourceLocation Location;
typedef AlifJumpTargetLabel JumpTargetLabel;

typedef AlifCompileFBlockInfo FBlockInfo;

#define LOCATION(_lno, _endLno, _col, _endCol)	\
	AlifSourceLocation(_lno, _endLno, _col, _endCol)

#define LOC(_x) SRC_LOCATION_FROM_AST(_x)

#define NEW_JUMP_TARGET_LABEL(_c, _name) \
    JumpTargetLabel _name = _alifInstructionSequence_newLabel(INSTR_SEQUENCE(_c)); \
    if (!IS_JUMP_TARGET_LABEL(_name)) { \
        return ERROR; \
    }

#define USE_LABEL(_c, _lbl) \
    RETURN_IF_ERROR(_alifInstructionSequence_useLabel(INSTR_SEQUENCE(_c), _lbl.id))

static const AlifIntT _compareMasks_[] = {
	COMPARISON_LESS_THAN,
	COMPARISON_LESS_THAN | COMPARISON_EQUALS,
	COMPARISON_EQUALS,
	COMPARISON_NOT_EQUALS,
	COMPARISON_GREATER_THAN,
	COMPARISON_GREATER_THAN | COMPARISON_EQUALS,
};











AlifIntT _alifCompiler_ensureArrayLargeEnough(AlifIntT _idx, void** _array,
	AlifIntT * _alloc, AlifIntT _defaultAlloc, AlifUSizeT _itemSize) {
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





























static AlifIntT codegen_nameOp(AlifCompiler*, Location, Identifier, ExprContext_);

static AlifIntT codegen_visitStmt(AlifCompiler*, StmtTy);
static AlifIntT codegen_visitKeyword(AlifCompiler*, KeywordTy);
static AlifIntT codegen_visitExpr(AlifCompiler*, ExprTy);
static AlifIntT codegen_augAssign(AlifCompiler*, StmtTy);

static AlifIntT codegen_subScript(AlifCompiler*, ExprTy);
static AlifIntT codegen_slice(AlifCompiler*, ExprTy);

static bool areAllItems_const(ASDLExprSeq*, AlifSizeT, AlifSizeT);





static AlifIntT codegen_callSimpleKwHelper(AlifCompiler*, Location,
	ASDLKeywordSeq*, AlifSizeT);


static AlifIntT codegen_callHelperImpl(AlifCompiler*, Location,
	AlifIntT, ASDLExprSeq*, AlifObject*, ASDLKeywordSeq*);



static AlifIntT codegen_callHelper(AlifCompiler*, Location,
	AlifIntT, ASDLExprSeq*, ASDLKeywordSeq*);




static AlifIntT codegen_syncComprehensionGenerator(AlifCompiler*,
	Location, ASDLComprehensionSeq*, AlifIntT, AlifIntT,
	ExprTy, ExprTy, AlifIntT, AlifIntT);




static AlifIntT codegen_asyncComprehensionGenerator(AlifCompiler*,
	Location, ASDLComprehensionSeq*, AlifIntT, AlifIntT,
	ExprTy, ExprTy, AlifIntT, AlifIntT);








static AlifIntT codegen_makeClosure(AlifCompiler*, Location,
	AlifCodeObject*, AlifSizeT);



static AlifIntT codegen_addOpI(InstrSequence* seq,
	AlifIntT opcode, AlifSizeT oparg, Location loc) {

	AlifIntT oparg_ = ALIF_SAFE_DOWNCAST(oparg, AlifSizeT, AlifIntT);
	return _alifInstructionSequence_addOp(seq, opcode, oparg_, loc);
}










#define ADDOP_I(_c, _loc, _op, _o) \
    RETURN_IF_ERROR(codegen_addOpI(INSTR_SEQUENCE(_c), _op, _o, _loc))

#define ADDOP_I_IN_SCOPE(_c, _loc, _op, _o) \
    RETURN_IF_ERROR_IN_SCOPE(_c, codegen_addOpI(INSTR_SEQUENCE(_c), _op, _o, _loc));

static AlifIntT codegen_addOpNoArg(InstrSequence* _seq,
	AlifIntT _opcode, Location _loc) {
	return _alifInstructionSequence_addOp(_seq, _opcode, 0, _loc);
}




#define ADDOP(_c, _loc, _op) \
    RETURN_IF_ERROR(codegen_addOpNoArg(INSTR_SEQUENCE(_c), _op, _loc)) 

#define ADDOP_IN_SCOPE(_c, _loc, _op) \
    RETURN_IF_ERROR_IN_SCOPE(_c, codegen_addOpNoArg(INSTR_SEQUENCE(_c), _op, _loc))

static AlifIntT codegen_addOpLoadConst(AlifCompiler* _c,
	Location _loc, AlifObject* _o) {
	AlifSizeT arg = _alifCompiler_addConst(_c, _o);
	if (arg < 0) {
		return ERROR;
	}
	ADDOP_I(_c, _loc, LOAD_CONST, arg);
	return SUCCESS;
}


#define ADDOP_LOAD_CONST(_c, _loc, _o) \
    RETURN_IF_ERROR(codegen_addOpLoadConst(_c, _loc, _o))





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
	AlifIntT _opcode, AlifObject* _dict, AlifObject* _o) {
	AlifSizeT arg = _alifCompiler_dictAddObj(_dict, _o);
	RETURN_IF_ERROR(arg);
	ADDOP_I(_c, _loc, _opcode, arg);
	return SUCCESS;
}



#define ADDOP_N(_c, _loc, _op, _o, _type) { \
    AlifIntT ret = codegen_addOpO(_c, _loc, _op, METADATA(_c)->_type, _o); \
    ALIF_DECREF(_o); \
    RETURN_IF_ERROR(ret); \
}


#define ADDOP_N_IN_SCOPE(_c, _loc, _op, _o, _type) { \
    AlifIntT ret = codegen_addOpO(_c, _loc, _op, METADATA(_c)->_type, _o); \
    ALIF_DECREF(_o); \
    RETURN_IF_ERROR_IN_SCOPE(_c, ret); \
}


#define LOAD_METHOD -1
#define LOAD_SUPER_METHOD -2
#define LOAD_ZERO_SUPER_ATTR -3
#define LOAD_ZERO_SUPER_METHOD -4

static AlifIntT codegen_addOpName(AlifCompiler* _c, Location _loc,
	AlifIntT _opcode, AlifObject* _dict, AlifObject* _o) {
	AlifObject* mangled = _alifCompiler_maybeMangle(_c, _o);
	if (!mangled) {
		return ERROR;
	}
	AlifSizeT arg = _alifCompiler_dictAddObj(_dict, mangled);
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
    RETURN_IF_ERROR(codegen_addOpName(_c, _loc, _op, METADATA(_c)->_type, _o))

static AlifIntT codegen_addOpJ(InstrSequence* _seq, Location _loc,
	AlifIntT _opcode, JumpTargetLabel _target) {
	return _alifInstructionSequence_addOp(_seq, _opcode, _target.id, _loc);
}






#define ADDOP_JUMP(_c, _loc, _op, _o) \
    RETURN_IF_ERROR(codegen_addOpJ(INSTR_SEQUENCE(_c), _loc, _op, _o))

#define ADDOP_COMPARE(_c, _loc, _cmp) \
    RETURN_IF_ERROR(codegen_addCompare(_c, _loc, (CmpOp_)_cmp))

#define ADDOP_BINARY(_c, _loc, _binOp) \
    RETURN_IF_ERROR(addop_binary(_c, _loc, _binOp, false))

#define ADDOP_INPLACE(_c, _loc, _binOp) \
    RETURN_IF_ERROR(addop_binary(_c, _loc, _binOp, true))

#define ADD_YIELD_FROM(_c, _loc, _await) \
    RETURN_IF_ERROR(codegen_addYieldFrom(_c, _loc, _await))




#define ADDOP_YIELD(_c, _loc) \
    RETURN_IF_ERROR(codegen_addOpYield(_c, _loc))





#define VISIT(_c, _type, _v) \
    RETURN_IF_ERROR(codegen_visit ## _type(_c, _v));

#define VISIT_IN_SCOPE(_c, _type, _v) \
    RETURN_IF_ERROR_IN_SCOPE(_c, codegen_visit ## _type(_c, _v))

#define VISIT_SEQ(_c, _type, _sequ) { \
    AlifIntT i_{}; \
    ASDL ## _type ## Seq *_seq = (_sequ); /* avoid variable capture */ \
    for (i_ = 0; i_ < ASDL_SEQ_LEN(_seq); i_++) { \
        _type ## Ty elt = (_type ## Ty)ASDL_SEQ_GET(_seq, i_); \
        RETURN_IF_ERROR(codegen_visit ## _type(_c, elt)); \
    } \
}













static AlifIntT codegen_callExitWithNones(AlifCompiler* _c, Location _loc) {
	ADDOP_LOAD_CONST(_c, _loc, ALIF_NONE);
	ADDOP_LOAD_CONST(_c, _loc, ALIF_NONE);
	ADDOP_LOAD_CONST(_c, _loc, ALIF_NONE);
	ADDOP_I(_c, _loc, CALL, 3);
	return SUCCESS;
}



static AlifIntT codegen_addYieldFrom(AlifCompiler* _c,
	Location _loc, AlifIntT _await) {
	NEW_JUMP_TARGET_LABEL(_c, send);
	NEW_JUMP_TARGET_LABEL(_c, fail);
	NEW_JUMP_TARGET_LABEL(_c, exit);

	USE_LABEL(_c, send);
	ADDOP_JUMP(_c, _loc, SEND, exit);

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
	FBlockInfo* _info, AlifIntT _preserveTos) {
	switch (_info->type) {
	case AlifCompileFBlockType::Compiler_FBlock_While_Loop:
	case AlifCompileFBlockType::Compiler_FBlock_Exception_Handler:
	case AlifCompileFBlockType::Compiler_FBlock_Exception_Group_Handler:
	case AlifCompileFBlockType::Compiler_FBlock_Async_Comprehension_Generator:
	case AlifCompileFBlockType::Compiler_FBlock_Stop_Iteration:
		return SUCCESS;

	case AlifCompileFBlockType::Compiler_FBlock_For_Loop:
		/* Pop the iterator */
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		ADDOP(_c, *_ploc, POP_TOP);
		return SUCCESS;

	case AlifCompileFBlockType::Compiler_FBlock_Try_Except:
		ADDOP(_c, *_ploc, POP_BLOCK);
		return SUCCESS;

		/* This POP_BLOCK gets the line number of the unwinding statement */
	case AlifCompileFBlockType::Compiler_FBlock_Finally_Try:
		ADDOP(_c, *_ploc, POP_BLOCK);
		if (_preserveTos) {
			RETURN_IF_ERROR(
				_alifCompiler_pushFBlock(_c, *_ploc, AlifCompileFBlockType::Compiler_FBlock_Pop_Value, NO_LABEL, NO_LABEL, nullptr));
		}
		/* Emit the finally block */
		VISIT_SEQ(_c, Stmt, (ASDLStmtSeq*)_info->datum);
		if (_preserveTos) {
			_alifCompiler_popFBlock(_c, AlifCompileFBlockType::Compiler_FBlock_Pop_Value, NO_LABEL);
		}
		*_ploc = _noLocation_;
		return SUCCESS;

	case AlifCompileFBlockType::Compiler_FBlock_Finally_End:
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

	case AlifCompileFBlockType::Compiler_FBlock_With:
	case AlifCompileFBlockType::Compiler_FBlock_Async_With:
		*_ploc = _info->loc;
		ADDOP(_c, *_ploc, POP_BLOCK);
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 3);
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		RETURN_IF_ERROR(codegen_callExitWithNones(_c, *_ploc));
		if (_info->type == AlifCompileFBlockType::Compiler_FBlock_Async_With) {
			ADDOP_I(_c, *_ploc, GET_AWAITABLE, 2);
			ADDOP_LOAD_CONST(_c, *_ploc, ALIF_NONE);
			ADD_YIELD_FROM(_c, *_ploc, 1);
		}
		ADDOP(_c, *_ploc, POP_TOP);

		*_ploc = _noLocation_;
		return SUCCESS;

	case AlifCompileFBlockType::Compiler_FBlock_Handler_Cleanup: {
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
	case AlifCompileFBlockType::Compiler_FBlock_Pop_Value: {
		if (_preserveTos) {
			ADDOP_I(_c, *_ploc, SWAP, 2);
		}
		ADDOP(_c, *_ploc, POP_TOP);
		return SUCCESS;
	}
	}
	ALIF_UNREACHABLE();
}










static AlifIntT codegen_unwindFBlockStack(AlifCompiler* _c, Location* _ploc,
	AlifIntT _preserveTos, FBlockInfo** _loop) {
	FBlockInfo* top = _alifCompiler_topFBlock(_c);
	if (top == nullptr) {
		return SUCCESS;
	}
	if (top->type == AlifCompileFBlockType::Compiler_FBlock_Exception_Group_Handler) {
		//return _alifCompiler_error(
		//	_c, *_ploc, "'break', 'continue' and 'return' cannot appear in an except* block");
	}
	if (_loop != nullptr
		and (top->type == AlifCompileFBlockType::Compiler_FBlock_While_Loop
			or top->type == AlifCompileFBlockType::Compiler_FBlock_For_Loop)) {
		*_loop = top;
		return SUCCESS;
	}
	FBlockInfo copy = *top;
	_alifCompiler_popFBlock(_c, top->type, top->block);
	RETURN_IF_ERROR(codegen_unwindFBlock(_c, _ploc, &copy, _preserveTos));
	RETURN_IF_ERROR(codegen_unwindFBlockStack(_c, _ploc, _preserveTos, _loop));
	_alifCompiler_pushFBlock(_c, copy.loc, copy.type, copy.block,
		copy.exit, copy.datum);
	return SUCCESS;
}


static AlifIntT codegen_enterScope(AlifCompiler* _c, Identifier _name,
	AlifIntT _scopeType, void* _key, AlifIntT _lineno,
	AlifObject* _private, AlifCompileCodeUnitMetadata* _umd) {
	RETURN_IF_ERROR(
		_alifCompiler_enterScope(_c, _name, _scopeType, _key, _lineno, _private, _umd));
	Location loc = LOCATION(_lineno, _lineno, 0, 0);
	if (_scopeType == ScopeType_::Compiler_Scope_Module) {
		loc.lineNo = 0;
	}
	ADDOP_I(_c, loc, RESUME, RESUME_AT_FUNC_START);
	return SUCCESS;
}





























































































AlifIntT _alifCodegen_expression(AlifCompiler* _c, ExprTy _e) {
	VISIT(_c, Expr, _e);
	return SUCCESS;
}






AlifIntT _alifCodegen_body(AlifCompiler* _c,
	Location _loc, ASDLStmtSeq* _stmts, bool _isInteractive) {
	//if ((FUTURE_FEATURES(_c) & CO_FUTURE_ANNOTATIONS) and SYMTABLE_ENTRY(_c)->annotationsUsed) {
	//	ADDOP(_c, _loc, SETUP_ANNOTATIONS);
	//}
	if (!ASDL_SEQ_LEN(_stmts)) {
		return SUCCESS;
	}
	AlifSizeT firstInstr = 0;
	//if (!_isInteractive) {
	//	AlifObject* docString = alifAST_getDocString(_stmts);
	//	if (docString) {
	//		firstInstr = 1;
	//		/* set docstring */
	//		AlifObject* cleanDoc = alifCompile_cleanDoc(docString);
	//		if (cleanDoc == nullptr) {
	//			return ERROR;
	//		}
	//		StmtTy st = (StmtTy)ASDL_SEQ_GET(_stmts, 0);
	//		Location loc = LOC(st->V.expression.val);
	//		ADDOP_LOAD_CONST(_c, loc, cleanDoc);
	//		ALIF_DECREF(cleanDoc);
	//		RETURN_IF_ERROR(compiler_nameOp(_c, NO_LOCATION, &ALIF_ID(__doc__), ExprContext_::Store));
	//		}
	//	}

	for (AlifSizeT i = firstInstr; i < ASDL_SEQ_LEN(_stmts); i++) {
		VISIT(_c, Stmt, (StmtTy)ASDL_SEQ_GET(_stmts, i));
	}

	//if (!(FUTURE_FEATURES(c) & CO_FUTURE_ANNOTATIONS)) {
	//	RETURN_IF_ERROR(codegen_processDeferredAnnotations(c, loc));
	//}
	return SUCCESS;
}








AlifIntT _alifCodegen_enterAnonymousScope(AlifCompiler* _c, ModuleTy _mod) {
	RETURN_IF_ERROR(codegen_enterScope(_c, &ALIF_STR(AnonModule),
		ScopeType_::Compiler_Scope_Module, _mod, 1, nullptr, nullptr));

	return SUCCESS;
}




static AlifIntT codegen_makeClosure(AlifCompiler* _c, Location _loc,
	AlifCodeObject* _co, AlifSizeT _flags) {
	if (_co->nFreeVars) {
		AlifIntT i = alifUnstableCode_getFirstFree(_co);
		for (; i < _co->nLocalsPlus; ++i) {
			/* Bypass com_addop_varname because it will generate
			   LOAD_DEREF but LOAD_CLOSURE is needed.
			*/
			AlifObject* name = ALIFTUPLE_GET_ITEM(_co->localsPlusNames, i);
			AlifIntT arg = _alifCompiler_lookupArg(_c, _co, name);
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
	ASDLArgSeq* _kwOnlyArgs, ASDLExprSeq* _kwDefaults) {

	AlifIntT defaultCount = 0;
	for (AlifIntT i = 0; i < ASDL_SEQ_LEN(_kwOnlyArgs); i++) {
		ArgTy arg = ASDL_SEQ_GET(_kwOnlyArgs, i);
		ExprTy default_ = ASDL_SEQ_GET(_kwDefaults, i);
		if (default_) {
			defaultCount++;
			AlifObject* mangled = _alifCompiler_maybeMangle(_c, arg->arg);
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
	Location _loc) {
	VISIT_SEQ(_c, Expr, _args->defaults);
	ADDOP_I(_c, _loc, BUILD_TUPLE, ASDL_SEQ_LEN(_args->defaults));
	return SUCCESS;
}



static AlifSizeT codegen_defaultArguments(AlifCompiler* _c, Location _loc,
	ArgumentsTy _args) {
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




static AlifIntT codegen_wrapInStopIterationHandler(AlifCompiler* _c) {
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
	AlifIntT _isAsync, AlifSizeT _funcFlags, AlifIntT _firstLineNo) {
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
		codegen_enterScope(_c, name, scopeType, (void*)_s, _firstLineNo, nullptr, &umd));

	AlifSizeT first_instr = 0;
	AlifObject* docstring = alifAST_getDocString(body);
	if (docstring) {
		first_instr = 1;
		/* add docstring */
		//docstring = _alifCompile_cleanDoc(docstring);
		//if (docstring == nullptr) {
		//	compiler_exitScope(_c);
		//	return ERROR;
		//}
	}

	AlifSizeT idx = _alifCompiler_addConst(_c, docstring ? docstring : ALIF_NONE);
	ALIF_XDECREF(docstring);
	RETURN_IF_ERROR_IN_SCOPE(_c, idx < 0 ? ERROR : SUCCESS);

	NEW_JUMP_TARGET_LABEL(_c, start);
	USE_LABEL(_c, start);
	SymTableEntry* ste = SYMTABLE_ENTRY(_c);
	bool add_stopiteration_handler = ste->coroutine or ste->generator;
	if (add_stopiteration_handler) {
		RETURN_IF_ERROR(
			_alifCompiler_pushFBlock(_c, _noLocation_,
				AlifCompileFBlockType::Compiler_FBlock_Stop_Iteration,
				start, NO_LABEL, nullptr));
	}

	for (AlifSizeT i = first_instr; i < ASDL_SEQ_LEN(body); i++) {
		VISIT_IN_SCOPE(_c, Stmt, (StmtTy)ASDL_SEQ_GET(body, i));
	}
	if (add_stopiteration_handler) {
		RETURN_IF_ERROR_IN_SCOPE(_c, codegen_wrapInStopIterationHandler(_c));
		_alifCompiler_popFBlock(_c, AlifCompileFBlockType::Compiler_FBlock_Stop_Iteration, start);
	}
	AlifCodeObject* co = _alifCompiler_optimizeAndAssemble(_c, 1);
	_alifCompiler_exitScope(_c);
	if (co == nullptr) {
		ALIF_XDECREF(co);
		return ERROR;
	}
	AlifIntT ret = codegen_makeClosure(_c, LOC(_s), co, _funcFlags);
	ALIF_DECREF(co);
	return ret;
}





static AlifIntT codegen_function(AlifCompiler* _c, StmtTy _s, AlifIntT _isAsync) {
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
		AlifIntT ret = codegen_enterScope(_c, typeParamsName, ScopeType_::Compiler_Scope_Annotations,
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

		AlifCodeObject* co = _alifCompiler_optimizeAndAssemble(_c, 0);
		_alifCompiler_exitScope(_c);
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






static AlifIntT codegen_setTypeParamsInClass(AlifCompiler* _c, Location _loc) {
	RETURN_IF_ERROR(codegen_nameOp(_c, _loc, &ALIF_STR(TypeParams), ExprContext_::Load));
	RETURN_IF_ERROR(codegen_nameOp(_c, _loc, &ALIF_ID(__typeParams__), ExprContext_::Store));
	return SUCCESS;
}





static AlifIntT codegen_classBody(AlifCompiler* _c,
	StmtTy _s, AlifIntT _firstLineNo) {

	/* 1. compile the class body into a code object */
	RETURN_IF_ERROR(
		codegen_enterScope(_c, _s->V.classDef.name, ScopeType_::Compiler_Scope_Class,
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
	RETURN_IF_ERROR_IN_SCOPE(_c, _alifCodegen_body(_c, loc, _s->V.classDef.body, false));
	AlifObject* staticAttributes = _alifCompiler_staticAttributesTuple(_c);
	if (staticAttributes == nullptr) {
		_alifCompiler_exitScope(_c);
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
		AlifIntT i = _alifCompiler_lookupCellVar(_c, &ALIF_ID(__classDict__));
		RETURN_IF_ERROR_IN_SCOPE(_c, i);
		ADDOP_I(_c, _noLocation_, LOAD_CLOSURE, i);
		RETURN_IF_ERROR_IN_SCOPE(
			_c, codegen_nameOp(_c, _noLocation_, &ALIF_ID(__classDictCell__), ExprContext_::Store));
	}
	/* Return __classcell__ if it is referenced, otherwise return None */
	if (SYMTABLE_ENTRY(_c)->needsClassClosure) {
		/* Store __classcell__ into class namespace & return it */
		AlifIntT i = _alifCompiler_lookupCellVar(_c, &ALIF_ID(__class__));
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
	AlifCodeObject* co = _alifCompiler_optimizeAndAssemble(_c, 1);

	/* leave the new scope */
	_alifCompiler_exitScope(_c);
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












static AlifIntT codegen_class(AlifCompiler* _c, StmtTy _s) {
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
	//	AlifIntT ret = codegen_enterScope(_c, type_params_name, COMPILER_SCOPE_ANNOTATIONS,
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

		RETURN_IF_ERROR_IN_SCOPE(_c, codegen_callHelperImpl(_c, loc, 2,
			_s->V.classDef.bases,
			&ALIF_STR(GenericBase),
			_s->V.classDef.keywords));

		AlifCodeObject* co = _alifCompiler_optimizeAndAssemble(_c, 0);

		_alifCompiler_exitScope(_c);
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




















































































static bool check_isArg(ExprTy _e) {
	if (_e->type != ExprK_::ConstantK) {
		return true;
	}
	AlifObject* value = _e->V.constant.val;
	return (value == ALIF_NONE
		or value == ALIF_FALSE
		or value == ALIF_TRUE
		or value == ALIF_ELLIPSIS);
}



static AlifTypeObject* infer_type(ExprTy);




static AlifIntT codegen_checkCompare(AlifCompiler* _c, ExprTy _e) {
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



static AlifIntT codegen_addCompare(AlifCompiler* _c, Location _loc, CmpOp_ _op) {
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
	ExprTy _e, JumpTargetLabel _next, AlifIntT _cond) {
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
		if (!SAME_JUMP_TARGET_LABEL(next2, _next)) {
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





static AlifIntT codegen_ifExpr(AlifCompiler* _c, ExprTy _e) {
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


















































static AlifIntT codegen_if(AlifCompiler* _c, StmtTy _s) {
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




static AlifIntT codegen_for(AlifCompiler* _c, StmtTy _s) {
	Location loc = LOC(_s);
	NEW_JUMP_TARGET_LABEL(_c, start);
	NEW_JUMP_TARGET_LABEL(_c, body);
	NEW_JUMP_TARGET_LABEL(_c, cleanup);
	NEW_JUMP_TARGET_LABEL(_c, end);

	RETURN_IF_ERROR(_alifCompiler_pushFBlock(_c, loc,
		AlifCompileFBlockType::Compiler_FBlock_For_Loop, start, end, nullptr));

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

	_alifCompiler_popFBlock(_c, AlifCompileFBlockType::Compiler_FBlock_For_Loop, start);

	//VISIT_SEQ(_c, Stmt, _s->V.for_.else_);

	USE_LABEL(_c, end);
	return SUCCESS;
}
















































static AlifIntT codegen_while(AlifCompiler* _c, StmtTy _s) {
	NEW_JUMP_TARGET_LABEL(_c, loop);
	NEW_JUMP_TARGET_LABEL(_c, end);
	NEW_JUMP_TARGET_LABEL(_c, anchor);

	USE_LABEL(_c, loop);

	RETURN_IF_ERROR(_alifCompiler_pushFBlock(_c, LOC(_s),
		AlifCompileFBlockType::Compiler_FBlock_While_Loop, loop, end, nullptr));
	RETURN_IF_ERROR(codegen_jumpIf(_c, LOC(_s), _s->V.while_.condition, anchor, 0));

	VISIT_SEQ(_c, Stmt, _s->V.while_.body);
	ADDOP_JUMP(_c, _noLocation_, JUMP, loop);

	_alifCompiler_popFBlock(_c, AlifCompileFBlockType::Compiler_FBlock_While_Loop, loop);

	USE_LABEL(_c, anchor);
	//if (_s->V.while_.else_) {
	//	VISIT_SEQ(_c, Stmt, _s->V.while_.else_);
	//}

	USE_LABEL(_c, end);
	return SUCCESS;
}


static AlifIntT codegen_return(AlifCompiler* _c, StmtTy _s) {
	Location loc = LOC(_s);
	AlifIntT preserve_tos = ((_s->V.return_.val != nullptr) and
		(_s->V.return_.val->type != ExprK_::ConstantK));

	SymTableEntry* ste = SYMTABLE_ENTRY(_c);
	if (!alifST_isFunctionLike(ste)) {
		//return _alifCompiler_error(_c, loc, "'return' outside function");
	}
	if (_s->V.return_.val != nullptr and ste->coroutine and ste->generator) {
		//return _alifCompiler_error(_c, loc, "'return' with value in async generator");
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


static AlifIntT codegen_break(AlifCompiler* _c, Location _loc) {
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



static AlifIntT codegen_continue(AlifCompiler* _c, Location _loc) {
	FBlockInfo* loop = nullptr;
	Location origin_loc = _loc;
	/* Emit instruction with line number */
	ADDOP(_c, _loc, NOP);
	RETURN_IF_ERROR(codegen_unwindFBlockStack(_c, &_loc, 0, &loop));
	if (loop == nullptr) {
		//return _alifCompiler_error(_c, origin_loc, "'continue' not properly in loop");
	}
	ADDOP_JUMP(_c, _loc, JUMP, loop->block);
	return SUCCESS;
}































































































































































































































































































































































































































































































































































































static AlifIntT codegen_import(AlifCompiler* _c, StmtTy _s) {
	Location loc = LOC(_s);
	AlifSizeT i, n = ASDL_SEQ_LEN(_s->V.import.names);

	AlifObject* zero = _alifLong_getZero();  // borrowed reference
	for (i = 0; i < n; i++) {
		AliasTy alias = (AliasTy)ASDL_SEQ_GET(_s->V.import.names, i);
		AlifIntT r{};

		ADDOP_LOAD_CONST(_c, loc, zero);
		ADDOP_LOAD_CONST(_c, loc, ALIF_NONE);
		ADDOP_NAME(_c, loc, IMPORT_NAME, alias->name, names);

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
			r = codegen_nameOp(_c, loc, tmp, ExprContext_::Store);
			if (dot != -1) {
				ALIF_DECREF(tmp);
			}
			RETURN_IF_ERROR(r);
		}
	}
	return SUCCESS;
}










static AlifIntT codegen_fromImport(AlifCompiler* _c, StmtTy _s) {
	AlifSizeT n = ASDL_SEQ_LEN(_s->V.importFrom.names);

	ADDOP_LOAD_CONST_NEW(_c, LOC(_s), alifLong_fromLong(_s->V.importFrom.level));

	AlifObject* names = alifTuple_new(n);
	if (!names) {
		return ERROR;
	}

	/* build up the names */
	for (AlifSizeT i = 0; i < n; i++) {
		AliasTy alias = (AliasTy)ASDL_SEQ_GET(_s->V.importFrom.names, i);
		ALIFTUPLE_SET_ITEM(names, i, ALIF_NEWREF(alias->name));
	}

	ADDOP_LOAD_CONST_NEW(_c, LOC(_s), names);

	if (_s->V.importFrom.module) {
		ADDOP_NAME(_c, LOC(_s), IMPORT_NAME, _s->V.importFrom.module, names);
	}
	else {
		ADDOP_NAME(_c, LOC(_s), IMPORT_NAME, &ALIF_STR(Empty), names);
	}
	for (AlifSizeT i = 0; i < n; i++) {
		AliasTy alias = (AliasTy)ASDL_SEQ_GET(_s->V.importFrom.names, i);
		Identifier store_name{};

		if (i == 0 and ALIFUSTR_READ_CHAR(alias->name, 0) == '*') {
			ADDOP_I(_c, LOC(_s), CALL_INTRINSIC_1, INTRINSIC_IMPORT_STAR);
			ADDOP(_c, _noLocation_, POP_TOP);
			return SUCCESS;
		}

		ADDOP_NAME(_c, LOC(_s), IMPORT_FROM, alias->name, names);
		store_name = alias->name;
		if (alias->asName) {
			store_name = alias->asName;
		}

		RETURN_IF_ERROR(codegen_nameOp(_c, LOC(_s), store_name, ExprContext_::Store));
	}
	/* remove imported module */
	ADDOP(_c, LOC(_s), POP_TOP);
	return SUCCESS;
}



































static AlifIntT codegen_stmtExpr(AlifCompiler* _c, Location _loc, ExprTy _value) {
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



static AlifIntT codegen_visitStmt(AlifCompiler* _c, StmtTy _s) {
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



static AlifIntT unaryop(UnaryOp_ _op) {
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
	Operator_ _binop, bool _inplace) {
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




static AlifIntT codegen_addOpYield(AlifCompiler* _c, Location _loc) {
	SymTableEntry* ste = SYMTABLE_ENTRY(_c);
	if (ste->generator and ste->coroutine) {
		ADDOP_I(_c, _loc, CALL_INTRINSIC_1, INTRINSIC_ASYNC_GEN_WRAP);
	}
	ADDOP_I(_c, _loc, YIELD_VALUE, 0);
	ADDOP_I(_c, _loc, RESUME, RESUME_AFTER_YIELD);
	return SUCCESS;
}


static AlifIntT codegen_loadClassDictFreeVar(AlifCompiler* _c, Location _loc) {
	ADDOP_N(_c, _loc, LOAD_DEREF, &ALIF_ID(__classDict__), freevars);
	return SUCCESS;
}



static AlifIntT codegen_nameOp(AlifCompiler* _c, Location _loc,
	Identifier _name, ExprContext_ _ctx) {

	AlifObject* mangled = _alifCompiler_maybeMangle(_c, _name);
	if (!mangled) {
		return ERROR;
	}

	AlifIntT scope = alifST_getScope(SYMTABLE_ENTRY(_c), mangled);
	RETURN_IF_ERROR(scope);
	AlifCompilerOpType optype{};
	AlifSizeT arg = 0;
	if (_alifCompiler_resolveNameOp(_c, mangled, scope, &optype, &arg) < 0) {
		ALIF_DECREF(mangled);
		return ERROR;
	}


	AlifIntT op = 0;
	switch (optype) {
	case AlifCompilerOpType::Compiler_Op_DeRef:
		switch (_ctx) {
		case ExprContext_::Load:
			if (SYMTABLE_ENTRY(_c)->type == BlockType_::Class_Block
				and !_alifCompiler_isInInlinedComp(_c)) {
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
		case ExprContext_::Store: op = STORE_DEREF; break;
		case ExprContext_::Del: op = DELETE_DEREF; break;
		}
		break;
	case AlifCompilerOpType::Compiler_Op_Fast:
		switch (_ctx) {
		case ExprContext_::Load: op = LOAD_FAST; break;
		case ExprContext_::Store: op = STORE_FAST; break;
		case ExprContext_::Del: op = DELETE_FAST; break;
		}
		ADDOP_N(_c, _loc, op, mangled, varnames);
		return SUCCESS;
	case AlifCompilerOpType::Compiler_Op_Global:
		switch (_ctx) {
		case ExprContext_::Load:
			if (SYMTABLE_ENTRY(_c)->canSeeClassScope and scope == GLOBAL_IMPLICIT) {
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
		case ExprContext_::Store: op = STORE_GLOBAL; break;
		case ExprContext_::Del: op = DELETE_GLOBAL; break;
		}
		break;
	case AlifCompilerOpType::Compiler_Op_Name:
		switch (_ctx) {
		case ExprContext_::Load:
			op = (SYMTABLE_ENTRY(_c)->type == BlockType_::Class_Block
				and _alifCompiler_isInInlinedComp(_c))
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
	ADDOP_I(_c, _loc, op, arg);
	return SUCCESS;

error:
	ALIF_DECREF(mangled);
	return ERROR;
}







static AlifIntT codegen_boolOp(AlifCompiler* _c, ExprTy _e) {
	AlifIntT jumpi{};
	AlifSizeT i{}, n{};
	ASDLExprSeq* s{};

	Location loc = LOC(_e);
	if (_e->V.boolOp.op == BoolOp_::And)
		jumpi = JUMP_IF_FALSE;
	else
		jumpi = JUMP_IF_TRUE;
	NEW_JUMP_TARGET_LABEL(_c, _end);
	s = _e->V.boolOp.vals;
	n = ASDL_SEQ_LEN(s) - 1;
	for (i = 0; i < n; ++i) {
		VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(s, i));
		ADDOP_JUMP(_c, loc, jumpi, _end);
		ADDOP(_c, loc, POP_TOP);
	}
	VISIT(_c, Expr, (ExprTy)ASDL_SEQ_GET(s, n));

	USE_LABEL(_c, _end);
	return SUCCESS;
}





static AlifIntT starUnpack_helperImpl(AlifCompiler* _c, Location _loc,
	ASDLExprSeq* _elts, AlifObject* _injectedArg, AlifIntT _pushed, AlifIntT _build,
	AlifIntT _add, AlifIntT _extend, AlifIntT _tuple) {
	AlifSizeT n = ASDL_SEQ_LEN(_elts);
	if (!_injectedArg and n > 2 and areAllItems_const(_elts, 0, n)) {
		AlifObject* folded = alifTuple_new(n);
		if (folded == nullptr) {
			return ERROR;
		}

		for (AlifSizeT i = 0; i < n; i++) {
			AlifObject* val = ((ExprTy)ASDL_SEQ_GET(_elts, i))->V.constant.val;
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

	AlifIntT big = n + _pushed + (_injectedArg ? 1 : 0) > STACK_USE_GUIDELINE;
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
		if (_injectedArg) {
			RETURN_IF_ERROR(codegen_nameOp(_c, _loc, _injectedArg, ExprContext_::Load));
			n++;
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
	if (_injectedArg) {
		RETURN_IF_ERROR(codegen_nameOp(_c, _loc, _injectedArg, ExprContext_::Load));
		ADDOP_I(_c, _loc, _add, 1);
	}
	if (_tuple) {
		ADDOP_I(_c, _loc, CALL_INTRINSIC_1, INTRINSIC_LIST_TO_TUPLE);
	}
	return SUCCESS;
}

static AlifIntT starUnpack_helper(AlifCompiler* _c, Location _loc,
	ASDLExprSeq* _elts, AlifIntT _pushed, AlifIntT _build,
	AlifIntT _add, AlifIntT _extend, AlifIntT _tuple) {
	return starUnpack_helperImpl(_c, _loc, _elts, nullptr, _pushed,
		_build, _add, _extend, _tuple);
}



static AlifIntT unpack_helper(AlifCompiler* _c, Location _loc, ASDLExprSeq* _elts) {
	AlifSizeT n = ASDL_SEQ_LEN(_elts);
	AlifIntT seenStar = 0;
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy elt = ASDL_SEQ_GET(_elts, i);
		if (elt->type == ExprK_::StarK and !seenStar) {
			if ((i >= (1 << 8)) or
				(n - i - 1 >= (INT_MAX >> 8))) {
				//return _alifCompiler_error(_c, _loc,
				//	"too many expressions in "
				//	"star-unpacking assignment");
			}
			ADDOP_I(_c, _loc, UNPACK_EX, (i + ((n - i - 1) << 8)));
			seenStar = 1;
		}
		else if (elt->type == ExprK_::StarK) {
			//return _alifCompiler_error(_c, _loc,
			//	"multiple starred expressions in assignment");
		}
	}
	if (!seenStar) {
		ADDOP_I(_c, _loc, UNPACK_SEQUENCE, n);
	}
	return SUCCESS;
}



static AlifIntT assignment_helper(AlifCompiler* _c,
	Location _loc, ASDLExprSeq* _elts) {
	AlifSizeT n = ASDL_SEQ_LEN(_elts);
	RETURN_IF_ERROR(unpack_helper(_c, _loc, _elts));
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy elt = ASDL_SEQ_GET(_elts, i);
		VISIT(_c, Expr, elt->type != ExprK_::StarK ? elt : elt->V.star.val);
	}
	return SUCCESS;
}


static AlifIntT codegen_list(AlifCompiler* _c, ExprTy _e) {
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



static AlifIntT codegen_tuple(AlifCompiler* _c, ExprTy _e) {
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











static bool areAllItems_const(ASDLExprSeq* _seq,
	AlifSizeT _begin, AlifSizeT _end) {
	for (AlifSizeT i = _begin; i < _end; i++) {
		ExprTy key = (ExprTy)ASDL_SEQ_GET(_seq, i);
		if (key == nullptr or key->type != ExprK_::ConstantK) {
			return false;
		}
	}
	return true;
}


static AlifIntT codegen_subDict(AlifCompiler* _c,
	ExprTy _e, AlifSizeT _begin, AlifSizeT _end) {
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


static AlifIntT codegen_dict(AlifCompiler* _c, ExprTy _e) {
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



static AlifIntT codegen_compare(AlifCompiler* _c, ExprTy _e) {
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




static AlifTypeObject* infer_type(ExprTy _e) {
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



static AlifIntT check_caller(AlifCompiler* _c, ExprTy _e) {
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
		//return _alifCompiler_warn(_c, loc, "'%.200s' object is not callable; "
		//	"perhaps you missed a comma?",
		//	infer_type(_e)->name);
		return -1; //* alif
	}
	default:
		return SUCCESS;
	}
}


static AlifIntT check_subScripter(AlifCompiler* _c, ExprTy _e) {
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
		//return _alifCompiler_warn(_c, loc, "'%.200s' object is not subscriptable; "
		//	"perhaps you missed a comma?",
		//	infer_type(_e)->name);
	}
	default:
		return SUCCESS;
	}
}



static AlifIntT check_index(AlifCompiler* _c, ExprTy _e, ExprTy _s) {
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
		//return _alifCompiler_warn(_c, loc, "%.200s indices must be integers "
		//	"or slices, not %.200s; "
		//	"perhaps you missed a comma?",
		//	infer_type(_e)->name,
		//	index_type->name);
	}
	default:
		return SUCCESS;
	}
}



static AlifIntT is_importOriginated(AlifCompiler* _c, ExprTy _e) {

	if (_e->type != ExprK_::NameK) {
		return 0;
	}

	long flags = _alifST_getSymbol(SYMTABLE(_c)->top, _e->V.name.name);
	RETURN_IF_ERROR(flags);
	return flags & DEF_IMPORT;
}








static AlifIntT canOptimize_superCall(AlifCompiler* _c, ExprTy _attr) { // 4617
	ExprTy e = _attr->V.attribute.val;
	if (e->type != ExprK_::CallK or
		e->V.call.func->type != ExprK_::NameK or
		!alifUStr_equalToASCIIString(e->V.call.func->V.name.name, "super") or
		alifUStr_equalToASCIIString(_attr->V.attribute.attr, "__class__") or
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
	if (_alifCompiler_getRefType(_c, &ALIF_ID(__class__)) == FREE) {
		return 1;
	}
	return 0;
}




static AlifIntT loadArgs_forSuper(AlifCompiler* _c, ExprTy _e) {
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
	Location _loc, ExprTy _attr) {
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






static AlifIntT maybeOptimize_methodCall(AlifCompiler* _c, ExprTy _e) {
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




static AlifIntT codegen_validateKeywords(AlifCompiler* _c,
	ASDLKeywordSeq* _keywords) {
	AlifSizeT nkeywords = ASDL_SEQ_LEN(_keywords);
	for (AlifSizeT i = 0; i < nkeywords; i++) {
		KeywordTy key = ((KeywordTy)ASDL_SEQ_GET(_keywords, i));
		if (key->arg == nullptr) {
			continue;
		}
		for (AlifSizeT j = i + 1; j < nkeywords; j++) {
			KeywordTy other = ((KeywordTy)ASDL_SEQ_GET(_keywords, j));
			if (other->arg and !alifUStr_compare(key->arg, other->arg)) {
				//_alifCompiler_error(_c, LOC(other), "keyword argument repeated: %U", key->arg);
				return ERROR;
			}
		}
	}
	return SUCCESS;
}


static AlifIntT codegen_call(AlifCompiler* _c, ExprTy _e) {
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





static AlifIntT codegen_joinedStr(AlifCompiler* _c, ExprTy _e) {
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






static AlifIntT codegen_formattedValue(AlifCompiler* _c, ExprTy _e) {
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
	ASDLKeywordSeq* _keywords, AlifSizeT _begin, AlifSizeT _end) {
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
	ASDLKeywordSeq* _keywords, AlifSizeT _nkwelts) {
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




static AlifIntT codegen_callHelperImpl(AlifCompiler* _c, Location _loc,
	AlifIntT _n, /* Args already pushed */ ASDLExprSeq* _args,
	AlifObject* _injectedArg, ASDLKeywordSeq* _keywords) {
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
	if (_injectedArg) {
		RETURN_IF_ERROR(codegen_nameOp(_c, _loc, _injectedArg, ExprContext_::Load));
		nelts++;
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
		RETURN_IF_ERROR(starUnpack_helperImpl(_c, _loc, _args, _injectedArg, _n, BUILD_LIST,
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







static AlifIntT codegen_callHelper(AlifCompiler* _c, Location _loc,
	AlifIntT _n, /* Args already pushed */ ASDLExprSeq* _args,
	ASDLKeywordSeq* _keywords) {
	return codegen_callHelperImpl(_c, _loc, _n, _args, NULL, _keywords);
}


















static AlifIntT compiler_comprehensionGenerator(AlifCompiler* _c, Location _loc,
	ASDLComprehensionSeq* _generators, AlifIntT _genIndex, AlifIntT _depth,
	ExprTy _elt, ExprTy _val, AlifIntT _type, AlifIntT _iterOnStack) {
	ComprehensionTy gen{};
	gen = (ComprehensionTy)ASDL_SEQ_GET(_generators, _genIndex);
	if (gen->isAsync) {
		return codegen_asyncComprehensionGenerator(
			_c, _loc, _generators, _genIndex, _depth, _elt, _val, _type,
			_iterOnStack);
	}
	else {
		return codegen_syncComprehensionGenerator(
			_c, _loc, _generators, _genIndex, _depth, _elt, _val, _type,
			_iterOnStack);
	}
}




static AlifIntT codegen_syncComprehensionGenerator(AlifCompiler* _c, Location _loc,
	ASDLComprehensionSeq* _generators, AlifIntT _genIndex, AlifIntT _depth,
	ExprTy _elt, ExprTy _val, AlifIntT _type, AlifIntT _iterOnStack) {

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
					start = NO_LABEL;
				}
			}
			if (IS_JUMP_TARGET_LABEL(start)) {
				VISIT(_c, Expr, gen->iter);
				ADDOP(_c, LOC(gen->iter), GET_ITER);
			}
		}
	}

	if (IS_JUMP_TARGET_LABEL(start)) {
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
	if (IS_JUMP_TARGET_LABEL(start)) {
		ADDOP_JUMP(_c, elt_loc, JUMP, start);

		USE_LABEL(_c, anchor);

		ADDOP(_c, _noLocation_, END_FOR);
		ADDOP(_c, _noLocation_, POP_TOP);
	}

	return SUCCESS;
}




















static AlifIntT codegen_asyncComprehensionGenerator(AlifCompiler* _c, Location _loc,
	ASDLComprehensionSeq* _generators, AlifIntT _genIndex, AlifIntT _depth,
	ExprTy _elt, ExprTy _val, AlifIntT _type, AlifIntT _iterOnStack) {
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
		_alifCompiler_pushFBlock(_c, _loc,
			AlifCompileFBlockType::Compiler_FBlock_Async_Comprehension_Generator,
			start, NO_LABEL, nullptr));

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

	_alifCompiler_popFBlock(_c,
		AlifCompileFBlockType::Compiler_FBlock_Async_Comprehension_Generator, start);

	USE_LABEL(_c, except);

	ADDOP(_c, _loc, END_ASYNC_FOR);

	return SUCCESS;
}










static AlifIntT codegen_pushInlinedComprehensionLocals(AlifCompiler* _c, Location _loc,
	SymTableEntry* _comp, AlifCompilerInlinedComprehensionState* _state) {
	AlifIntT inClassBlock = (SYMTABLE_ENTRY(_c)->type == BlockType_::Class_Block)
		and !_alifCompiler_isInInlinedComp(_c);
	SymTableEntry* outer = SYMTABLE_ENTRY(_c);
	AlifObject* k{}, * v{};
	AlifSizeT pos = 0;
	while (alifDict_next(_comp->symbols, &pos, &k, &v)) {
		long symbol = alifLong_asLong(v);
		RETURN_IF_ERROR(symbol);
		long scope = SYMBOL_TO_SCOPE(symbol);

		long outsymbol = _alifST_getSymbol(outer, k);
		RETURN_IF_ERROR(outsymbol);
		long outsc = SYMBOL_TO_SCOPE(outsymbol);

		if ((symbol & DEF_LOCAL and !(symbol & DEF_NONLOCAL)) or inClassBlock) {
			if (_state->pushedLocals == nullptr) {
				_state->pushedLocals = alifList_new(0);
				if (_state->pushedLocals == nullptr) {
					return ERROR;
				}
			}
			ADDOP_NAME(_c, _loc, LOAD_FAST_AND_CLEAR, k, varnames);
			if (scope == CELL) {
				if (outsc == FREE) {
					ADDOP_NAME(_c, _loc, MAKE_CELL, k, freevars);
				}
				else {
					ADDOP_NAME(_c, _loc, MAKE_CELL, k, cellvars);
				}
			}
			if (alifList_append(_state->pushedLocals, k) < 0) {
				return ERROR;
			}
		}
	}
	if (_state->pushedLocals) {
		ADDOP_I(_c, _loc, SWAP, ALIFLIST_GET_SIZE(_state->pushedLocals) + 1);

		NEW_JUMP_TARGET_LABEL(_c, cleanup);
		_state->cleanup = cleanup;

		ADDOP_JUMP(_c, _loc, SETUP_FINALLY, cleanup);
	}
	return SUCCESS;
}





















static AlifIntT pushInlined_comprehensionState(AlifCompiler* _c, Location _loc,
	SymTableEntry* _comp, AlifCompilerInlinedComprehensionState* _state) {
	RETURN_IF_ERROR(
		_alifCompiler_tweakInlinedComprehensionScopes(_c, _loc, _comp, _state));
	RETURN_IF_ERROR(
		codegen_pushInlinedComprehensionLocals(_c, _loc, _comp, _state));
	return SUCCESS;
}




static AlifIntT restoreInlined_comprehensionLocals(AlifCompiler* _c, Location _loc,
	AlifCompilerInlinedComprehensionState* _state) {
	AlifObject* k{};
	AlifSizeT npops = ALIFLIST_GET_SIZE(_state->pushedLocals);
	ADDOP_I(_c, _loc, SWAP, npops + 1);
	for (AlifSizeT i = npops - 1; i >= 0; --i) {
		k = alifList_getItem(_state->pushedLocals, i);
		if (k == nullptr) {
			return ERROR;
		}
		ADDOP_NAME(_c, _loc, STORE_FAST_MAYBE_NULL, k, varnames);
	}
	return SUCCESS;
}








static AlifIntT codegen_popInlinedComprehensionLocals(AlifCompiler* _c, Location _loc,
	AlifCompilerInlinedComprehensionState* _state) {
	if (_state->pushedLocals) {
		ADDOP(_c, _noLocation_, POP_BLOCK);

		NEW_JUMP_TARGET_LABEL(_c, end);
		ADDOP_JUMP(_c, _noLocation_, JUMP_NO_INTERRUPT, end);

		// cleanup from an exception inside the comprehension
		USE_LABEL(_c, _state->cleanup);
		// discard incomplete comprehension result (beneath exc on stack)
		ADDOP_I(_c, _noLocation_, SWAP, 2);
		ADDOP(_c, _noLocation_, POP_TOP);
		RETURN_IF_ERROR(restoreInlined_comprehensionLocals(_c, _loc, _state));
		ADDOP_I(_c, _noLocation_, RERAISE, 0);

		USE_LABEL(_c, end);
		RETURN_IF_ERROR(restoreInlined_comprehensionLocals(_c, _loc, _state));
		ALIF_CLEAR(_state->pushedLocals);
	}
	return SUCCESS;
}



static AlifIntT popInlined_comprehensionState(AlifCompiler* _c,
	Location _loc, AlifCompilerInlinedComprehensionState* _state) {
	RETURN_IF_ERROR(codegen_popInlinedComprehensionLocals(_c, _loc, _state));
	RETURN_IF_ERROR(_alifCompiler_revertInlinedComprehensionScopes(_c, _loc, _state));
	return SUCCESS;
}



static inline AlifIntT codegen_comprehensionIter(AlifCompiler* _c, Location _loc,
	ComprehensionTy _comp) {
	VISIT(_c, Expr, _comp->iter);
	if (_comp->isAsync) {
		ADDOP(_c, _loc, GET_AITER);
	}
	else {
		ADDOP(_c, _loc, GET_ITER);
	}
	return SUCCESS;
}


static AlifIntT codegen_comprehension(AlifCompiler* _c, ExprTy _e, AlifIntT _type, Identifier _name,
	ASDLComprehensionSeq* _generators, ExprTy _elt, ExprTy _val) {

	AlifIntT isInlined{}; //* alif
	AlifIntT isAsyncComprehension{}; //* alif
	Location loc{}; //* alif

	AlifCodeObject* co = nullptr;
	AlifCompilerInlinedComprehensionState inline_state =
			{ nullptr, nullptr, nullptr, NO_LABEL };
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
		if (codegen_enterScope(_c, _name, ScopeType_::Compiler_Scope_Comprehension,
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

	co = _alifCompiler_optimizeAndAssemble(_c, 1);
	_alifCompiler_exitScope(_c);
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

	if (isAsyncComprehension and _type != COMP_GENEXP) {
		ADDOP_I(_c, loc, GET_AWAITABLE, 0);
		ADDOP_LOAD_CONST(_c, loc, ALIF_NONE);
		ADD_YIELD_FROM(_c, loc, 1);
	}

	return SUCCESS;
error_in_scope:
	if (!isInlined) {
		_alifCompiler_exitScope(_c);
	}
error:
	ALIF_XDECREF(co);
	ALIF_XDECREF(entry);
	ALIF_XDECREF(inline_state.pushedLocals);
	ALIF_XDECREF(inline_state.tempSymbols);
	ALIF_XDECREF(inline_state.fastHidden);
	return ERROR;
}


















static AlifIntT codegen_listComp(AlifCompiler* _c, ExprTy _e) {
	return codegen_comprehension(_c, _e, COMP_LISTCOMP, &ALIF_STR(AnonListComp),
		_e->V.listComp.generators,
		_e->V.listComp.elt, nullptr);
}



























static AlifIntT codegen_visitKeyword(AlifCompiler* _c, KeywordTy _k) {
	VISIT(_c, Expr, _k->val);
	return SUCCESS;
}
































































































































































































































static AlifIntT codegen_visitExpr(AlifCompiler* _c, ExprTy _e) {
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
		RETURN_IF_ERROR(_alifCompiler_maybeAddStaticAttributeToClass(_c, _e));
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








static bool is_twoElementSlice(ExprTy _s) {
	return _s->type == ExprK_::SliceK and
		_s->V.slice.step == nullptr;
}



static AlifIntT codegen_augAssign(AlifCompiler* _c, StmtTy _s) {
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






















































































































static AlifIntT codegen_subScript(AlifCompiler* _c, ExprTy _e) {
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







static AlifIntT codegen_slice(AlifCompiler* _c, ExprTy _s) {
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






























































































































































































































































































































































































































































































































































































































































































































































































































































AlifIntT _alifCodegen_addReturnAtEnd(AlifCompiler* _c, AlifIntT _addNone) {
	/* Make sure every instruction stream that falls off the end returns None.
	 * This also ensures that no jump target offsets are out of bounds.
	 */
	if (_addNone) {
		ADDOP_LOAD_CONST(_c, _noLocation_, ALIF_NONE);
	}
	ADDOP(_c, _noLocation_, RETURN_VALUE);
	return SUCCESS;
}






