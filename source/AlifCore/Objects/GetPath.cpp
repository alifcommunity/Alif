#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_PathConfig.h"

#include "AlifCore_State.h"


















AlifIntT _alifConfig_initPathConfig(AlifConfig* _config, AlifIntT _computePathConfig) { // 863
	AlifIntT status = _alifPathConfig_readGlobal(_config);

	if (status != 1 or !_computePathConfig) {
		return status;
	}

	//* alif

	const wchar_t* folderName = L"lib";

	wchar_t* buffer = nullptr;

#ifdef _WINDOWS
	// Windows-specific code
	DWORD size = GetCurrentDirectoryW(0, nullptr);
	if (size == 0) {
		return -1;
	}
	buffer = (wchar_t*)alifMem_dataAlloc(size * sizeof(wchar_t));
	if (buffer == nullptr) {
		return -1;
	}
	if (GetCurrentDirectoryW(size, buffer) == 0) {
		free(buffer);
		return -1;
	}
#else
	// POSIX-specific code (Linux, macOS, etc.)
	char cwd[PATH_MAX]{};
	if (getcwd(cwd, sizeof(cwd)) {
		// Convert the current directory to a wide string
		AlifUSizeT len = mbstowcs(nullptr, cwd, 0) + 1;
		buffer = (wchar_t*)alifMem_dataAlloc(len * sizeof(wchar_t));
		if (buffer == nullptr) {
			return -1;
		}
		mbstowcs(buffer, cwd, len);
	}
	else {
		return -1;
	}
#endif


	wchar_t* current_dir = buffer;
	if (current_dir == nullptr) {
		return -1;
	}

	// Calculate the length of the final path
	size_t current_dir_len = wcslen(current_dir);
	size_t folder_name_len = wcslen(folderName);
	size_t total_len = current_dir_len + folder_name_len + 2; // +2 for '/' and '\0'

	// Allocate memory for the final path
	wchar_t* full_path = (wchar_t*)malloc(total_len * sizeof(wchar_t));
	if (full_path == nullptr) {
		alifMem_dataFree(current_dir);
		return -1;
	}

	// Construct the full path
	wcscpy(full_path, current_dir);
	wcscat(full_path, L"/");
	wcscat(full_path, folderName);



	AlifWStringList paths{};
	paths.length = 1;
	paths.items = (wchar_t**)alifMem_dataAlloc(1);
	paths.items[0] = full_path;

	_config->moduleSearchPathsSet = 1;
	_config->moduleSearchPaths = paths;

	//* alif

//	if (!_alifThread_get()) {
//		//return alifStatus_error("cannot calculate path configuration");
//	}
//
//	AlifObject* configDict = _alifConfig_asDict(_config);
//	if (!configDict) {
//		//alifErr_clear();
//		//return alifStatus_noMemory();
//		return -1; //* alif
//	}
//
//	AlifObject* dict = alifDict_new();
//	if (!dict) {
//		alifErr_clear();
//		ALIF_DECREF(configDict);
//		//return alifStatus_noMemory();
//	}
//
//	if (alifDict_setItemString(dict, "config", configDict) < 0) {
//		alifErr_clear();
//		ALIF_DECREF(configDict);
//		ALIF_DECREF(dict);
//		return alifStatus_noMemory();
//	}
//	/* reference now held by dict */
//	ALIF_DECREF(configDict);
//
//	AlifObject* co = _alifGet_getPathCodeObject();
//	if (!co or !ALIFCODE_CHECK(co)) {
//		alifErr_clear();
//		ALIF_XDECREF(co);
//		ALIF_DECREF(dict);
//		return alifStatus_error("error reading frozen getpath.alif");
//	}
//
//#ifdef _WINDOWS
//	AlifObject* winreg = alifImport_importModule("winreg");
//	if (!winreg or alifDict_setItemString(dict, "winreg", winreg) < 0) {
//		alifErr_clear();
//		ALIF_XDECREF(winreg);
//		if (alifDict_setItemString(dict, "winreg", ALIF_NONE) < 0) {
//			alifErr_clear();
//			ALIF_DECREF(co);
//			ALIF_DECREF(dict);
//			return alifStatus_error("error importing winreg module");
//		}
//	}
//	else {
//		ALIF_DECREF(winreg);
//	}
//#endif
//
//	if (
//#ifdef _WINDOWS
//		!decode_toDict(dict, "os_name", "nt") or
//#elif defined(__APPLE__)
//		!decode_toDict(dict, "os_name", "darwin") or
//#else
//		!decode_toDict(dict, "os_name", "posix") or
//#endif
//#ifdef WITH_NEXT_FRAMEWORK
//		!int_toDict(dict, "WITH_NEXT_FRAMEWORK", 1) or
//#else
//		!int_toDict(dict, "WITH_NEXT_FRAMEWORK", 0) or
//#endif
//		!decode_toDict(dict, "PREFIX", PREFIX) or
//		!decode_toDict(dict, "EXEC_PREFIX", EXEC_PREFIX) or
//		!decode_toDict(dict, "ALIFPATH", ALIFPATH) or
//		!decode_toDict(dict, "VPATH", VPATH) or
//		!decode_toDict(dict, "PLATLIBDIR", PLATLIBDIR) or
//		!decode_toDict(dict, "ALIFDEBUGEXT", ALIFDEBUGEXT) or
//		!int_toDict(dict, "VERSION_MAJOR", ALIF_MAJOR_VERSION) or
//		!int_toDict(dict, "VERSION_MINOR", ALIF_MINOR_VERSION) or
//		!decode_toDict(dict, "ALIFWINVER", ALIFWINVER) or
//		!wchar_toDict(dict, "EXE_SUFFIX", EXE_SUFFIX) or
//		!env_toDict(dict, "ENV_PATH", 0) or
//		!env_toDict(dict, "ENV_ALIFHOME", 0) or
//		!env_toDict(dict, "ENV_ALIFEXECUTABLE", 0) or
//		!env_toDict(dict, "ENV___ALIFVENV_LAUNCHER__", 1) or
//		!progname_toDict(dict, "real_executable") or
//		!library_toDict(dict, "library") or
//		!wchar_toDict(dict, "executable_dir", nullptr) or
//		!wchar_toDict(dict, "alif_setpath", _alifPathConfig_getGlobalModuleSearchPath()) or
//		!funcs_toDict(dict, _config->pathconfig_warnings) or
//
//		!decode_toDict(dict, "ABI_THREAD", "t") or
//
//#ifndef _WINDOWS
//		alifDict_setItemString(dict, "winreg", ALIF_NONE) < 0 or
//#endif
//		alifDict_setItemString(dict, "__builtins__", alifEval_getBuiltins()) < 0
//		) {
//		ALIF_DECREF(co);
//		ALIF_DECREF(dict);
//		alifErr_formatUnraisable("Exception ignored in preparing getpath");
//		return alifStatus_error("error evaluating initial values");
//	}
//
//	AlifObject* r = alifEval_evalCode(co, dict, dict);
//	ALIF_DECREF(co);
//
//	if (!r) {
//		ALIF_DECREF(dict);
//		alifErr_formatUnraisable("Exception ignored in running getpath");
//		return alifStatus_error("error evaluating path");
//	}
//	ALIF_DECREF(r);
//
//	if (_alifConfig_fromDict(_config, configDict) < 0) {
//		alifErr_formatUnraisable("Exception ignored in reading getpath results");
//		ALIF_DECREF(dict);
//		return alifStatus_error("error getting getpath results");
//	}
//
//	ALIF_DECREF(dict);

	return 1;
}
