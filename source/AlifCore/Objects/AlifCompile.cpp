#include "alif.h"

//#include "Opcode.h"
#include "AlifCore_AST.h"
#define NEED_OPCODE_TABLES
//#include "AlifCore_OpcodeUtils.h"
#undef NEED_OPCODE_TABLES
//#include "AlifCore_Code.h"
#include "AlifCore_Compile.h"
//#include "AlifCore_FlowGraph.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_State.h"

#include "AlifCore_SymTable.h"


#define NEED_OPCODE_METADATA
//#include "AlifCore_OpcodeMetadata.h"
#undef NEED_OPCODE_METADATA


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


typedef AlifInstructionSequence InstrSequence; // 84


typedef AlifSourceLocation Location; // 99


#define LOCATION(_lno, _endLno, _col, _endCol) {_lno, _endLno, _col, _endCol} // 108

typedef AlifJumpTargetLabel JumpTargetLabel; // 113

static JumpTargetLabel _NoLabel_ = { -1 }; // 115


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




/* The following items change on entry and exit of code blocks.
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
	u->ste = alifSymtable_lookup(_c->st, _key);
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
		VISIT(_c, expr, _mod->V.expression.body);
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
