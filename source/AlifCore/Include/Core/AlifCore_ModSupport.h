#pragma once




extern AlifIntT _alifArg_noKwnames(const char*, AlifObject*); // 15
#define _ALIFARG_NOKWNAMES(_funcname, _kwnames) \
    (_kwnames == nullptr or _alifArg_noKwnames(_funcname, _kwnames))


AlifIntT _alifArg_checkPositional(const char*, AlifSizeT,
	AlifSizeT, AlifSizeT); // 30
#define _ALIF_ANY_VARARGS(_n) (_n == ALIF_SIZET_MAX)
#define _ALIFARG_CHECKPOSITIONAL(_funcname, _nargs, _min, _max) \
    ((!_ALIF_ANY_VARARGS(_max) and (_min) <= (_nargs) and (_nargs) <= (_max)) \
     or _alifArg_checkPositional((_funcname), (_nargs), (_min), (_max)))


extern AlifObject** _alif_vaBuildStack(AlifObject**, AlifSizeT,
	const char*, va_list, AlifSizeT*); // 37




extern AlifObject* alifModule_createInitialized(AlifModuleDef*); // 44





AlifObject* const* _alifArg_unpackKeywordsWithVarArg(AlifObject* const*, AlifSizeT, AlifObject*,
	AlifObject*, AlifArgParser*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifObject**); // 96
