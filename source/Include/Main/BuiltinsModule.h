#pragma once

#include "AlifCore_ModSupport.h"



#define BUILTIN_PRINT_METHODDEF {L"اطبع", ALIF_CPPFUNCTION_CAST(builtin_print), METHOD_FASTCALL|METHOD_KEYWORDS}

static AlifObject* builtin_print(AlifObject* module, AlifObject* args, AlifSizeT nargs, AlifObject* kwnames) {

	const wchar_t* res{};

	if (!args) {
		goto exit;
	}

	res = alifUStr_asUTF8(args);

	std::wcout << res << std::endl;

exit:
	return nullptr;
}
