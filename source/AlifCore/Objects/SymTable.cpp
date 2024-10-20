#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Parser.h"
#include "AlifCore_State.h"
#include "AlifCore_SymTable.h"

class AlifSymTable* alifSymtable_build(ModuleTy mod, AlifObject* filename, AlifFutureFeatures* future) { // 394
	AlifSymTable* st_ = symtable_new();
	ASDLStmtSeq* seq_{};
	AlifSizeT i_{};
	AlifThread* tstate{};
	AlifIntT startingRecursionDepth{};

	if (st_ == nullptr)
		return nullptr;
	if (filename == nullptr) {
		//alifSymtable_free(st_);
		return nullptr;
	}
	st_->fileName = ALIF_NEWREF(filename);
	st_->future = future;

	tstate = _alifThread_get();
	if (!tstate) {
		//alifSymtable_free(st_);
		return nullptr;
	}
	AlifIntT recursionDepth = ALIFCPP_RECURSION_LIMIT - tstate->cppRecursionRemaining;
	startingRecursionDepth = recursionDepth;
	st_->recursionDepth = startingRecursionDepth;
	st_->recursionLimit = ALIFCPP_RECURSION_LIMIT;

	if (!symtable_enter_block(st_, &ALIF_ID(top), ModuleBlock, (void*)mod)) {
		//alifSymtable_free(st_);
		return nullptr;
	}

	st_->top = st_->cur;
	switch (mod->type) {
	case ModuleK:
		seq_ = mod->V.module.body;
		for (i_ = 0; i_ < ASDL_SEQ_LEN(seq_); i_++)
			if (!symtable_visit_stmt(st_,
				(StmtTy)ASDL_SEQ_GET(seq_, i_)))
				goto error;
		break;
	case ExpressionK:
		if (!symtable_visit_expr(st_, mod->V.expression.body))
			goto error;
		break;
	case InteractiveK:
		seq_ = mod->V.interactive.body;
		for (i_ = 0; i_ < ASDL_SEQ_LEN(seq_); i_++)
			if (!symtable_visit_stmt(st_,
				(StmtTy)ASDL_SEQ_GET(seq_, i_)))
				goto error;
		break;
	case FunctionK:
		//alifErr_setString(_alifExcRuntimeError_,
			//"this compiler does not handle FunctionTypes");
		goto error;
	}
	if (!symtable_exit_block(st_)) {
		//alifSymtable_free(st_);
		return nullptr;
	}
	/* Check that the recursion depth counting balanced correctly */
	if (st_->recursionDepth != startingRecursionDepth) {
		//alifErr_format(_alifExcsystemError_,
			//"symtable analysis recursion depth mismatch (before=%d, after=%d)",
			//startingRecursionDepth, st_->recursionDepth);
		//alifSymtable_free(st_);
		return nullptr;
	}
	if (symtable_analyze(st_)) {
#if _ALIF_DUMP_SYMTABLE
		dump_symtable(st_->st_top);
#endif
		return st_;
	}
	//alifSymtable_free(st_);
	return nullptr;
error:
	(void)symtable_exit_block(st_);
	//alifSymtable_free(st_);
	return nullptr;
}

