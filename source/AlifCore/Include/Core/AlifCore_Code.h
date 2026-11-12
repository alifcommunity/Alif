#pragma once

#include "AlifCore_StackRef.h"
#include "AlifCore_Lock.h"
#include "AlifCore_Backoff.h"
#include "AlifCore_ThreadState.h"




union AlifCodeUnit { // 25
	uint16_t cache{};
	class {
	public:
		uint8_t code{};
		uint8_t arg{};
	} op;
	AlifBackoffCounter counter;  // First cache entry of specializable op
};

#define ALIFCODE_CODE(_co) ALIF_RVALUE((AlifCodeUnit *)(_co)->codeAdaptive) // 34
#define ALIFCODE_NBYTES(_co) (ALIF_SIZE(_co) * (AlifSizeT)sizeof(AlifCodeUnit)) // 35



class AlifCodeState { // 69
public:
	AlifMutex mutex{};
	class AlifHashTableT* constants{};
};


extern AlifStatus alifCode_init(AlifInterpreter*); // 75

#define CODE_MAX_WATCHERS 8 // 78



#define CACHE_ENTRIES(cache) (sizeof(cache)/sizeof(AlifCodeUnit)) // 89




class AlifBinaryOpCache { // 101
public:
	AlifBackoffCounter counter{};
};
#define INLINE_CACHE_ENTRIES_BINARY_OP CACHE_ENTRIES(AlifBinaryOpCache)


class AlifSendCache { // 173
public:
	AlifBackoffCounter counter{};
};

#define INLINE_CACHE_ENTRIES_SEND CACHE_ENTRIES(AlifSendCache)


class AlifContainsOpCache { // 187
public:
	AlifBackoffCounter counter{};
};

#define INLINE_CACHE_ENTRIES_CONTAINS_OP CACHE_ENTRIES(AlifContainsOpCache)


class AlifCallCache { // 153
public:
	AlifBackoffCounter counter{};
	uint16_t funcVersion[2]{};
};

#define INLINE_CACHE_ENTRIES_CALL_KW CACHE_ENTRIES(AlifCallCache) // 159




// 219
#define CO_FAST_HIDDEN  0x10
#define CO_FAST_LOCAL   0x20
#define CO_FAST_CELL    0x40
#define CO_FAST_FREE    0x80

typedef unsigned char AlifLocalsKind; // 224

static inline AlifLocalsKind _alifLocals_getKind(AlifObject* kinds,
	AlifIntT i) { // 226
	char* ptr = ALIFBYTES_AS_STRING(kinds);
	return (AlifLocalsKind)(ptr[i]);
}



static inline void _alifLocals_setKind(AlifObject* kinds,
	AlifIntT i, AlifLocalsKind kind) { // 235
	char* ptr = ALIFBYTES_AS_STRING(kinds);
	ptr[i] = (char)kind;
}


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



extern AlifIntT _alifCode_validate(AlifCodeConstructor*); // 287
extern AlifCodeObject* alifCode_new(AlifCodeConstructor*); // 288


extern AlifObject* _alifCode_getCode(AlifCodeObject*); // 297


extern AlifIntT _alifCode_initAddressRange(AlifCodeObject*, AlifCodeAddressRange*); // 300


extern AlifIntT _alifLineTable_nextAddressRange(AlifCodeAddressRange*); // 310
extern AlifIntT _alifLineTable_previousAddressRange(AlifCodeAddressRange*); // 311


#define ENABLE_SPECIALIZATION_FT 1 // 324


extern void _alifSpecialize_binaryOp(AlifStackRef, AlifStackRef,
	AlifCodeUnit*, AlifIntT, AlifStackRef*); // 348

extern void _alifSpecialize_containsOp(AlifStackRef, AlifCodeUnit*); // 357



static inline uint16_t read_u16(uint16_t* _p) { // 439
	return *_p;
}


static inline unsigned char* parse_varint(unsigned char* _p, AlifIntT* _result) { // 471
	AlifIntT val = _p[0] & 63;
	while (_p[0] & 64) {
		_p++;
		val = (val << 6) | (_p[0] & 63);
	}
	*_result = val;
	return _p + 1;
}


static inline AlifIntT write_varint(uint8_t* _ptr, AlifUIntT _val) { // 482
	AlifIntT written = 1;
	while (_val >= 64) {
		*_ptr++ = 64 | (_val & 63);
		_val >>= 6;
		written++;
	}
	*_ptr = (uint8_t)_val;
	return written;
}

static inline AlifIntT write_signedVarint(uint8_t* _ptr, AlifIntT _val) { // 496
	AlifUIntT uVal{};
	if (_val < 0) {
		uVal = ((0 - (AlifUIntT)_val) << 1) | 1;
	}
	else {
		uVal = (AlifUIntT)_val << 1;
	}
	return write_varint(_ptr, uVal);
}


static inline AlifIntT write_locationEntryStart(uint8_t* _ptr,
	AlifIntT _code, AlifIntT _length) { // 510
	*_ptr = 128 | (uint8_t)(_code << 3) | (uint8_t)(_length - 1);
	return 1;
}




// 540
#define ADAPTIVE_WARMUP_VALUE 1
#define ADAPTIVE_WARMUP_BACKOFF 1


// 549
#define ADAPTIVE_COOLDOWN_VALUE 52
#define ADAPTIVE_COOLDOWN_BACKOFF 0


static inline AlifBackoffCounter adaptive_counterBits(uint16_t value,
	uint16_t backoff) { // 557
	return make_backoffCounter(value, backoff);
}

static inline AlifBackoffCounter adaptive_counterWarmup(void) {
	return adaptive_counterBits(ADAPTIVE_WARMUP_VALUE,
		ADAPTIVE_WARMUP_BACKOFF);
}

static inline AlifBackoffCounter adaptive_counterCooldown(void) { // 568
	return adaptive_counterBits(ADAPTIVE_COOLDOWN_VALUE,
		ADAPTIVE_COOLDOWN_BACKOFF);
}

static inline AlifBackoffCounter adaptive_counterBackoff(AlifBackoffCounter _counter) { // 574
	return restart_backoffCounter(_counter);
}


// 584
#define COMPARISON_UNORDERED 1

#define COMPARISON_LESS_THAN 2
#define COMPARISON_GREATER_THAN 4
#define COMPARISON_EQUALS 8

#define COMPARISON_NOT_EQUALS (COMPARISON_UNORDERED | COMPARISON_LESS_THAN | COMPARISON_GREATER_THAN)

extern AlifIntT _alif_instrument(AlifCodeObject*, AlifInterpreter*);

extern AlifCodeUnit _alif_getBaseCodeUnit(AlifCodeObject*, AlifIntT);


extern AlifIntT _alifInstruction_getLength(AlifCodeObject*, AlifIntT); // 604


// Return a pointer to the thread-local bytecode for the current thread, if it
// exists.
static inline AlifCodeUnit* _alifCode_getTLBCFast(AlifThread* _thread,
	AlifCodeObject* _co) { // 614
	AlifCodeArray* code = (AlifCodeArray*)alifAtomic_loadPtrAcquire(&_co->coTlbc);
	int32_t idx = ((AlifThreadImpl*)_thread)->tlbcIndex;
	if (idx < code->size and code->entries[idx] != nullptr) {
		return (AlifCodeUnit*)code->entries[idx];
	}
	return nullptr;
}

// Return a pointer to the thread-local bytecode for the current thread,
// creating it if necessary.
extern AlifCodeUnit* _alifCode_getTLBC(AlifCodeObject*);

// Reserve an index for the current thread into thread-local bytecode
// arrays
//
// Returns the reserved index or -1 on error.
extern int32_t _alif_reserveTLBCIndex(AlifInterpreter*);

// Release the current thread's index into thread-local bytecode arrays
extern void _alif_clearTLBCIndex(AlifThreadImpl*);

// Free all TLBC copies not associated with live threads.
//
// Returns 0 on success or -1 on error.
extern AlifIntT _alif_clearUnusedTLBC(AlifInterpreter*);
