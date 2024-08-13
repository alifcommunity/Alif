#pragma once





extern AlifObject* alifModule_createInitialized(AlifModuleDef*);

extern int alifSubArg_noKwnames(const wchar_t* , AlifObject* );

#define ALIFSUBARG_NOKWNAMES(_funcname, _kwnames) ((_kwnames) == nullptr or alifSubArg_noKwnames((_funcname), (_kwnames)))


#define ALIF_ANY_VARARGS(_n) ((_n) == LLONG_MAX)

int _alifArg_checkPositional(const wchar_t*, int64_t, int64_t, int64_t);
#define ALIFARG_CHECKPOSITIONAL(funcname, nargs, _min, _max) \
    ((!ALIF_ANY_VARARGS(_max) && (_min) <= (nargs) && (nargs) <= (_max)) or _alifArg_checkPositional((funcname), (nargs), (_min), (_max)))







class AlifArgParser {
public:
	int initialized;
	const wchar_t* format;
	const wchar_t* const* keywords;
	const wchar_t* fname;
	const wchar_t* customMsg;
	int pos;
	int min;
	int max;
	AlifObject* kwTuple{};
	class AlifArgParser* next;
};




AlifObject* const* alifArg_unpackKeywords(AlifObject* const*, int64_t, AlifObject*, AlifObject*, AlifArgParser*, int, int, int, AlifObject**);
#define ALIFARG_UNPACKKEYWORDS(args, nargs, kwargs, kwnames, parser, minpos, maxpos, minkw, buf) \
    (((minkw) == 0 and (kwargs) == nullptr and (kwnames) == nullptr and (minpos) <= (nargs) and (nargs) <= (maxpos) and (args) != nullptr) ? (args) : \
     alifArg_unpackKeywords((args), (nargs), (kwargs), (kwnames), (parser), (minpos), (maxpos), (minkw), (buf)))
