#pragma once


AlifCodeObject* alifAST_compile(Module*, AlifObject*, AlifIntT, AlifASTMem*);


extern AlifIntT alifAST_optimize(Module*, AlifASTMem*, AlifIntT);
