#include "alif_version.h"

#define MS_WINDOWS

#ifdef MS_WINDOWS
int wmain(int argc, wchar_t** argv)
{
	wchar_t* argsv[] = { L"alif", L"example.alif" };
	return Alif_Main(2, argsv);
}
#else
int main(int argc, char** argv)
{
	char* argsv[] = { "alif5", "example.alif5" };
	return Py_BytesMain(2, argsv);
}
#endif
