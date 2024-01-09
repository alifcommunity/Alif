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
	AlifWStringList origArgv{};
	AlifWStringList argv;

	wchar_t* programName{};

	wchar_t* runCommand{};
	wchar_t* runModule{};
	wchar_t* runFilename{};
};


void alif_localeInit();
