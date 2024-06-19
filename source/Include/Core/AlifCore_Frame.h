#pragma once

#include "AlifCore_Code.h"

class AlifFrameObject {
public:
	ALIFOBJECT_HEAD;
	AlifFrameObject* back;      /* previous frame, or nullptr */
	class AlifInterpreterFrame* frame; /* points to the frame data */
	AlifObject* trace;          /* Trace function */
	int lineNo;               /* Current line number. Only valid if non-zero */
	char traceLines;         /* Emit per-line trace events? */
	char traceOpCodes;       /* Emit per-opcode trace events? */
	AlifObject* extraLocals;   /* Dict for locals set by users using f_locals, could be NULL */
	/* The frame data, if this frame object owns the frame */
	AlifObject* frameData[1];
};





class AlifInterpreterFrame {
public:
	AlifObject* executable; /* Strong reference (code object or None) */
	AlifInterpreterFrame* previous;
	AlifObject* funcobj; /* Strong reference. Only valid if not on C stack */
	AlifObject* globals; /* Borrowed reference. Only valid if not on C stack */
	AlifObject* builtins; /* Borrowed reference. Only valid if not on C stack */
	AlifObject* locals; /* Strong reference, may be nullptr. Only valid if not on C stack */
	AlifFrameObject* frameObj; /* Strong reference, may be nullptr. Only valid if not on C stack */
	AlifCodeUnit* instrPtr; /* Instruction currently executing (or about to begin) */
	int stacktop;  /* Offset of TOS from localsplus  */
	uint16_t returnOffset;  /* Only relevant during a function call */
	char owner;
	/* Locals and stack */
	AlifObject* localsPlus[1];
};
