#include "alif.h"
#include "AlifCore_Long.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_LifeCycle.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>             // getpid()
#endif
#ifdef HAVE_PROCESS_H
#  include <process.h>            // getpid()
#endif
#ifdef _WINDOWS
#  include <windows.h>            // GetCurrentProcessId()
#endif


/* Period parameters -- These are all magic.  Don't change. */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfU    /* constant vector a */
#define UPPER_MASK 0x80000000U  /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffU  /* least significant r bits */

class RandomState { // 96
public:
	AlifObject *RandomType{};
	AlifObject *Long__abs__{};
};

static inline RandomState* get_randomState(AlifObject *_module) { // 101
	void *state = _alifModule_getState(_module);
	return (RandomState*)state;
}

extern AlifModuleDef __randomModule_; // 109

#define _RANDOMSTATE_TYPE(_type) \
    (get_randomState(alifType_getModuleByDef(_type, &__randomModule_))) // 111

class RandomObject { // 114
public:
	ALIFOBJECT_HEAD{};
	AlifIntT index{};
	uint32_t state[N]{};
};

#include "clinic/_RandomModule.cpp.h"

/* generates a random number on [0,0xffffffff]-interval */
static uint32_t genrand_uint32(RandomObject *_self) { // 132
	uint32_t y{};
	static const uint32_t mag01[2] = {0x0U, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */
	uint32_t *mt{};

	mt = _self->state;
	if (_self->index >= N) { /* generate N words at one time */
		int kk;

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1U];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1U];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1U];

		_self->index = 0;
	}

	y = mt[_self->index++];
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680U;
	y ^= (y << 15) & 0xefc60000U;
	y ^= (y >> 18);
	return y;
}

static AlifObject* _random_randomRandomImpl(RandomObject *_self) { // 186
	uint32_t a=genrand_uint32(_self)>>5, b=genrand_uint32(_self)>>6;
	return alifFloat_fromDouble((a*67108864.0+b)*(1.0/9007199254740992.0));
}


/* initializes mt[N] with a seed */
static void init_genRand(RandomObject *_self, uint32_t _s) { // 194
	AlifIntT mti{};
	uint32_t *mt{};

	mt = _self->state;
	mt[0]= _s;
	for (mti=1; mti<N; mti++) {
		mt[mti] =
			(1812433253U * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
	}
	_self->index = mti;
	return;
}

/* initialize by an array with array-length */
/* _initKey is the array for initializing keys */
/* _keyLength is its length */
static void init_byArray(RandomObject *_self,
	uint32_t _initKey[], AlifUSizeT _keyLength) { // 218
	AlifUSizeT i{}, j{}, k{};
	uint32_t *mt{};

	mt = _self->state;
	init_genRand(_self, 19650218U);
	i=1; j=0;
	k = (N>_keyLength ? N : _keyLength);
	for (; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525U))
			+ _initKey[j] + (uint32_t)j; /* non linear */
		i++; j++;
		if (i>=N) { mt[0] = mt[N-1]; i=1; }
		if (j>=_keyLength) j=0;
	}
	for (k=N-1; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941U))
			- (uint32_t)i; /* non linear */
		i++;
		if (i>=N) { mt[0] = mt[N-1]; i=1; }
	}

	mt[0] = 0x80000000U; /* MSB is 1; assuring non-zero initial array */
}

static AlifIntT random_seedURandom(RandomObject *_self) { // 250
	uint32_t key[N]{};

	if (_alifOS_uRandomNonBlock(key, sizeof(key)) < 0) {
		return -1;
	}
	init_byArray(_self, key, ALIF_ARRAY_LENGTH(key));
	return 0;
}

static AlifIntT random_seedTimePID(RandomObject *_self) { // 262
	AlifTimeT now{};
	if (alifTime_time(&now) < 0) {
		return -1;
	}

	uint32_t key[5];
	key[0] = (uint32_t)(now & 0xffffffffU);
	key[1] = (uint32_t)(now >> 32);

#if defined(_WINDOWS) and !defined(MS_WINDOWS_DESKTOP) and !defined(MS_WINDOWS_SYSTEM)
	key[2] = (uint32_t)GetCurrentProcessId();
#elif defined(HAVE_GETPID)
	key[2] = (uint32_t)getpid();
#else
	key[2] = 0;
#endif

	if (alifTime_monotonic(&now) < 0) {
		return -1;
	}
	key[3] = (uint32_t)(now & 0xffffffffU);
	key[4] = (uint32_t)(now >> 32);

	init_byArray(_self, key, ALIF_ARRAY_LENGTH(key));
	return 0;
}

static AlifIntT random_seed(RandomObject *self, AlifObject *arg) { // 292
	AlifIntT result = -1;
	AlifObject *n{};
	uint32_t *key{};
	int64_t bits{};
	AlifUSizeT keyused{};
	AlifIntT res{};

	if (arg == nullptr or arg == ALIF_NONE) {
		if (random_seedURandom(self) < 0) {
			alifErr_clear();

			/* Reading system entropy failed, fall back on the worst entropy:
			use the current time and process identifier. */
			if (random_seedTimePID(self) < 0) {
				return -1;
			}
		}
		return 0;
	}

	/* This algorithm relies on the number being unsigned.
	* So: if the arg is a AlifLong, use its absolute value.
	* Otherwise use its hash value, cast to unsigned.
	*/
	if (ALIFLONG_CHECKEXACT(arg)) {
		n = alifNumber_absolute(arg);
	} else if (ALIFLONG_CHECK(arg)) {
		RandomState *state = _RANDOMSTATE_TYPE(ALIF_TYPE(self));
		n = alifObject_callOneArg(state->Long__abs__, arg);
	}
	else {
		AlifHashT hash = alifObject_hash(arg);
		if (hash == -1)
			goto Done;
		n = alifLong_fromSizeT((AlifUSizeT)hash);
	}
	if (n == nullptr)
		goto Done;

	/* Now split n into 32-bit chunks, from the right. */
	bits = _alifLong_numBits(n);

	/* Figure out how many 32-bit chunks this gives us. */
	keyused = bits == 0 ? 1 : (AlifUSizeT)((bits - 1) / 32 + 1);

	/* Convert seed to byte sequence. */
	key = (uint32_t*)alifMem_dataAlloc((AlifUSizeT)4 * keyused);
	if (key == nullptr) {
		//alifErr_noMemory();
		goto Done;
	}
	res = _alifLong_asByteArray((AlifLongObject*)n,
		(unsigned char *)key, keyused * 4,
		ALIF_LITTLE_ENDIAN,
		0, /* unsigned */
		1); /* with exceptions */
	if (res == -1) {
		goto Done;
	}

#if ALIF_BIG_ENDIAN
	{
		AlifUSizeT i{}, j{};
		/* Reverse an array. */
		for (i = 0, j = keyused - 1; i < j; i++, j--) {
			uint32_t tmp = key[i];
			key[i] = key[j];
			key[j] = tmp;
		}
	}
#endif
	init_byArray(self, key, keyused);

	result = 0;

Done:
	ALIF_XDECREF(n);
	alifMem_dataFree(key);
	return result;
}



static AlifObject* _random_randomSeedImpl(RandomObject* _self,
	AlifObject* _n) { // 394
	if (random_seed(_self, _n) < 0) {
		return nullptr;
	}
	return ALIF_NONE;
}



static AlifObject* _random_randomGetRandBitsImpl(RandomObject* _self,
	AlifIntT _k) { // 505
	AlifIntT i{}, words{};
	uint32_t r{};
	uint32_t* wordarray{};
	AlifObject* result{};

	if (_k < 0) {
		alifErr_setString(_alifExcValueError_,
			"عدد البتات يجب أن يكون اكبر من صفر");
		return nullptr;
	}

	if (_k == 0)
		return alifLong_fromLong(0);

	if (_k <= 32)  /* Fast path */
		return alifLong_fromUnsignedLong(genrand_uint32(_self) >> (32 - _k));

	words = (_k - 1) / 32 + 1;
	wordarray = (uint32_t*)alifMem_dataAlloc(words * 4);
	if (wordarray == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}

	/* Fill-out bits of long integer, by 32-bit words, from least significant
	to most significant. */
#if ALIF_LITTLE_ENDIAN
	for (i = 0; i < words; i++, _k -= 32)
	#else
	for (i = words - 1; i >= 0; i--, _k -= 32)
	#endif
	{
		r = genrand_uint32(_self);
		if (_k < 32)
			r >>= (32 - _k);  /* Drop least significant bits */
		wordarray[i] = r;
	}

	result = _alifLong_fromByteArray((unsigned char*)wordarray, words * 4,
		ALIF_LITTLE_ENDIAN, 0 /* unsigned */);
	alifMem_dataFree(wordarray);
	return result;
}

static AlifIntT random_init(RandomObject *self,
	AlifObject *args, AlifObject *kwds) { // 553
	AlifObject *arg = nullptr;
	RandomState *state = _RANDOMSTATE_TYPE(ALIF_TYPE(self));

	if ((ALIF_IS_TYPE(self, (AlifTypeObject *)state->RandomType) or
		ALIF_TYPE(self)->init == ((AlifTypeObject*)state->RandomType)->init) and
		!_ALIFARG_NOKEYWORDS("عشوائية", kwds)) {
		return -1;
	}

	if (ALIFTUPLE_GET_SIZE(args) > 1) {
		alifErr_setString(_alifExcTypeError_, "عشوائية() يتطلب 0 او 1 وسيطات");
		return -1;
	}

	if (ALIFTUPLE_GET_SIZE(args) == 1)
		arg = ALIFTUPLE_GET_ITEM(args, 0);

	return random_seed(self, arg);
}

static AlifMethodDef _randomMethods_[] = { // 577
	_RANDOM_RANDOM_RANDOM_METHODDEF,
	_RANDOM_RANDOM_SEED_METHODDEF,
	_RANDOM_RANDOM_GETRANDBITS_METHODDEF,
	{nullptr,              nullptr}           /* sentinel */
};

static AlifTypeSlot _randomTypeSlots_[] = { // 589
	//{ALIF_TP_DOC, (void *)_randomDoc_},
	{ALIF_TP_METHODS, _randomMethods_},
	{ALIF_TP_NEW, (void*)alifType_genericNew},
	{ALIF_TP_INIT, (void*)random_init},
	{ALIF_TP_FREE, (void*)alifMem_objFree},
	{0, 0},
};

static AlifTypeSpec _randomTypeSpec_ = { // 598
	.name = "عشوائي.عشوائية",
	.basicsize = sizeof(RandomObject),
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE,
	.slots = _randomTypeSlots_
};



static AlifIntT random_exec(AlifObject *_module) { // 609
	RandomState *state = get_randomState(_module);

	state->RandomType = alifType_fromModuleAndSpec(
		_module, &_randomTypeSpec_, nullptr);
	if (state->RandomType == nullptr) {
		return -1;
	}
	if (alifModule_addType(_module, (AlifTypeObject *)state->RandomType) < 0) {
		return -1;
	}

	AlifObject *longval = alifLong_fromLong(0);
	if (longval == nullptr) {
		return -1;
	}

	AlifObject *longtype = alifObject_type(longval);
	ALIF_DECREF(longval);
	if (longtype == nullptr) {
		return -1;
	}

	state->Long__abs__ = alifObject_getAttrString(longtype, "__مطلق__");
	ALIF_DECREF(longtype);
	if (state->Long__abs__ == nullptr) {
		return -1;
	}
	return 0;
}



static AlifModuleDefSlot __randomSlots_[] = { // 643
	{ALIF_MOD_EXEC, (void*)random_exec},
	{ALIF_MOD_MULTIPLE_INTERPRETERS, ALIF_MOD_PER_INTERPRETER_GIL_SUPPORTED},
	{ALIF_MOD_GIL, ALIF_MOD_GIL_NOT_USED},
	{0, nullptr}
};






AlifModuleDef __randomModule_ = { // 671
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "عشوائي",
	.size = sizeof(RandomState),
	.slots = __randomSlots_,
};

ALIFMODINIT_FUNC alifInit__random(void) { // 683
	return alifModuleDef_init(&__randomModule_);
}
