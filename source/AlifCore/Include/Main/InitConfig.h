#pragma once



/* ----------------------------------- AlifStatus ------------------------------------ */

// هذا الصنف يستخدم في مرحلة تهيئة اللغة
// حيث يحدد حالة المرحلة
class AlifStatus {
public:
	enum {
		AlifStatus_Type_OK = 0,
		AlifStatus_Type_ERROR = 1,
		AlifStatus_Type_EXIT = 2
	} type{};
	const char* func{};
	const char* errMsg{};
	AlifIntT exitcode{};
};

/* --------------------------------- AlifWStringList --------------------------------- */
class AlifWStringList { // 31
public:
	AlifSizeT length{};
	wchar_t** items{};
};

// لماذا يوجد append و insert وتقومان بنفس الوظيفة ??
AlifStatus alifWStringList_append(AlifWStringList*, const wchar_t*); // 38
AlifStatus alifWStringList_insert(AlifWStringList*, AlifSizeT, const wchar_t*);


/* ---------------------------------- AlifPreConfig ---------------------------------- */

class AlifPreConfig {
public:
	AlifIntT configInit{};
	AlifIntT parseArgv{};
	AlifIntT isolated{};
	AlifIntT useEnvironment{};
	AlifIntT configureLocale{};
	AlifIntT coerceCLocale{};
	AlifIntT coerceCLocaleWarn{};

#ifdef _WINDOWS
	AlifIntT legacyWindowsFSEncoding{};
#endif

	AlifIntT utf8Mode{};
	AlifIntT devMode{};
};

void alifPreConfig_initAlifConfig(AlifPreConfig*);



/* ----------------------------------- AlifConfig ------------------------------------ */
class AlifConfig { // 134
public:
	AlifIntT configInit{};

	AlifIntT isolated{};
	AlifIntT useEnvironment{};
	AlifIntT devMode{};
	AlifIntT installSignalHandlers{};
	AlifIntT useHashSeed{};
	unsigned long hashSeed{};
	AlifIntT faultHandler{};
	AlifIntT tracemalloc{};
	AlifIntT perfProfiling{};
	AlifIntT importTime{};
	AlifIntT codeDebugRanges{};
	AlifIntT showRefCount{};
	AlifIntT dumpRefs{};
	wchar_t* dumpRefsFile{};
	AlifIntT mallocStats{};
	wchar_t* fileSystemEncoding{};
	wchar_t* fileSystemErrors{};
	wchar_t* alifCachePrefix{};
	AlifIntT parseArgv{};
	AlifWStringList origArgv{};
	AlifWStringList argv{};
	AlifWStringList xoptions{};
	AlifWStringList warnoptions{};
	AlifIntT siteImport{};
	AlifIntT bytesWarning{};
	AlifIntT warnDefaultEncoding{};
	AlifIntT inspect{};
	AlifIntT interactive{};
	AlifIntT optimizationLevel{};
	AlifIntT parserDebug{};
	AlifIntT writeBytecode{};
	AlifIntT verbose{};
	AlifIntT quiet{};
	AlifIntT userSiteDirectory{};
	AlifIntT configureCStdio{};
	AlifIntT bufferedStdio{};
	wchar_t* stdioEncoding{};
	wchar_t* stdioErrors{};
#ifdef _WINDOWS
	AlifIntT legacyWindowsStdio{};
#endif
	wchar_t* checkHashAlifCSMode{};
	AlifIntT useFrozenModules{};
	AlifIntT safePath{};
	AlifIntT intMaxStrDigits{};

	AlifIntT cpuCount{};


	/* ------- path config inputs ------- */
	AlifIntT pathConfigWarnings{};
	wchar_t* programName{};
	wchar_t* alifPathEnv{};
	wchar_t* home{};
	wchar_t* platLibDir{};

	/* ------- path config outputs ------- */
	AlifIntT moduleSearchPathsSet{};
	AlifWStringList moduleSearchPaths{};
	wchar_t* stdLibDir{};
	wchar_t* executable{};
	wchar_t* baseExecutable{};
	wchar_t* prefix{};
	wchar_t* basePrefix{};
	wchar_t* execPrefix{};
	wchar_t* baseExecPrefix{};



	AlifIntT skipFirstLine{};
	wchar_t* runCommand{};
	wchar_t* runModule{};
	wchar_t* runFilename{};

	wchar_t* sysPath0{};

	AlifIntT installImportLib{};

	AlifIntT initMain{};

	AlifIntT isAlifBuild{};

#ifdef ALIF_STATS
	AlifIntT alifStats{};
#endif
};


void alifConfig_initAlifConfig(AlifConfig*); // 239

void alifConfig_clear(AlifConfig*); // 241
AlifStatus alifConfig_setString(AlifConfig*, wchar_t**, const wchar_t*);


AlifStatus alifConfig_setBytesArgv(AlifConfig*, AlifSizeT, char* const*); // 251
AlifStatus alifConfig_setArgv(AlifConfig*, AlifSizeT, wchar_t* const*); // 255
