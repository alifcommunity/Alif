/*

Entry point for the Windows NT DLL.

About the only reason for having this, is so initall() can automatically
be called, removing that burden (and possible source of frustration if
forgotten) from the programmer.

*/

#include "alif.h"
#include "windows.h"

#ifdef ALIF_ENABLE_SHARED

// Alif Globals
HMODULE _alifWinDLLhModule_ = NULL;
//const char *_alifWinDLLVersionString_ = MS_DLL_ID;
const char* _alifWinDLLVersionString_ = "5.4"; //* alif //* todo

BOOL    WINAPI  DllMain (HANDLE hInst,
	ULONG ul_reason_for_call,
	LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		_alifWinDLLhModule_ = (HMODULE)hInst;
		break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif
