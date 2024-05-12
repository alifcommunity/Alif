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
	PyObject* bytes;
	const char* path_bytes;

	assert(PyGILState_Check());

	if (!PyUnicode_FSConverter(path, &bytes))
		return NULL;
	path_bytes = PyBytes_AS_STRING(bytes);

	if (PySys_Audit("open", "Osi", path, mode, 0) < 0) {
		Py_DECREF(bytes);
		return NULL;
	}

	do {
		Py_BEGIN_ALLOW_THREADS
			f = fopen(path_bytes, mode);
		Py_END_ALLOW_THREADS
	} while (f == NULL
		&& errno == EINTR && !(async_err = PyErr_CheckSignals()));
		int saved_errno = errno;
		Py_DECREF(bytes);
#endif

	return f;

}
