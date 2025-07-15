#include "AlifCore_Abstract.h"
#include "AlifCore_ModSupport.h"
























// 307
#define _IO__IOBASE_READLINE_METHODDEF    \
    {"قراءة_سطر", ALIF_CPPFUNCTION_CAST(_io_IOBase_readline), METHOD_FASTCALL},

static AlifObject* _io_IOBase_readlineImpl(AlifObject*, AlifSizeT);

static AlifObject* _io_IOBase_readline(AlifObject* self,
	AlifObject* const* args, AlifSizeT nargs) { // 313
	AlifObject* returnValue = nullptr;
	AlifSizeT limit = -1;

	if (!_ALIFARG_CHECKPOSITIONAL("قراءة_سطر", nargs, 0, 1)) {
		goto exit;
	}
	if (nargs < 1) {
		goto skip_optional;
	}
	if (!_alifConvertOptional_toSizeT(args[0], &limit)) {
		goto exit;
	}
skip_optional:
	returnValue = _io_IOBase_readlineImpl(self, limit);

exit:
	return returnValue;
}
