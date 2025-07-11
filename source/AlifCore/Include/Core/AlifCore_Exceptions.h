#pragma once






extern AlifIntT _alifExc_initTypes(AlifInterpreter*); // 16




class AlifExcState { // 22
public:
	AlifObject* errNoMap{};
	AlifBaseExceptionObject* memErrorsFreeList{};
	AlifIntT memErrorsNumFree{};
	// The ExceptionGroup type
	AlifObject* alifExcExceptionGroup{};
};
