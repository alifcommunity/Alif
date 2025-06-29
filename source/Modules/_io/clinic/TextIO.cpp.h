
#include "AlifCore_Abstract.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"




































#define _IO_TEXTIOWRAPPER_WRITE_METHODDEF    \
    {"اكتب", (AlifCPPFunction)_ioTextIOWrapper_write, METHOD_O},

//static AlifObject* _ioTextIOWrapper_writeImpl(TextIO* self, AlifObject* text);

static AlifObject* _ioTextIOWrapper_write(TextIO* self, AlifObject* arg) { // 759
	AlifObject* returnValue = nullptr;
	AlifObject* text{};

	if (!ALIFUSTR_CHECK(arg)) {
		//_alifArg_badArgument("write", "argument", "str", arg);
		goto exit;
	}
	text = arg;
	ALIF_BEGIN_CRITICAL_SECTION(self);
	//returnValue = _ioTextIOWrapper_writeImpl(self, text);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}
