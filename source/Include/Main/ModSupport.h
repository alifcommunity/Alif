#pragma once

int _alifArg_checkPositional(const wchar_t*, int64_t, int64_t, int64_t);

#define ALIFARG_CHECKPOSITIONAL(funcname, nargs, min, max) \
    ((!_Py_ANY_VARARGS(max) && (min) <= (nargs) && (nargs) <= (max)) or _alifArg_checkPositional((funcname), (nargs), (min), (max)))


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

int alifArg_parseTuple(AlifObject* , const wchar_t* , ...);

AlifObject* const* alifArg_unpackKeywords(AlifObject* const*, int64_t, AlifObject* , AlifObject* ,
    AlifArgParser* , int , int , int , AlifObject**);

#define ALIFARG_UNPACKKEYWORDS(args, nargs, kwargs, kwnames, parser, minpos, maxpos, minkw, buf) \
    (((minkw) == 0 and (kwargs) == nullptr and (kwnames) == nullptr and (minpos) <= (nargs) and (nargs) <= (maxpos) and (args) != nullptr) ? (args) : \
     alifArg_unpackKeywords((args), (nargs), (kwargs), (kwnames), (parser), (minpos), (maxpos), (minkw), (buf)))
