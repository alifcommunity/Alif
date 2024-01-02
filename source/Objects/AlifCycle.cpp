#include "alif.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Memory.h"
#include "AlifCore_AlifCycle.h"

#ifdef _WINDOWS
#include <windows.h>
#include <fcntl.h>
#else
#include <locale>
#include <codecvt>
#endif


AlifRuntime alifRuntime{};



void alifRuntime_init() {

	alifRuntime.selfInitializing = 1;


	alifRuntime.mainThread = GetCurrentThreadId();


	alifRuntime.selfInitialized = 1;
}




void alif_consoleInit() {
#ifdef _WINDOWS
	bool modeIn = _setmode(_fileno(stdin), _O_WTEXT);
	bool modeOut = _setmode(_fileno(stdout), _O_WTEXT);
	if (!modeIn or !modeOut) {
		std::wcout << L"لم يستطع تهيئة الطرفية لقراءة الأحرف العربية" << std::endl;
		exit(-1);
	}
#endif // _WINDOWS

	const char* locale = setlocale(LC_ALL, nullptr);
	if (locale == nullptr) {
		std::wcout << L"لم يستطع تهيئة الطرفية لقراءة الأحرف العربية" << std::endl;
		exit(-1);
	}

#ifndef __ANDROID__
	char* res = setlocale(LC_ALL, "");
#endif // !__ANDROID__

	setlocale(LC_ALL, locale);
}







void alif_initFromConfig(AlifConfig* _config) {

	alifConfig_read(_config);

}
