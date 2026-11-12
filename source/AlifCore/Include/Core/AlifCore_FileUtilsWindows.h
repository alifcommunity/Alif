#pragma once


#ifdef _WINDOWS












typedef BOOL (WINAPI *PGetFileInformationByName)(
	PCWSTR FileName,
	FILE_INFO_BY_NAME_CLASS FileInformationClass,
	PVOID FileInfoBuffer,
	ULONG FileInfoBufferSize
	);

static inline BOOL _alif_getFileInformationByName(
	PCWSTR FileName,
	FILE_INFO_BY_NAME_CLASS FileInformationClass,
	PVOID FileInfoBuffer,
	ULONG FileInfoBufferSize
) {
	static PGetFileInformationByName GetFileInformationByName = NULL;
	static int GetFileInformationByName_init = -1;

	if (GetFileInformationByName_init < 0) {
		HMODULE hMod = LoadLibraryW(L"api-ms-win-core-file-l2-1-4");
		GetFileInformationByName_init = 0;
		if (hMod) {
			GetFileInformationByName = (PGetFileInformationByName)GetProcAddress(
				hMod, "GetFileInformationByName");
			if (GetFileInformationByName) {
				GetFileInformationByName_init = 1;
			} else {
				FreeLibrary(hMod);
			}
		}
	}

	if (GetFileInformationByName_init <= 0) {
		SetLastError(ERROR_NOT_SUPPORTED);
		return FALSE;
	}
	return GetFileInformationByName(FileName, FileInformationClass, FileInfoBuffer, FileInfoBufferSize);
}







#endif
