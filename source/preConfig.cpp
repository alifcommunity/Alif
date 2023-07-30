#include "Alif.h"
#include "alifcore_initConfig.h"    // _AlifArgv



AlifStatus _alifArgv_asWstrList(const _AlifArgv* args, AlifWideStringList* list)
{
	AlifWideStringList wArgv = { .length = 0, .items = nullptr };
	if (args->use_bytes_argv) {
		//size_t size = sizeof(wchar_t*) * args->argc;
		//wargv.items = (wchar_t**)PyMem_RawMalloc(size);
		//if (wargv.items == NULL) {
		//	return _PyStatus_NO_MEMORY();
		//}

		//for (Py_ssize_t i = 0; i < args->argc; i++) {
		//	size_t len;
		//	wchar_t* arg = Py_DecodeLocale(args->bytes_argv[i], &len);
		//	if (arg == NULL) {
		//		_PyWideStringList_Clear(&wargv);
		//		return DECODE_LOCALE_ERR("command line arguments", len);
		//	}
		//	wargv.items[i] = arg;
		//	wargv.length++;
		//}

		//_PyWideStringList_Clear(list);
		//*list = wargv;
	}
	else {
		wArgv.length = args->argc;
		wArgv.items = (wchar_t**)args->wchar_argv;
		//if (_AlifWideStringList_Copy(list, &wargv) < 0) {
		//	return _ALifStatus_NO_MEMORY();
		//}
	}
	return _AlifStatus_OK();
}
