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
	Alif_ssize_t length;
	wchar_t** items;
};


/* ___________ AlifConfig ___________ */

class AlifConfig {
public:
	AlifWideStringList orig_argv;
	AlifWideStringList argv;

	int skip_source_first_line;
	wchar_t* run_command;
	wchar_t* run_module;
	wchar_t* run_filename;
};


AlifStatus alifConfig_setCharArgv(AlifConfig* config, Alif_ssize_t argc, char* const* argv);
AlifStatus alifConfig_setArgv(AlifConfig* config, Alif_ssize_t argc, wchar_t* const* argv);
