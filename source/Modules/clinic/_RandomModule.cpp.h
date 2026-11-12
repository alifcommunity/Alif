#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"



#define _RANDOM_RANDOM_RANDOM_METHODDEF    \
    {"عشوائي", (AlifCPPFunction)_random_randomRandom, METHOD_NOARGS}

static AlifObject* _random_randomRandomImpl(RandomObject*);

static AlifObject * _random_randomRandom(RandomObject *_self,
	AlifObject *ALIF_UNUSED(ignored)) { // 20
	AlifObject *returnValue{};

	ALIF_BEGIN_CRITICAL_SECTION(_self);
	returnValue = _random_randomRandomImpl(_self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}


// البذرة seed
// كتعريف: البذرة التي سيتفرع منها عدة فروع بعد نموها
// أي نقطة البداية لما بعدها من خيارات
// 41
#define _RANDOM_RANDOM_SEED_METHODDEF    \
    {"بذرة", ALIF_CPPFUNCTION_CAST(_random_randomSeed), METHOD_FASTCALL}

static AlifObject* _random_randomSeedImpl(RandomObject*, AlifObject*);

static AlifObject* _random_randomSeed(RandomObject *_self,
	AlifObject *const *_args, AlifSizeT _nargs) { // 47
	AlifObject *returnValue = nullptr;
	AlifObject *n = ALIF_NONE;

	if (!_ALIFARG_CHECKPOSITIONAL("بذرة", _nargs, 0, 1)) {
		goto exit;
	}
	if (_nargs < 1) {
		goto skip_optional;
	}
	n = _args[0];
skip_optional:
	ALIF_BEGIN_CRITICAL_SECTION(_self);
	returnValue = _random_randomSeedImpl(_self, n);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}










// 123
#define _RANDOM_RANDOM_GETRANDBITS_METHODDEF    \
    {"اجلب_بتات_عشوائي", (AlifCPPFunction)_random_randomGetRandBits, METHOD_O}

static AlifObject*
_random_randomGetRandBitsImpl(RandomObject*, AlifIntT);

static AlifObject* _random_randomGetRandBits(RandomObject* self,
	AlifObject* arg) { // 129
	AlifObject* returnValue{};
	AlifIntT k{};

	k = alifLong_asInt(arg);
	if (k == -1 and alifErr_occurred()) {
		goto exit;
	}
	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _random_randomGetRandBitsImpl(self, k);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}
