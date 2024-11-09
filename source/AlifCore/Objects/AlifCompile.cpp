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


typedef AlifInstructionSequence InstrSequence; // 84


typedef AlifSourceLocation Location; // 99


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

	if (!_alifAST_optimize(_mod, _astMem, _c->optimize, merged)) {
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
