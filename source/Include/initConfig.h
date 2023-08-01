#pragma once


/* ___________ AlifStatus ___________ */

class AlifStatus
{
public:
	int type; // 0 -> Ok , 1 -> Error ,  2 -> Exit
	const wchar_t* func;
	const wchar_t* mesError;
	int exitCode;
};

/* ___________ AlifWideStringList ___________ */

class AlifWideStringList {
public:
	alif_size_t length;
	wchar_t** items;
};


/* ___________ AlifConfig ___________ */

class AlifConfig {
public:
	AlifWideStringList origArgv;
	AlifWideStringList argv;

	int skipSourceFirstLine;
	wchar_t* runCommand;
	wchar_t* runModule;
	wchar_t* runFilename;
};


AlifStatus alifConfig_setCharArgv(AlifConfig* config, alif_size_t argc, char* const* argv);
AlifStatus alifConfig_setArgv(AlifConfig*, alif_size_t, wchar_t* const*);
