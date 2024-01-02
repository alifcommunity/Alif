#include "alif.h"
#include "AlifCore_GetConsoleLine.h"
#include "AlifCore_Memory.h"
#include "AlifCore_InitConfig.h"

#ifdef _WINDOWS
#include <windows.h>
#endif // _WINDOWS


#define ALIF_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))





void alifConfig_initAlifConfig(AlifConfig* _config) {
	_config->configInOutMode = 1;
	_config->useEnvironment = 1;
	_config->parseArgv = 1;

}


void alifArgv_asWStrList(AlifConfig* _config, AlifArgv* _args) {
	if (_args->useBytesArgv)
	{
	}
	else {
		_config->argv.length = _args->argc;
		_config->argv.items = (wchar_t**)_args->wcharArgv;
	}
}

static void parse_consoleLine(AlifConfig* _config, AlifSizeT* _index) {
	int printVer{};
	const AlifWStringList* argv = &_config->argv;
	const wchar_t* program = _config->programName;
	if (!program and argv->length >= 1) {
		program = argv->items[0];
	}

	alif_resetConsoleLine();
	do {
		int index = -1;
		int c = alif_getConsoleLine(argv->length, argv->items, &index);

		if (c == EOF or c == 'm') {
			break;
		}

		if (c == 'E') {
			_config->useEnvironment = 0;
		}
		else if (c == 'V') {
			printVer++;
		}

	} while (true);

	if (printVer) {
		wprintf(L"alif %ls\n", alif_getVersion());
	}

	if (_config->runModule == nullptr and _config->runCommand == nullptr
		and optInd < argv->length and wcscmp(argv->items[optInd], L"-") != 0
		and _config->runFilename == nullptr) {
		_config->runFilename = (wchar_t*)alifMem_dataAlloc(wcslen(argv->items[optInd]) * sizeof(wchar_t));
		memcpy(_config->runFilename, argv->items[optInd], wcslen(argv->items[optInd]) * sizeof(wchar_t));
	}

	*_index = optInd;
}

static void config_read(AlifConfig* _config) {

	if (_config->parseArgv == 1) {
		_config->parseArgv = 2;
	}
}

static void update_argv(AlifConfig* _config,AlifSizeT _index) {
	const AlifWStringList* cmdlineArgv = &_config->argv;
	AlifWStringList configArgv = { .length = 0, .items = nullptr };
	if (cmdlineArgv->length <= _index) {

	}
	else {
		AlifWStringList slice{};
		slice.length = cmdlineArgv->length - _index;
		slice.items = &cmdlineArgv->items[_index];
		configArgv = slice;
	}
	_config->argv = configArgv;
}

static void run_absPathFilename(AlifConfig* _config) {
	wchar_t* absFilename{};
	wchar_t wOutBuf[MAX_PATH], *wOutBufP = wOutBuf;
	DWORD result{};
#ifdef _WINDOWS
	result = GetFullPathNameW(_config->runFilename, ALIF_ARRAY_LENGTH(wOutBuf), wOutBuf, nullptr);
#else
#endif // _WINDOWS

}

static void config_readConsole(AlifConfig* _config) {
	if (_config->parseArgv == 1) {
		AlifSizeT index{};
		parse_consoleLine(_config, &index);

		run_absPathFilename(_config);

		update_argv(_config, index);
	}

}


void alifConfig_read(AlifConfig* _config) {
	config_readConsole(_config);

	config_read(_config);
}
