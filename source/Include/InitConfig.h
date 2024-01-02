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
	short useEnvironment{};
	short configInOutMode{};
	short parseArgv{};

	AlifWStringList argv;

	wchar_t* programName{};

	int skipFirstLine{};
	wchar_t* runCommand{};
	wchar_t* runModule{};
	wchar_t* runFilename{};
};



void alifConfig_initAlifConfig(AlifConfig*);
