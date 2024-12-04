#pragma once


#include "AlifCore_Lock.h"
#include "AlifCore_Backoff.h"




union AlifCodeUnit { // 25
	uint16_t cache{};
	class {
	public:
		uint8_t code{};
		uint8_t arg{};
	} op;
	AlifBackoffCounter counter;  // First cache entry of specializable op
};









class AlifCodeConstructor { // 245
public:
	/* metadata */
	AlifObject* filename{};
	AlifObject* name{};
	AlifObject* qualname{};
	AlifIntT flags{};

	/* the code */
	AlifObject* code{};
	AlifIntT firstLineno{};
	AlifObject* lineTable{};

	/* used by the code */
	AlifObject* consts{};
	AlifObject* names{};

	/* mapping frame offsets to information */
	AlifObject* localsPlusNames{};  // Tuple of strings
	AlifObject* localsPlusKinds{};  // Bytes object, one byte per variable

	/* args (within varnames) */
	AlifIntT argCount{};
	AlifIntT posOnlyArgCount{};
	AlifIntT kwOnlyArgCount{};

	/* needed to create the frame */
	AlifIntT stackSize{};

	/* used by the eval loop */
	AlifObject* exceptionTable{};
};










 // 584
#define COMPARISON_UNORDERED 1

#define COMPARISON_LESS_THAN 2
#define COMPARISON_GREATER_THAN 4
#define COMPARISON_EQUALS 8

#define COMPARISON_NOT_EQUALS (COMPARISON_UNORDERED | COMPARISON_LESS_THAN | COMPARISON_GREATER_THAN)

