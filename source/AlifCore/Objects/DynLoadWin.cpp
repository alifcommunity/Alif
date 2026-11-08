#include "alif.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_ImportDL.h"

#include <windows.h>















const char* _alifImportDynLoadFiletab_[] = { // 12
	//ALIFD_TAGGED_SUFFIX,
	ALIFD_UNTAGGED_SUFFIX,
	nullptr
};






// 36
#define DWORD_AT(mem) (*(DWORD *)(mem))
#define WORD_AT(mem)  (*(WORD *)(mem))

static char* get_alifImport(HINSTANCE hModule) { // 39
	unsigned char* dllbase{}, * import_data{}, * import_name{};
	DWORD pe_offset, opt_offset;
	WORD opt_magic;
	int num_dict_off, import_off;

	/* Safety check input */
	if (hModule == nullptr) {
		return nullptr;
	}

	/* Module instance is also the base load address.  First portion of
	memory is the MS-DOS loader, which holds the offset to the PE
	header (from the load base) at 0x3C */
	dllbase = (unsigned char*)hModule;
	pe_offset = DWORD_AT(dllbase + 0x3C);

	/* The PE signature must be "PE\0\0" */
	if (memcmp(dllbase + pe_offset, "PE\0\0", 4)) {
		return nullptr;
	}

	/* Following the PE signature is the standard COFF header (20
	bytes) and then the optional header.  The optional header starts
	with a magic value of 0x10B for PE32 or 0x20B for PE32+ (PE32+
	uses 64-bits for some fields).  It might also be 0x107 for a ROM
	image, but we don't process that here.

	The optional header ends with a data dictionary that directly
	points to certain types of data, among them the import entries
	(in the second table entry). Based on the header type, we
	determine offsets for the data dictionary count and the entry
	within the dictionary pointing to the imports. */

	opt_offset = pe_offset + 4 + 20;
	opt_magic = WORD_AT(dllbase + opt_offset);
	if (opt_magic == 0x10B) {
		/* PE32 */
		num_dict_off = 92;
		import_off = 104;
	}
	else if (opt_magic == 0x20B) {
		/* PE32+ */
		num_dict_off = 108;
		import_off = 120;
	}
	else {
		/* Unsupported */
		return nullptr;
	}

	/* Now if an import table exists, offset to it and walk the list of
	imports.  The import table is an array (ending when an entry has
	empty values) of structures (20 bytes each), which contains (at
	offset 12) a relative address (to the module base) at which a
	string constant holding the import name is located. */

	if (DWORD_AT(dllbase + opt_offset + num_dict_off) >= 2) {
		/* We have at least 2 tables - the import table is the second
		one.  But still it may be that the table size is zero */
		if (0 == DWORD_AT(dllbase + opt_offset + import_off + sizeof(DWORD)))
			return NULL;
		import_data = dllbase + DWORD_AT(dllbase +
			opt_offset +
			import_off);
		while (DWORD_AT(import_data)) {
			import_name = dllbase + DWORD_AT(import_data + 12);
			if (strlen((const char*)import_name) >= 4 and
				!strncmp((const char*)import_name, "alif", 4)) {
				char* pch;

				/* Don't claim that alif.dll is a Alif DLL. */
			#ifdef _DEBUG
				if (strcmp((const char*)import_name, "alif_d.dll") == 0) {
				#else
				if (strcmp((const char*)import_name, "alif.dll") == 0) {
				#endif
					import_data += 20;
					continue;
				}

				/* Ensure alif prefix is followed only
				by numbers to the end of the basename */
				pch = (char*)(import_name + 6);
			#ifdef _DEBUG
				while (*pch && pch[0] != '_' && pch[1] != 'd' && pch[2] != '.') {
				#else
				while (*pch && *pch != '.') {
				#endif
					if (*pch >= '0' && *pch <= '9') {
						pch++;
					}
					else {
						pch = nullptr;
						break;
					}
				}

				if (pch) {
					/* Found it - return the name */
					return (char*)import_name;
				}
			}
			import_data += 20;
		}
	}

	return nullptr;
}









extern HMODULE _alifWinDLLhModule_; // 154






DLFuncPtr _alifImport_findSharedFuncptrWindows(const char* _prefix,
	const char* _shortname, AlifObject* _pathname, FILE* _fp) { // 203
	DLFuncPtr p{};
	char funcname[258]{}, * importAlif{};

//#ifdef ALIF_ENABLE_SHARED
//	_alif_checkAlif();
//#endif 

	wchar_t* wpathname = alifUStr_asWideCharString(_pathname, nullptr);
	if (wpathname == nullptr)
		return nullptr;

	alifOS_snprintf(funcname, sizeof(funcname), "%.20s_%.200s", _prefix, _shortname);

	{
		HINSTANCE hDLL = nullptr;
#ifdef MS_WINDOWS_DESKTOP
		AlifUIntT old_mode{};

		/* Don't display a message box when Alif can't load a DLL */
		old_mode = SetErrorMode(SEM_FAILCRITICALERRORS);
#endif

		ALIF_BEGIN_ALLOW_THREADS
		hDLL = LoadLibraryExW(wpathname, NULL,
				LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
				LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
		ALIF_END_ALLOW_THREADS
		alifMem_dataFree(wpathname);

#ifdef MS_WINDOWS_DESKTOP
		/* restore old error mode settings */
		SetErrorMode(old_mode);
#endif

		if (hDLL == nullptr) {
			AlifObject* message{};
			AlifUIntT errorCode{};

			/* Get an error string from Win32 error code */
			wchar_t theInfo[256]{}; /* Pointer to error text
								  from system */
			int theLength; /* Length of error text */

			errorCode = GetLastError();

			theLength = FormatMessageW(
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS, /* flags */
				nullptr, /* message source */
				errorCode, /* the message (error) ID */
				MAKELANGID(LANG_NEUTRAL,
					SUBLANG_DEFAULT),
				/* Default language */
				theInfo, /* the buffer */
				sizeof(theInfo) / sizeof(wchar_t), /* size in wchars */
				nullptr); /* no additional format args. */

			/* Problem: could not get the error message.
			   This should not happen if called correctly. */
			if (theLength == 0) {
				message = alifUStr_fromFormat(
					"فشل تحميل DLL مع إرجاع رمز الخطأ %u اثناء استيراد %s",
					errorCode, _shortname);
			}
			else {
				/* For some reason a \r\n
				   is appended to the text */
				if (theLength >= 2 and
					theInfo[theLength - 2] == '\r' and
					theInfo[theLength - 1] == '\n') {
					theLength -= 2;
					theInfo[theLength] = '\0';
				}
				message = alifUStr_fromFormat(
					"فشل تحميل DLL اثناء استيراد %s: ", _shortname);

				alifUStr_appendAndDel(&message,
					alifUStr_fromWideChar(
						theInfo,
						theLength));
			}
			if (message != nullptr) {
				AlifObject* shortname_obj = alifUStr_fromString(_shortname);
				alifErr_setImportError(message, shortname_obj, _pathname);
				ALIF_XDECREF(shortname_obj);
				ALIF_DECREF(message);
			}
			return nullptr;
		}
		else {
			char buffer[256]{};

			alifOS_snprintf(buffer, sizeof(buffer),
#ifdef _DEBUG
				"alif%d%d_d.dll",
#else
				"alif%d%d.dll",
#endif
				ALIF_MAJOR_VERSION, ALIF_MINOR_VERSION);
			importAlif = get_alifImport(hDLL);

			if (importAlif and
				_stricmp(buffer, importAlif)) {
				alifErr_format(_alifExcImportError_,
					"تضارب الوحدة %.150s "
					"مع هذه النسخة من ألف.",
					importAlif);
				ALIF_BEGIN_ALLOW_THREADS
					FreeLibrary(hDLL);
				ALIF_END_ALLOW_THREADS
					return nullptr;
			}
		}
		ALIF_BEGIN_ALLOW_THREADS
		p = GetProcAddress(hDLL, funcname);
		ALIF_END_ALLOW_THREADS

		//* alif
		// to detect not loaded mangaled names from c++
		if (p == nullptr) {
			AlifObject* message = alifUStr_fromFormat(
				"لم يتم العثور على الدالة %s اثناء استيراد الوحدة %s: ", funcname, _shortname);
			AlifObject* shortname_obj = alifUStr_fromString(_shortname);
			alifErr_setImportError(message, shortname_obj, _pathname);
			ALIF_XDECREF(shortname_obj);
			ALIF_DECREF(message);

			return nullptr;
		}
		//* alif
	}

	return p;
}
