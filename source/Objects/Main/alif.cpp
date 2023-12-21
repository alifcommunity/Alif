#include "alif.h"
#include "AlifCore_Memory.h"

#include <chrono>

#ifdef _WINDOWS
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#else
#include <codecvt>
#include <locale>
#endif


/* ------ !for test only ------ */
class AlifObj {
public:
	AlifSizeT ref{};
	const char* type{};
};
/* ------ !for test only ------ */


int wmain()
{

#if defined(_WINDOWS)
	bool a = _setmode(_fileno(stdout), _O_WTEXT);
	bool b = _setmode(_fileno(stdin), _O_WTEXT);
	if (!a or !b) { exit(-3); }
#else
	setlocale(LC_ALL, "");
#endif

	alif_memoryInit();

	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < 20000000; i++)
	{
		wchar_t* data1 = (wchar_t*)alifMem_dataAlloc(16);
		memcpy(data1, L"abcdefg", sizeof(L"abcdefg"));
		wchar_t* data2 = (wchar_t*)alifMem_dataRealloc(data1, 64);
		alifMem_dataFree(data2);

		//wchar_t* data1 = (wchar_t*)malloc(16);
		//memcpy(data1, L"abcdefg", sizeof(L"abcdefg"));
		//wchar_t* data2 = (wchar_t*)realloc(data1, 64);
		//free(data2);
	}

	auto end = std::chrono::high_resolution_clock::now() - start;
	alif_getMemState();

	std::wcout << end.count() / 1000000 << L"ms" << std::endl;

	return 0;
}
