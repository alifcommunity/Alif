#pragma once

#include "AlifCore_ModSupport.h"



#define BUILTIN_PRINT_METHODDEF {"اطبع", ALIF_CPPFUNCTION_CAST(builtin_print), METHOD_FASTCALL | METHOD_KEYWORDS} // 900


static AlifObject* builtin_print(AlifObject* module, AlifObject** args,
	AlifSizeT nargs, AlifObject* kwnames) { // 907

	wchar_t res[10]{};

	//if (!args) {
	//	goto exit;
	//}

	//swprintf(res, 10, L"%d", ((AlifIntegerObject*)*args)->digits_);

	std::wcout << res << std::endl;


//exit:
	//return ALIF_NONE;
	return (AlifObject*)1;
}
