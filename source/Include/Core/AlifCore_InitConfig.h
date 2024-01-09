#pragma once

/* ----- AlifArgv ------------------------------------------- */

class AlifArgv {
public:
	AlifSizeT argc;
	int useBytesArgv;
	char* const* bytesArgv;
	wchar_t* const* wcharArgv;
};



void alifArgv_asWStrList(AlifConfig*, AlifArgv*);
void alifConfig_read(AlifConfig*);
