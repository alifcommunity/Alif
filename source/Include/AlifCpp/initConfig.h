#pragma once


/* ___________ AlifStatus ___________ */

class AlifStatus
{
public:
	int type; // 0 -> Ok , 1 -> Error ,  2 -> Exit
	const char* func;
	const char* mesError;
	int exitCode;
};

/* ___________ AlifWideStringList ___________ */

class AlifWideStringList {
public:
	AlifSizeT length;
	wchar_t** items;
};

ALIFAPI_FUNC(AlifStatus) alifWideStringList_append(AlifWideStringList*, const wchar_t*);
ALIFAPI_FUNC(AlifStatus) alifWideStringList_insert(AlifWideStringList*, AlifSizeT, const wchar_t*);


/* ___________ AlifPreConfig ___________ */

class AlifPreConfig {
public:
	int configInit;
	int parseArgv;
	int isolated;
	int useEnvironment;
	int configureLocale;
	int cppLocale;

#ifdef MS_WINDOWS
	int EncodingLegacyWindowsFS;
#endif

	int utf8Mode;
	int allocator;
};

ALIFAPI_FUNC(void) alifPreConfig_initAlifConfig(AlifPreConfig*);


/* ___________ AlifConfig ___________ */

class AlifConfig {
public:
	int configInit;

	int isolated;
	int useEnvironment;
	//int install_signal_handlers;
	//int use_hash_seed;
	//unsigned long hash_seed;
	//int faultHandler;
	int traceMalloc;
	int perfProfiling;
	int importTime;
	int codeDebugRanges;
	int showRefCount;
	int dumpRefs;
	wchar_t* dumpRefsFile;
	int mallocStats;
	wchar_t* filesystemEncoding;
	wchar_t* filesystemErrors;
	wchar_t* pycachePrefix;
	int parseArgv;
	AlifWideStringList origArgv;
	AlifWideStringList argv;
	AlifWideStringList xoptions;
	AlifWideStringList warnOptions;
	int siteImport;
	int bytesWarning;
	int warnDefaultEncoding;
	int inspect;
	int interactive;
	int optimizationLevel;
	int parserDebug;
	int writeByteCode;
	int verbose;
	int quiet;
	int userSiteDirectory;
	//int configure_c_stdio;
	//int buffered_stdio;
	//wchar_t* stdio_encoding;
	//wchar_t* stdio_errors;
//#ifdef MS_WINDOWS
	//int legacy_windows_stdio;
//#endif
	wchar_t* checkHashAlfcsMode;
	int useFrozenModules;
	int safePath;
	int intMaxStrDigits;

	/* --- Path configuration inputs ------------ */
	int pathConfigWarnings;
	wchar_t* programName;
	wchar_t* alifPathEnv;
	wchar_t* home;
	wchar_t* platLibDir;

	/* --- Path configuration outputs ----------- */
	int moduleSearchPathsSet;
	AlifWideStringList moduleSearchPaths;
	wchar_t* stdLibDir;
	wchar_t* executable;
	wchar_t* baseExecutable;
	wchar_t* prefix;
	wchar_t* basePrefix;
	wchar_t* execPrefix;
	wchar_t* baseExecPrefix;

	/* --- Parameter only used by alif_main() ---------- */
	int skipSourceFirstLine;
	wchar_t* runCommand;
	wchar_t* runModule;
	wchar_t* runFilename;

	/* --- Private fields ---------------------------- */

	// Install importlib? If equals to 0, importlib is not initialized at all.
	// Needed by freeze_importlib.
	int installImportLib;

	// If equal to 0, stop Alif initialization before the "main" phase.
	int initMain;

	// If non-zero, we believe we're running from a source tree.
	int isAlifBuild;
};

ALIFAPI_FUNC(void) alifConfig_initAlifConfig(AlifConfig*);
ALIFAPI_FUNC(AlifStatus) alifConfig_setCharArgv(AlifConfig* config, AlifSizeT argc, char* const* argv);
ALIFAPI_FUNC(AlifStatus) alifConfig_setArgv(AlifConfig*, AlifSizeT, wchar_t* const*);
