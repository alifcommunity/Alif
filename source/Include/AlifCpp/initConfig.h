#pragma once






/* ----- AlifStatus ------------------------------------------- */





















/* ----- AlifWideStringList ------------------------------------------- */

class AlifWideStringList {
public:
	AlifSizeT length;
	wchar_t** items;
};


ALIFAPI_FUNC(void) alifWideStringList_append(AlifWideStringList*, const wchar_t*);

ALIFAPI_FUNC(void) alifWideStringList_insert(AlifWideStringList*,AlifSizeT, const wchar_t*);




/* ----- AlifPreConfig ------------------------------------------- */

class AlifPreConfig {
public:
	int configInit;

	int parseArgv;

	int isolated;

	int useEnvironment;

	int configureLocale;

	int coerceCLocale;

	int coerceCLocaleWarn;

#ifdef MS_WINDOWS
	int legacyWindowsFSEncoding;
#endif

	int utf8Mode;

	int devMode;

	int allocator;
};






















































ALIFAPI_FUNC(void) alifPreConfig_initAlifConfig(AlifPreConfig*);
ALIFAPI_FUNC(void) alifPreConfig_initIsolatedConfig(AlifPreConfig*);


/* ----- AlifConfig ------------------------------------------- */

class AlifConfig {
public:
	int configInit;

	int isolated;
	int useEnvironment;
	int devMode;
	int installSignalHandlers;
	int useHashSeed;
	unsigned long hashSeed;
	int faultHandler;
	int traceMalloc;
	int perfProfiling;
	int importTime;
	int codeDebugRanges;
	int showRefCount;
	int dumpRefs;
	wchar_t* dumpRefsFile;
	int mallocStats;
	wchar_t* fileSystemEncoding;
	wchar_t* fileSystemErrors;
	wchar_t* alifCachePrefix;
	int parseArgv;
	AlifWideStringList origArgv;
	AlifWideStringList argv;
	AlifWideStringList xOptions;
	AlifWideStringList warnOptions;
	int siteImport;
	int bytesWarning;
	int warnDefaultEncoding;
	int inspect;
	int interactive;
	int optimizationLevel;
	int parserDebug;
	int writeBytecode;
	int verbose;
	int quiet;
	int userSiteDirectory;
	int configureCStdio;
	int bufferedStdio;
	wchar_t* stdioEncoding;
	wchar_t* stdioErrors;
#ifdef MS_WINDOWS
	int legacyWindowsStdio;
#endif
	wchar_t* checkHashAlifCSMode;
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
	wchar_t* stdlibDir;
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

	int installImportLib;

	int initMain;

	int isAlifBuild;
};





ALIFAPI_FUNC(void) alifConfig_initAlifConfig(AlifConfig*);

ALIFAPI_FUNC(void) alifConfig_clear(AlifConfig*);
ALIFAPI_FUNC(void) alifConfig_setString(AlifConfig*,wchar_t**,const wchar_t*);








ALIFAPI_FUNC(void) alifConfig_setBytesArgv(AlifConfig*, AlifSizeT, char* const*);



ALIFAPI_FUNC(void) alifConfig_setArgv(AlifConfig*,AlifSizeT, wchar_t* const*);
