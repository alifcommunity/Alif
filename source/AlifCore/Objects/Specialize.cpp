#include "alif.h"

#include "Opcode.h"

#include "AlifCore_Code.h"
#include "AlifCore_Dict.h" 
#include "AlifCore_Function.h" 
#include "AlifCore_Long.h" 
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpcodeMetaData.h"
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Runtime.h"

#include <stdlib.h> // rand()







// 435
#ifndef SPECIALIZATION_FAIL
#  define SPECIALIZATION_FAIL(opcode, kind) ((void)0)
#endif


// Initialize warmup counters and optimize instructions. This cannot fail.
void _alifCode_quicken(AlifCodeUnit* _instructions, AlifSizeT _size,
	AlifObject* _consts, AlifIntT _enableCounters) { // 459
#if ENABLE_SPECIALIZATION_FT
	AlifBackoffCounter jumpCounter{}, adaptiveCounter{};
	if (_enableCounters) {
		jumpCounter = initial_jumpBackoffCounter();
		adaptiveCounter = adaptive_counterWarmup();
	}
	else {
		jumpCounter = initial_unreachableBackoffCounter();
		adaptiveCounter = initial_unreachableBackoffCounter();
	}
	AlifIntT opcode = 0;
	AlifIntT oparg = 0;
	/* The last code unit cannot have a cache, so we don't need to check it */
	for (AlifSizeT i = 0; i < _size-1; i++) {
		opcode = _instructions[i].op.code;
		AlifIntT caches = _alifOpcodeCaches_[opcode];
		oparg = (oparg << 8) | _instructions[i].op.arg;
		if (caches) {
			// The initial value depends on the opcode
			switch (opcode) {
			case JUMP_BACKWARD:
				_instructions[i + 1].counter = jumpCounter;
				break;
			case POP_JUMP_IF_FALSE:
			case POP_JUMP_IF_TRUE:
			case POP_JUMP_IF_NONE:
			case POP_JUMP_IF_NOT_NONE:
				_instructions[i + 1].cache = 0x5555;  // Alternating 0, 1 bits
				break;
			default:
				_instructions[i + 1].counter = adaptiveCounter;
				break;
			}
			i += caches;
		}
		else if (opcode == LOAD_CONST) {
			/* We can't do this in the bytecode compiler as
			* marshalling can intern strings and make them immortal. */

			AlifObject *obj = ALIFTUPLE_GET_ITEM(_consts, oparg);
			if (ALIF_ISIMMORTAL(obj)) {
				_instructions[i].op.code = LOAD_CONST_IMMORTAL;
			}
		}
		if (opcode != EXTENDED_ARG) {
			oparg = 0;
		}
	}
#endif /* ENABLE_SPECIALIZATION_FT */
}





// Common // 497
#define SPEC_FAIL_OTHER 0
#define SPEC_FAIL_NO_DICT 1
#define SPEC_FAIL_OVERRIDDEN 2
#define SPEC_FAIL_OUT_OF_VERSIONS 3
#define SPEC_FAIL_OUT_OF_RANGE 4
#define SPEC_FAIL_EXPECTED_ERROR 5
#define SPEC_FAIL_WRONG_NUMBER_ARGUMENTS 6
#define SPEC_FAIL_CODE_COMPLEX_PARAMETERS 7
#define SPEC_FAIL_CODE_NOT_OPTIMIZED 8


// Binary op // 569
#define SPEC_FAIL_BINARY_OP_ADD_DIFFERENT_TYPES          9
#define SPEC_FAIL_BINARY_OP_ADD_OTHER                   10
#define SPEC_FAIL_BINARY_OP_AND_DIFFERENT_TYPES         11
#define SPEC_FAIL_BINARY_OP_AND_INT                     12
#define SPEC_FAIL_BINARY_OP_AND_OTHER                   13
#define SPEC_FAIL_BINARY_OP_FLOOR_DIVIDE                14
#define SPEC_FAIL_BINARY_OP_LSHIFT                      15
#define SPEC_FAIL_BINARY_OP_MATRIX_MULTIPLY             16
#define SPEC_FAIL_BINARY_OP_MULTIPLY_DIFFERENT_TYPES    17
#define SPEC_FAIL_BINARY_OP_MULTIPLY_OTHER              18
#define SPEC_FAIL_BINARY_OP_OR                          19
#define SPEC_FAIL_BINARY_OP_POWER                       20
#define SPEC_FAIL_BINARY_OP_REMAINDER                   21
#define SPEC_FAIL_BINARY_OP_RSHIFT                      22
#define SPEC_FAIL_BINARY_OP_SUBTRACT_DIFFERENT_TYPES    23
#define SPEC_FAIL_BINARY_OP_SUBTRACT_OTHER              24
#define SPEC_FAIL_BINARY_OP_TRUE_DIVIDE_DIFFERENT_TYPES 25
#define SPEC_FAIL_BINARY_OP_TRUE_DIVIDE_FLOAT           26
#define SPEC_FAIL_BINARY_OP_TRUE_DIVIDE_OTHER           27
#define SPEC_FAIL_BINARY_OP_XOR                         28





// CONTAINS_OP // 665
#define SPEC_FAIL_CONTAINS_OP_STR        9
#define SPEC_FAIL_CONTAINS_OP_TUPLE      10
#define SPEC_FAIL_CONTAINS_OP_LIST       11
#define SPEC_FAIL_CONTAINS_OP_USER_CLASS 12





static inline AlifIntT set_opcode(AlifCodeUnit* instr, uint8_t opcode) { // 671
	uint8_t oldOp = alifAtomic_loadUint8Relaxed(&instr->op.code);
	if (oldOp >= MIN_INSTRUMENTED_OPCODE) {
		/* Lost race with instrumentation */
		return 0;
	}
	if (!alifAtomic_compareExchangeUint8(&instr->op.code, &oldOp, opcode)) {
		/* Lost race with instrumentation */
		return 0;
	}
	return 1;
}


static inline void set_counter(AlifBackoffCounter* _counter, AlifBackoffCounter _value) { // 692
	alifAtomic_storeUint16Relaxed(&_counter->valueAndBackoff,
		_value.valueAndBackoff);
}

static inline AlifBackoffCounter load_counter(AlifBackoffCounter* _counter) { // 692
	AlifBackoffCounter result = {
		.valueAndBackoff = alifAtomic_loadUint16Relaxed(&_counter->valueAndBackoff)
	};
	return result;
}

static inline void specialize(AlifCodeUnit* _instr, uint8_t _specializedOpcode) {
	if (!set_opcode(_instr, _specializedOpcode)) {
		//STAT_INC(_alifOpcodeDeopt_[_specializedOpcode], failure);
		SPECIALIZATION_FAIL(_alifOpcodeDeopt_[_specializedOpcode], SPEC_FAIL_OTHER);
		return;
	}
	set_counter((AlifBackoffCounter*)_instr + 1, adaptive_counterCooldown());
}

static inline void unspecialize(AlifCodeUnit* _instr) { // 721
	uint8_t opcode = alifAtomic_loadUint8Relaxed(&_instr->op.code);
	uint8_t genericOpcode = _alifOpcodeDeopt_[opcode];
	//STAT_INC(generic_opcode, failure);
	if (!set_opcode(_instr, genericOpcode)) {
		SPECIALIZATION_FAIL(genericOpcode, SPEC_FAIL_OTHER);
		return;
	}
	AlifBackoffCounter *counter = (AlifBackoffCounter*)_instr + 1;
	AlifBackoffCounter cur = load_counter(counter);
	set_counter(counter, adaptive_counterBackoff(cur));
}



#ifdef ALIF_STATS
static AlifIntT binaryOp_failKind(AlifIntT oparg,
	AlifObject *lhs, AlifObject *rhs) { // 2246
	switch (oparg) {
	case NB_ADD:
	case NB_INPLACE_ADD:
		if (!ALIF_IS_TYPE(lhs, ALIF_TYPE(rhs))) {
			return SPEC_FAIL_BINARY_OP_ADD_DIFFERENT_TYPES;
		}
		return SPEC_FAIL_BINARY_OP_ADD_OTHER;
	case NB_AND:
	case NB_INPLACE_AND:
		if (!ALIF_IS_TYPE(lhs, ALIF_TYPE(rhs))) {
			return SPEC_FAIL_BINARY_OP_AND_DIFFERENT_TYPES;
		}
		if (ALIFLONG_CHECKEXACT(lhs)) {
			return SPEC_FAIL_BINARY_OP_AND_INT;
		}
		return SPEC_FAIL_BINARY_OP_AND_OTHER;
	case NB_FLOOR_DIVIDE:
	case NB_INPLACE_FLOOR_DIVIDE:
		return SPEC_FAIL_BINARY_OP_FLOOR_DIVIDE;
	case NB_LSHIFT:
	case NB_INPLACE_LSHIFT:
		return SPEC_FAIL_BINARY_OP_LSHIFT;
	case NB_MATRIX_MULTIPLY:
	case NB_INPLACE_MATRIX_MULTIPLY:
		return SPEC_FAIL_BINARY_OP_MATRIX_MULTIPLY;
	case NB_MULTIPLY:
	case NB_INPLACE_MULTIPLY:
		if (!ALIF_IS_TYPE(lhs, ALIF_TYPE(rhs))) {
			return SPEC_FAIL_BINARY_OP_MULTIPLY_DIFFERENT_TYPES;
		}
		return SPEC_FAIL_BINARY_OP_MULTIPLY_OTHER;
	case NB_OR:
	case NB_INPLACE_OR:
		return SPEC_FAIL_BINARY_OP_OR;
	case NB_POWER:
	case NB_INPLACE_POWER:
		return SPEC_FAIL_BINARY_OP_POWER;
	case NB_REMAINDER:
	case NB_INPLACE_REMAINDER:
		return SPEC_FAIL_BINARY_OP_REMAINDER;
	case NB_RSHIFT:
	case NB_INPLACE_RSHIFT:
		return SPEC_FAIL_BINARY_OP_RSHIFT;
	case NB_SUBTRACT:
	case NB_INPLACE_SUBTRACT:
		if (!ALIF_IS_TYPE(lhs, ALIF_TYPE(rhs))) {
			return SPEC_FAIL_BINARY_OP_SUBTRACT_DIFFERENT_TYPES;
		}
		return SPEC_FAIL_BINARY_OP_SUBTRACT_OTHER;
	case NB_TRUE_DIVIDE:
	case NB_INPLACE_TRUE_DIVIDE:
		if (!ALIF_IS_TYPE(lhs, ALIF_TYPE(rhs))) {
			return SPEC_FAIL_BINARY_OP_TRUE_DIVIDE_DIFFERENT_TYPES;
		}
		if (ALIFFLOAT_CHECKEXACT(lhs)) {
			return SPEC_FAIL_BINARY_OP_TRUE_DIVIDE_FLOAT;
		}
		return SPEC_FAIL_BINARY_OP_TRUE_DIVIDE_OTHER;
	case NB_XOR:
	case NB_INPLACE_XOR:
		return SPEC_FAIL_BINARY_OP_XOR;
	}
	ALIF_UNREACHABLE();
}
#endif

void _alifSpecialize_binaryOp(AlifStackRef _lhsSt, AlifStackRef _rhsSt,
	AlifCodeUnit* _instr, AlifIntT _oparg, AlifStackRef* _locals) { // 2268
	AlifObject *lhs = alifStackRef_asAlifObjectBorrow(_lhsSt);
	AlifObject *rhs = alifStackRef_asAlifObjectBorrow(_rhsSt);
	switch (_oparg) {
	case NB_ADD:
	case NB_INPLACE_ADD:
		if (!ALIF_IS_TYPE(lhs, ALIF_TYPE(rhs))) {
			break;
		}
		if (ALIFUSTR_CHECKEXACT(lhs)) {
			AlifCodeUnit next = _instr[INLINE_CACHE_ENTRIES_BINARY_OP + 1];
			bool toStore = (next.op.code == STORE_FAST);
			if (toStore and alifStackRef_asAlifObjectBorrow(_locals[next.op.arg]) == lhs) {
				specialize(_instr, BINARY_OP_INPLACE_ADD_UNICODE);
				return;
			}
			specialize(_instr, BINARY_OP_ADD_UNICODE);
			return;
		}
		if (ALIFLONG_CHECKEXACT(lhs)) {
			specialize(_instr, BINARY_OP_ADD_INT);
			return;
		}
		if (ALIFFLOAT_CHECKEXACT(lhs)) {
			specialize(_instr, BINARY_OP_ADD_FLOAT);
			return;
		}
		break;
	case NB_MULTIPLY:
	case NB_INPLACE_MULTIPLY:
		if (!ALIF_IS_TYPE(lhs, ALIF_TYPE(rhs))) {
			break;
		}
		if (ALIFLONG_CHECKEXACT(lhs)) {
			specialize(_instr, BINARY_OP_MULTIPLY_INT);
			return;
		}
		if (ALIFFLOAT_CHECKEXACT(lhs)) {
			specialize(_instr, BINARY_OP_MULTIPLY_FLOAT);
			return;
		}
		break;
	case NB_SUBTRACT:
	case NB_INPLACE_SUBTRACT:
		if (!ALIF_IS_TYPE(lhs, ALIF_TYPE(rhs))) {
			break;
		}
		if (ALIFLONG_CHECKEXACT(lhs)) {
			specialize(_instr, BINARY_OP_SUBTRACT_INT);
			return;
		}
		if (ALIFFLOAT_CHECKEXACT(lhs)) {
			specialize(_instr, BINARY_OP_SUBTRACT_FLOAT);
			return;
		}
		break;
	}
	SPECIALIZATION_FAIL(BINARY_OP, binaryOp_failKind(_oparg, lhs, rhs));
	unspecialize(_instr);
}


#ifdef ALIF_STATS
static AlifIntT containsOp_failKind(AlifObject* value) { // 2763
	if (ALIFUSTR_CHECKEXACT(value)) {
		return SPEC_FAIL_CONTAINS_OP_STR;
	}
	if (ALIFLIST_CHECKEXACT(value)) {
		return SPEC_FAIL_CONTAINS_OP_LIST;
	}
	if (ALIFTUPLE_CHECKEXACT(value)) {
		return SPEC_FAIL_CONTAINS_OP_TUPLE;
	}
	if (ALIFTYPE_CHECK(value)) {
		return SPEC_FAIL_CONTAINS_OP_USER_CLASS;
	}
	return SPEC_FAIL_OTHER;
}
#endif

void _alifSpecialize_containsOp(AlifStackRef _valueSt, AlifCodeUnit* _instr) { // 2780
	AlifObject* value = alifStackRef_asAlifObjectBorrow(_valueSt);

	if (ALIFDICT_CHECKEXACT(value)) {
		specialize(_instr, CONTAINS_OP_DICT);
		return;
	}
	if (ALIFSET_CHECKEXACT(value) or ALIFFROZENSET_CHECKEXACT(value)) {
		specialize(_instr, CONTAINS_OP_SET);
		return;
	}

	SPECIALIZATION_FAIL(CONTAINS_OP, containsOp_failKind(value));
	unspecialize(_instr);
	return;
}
