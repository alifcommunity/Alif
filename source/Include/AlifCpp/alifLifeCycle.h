#pragma once









ALIFAPI_FUNC(void) alif_preInitialize(const AlifPreConfig*);











/* Initialization and finalization */

ALIFAPI_FUNC(void) alif_initializeFromConfig(const AlifConfig*);














#define ALIFINTERPRETERCONFIG_DEFAULT_GIL (0)
#define ALIFINTERPRETERCONFIG_SHARED_GIL (1)
#define ALIFINTERPRETERCONFIG_OWN_GIL (2)

class AlifInterpreterConfig {
public:
	int useMainObmalloc;
	int allowFork;
	int allowExec;
	int allowThreads;
	int allowDaemonThreads;
	int checkMultiInterpExtensions;
	int gil;
};












#define ALIFINTERPRETEECONFIG_LEGACY_INIT { \
        .useMainObmalloc = 1, \
        .allowFork = 1, \
        .allowExec = 1, \
        .allowThreads = 1, \
        .allowDaemonThreads = 1, \
        .checkMultiInterpExtensions = 0, \
        .gil = ALIFINTERPRETERCONFIG_SHARED_GIL, \
    }
