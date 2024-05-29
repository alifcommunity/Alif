#pragma once


AlifCodeObject* alifAST_compile(Module*, AlifObject*, AlifIntT, AlifASTMem*);


extern AlifIntT alifAST_optimize(Module*, AlifASTMem*, AlifIntT);





class AlifCompileCodeUnitData {
public:
	AlifObject* name{};
	AlifObject* qualName{};

	AlifObject* consts{};
	AlifObject* names{};
	AlifObject* varNames{};
	AlifObject* cellVars{};
	AlifObject* freeVars{};
	AlifObject* fastHidden{};

	AlifSizeT argCount{};
	AlifSizeT posOnlyArgCount{};
	AlifSizeT kwOnlyArgCount{};

	AlifIntT firstLineNo{};
};
