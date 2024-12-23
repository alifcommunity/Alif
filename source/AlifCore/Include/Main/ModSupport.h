#pragma once






AlifObject* alif_buildValue(const char*, ...); // 19











AlifIntT alifModule_addFunctions(AlifObject*, AlifMethodDef*); // 53

#define ALIF_CLEANUP_SUPPORTED 0x20000 // 57

 // 59
#define ALIF_API_VERSION 5
#define ALIF_API_STRING "5"



/* ----------------------------------------------------------------------------------------------------------- */


class AlifOnceFlag { // 7
public:
	uint8_t v{};
};

class AlifArgParser { // 11
public:
	const char* format{};
	const char* const* keywords{};
	const char* fname{};
	const char* customMsg{};
	AlifOnceFlag once{};
	AlifIntT isKwTupleOwned{};
	AlifIntT pos{};
	AlifIntT min{};
	AlifIntT max{};
	AlifObject* kwTuple{};
	AlifArgParser* next{};
};
