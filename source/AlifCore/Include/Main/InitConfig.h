#pragma once


/* --------------------------------- AlifWStringList --------------------------------- */
class AlifWStringList { // 31
public:
	AlifSizeT length;
	wchar_t** items;
};


/* ----------------------------------- AlifConfig ------------------------------------ */
class AlifConfig { // 134
public:
	AlifIntT configInit{};

	AlifIntT tracemalloc{};
	AlifIntT parseArgv{};

	AlifWStringList origArgv{};
	AlifWStringList argv{};

	AlifIntT interactive{};
	AlifIntT optimizationLevel{};
	//AlifIntT configStdio{}; // نظام ألف يستخدم ترميز utf-8 بشكل إفتراضي
	AlifIntT bufferedStdio{};

	AlifIntT quite{};

	AlifIntT cpuCount{};

	wchar_t* programName{};

	AlifIntT skipFirstLine{};
	wchar_t* runCommand{};
	wchar_t* runModule{};
	wchar_t* runFilename{};

	AlifIntT initMain{};
};


AlifIntT alif_setStdioLocale(const AlifConfig*); // alif
void alifConfig_initAlifConfig(AlifConfig*); // 239
