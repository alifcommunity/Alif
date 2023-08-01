#include "Alif.h"
#include "alifcore_initConfig.h"    // _AlifArgv



AlifStatus alifArgv_asWstrList(const AlifArgv* args, AlifWideStringList* list)
{
	AlifWideStringList wArgv = { .length = 0, .items = nullptr };
	if (args->useCharArgv) {

	}
	else {
		wArgv.length = args->argc;
		wArgv.items = (wchar_t**)args->wcharArgv;

		*list = wArgv;
	}


	return ALIFSTATUS_OK();
}
