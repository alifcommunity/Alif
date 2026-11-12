#pragma once








 // 73
#if USE_COMPUTED_GOTOS
#  define TARGET(op) TARGET_##op:
#  define DISPATCH_GOTO() goto *opcode_targets[opcode]
#else
#  define TARGET(_op) case _op: TARGET_##_op:
#  define DISPATCH_GOTO() goto dispatch_opcode
#endif


#ifdef LLTRACE
#define PRE_DISPATCH_GOTO() if (lltrace >= 5) { \
    lltrace_instruction(frame, stackPointer, nextInstr, opcode, oparg); }
#else
#define PRE_DISPATCH_GOTO() ((void)0)
#endif


 // 101
#define QSBR_QUIESCENT_STATE(_thread) _alifQSBR_quiescentState(((AlifThreadImpl*)_thread)->qsbr)

 // 109
#define DISPATCH() \
    { \
        NEXTOPARG(); \
        PRE_DISPATCH_GOTO(); \
        DISPATCH_GOTO(); \
    }
// 117
#define DISPATCH_SAME_OPARG() \
    { \
        opcode = nextInstr->op.code; \
        PRE_DISPATCH_GOTO(); \
        DISPATCH_GOTO(); \
    }


 // 123
#define DISPATCH_INLINED(_newFrame)                     \
    do {                                                \
        _alifFrame_setStackPointer(_frame, stackPointer); \
        _frame = _thread->currentFrame = (_newFrame);     \
        goto start_frame;                               \
    } while (0)



#define GETITEM(_v, _i) ALIFTUPLE_GET_ITEM(_v, _i) // 139





#define INSTR_OFFSET() ((AlifIntT)(nextInstr - _alifFrame_getBytecode(_frame))) // 154
 // 155
#define NEXTOPARG()  do { \
        AlifCodeUnit word  = {.cache = alifAtomic_loadUint16Relaxed(&*(uint16_t*)nextInstr)}; \
        opcode = word.op.code; \
        oparg = word.op.arg; \
    } while (0)



#define JUMPBY(_x)       (nextInstr += (_x)) // 164
#define SKIP_OVER(_x)    (nextInstr += (_x))


 // 193
#define PREDICT_ID(_op)          PRED_##_op
#define PREDICTED(_op)           PREDICT_ID(_op):




#define BASIC_STACKADJ(_n) (stackPointer += _n) // 212
// 214
#define BASIC_PUSH(v)     (*stackPointer++ = (v))
#define BASIC_POP()       (*--stackPointer)


// 234
#define PUSH(v)                BASIC_PUSH(v)
#define POP()                  BASIC_POP()
#define STACK_GROW(n)          BASIC_STACKADJ(n)
#define STACK_SHRINK(n)        BASIC_STACKADJ(-(n))


/* Data access macros */
#define FRAME_CO_CONSTS (_alifFrame_getCode(_frame)->consts) // 243
#define FRAME_CO_NAMES  (_alifFrame_getCode(_frame)->names)


#define LOCALS_ARRAY    (_frame->localsPlus) // 248
#define GETLOCAL(_i)     (_frame->localsPlus[_i])


 // 257
#define SETLOCAL(_i, _value)	do { AlifStackRef tmp = GETLOCAL(_i); \
                                     GETLOCAL(_i) = _value; \
                                     ALIFSTACKREF_XCLOSE(tmp); } while (0)

#define GO_TO_INSTRUCTION(_op) goto PREDICT_ID(_op) // 262

// 278
#define DEOPT_IF(_cond, _instname)                            \
    if (_cond) {                                           \
        /* This is only a single jump on release builds! */ \
       /* UPDATE_MISS_STATS((_instname));*/                      \
        GO_TO_INSTRUCTION(_instname);                        \
    }


#define GLOBALS() _frame->globals // 286
#define BUILTINS() _frame->builtins
#define LOCALS() _frame->locals // 288
#define CONSTS() _alifFrame_getCode(_frame)->consts
#define NAMES() _alifFrame_getCode(_frame)->names


#define ADAPTIVE_COUNTER_TRIGGERS(_counter) \
    backoff_counterTriggers(forge_backoffCounter((_counter))) // 301

// 304
#define ADVANCE_ADAPTIVE_COUNTER(_counter) \
    do { \
        (_counter) = advance_backoffCounter((_counter)); \
    } while (0);


#ifdef ENABLE_SPECIALIZATION_FT
 /* Multiple threads may execute these concurrently if thread-local bytecode is
 * disabled and they all execute the main copy of the bytecode. Specialization
 * is disabled in that case so the value is unused, but the RMW cycle should be
 * free of data races.
 */
#define RECORD_BRANCH_TAKEN(bitset, flag) \
    alifAtomic_storeUint16Relaxed(       \
        &bitset, (alifAtomic_loadUint16Relaxed(&bitset) << 1) | (flag))
#else
#define RECORD_BRANCH_TAKEN(bitset, flag)
#endif

#define NAME_ERROR_MSG "الاسم '%.200s' غير معرف"

static inline AlifIntT _alif_enterRecursiveAlif(AlifThread* _thread) { // 368
	return (_thread->alifRecursionRemaining-- <= 0) and
		_alif_checkRecursiveCallAlif(_thread);
}


static inline void _alif_leaveRecursiveCallAlif(AlifThread* _thread) { // 373
	_thread->alifRecursionRemaining++;
}


 // 382
#define LOAD_IP(_offset) do { \
        nextInstr = _frame->instrPtr + (_offset); \
    } while (0)

 // 388
#define LOAD_SP() \
stackPointer = _alifFrame_getStackPointer(_frame)
 // 391
#define SAVE_SP() \
_alifFrame_setStackPointer(_frame, stackPointer)



#define MAX_STACKREF_SCRATCH 10 // 442

 // 445
#define STACKREFS_TO_ALIFOBJECTS(_args, _argCount, _name) \
    /* +1 because vectorcall might use -1 to write self */ \
    AlifObject *_name##_temp[MAX_STACKREF_SCRATCH+1]{}; \
    AlifObject **_name = _alifObjectArray_fromStackRefArray(_args, _argCount, _name##_temp + 1);

 // 456
#define STACKREFS_TO_ALIFOBJECTS_CLEANUP(_name) \
    /* +1 because we +1 previously */ \
    _alifObjectArray_free(_name - 1, _name##_temp);

#define CONVERSION_FAILED(_name) (_name == nullptr) // 465
