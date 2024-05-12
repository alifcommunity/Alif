#pragma once

extern int alifSubArg_noKwnames(const wchar_t* , AlifObject* );

#define ALIFSUBARG_NOKWNAMES(_funcname, _kwnames) ((_kwnames) == nullptr or alifSubArg_noKwnames((_funcname), (_kwnames)))

int alifSubArg_checkPositional(const wchar_t*, int64_t, int64_t, int64_t);

#define ALIFSUB_ANY_VARARGS(_n) ((_n) == LLONG_MAX)

#define ALIFSUBARG_CHECKPOSITIONAL(_funcname, _nargs, min, max) ((!ALIFSUB_ANY_VARARGS(max) and (min) <= (_nargs) && (_nargs) <= (max)) or alifSubArg_checkPositional((_funcname), (_nargs), (min), (max)))