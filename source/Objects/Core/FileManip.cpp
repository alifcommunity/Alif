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
	const char* path_bytes;

	assert(alifGILState_Check());

	if (!alifUnicode_FSConverter(path, &bytes))
		return nullptr;
	path_bytes = alifBytes_asString(bytes);

	if (alifSys_audit("open", "Osi", path, mode, 0) < 0) {
		ALIF_DECREF(bytes);
		return nullptr;
	}

	do {
		ALIF_BEGIN_ALLOW_THREADS
			f = fopen(pathBytes, mode);
		ALIF_END_ALLOW_THREADS
	} while (f == nullptr
		and errno == EINTR and !(async_err = alifErr_checkSignals()));
		int savedErrno = errno;
		Alif_DECREF(bytes);
#endif

	return f;
}
