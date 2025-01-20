#pragma once












extern AlifIntT alifFloat_initTypes(AlifInterpreter*); // 16





extern AlifIntT _alifFloat_formatAdvancedWriter(AlifUStrWriter*, AlifObject*,
	AlifObject*, AlifSizeT, AlifSizeT); // 44


extern AlifObject* _alifString_toNumberWithUnderscores(const char*, AlifSizeT,
	const char*, AlifObject*, void*,
	AlifObject* (*_innerFunc)(const char*, AlifSizeT, void*)); // 51
