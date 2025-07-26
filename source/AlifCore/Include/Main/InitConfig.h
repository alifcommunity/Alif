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

	AlifIntT useEnvironment{};

	AlifIntT installSignalHandlers{};

	AlifIntT tracemalloc{};
	AlifIntT parseArgv{};

	AlifWStringList origArgv{};
	AlifWStringList argv{};

	AlifIntT interactive{};
	AlifIntT optimizationLevel{};
	//AlifIntT configStdio{}; // نظام ألف يستخدم ترميز utf-8 بشكل إفتراضي
	AlifIntT bufferedStdio{};
	wchar_t* stdioEncoding{};
	wchar_t* stdioErrors{};
#ifdef _WINDOWS
	AlifIntT legacyWindowsStdio{};
#endif

	AlifIntT safePath{};

	AlifIntT intMaxStrDigits{};

	/* ------- path config inputs ------- */
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
};


AlifIntT alif_setStdioLocale(const AlifConfig*); //* alif
void alifConfig_initAlifConfig(AlifConfig*); // 239

void alifConfig_clear(AlifConfig*); // 241
AlifStatus alifConfig_setString(AlifConfig*, wchar_t**, const wchar_t*);


