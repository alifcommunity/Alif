#pragma once




#include "AlifCore_Code.h"
#include "AlifCore_StackRef.h"






class Frame { // 21
public:
	ALIFOBJECT_HEAD;
	AlifFrameObject* back{};
	AlifInterpreterFrame* frame{};
	AlifObject* trace{};
	AlifIntT lineno{};
	char traceLines{};
	char traceOpcodes{};
	AlifObject* extraLocals{};
	AlifObject* localsCache{};
	AlifObject* frameData[1]{};
};












class AlifInterpreterFrame { // 62
public:
	AlifObject* executable{};
	AlifInterpreterFrame* previous{};
	AlifObject* funcObj{};
	AlifObject* globals{};
	AlifObject* builtins{};
	AlifObject* locals{};
	AlifFrameObject* frameObj{};
	AlifCodeUnit* instrPtr{};
	AlifStackRef* stackPointer{};
	uint16_t returnOffset{};
	char owner{};
	/* Locals and stack */
	AlifStackRef localsPlus[1]{};
} _PyInterpreterFrame;






#define FRAME_SPECIALS_SIZE ((AlifIntT)((sizeof(AlifInterpreterFrame)-1)/sizeof(AlifObject*))) // 107
