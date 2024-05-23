#include "alif.h"


#include "AlifCore_AST.h"
#include "AlifCore_Compile.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_SymTable.h"




class AlifCompiler { // 275
public:
	AlifObject* fileName{};
	AlifSymTable* symTable{};

	AlifIntT optimize{};
	AlifIntT nestLevel{};

	AlifObject* stack{};
	AlifASTMem* astMem{};
};

static AlifIntT compiler_setup(AlifCompiler* _compiler, Module* _module,
	AlifObject* _fn, AlifIntT _optimize, AlifASTMem* _astMem) { // 380

	_compiler->stack = alifNew_list(0);
	if (_compiler->stack) return -1;

	_compiler->fileName = alif_newRef(_fn); // need Change to MACRO
	_compiler->astMem = _astMem;

	_compiler->optimize = (_optimize == -1) ? alifConfig_get()->optimizationLevel : _optimize;
	_compiler->nestLevel = 0;

	if (!alifAST_optimize(_module, _astMem, _compiler->optimize)) return -1;

	_compiler->symTable = alifSymTable_build(_module, _fn);
	if (_compiler->symTable == nullptr) {
		// error
		return -1;
	}

	return 1;
}

static AlifCompiler* compiler_new(Module* _module, AlifObject* _fn, AlifIntT _optimize, AlifASTMem* _astMem) { // 425

	AlifCompiler* compiler_ = (AlifCompiler*)alifMem_dataAlloc(sizeof(AlifCompiler));
	if (compiler_ == nullptr) return nullptr;

	if (compiler_setup(compiler_, _module, _fn, _optimize, _astMem) < 0) {
		//compiler_free(compiler_);
		return nullptr;
	}

	return compiler_;
}

AlifCodeObject* alifAST_compile(Module* _module, AlifObject* _fn, AlifIntT _optimize, AlifASTMem* _astMem) { // 440

	AlifCompiler* compiler_ = compiler_new(_module, _fn, _optimize, _astMem);
	if (compiler_ == nullptr) return nullptr;

	//AlifCodeObject* codeObject = compiler_module(compiler_, _module);
	//compiler_free(compiler_);

	//return codeObject;
	return nullptr; // temp
}

