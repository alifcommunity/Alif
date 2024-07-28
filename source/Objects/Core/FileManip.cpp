#include "alif.h"


#ifdef _WINDOWS
#include <Windows.h>
#endif // _WINDOWS

#define ALIF_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#pragma warning(disable : 4996) // for disable unsafe functions error











FILE* alif_fOpenObj(AlifObject* _path, const char* _mode) {
	FILE* f{};
#ifdef _WINDOWS
	wchar_t wmode[10];
	int uSize;

	uSize = MultiByteToWideChar(CP_ACP, 0, _mode, -1,
		wmode, ALIF_ARRAY_LENGTH(wmode));

	f = _wfopen((wchar_t*)((AlifUStrObject*)_path)->UTF, wmode);

#else
	AlifObject* bytes;
	const char* pathBytes;

	if (!alifUStr_fsConverter(_path, &bytes))
		return nullptr;
	pathBytes = (const char*)_alifWBytes_asString(bytes); // need review

	//if (alifSys_audit("open", "Osi", _path, _mode, 0) < 0) {
	//	ALIF_DECREF(bytes);
	//	return nullptr;
	//}

	do {
		f = fopen(pathBytes, _mode);
	} while (f == nullptr and errno == EINTR);
	int savedErrno = errno;
	ALIF_DECREF(bytes);
#endif

	return f;
}
