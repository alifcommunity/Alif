#pragma once






AlifIntT alif_initFromConfig(const AlifConfig*); // 25




const char* alif_getVersion(); // 52



typedef void (*AlifOSSigHandlerT)(int); // 59
AlifOSSigHandlerT alifOS_setSig(AlifIntT, AlifOSSigHandlerT); // 61




/* ----------------------------------------------------------------------------- */


AlifIntT alif_runMain(); // 31

// 40
#define ALIF_INTERPRETERCONFIG_DEFAULT_GIL (0)
#define ALIF_INTERPRETERCONFIG_SHARED_GIL (1)
#define ALIF_INTERPRETERCONFIG_OWN_GIL (2)

class AlifInterpreterConfig { // 44
public:
	AlifIntT useMainAlifMem{};
	AlifIntT allowFork{};
	AlifIntT allowExec{};
	AlifIntT allowThreads{};
	AlifIntT allowDaemonThreads{};
	AlifIntT checkMultiInterpExtensions{};
	AlifIntT gil{};
};

// 55
#define ALIF_INTERPRETERCONFIG_INIT \
    { \
        .useMainAlifMem = 0, \
        .allowFork = 0, \
        .allowExec = 0, \
        .allowThreads = 1, \
        .allowDaemonThreads = 0, \
        .checkMultiInterpExtensions = 1, \
        .gil = ALIF_INTERPRETERCONFIG_OWN_GIL, \
    }

// 69
#  define ALIFINTERPRETERCONFIG_LEGACY_CHECK_MULTI_INTERP_EXTENSIONS 1

// 75
#define ALIF_INTERPRETERCONFIG_LEGACY_INIT \
    { \
        .useMainAlifMem = 1, \
        .allowFork = 1, \
        .allowExec = 1, \
        .allowThreads = 1, \
        .allowDaemonThreads = 1, \
        .checkMultiInterpExtensions = ALIFINTERPRETERCONFIG_LEGACY_CHECK_MULTI_INTERP_EXTENSIONS, \
        .gil = ALIF_INTERPRETERCONFIG_SHARED_GIL, \
    }
