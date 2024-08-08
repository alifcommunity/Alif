#pragma once


/* --------------------------------- AlifWStringList --------------------------------- */
class AlifWStringList {
public:
	AlifSizeT length;
	wchar_t** items;
};


/* ----------------------------------- AlifConfig ------------------------------------ */
class AlifConfig {
public:
	AlifIntT configInit{};

	AlifIntT tracemalloc{};
	AlifIntT parseArgv{};

	AlifWStringList origArgv{};
	AlifWStringList argv{};

	AlifIntT interactive{};
	AlifIntT optimizationLevel{};
	AlifIntT configStdio{};
	AlifIntT bufferedStdio{};

	AlifIntT quite{};

	wchar_t* programName{};

	AlifIntT skipFirstLine{};
	wchar_t* runCommand{};
	wchar_t* runModule{};
	wchar_t* runFilename{};

	AlifIntT initMain{};
};


AlifIntT alif_setStdioLocale(const AlifConfig*);
void alifConfig_initAlifConfig(AlifConfig*);
