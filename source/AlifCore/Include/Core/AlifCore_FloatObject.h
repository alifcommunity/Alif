#pragma once












extern AlifIntT alifFloat_initTypes(AlifInterpreter*); // 16


enum AlifFloatFormatType { // 22
	Alif_Float_Format_Unknown,
	Alif_Float_Format_IEEE_Big_Endian,
	Alif_Float_Format_IEEE_Little_Endian,
};

class AlifFloatRuntimeState { // 28
public:
	AlifFloatFormatType floatFormat{};
	AlifFloatFormatType doubleFormat{};
};



extern AlifIntT _alifFloat_formatAdvancedWriter(AlifUStrWriter*, AlifObject*,
	AlifObject*, AlifSizeT, AlifSizeT); // 44


extern AlifObject* _alifString_toNumberWithUnderscores(const char*, AlifSizeT,
	const char*, AlifObject*, void*,
	AlifObject* (*_innerFunc)(const char*, AlifSizeT, void*)); // 51
