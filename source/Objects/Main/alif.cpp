#include "alif.h"
#include "AlifCore_InitConfig.h"


//#ifdef _WINDOWS
//#include <Windows.h>
//#include <fcntl.h>
//#include <io.h>
//#else
//#include <cstring>
//#include <codecvt>
//#include <locale>
//#endif





int alif_mainWchar(int _argc, wchar_t** _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 0,
		.bytesArgv = nullptr,
		.wcharArgv = _argv
	};
	return alifMain_main(&args);
}



int alif_mainBytes(int _argc, char** _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 1,
		.bytesArgv = _argv,
		.wcharArgv = nullptr
	};
	return 0;
	//return alifMain_main(&args);
}

#ifdef _WINDOWS
int wmain(int _argc, wchar_t** _argv)
{
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stdin), _O_WTEXT);

	std::wcout << alif_getVersion() << std::endl;
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	return alif_mainWchar(2, argsv);
}
#else
int main(int _argc, char** _argv)
{
	char* argsv[] = { (char*)"alif", (char*)"example.alif" };
	return alif_mainBytes(2, argsv);
}
#endif
