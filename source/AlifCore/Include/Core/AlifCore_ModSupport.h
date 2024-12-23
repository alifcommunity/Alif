#pragma once









extern AlifObject** _alif_vaBuildStack(AlifObject**, AlifSizeT,
	const char*, va_list, AlifSizeT*); // 37




extern AlifObject* alifModule_createInitialized(AlifModuleDef*); // 44





AlifObject* const* _alifArg_unpackKeywordsWithVarArg(AlifObject* const*, AlifSizeT, AlifObject*,
	AlifObject*, AlifArgParser*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifObject**); // 96
