#pragma once




extern AlifIntT _alifArg_noKwnames(const char*, AlifObject*); // 15
#define _ALIFARG_NOKWNAMES(_funcname, _kwnames) \
    (_kwnames == nullptr or _alifArg_noKwnames(_funcname, _kwnames))


AlifIntT _alifArg_noKeywords(const char*, AlifObject*); // 25
#define _ALIFARG_NOKEYWORDS(_funcname, _kwargs) \
    (_kwargs == nullptr or _alifArg_noKeywords(_funcname, _kwargs))

AlifIntT _alifArg_checkPositional(const char*, AlifSizeT,
	AlifSizeT, AlifSizeT); // 30
#define _ALIF_ANY_VARARGS(_n) (_n == ALIF_SIZET_MAX)
#define _ALIFARG_CHECKPOSITIONAL(_funcname, _nargs, _min, _max) \
    ((!_ALIF_ANY_VARARGS(_max) and (_min) <= (_nargs) and (_nargs) <= (_max)) \
     or _alifArg_checkPositional((_funcname), (_nargs), (_min), (_max)))


extern AlifObject** _alif_vaBuildStack(AlifObject**, AlifSizeT,
	const char*, va_list, AlifSizeT*); // 37




extern AlifObject* alifModule_createInitialized(AlifModuleDef*); // 44

AlifIntT _alifArg_parseStackAndKeywords(AlifObject* const*,
	AlifSizeT, AlifObject*, AlifArgParser*, ...); // 71

AlifObject* const* _alifArg_unpackKeywords(AlifObject* const*, AlifSizeT,
	AlifObject*, AlifObject*, AlifArgParser*, AlifIntT, AlifIntT, AlifIntT, AlifObject**); // 79

 // 89
#define ALIFARG_UNPACKKEYWORDS(args, nargs, kwargs, kwnames, parser, minpos, maxpos, minkw, buf) \
    (((minkw) == 0 and (kwargs) == nullptr and (kwnames) == nullptr and \
      (minpos) <= (nargs) and (nargs) <= (maxpos) and (args) != nullptr) ? (args) : \
     _alifArg_unpackKeywords((args), (nargs), (kwargs), (kwnames), (parser), \
                           (minpos), (maxpos), (minkw), (buf)))



AlifObject* const* _alifArg_unpackKeywordsWithVarArg(AlifObject* const*, AlifSizeT, AlifObject*,
	AlifObject*, AlifArgParser*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifObject**); // 96
