#pragma once

#include "AlifCore_SymTable.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_Memory.h"





AlifCodeObject* alifAST_compile(Module*, AlifObject*, AlifCompilerFlags*, AlifIntT, AlifASTMem*); // 18


extern AlifIntT alifAST_optimize(Module*, AlifASTMem*, AlifIntT, AlifIntT); // 35



class AlifCompileCodeUnitMetadata { // 42
public:
	AlifObject* name{};
	AlifObject* qualname{};  /* dot-separated qualified name (lazy) */

	/* The following fields are dicts that map objects to
	   the index of them in co_XXX.      The index is used as
	   the argument for opcodes that refer to those collections.
	*/
	AlifObject* consts{};    /* all constants */
	AlifObject* names{};     /* all names */
	AlifObject* varnames{};  /* local variables */
	AlifObject* cellvars{};  /* cell variables */
	AlifObject* freevars{};  /* free variables */
	AlifObject* fasthidden{}; /* dict; keys are names that are fast-locals only
							   temporarily within an inlined comprehension. When
							   value is True, treat as fast-local. */

	AlifSizeT argCount{};        /* number of arguments for block */
	AlifSizeT posOnlyArgCount{};        /* number of positional only arguments for block */
	AlifSizeT kwOnlyArgCount{}; /* number of keyword only arguments for block */

	AlifIntT firstLineno; /* the first lineno of the block */
};




AlifIntT _alifCompile_ensureArrayLargeEnough(AlifIntT,
	void**, AlifIntT*, AlifIntT, AlifUSizeT); // 68


AlifIntT _alifCompile_constCacheMergeOne(AlifObject*, AlifObject**); // 75
