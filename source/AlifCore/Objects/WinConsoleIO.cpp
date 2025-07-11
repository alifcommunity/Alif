#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_Object.h"




#ifdef HAVE_WINDOWS_CONSOLE_IO // 14






char _get_consoleType(HANDLE handle) { // 51
	DWORD mode{}, peek_count{};

	if (handle == INVALID_HANDLE_VALUE)
		return '\0';

	if (!GetConsoleMode(handle, &mode))
		return '\0';

	/* Peek at the handle to see whether it is an input or output handle */
	if (GetNumberOfConsoleInputEvents(handle, &peek_count))
		return 'r';
	return 'w';
}


char _alifIO_getConsoleType(AlifObject* _pathOrFd) { // 66
	AlifIntT fd = alifLong_asLong(_pathOrFd);
	alifErr_clear();
	if (fd >= 0) {
		HANDLE handle = _alifGet_osfHandleNoRaise(fd);
		if (handle == INVALID_HANDLE_VALUE)
			return '\0';
		return _get_consoleType(handle);
	}

	AlifObject* decoded{};
	wchar_t* decodedWstr{};

	if (!alifUStr_fsDecoder(_pathOrFd, &decoded)) {
		alifErr_clear();
		return '\0';
	}
	decodedWstr = alifUStr_asWideCharString(decoded, nullptr);
	ALIF_CLEAR(decoded);
	if (!decodedWstr) {
		alifErr_clear();
		return '\0';
	}

	char m = '\0';
	if (!_wcsicmp(decodedWstr, L"CONIN$")) {
		m = 'r';
	}
	else if (!_wcsicmp(decodedWstr, L"CONOUT$")) {
		m = 'w';
	}
	else if (!_wcsicmp(decodedWstr, L"CON")) {
		m = 'x';
	}
	if (m) {
		alifMem_dataFree(decodedWstr);
		return m;
	}

	DWORD length{};
	wchar_t nameBuf[MAX_PATH], * pnameBuf = nameBuf;

	length = GetFullPathNameW(decodedWstr, MAX_PATH, pnameBuf, NULL);
	if (length > MAX_PATH) {
		pnameBuf = (AlifUSizeT)(length) > ALIF_SIZET_MAX / sizeof(wchar_t) ? nullptr : \
			(wchar_t*)alifMem_dataAlloc((length) * sizeof(length));
		if (pnameBuf)
			length = GetFullPathNameW(decodedWstr, length, pnameBuf, NULL);
		else
			length = 0;
	}
	alifMem_dataFree(decodedWstr);

	if (length) {
		wchar_t* name = pnameBuf;
		if (length >= 4 && name[3] == L'\\' &&
			(name[2] == L'.' || name[2] == L'?') &&
			name[1] == L'\\' && name[0] == L'\\') {
			name += 4;
		}
		if (!_wcsicmp(name, L"CONIN$")) {
			m = 'r';
		}
		else if (!_wcsicmp(name, L"CONOUT$")) {
			m = 'w';
		}
		else if (!_wcsicmp(name, L"CON")) {
			m = 'x';
		}
	}

	if (pnameBuf != nameBuf)
		alifMem_dataFree(pnameBuf);
	return m;
}





#endif /* HAVE_WINDOWS_CONSOLE_IO */
