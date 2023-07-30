#pragma once

#define _AlifStatus_OK() {.type = 0,} // 0 -> Ok , 1 -> Error ,  2 -> Exit

/* ___________ AlifArgv ___________ */

class _AlifArgv {
public:
	Alif_ssize_t argc;
	int useCharArgv;
	char* const* charArgv;
	wchar_t* const* wcharArgv;
};

AlifStatus _alifArgv_asWstrList(const _AlifArgv* args, AlifWideStringList* list);


/* ___________ AlifConfig ___________ */

extern AlifStatus _alifConfig_setAlifArgv(AlifConfig* config, const _AlifArgv* args);
