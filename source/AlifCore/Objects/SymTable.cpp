#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Parser.h"
#include "AlifCore_State.h"
#include "AlifCore_SymTable.h"

class AlifSymTable* alifSymtable_build(ModuleTy mod, AlifObject* filename, AlifFutureFeatures* future) { // 394
	class symtable* st = symtable_new();
	ASDLStmtSeq* seq{};
	AlifSizeT i{};
	AlifThread* tstate{};
	AlifIntT startingRecursionDepth;

	if (st == NULL)
		return NULL;
	if (filename == NULL) {
		_Symtable_Free(st);
		return NULL;
	}
	st->filename = ALIF_NEWREF(filename);
	st->future = future;

	tstate = alifThread_get();
	if (!tstate) {
		_Symtable_Free(st);
		return NULL;
	}
	/* Be careful here to prevent overflow. */
	AlifIntT recursion_depth = _C_RECURSION_LIMIT - tstate->c_recursion_remaining;
	starting_recursion_depth = recursion_depth;
	st->recursion_depth = starting_recursion_depth;
	st->recursion_limit = _C_RECURSION_LIMIT;

	/* Make the initial symbol information gathering pass */

	__SourceLocation loc0 = { 0, 0, 0, 0 };
	if (!symtable_enter_block(st, &__ID(top), ModuleBlock, (void*)mod, loc0)) {
		_Symtable_Free(st);
		return NULL;
	}

	st->st_top = st->st_cur;
	switch (mod->kind) {
	case Module_kind:
		seq = mod->v.Module.body;
		for (i = 0; i < asdl_seq_LEN(seq); i++)
			if (!symtable_visit_stmt(st,
				(stmt_ty)asdl_seq_GET(seq, i)))
				goto error;
		break;
	case Expression_kind:
		if (!symtable_visit_expr(st, mod->v.Expression.body))
			goto error;
		break;
	case Interactive_kind:
		seq = mod->v.Interactive.body;
		for (i = 0; i < asdl_seq_LEN(seq); i++)
			if (!symtable_visit_stmt(st,
				(stmt_ty)asdl_seq_GET(seq, i)))
				goto error;
		break;
	case FunctionType_kind:
		Err_SetString(Exc_RuntimeError,
			"this compiler does not handle FunctionTypes");
		goto error;
	}
	if (!symtable_exit_block(st)) {
		_Symtable_Free(st);
		return NULL;
	}
	/* Check that the recursion depth counting balanced correctly */
	if (st->recursion_depth != starting_recursion_depth) {
		Err_Format(Exc_SystemError,
			"symtable analysis recursion depth mismatch (before=%d, after=%d)",
			starting_recursion_depth, st->recursion_depth);
		_Symtable_Free(st);
		return NULL;
	}
	/* Make the second symbol analysis pass */
	if (symtable_analyze(st)) {
#if __DUMP_SYMTABLE
		dump_symtable(st->st_top);
#endif
		return st;
	}
	_Symtable_Free(st);
	return NULL;
error:
	(void)symtable_exit_block(st);
	_Symtable_Free(st);
	return NULL;
}

