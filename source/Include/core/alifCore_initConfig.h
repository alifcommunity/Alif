#pragma once

#define _AlifStatus_OK() {.type = 0,} // 0 -> Ok , 1 -> Error ,  2 -> Exit

/* ___________ AlifArgv ___________ */

class AlifArgv {
public:
	Alif_ssize_t argc;
	int useCharArgv;
	char* const* charArgv;
	wchar_t* const* wcharArgv;
};

AlifStatus alifArgv_asWstrList(const AlifArgv* args, AlifWideStringList* list);


/* ___________ AlifConfig ___________ */

extern AlifStatus alifConfig_read(AlifConfig*);
extern AlifStatus alifConfig_setAlifArgv(AlifConfig* config, const AlifArgv* args);
