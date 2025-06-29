#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Long.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Range.h"
#include "AlifCore_Tuple.h"





class RangeObject { // 19
public:
	ALIFOBJECT_HEAD{};
	AlifObject* start{};
	AlifObject* stop{};
	AlifObject* step{};
	AlifObject* length{};
};


static AlifObject* validate_step(AlifObject* _step) { // 30
	/* No step specified, use a step of 1. */
	if (!_step)
		return alifLong_fromLong(1);

	_step = alifNumber_index(_step);
	if (_step and _alifLong_isZero((AlifLongObject*)_step)) {
		//alifErr_setString(_alifExcValueError_,
		//	"range() arg 3 must not be zero");
		ALIF_CLEAR(_step);
	}

	return _step;
}



static AlifObject* compute_rangeLength(AlifObject*, AlifObject*, AlifObject*); // 47

static RangeObject* make_rangeObject(AlifTypeObject* type, AlifObject* start,
	AlifObject* stop, AlifObject* step) { // 50
	RangeObject* obj = nullptr;
	AlifObject* length;
	length = compute_rangeLength(start, stop, step);
	if (length == nullptr) {
		return nullptr;
	}
	obj = (RangeObject*)alifObject_new(type); //* alif
	if (obj == nullptr) {
		ALIF_DECREF(length);
		return nullptr;
	}
	obj->start = start;
	obj->stop = stop;
	obj->step = step;
	obj->length = length;
	return obj;
}


static AlifObject* range_fromArray(AlifTypeObject* type,
	AlifObject* const* args, AlifSizeT num_args) { // 77
	RangeObject* obj{};
	AlifObject* start = nullptr, * stop = nullptr, * step = nullptr;

	switch (num_args) {
	case 3:
		step = args[2];
		ALIF_FALLTHROUGH;
	case 2:
		/* Convert borrowed refs to owned refs */
		start = alifNumber_index(args[0]);
		if (!start) {
			return nullptr;
		}
		stop = alifNumber_index(args[1]);
		if (!stop) {
			ALIF_DECREF(start);
			return nullptr;
		}
		step = validate_step(step);  /* Caution, this can clear exceptions */
		if (!step) {
			ALIF_DECREF(start);
			ALIF_DECREF(stop);
			return nullptr;
		}
		break;
	case 1:
		stop = alifNumber_index(args[0]);
		if (!stop) {
			return nullptr;
		}
		start = _alifLong_getZero();
		step = _alifLong_getOne();
		break;
	case 0:
		//alifErr_setString(_alifExcTypeError_,
		//	"range expected at least 1 argument, got 0");
		return nullptr;
	default:
		//alifErr_format(_alifExcTypeError_,
		//	"range expected at most 3 arguments, got %zd",
		//	num_args);
		return nullptr;
	}
	obj = make_rangeObject(type, start, stop, step);
	if (obj != nullptr) {
		return (AlifObject*)obj;
	}

	/* Failed to create object, release attributes */
	ALIF_DECREF(start);
	ALIF_DECREF(stop);
	ALIF_DECREF(step);
	return nullptr;
}



static AlifObject* range_vectorCall(AlifObject* rangeType, AlifObject* const* args,
	AlifUSizeT nargsf, AlifObject* kwnames) { // 145
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);
	if (!_ALIFARG_NOKWNAMES("مدى", kwnames)) {
		return nullptr;
	}
	return range_fromArray((AlifTypeObject*)rangeType, args, nargs);
}





static unsigned long get_lenOfRange(long, long, long); // 176

static long compute_rangeLengthLong(AlifObject* start,
	AlifObject* stop, AlifObject* step) { // 183
	int overflow = 0;

	long long_start = alifLong_asLongAndOverflow(start, &overflow);
	if (overflow) {
		return -2;
	}
	if (long_start == -1 /*and alifErr_occurred()*/) {
		return -1;
	}
	long long_stop = alifLong_asLongAndOverflow(stop, &overflow);
	if (overflow) {
		return -2;
	}
	if (long_stop == -1 /*and alifErr_occurred()*/) {
		return -1;
	}
	long long_step = alifLong_asLongAndOverflow(step, &overflow);
	if (overflow) {
		return -2;
	}
	if (long_step == -1 /*and alifErr_occurred()*/) {
		return -1;
	}

	unsigned long ulen = get_lenOfRange(long_start, long_stop, long_step);
	if (ulen > (unsigned long)LONG_MAX) {
		/* length too large for a long */
		return -2;
	}
	else {
		return (long)ulen;
	}
}




static AlifObject* compute_rangeLength(AlifObject* _start,
	AlifObject* _stop, AlifObject* _step) { // 223

	AlifIntT cmpResult{};
	AlifObject* lo{}, * hi{};
	AlifObject* diff = nullptr;
	AlifObject* tmp1 = nullptr, * tmp2 = nullptr, * result;

	AlifObject* zero = _alifLong_getZero();  // borrowed reference
	AlifObject* one = _alifLong_getOne();  // borrowed reference


	long len = compute_rangeLengthLong(_start, _stop, _step);
	if (len >= 0) {
		return alifLong_fromLong(len);
	}
	else if (len == -1) {
		return nullptr;
	}

	cmpResult = alifObject_richCompareBool(_step, zero, ALIF_GT);
	if (cmpResult == -1)
		return nullptr;

	if (cmpResult == 1) {
		lo = _start;
		hi = _stop;
		ALIF_INCREF(_step);
	}
	else {
		lo = _stop;
		hi = _start;
		_step = alifNumber_negative(_step);
		if (!_step)
			return nullptr;
	}

	/* if (lo >= hi), return length of 0. */
	cmpResult = alifObject_richCompareBool(lo, hi, ALIF_GE);
	if (cmpResult != 0) {
		ALIF_DECREF(_step);
		if (cmpResult < 0)
			return nullptr;
		result = zero;
		return ALIF_NEWREF(result);
	}

	if ((tmp1 = alifNumber_subtract(hi, lo)) == nullptr)
		goto Fail;

	if ((diff = alifNumber_subtract(tmp1, one)) == nullptr)
		goto Fail;

	if ((tmp2 = alifNumber_floorDivide(diff, _step)) == nullptr)
		goto Fail;

	if ((result = alifNumber_add(tmp2, one)) == nullptr)
		goto Fail;

	ALIF_DECREF(tmp2);
	ALIF_DECREF(diff);
	ALIF_DECREF(_step);
	ALIF_DECREF(tmp1);
	return result;

Fail:
	ALIF_DECREF(_step);
	ALIF_XDECREF(tmp2);
	ALIF_XDECREF(diff);
	ALIF_XDECREF(tmp1);
	return nullptr;
}









static AlifObject* range_iter(AlifObject*); // 739






AlifTypeObject _alifRangeType_ = { // 767
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
		.name = "مدى",
		.basicSize = sizeof(RangeObject),
		//.dealloc = (Destructor)range_dealloc,
		//.repr = (ReprFunc)range_repr,
		//.asNumber = &_rangeAsNumber,
		//.asSequence = &_rangeAsSequence_,
		//.asMapping = &_rangeAsMapping,
		//.hash = (HashFunc)range_hash,
		.getAttro = alifObject_genericGetAttr,
		.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_SEQUENCE,
		//.richCompare = range_richCompare,
		.iter = range_iter,
		//.methods = range_methods,
		//.members = range_members,
		//.new_ = range_new,
		.vectorCall = range_vectorCall
};



static AlifObject* rangeIter_next(AlifRangeIterObject* _r) { // 816
	if (_r->len > 0) {
		long result = _r->start;
		_r->start = result + _r->step;
		_r->len--;
		return alifLong_fromLong(result);
	}
	return nullptr;
}


AlifTypeObject _alifRangeIterType_ = { // 896
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مكرر_مدى",                       /* tp_name */
	.basicSize = sizeof(AlifRangeIterObject),             /* tp_basicsize */
	/* methods */
	.dealloc = (Destructor)alifMem_objFree,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT,
	.iter = alifObject_selfIter,
	.iterNext = (IterNextFunc)rangeIter_next,
	//.methods = rangeIter_methods,
};






static unsigned long get_lenOfRange(long _lo, long _hi, long _step) { // 932
	if (_step > 0 and _lo < _hi)
		return 1UL + (_hi - 1UL - _lo) / _step;
	else if (_step < 0 and _lo > _hi)
		return 1UL + (_lo - 1UL - _hi) / (0UL - _step);
	else
		return 0UL;
}


static AlifObject* fast_rangeIter(long start, long stop, long step, long len) { // 960
	AlifRangeIterObject* it = (AlifRangeIterObject*)alifObject_new(&_alifRangeIterType_);
	if (it == nullptr)
		return nullptr;
	it->start = start;
	it->step = step;
	it->len = len;
	return (AlifObject*)it;
}

class LongRangeIterObject { // 972
public:
	ALIFOBJECT_HEAD{};
	AlifObject* start{};
	AlifObject* step{};
	AlifObject* len{};
};


static AlifObject* longRangeIter_next(LongRangeIterObject* r) { // 1072
	if (alifObject_richCompareBool(r->len, _alifLong_getZero(), ALIF_GT) != 1)
		return nullptr;

	AlifObject* newStart = alifNumber_add(r->start, r->step);
	if (newStart == nullptr) {
		return nullptr;
	}
	AlifObject* newLen = alifNumber_subtract(r->len, _alifLong_getOne());
	if (newLen == nullptr) {
		ALIF_DECREF(newStart);
		return nullptr;
	}
	AlifObject* result = r->start;
	r->start = newStart;
	ALIF_SETREF(r->len, newLen);
	return result;
}

AlifTypeObject _alifLongRangeIterType_ = { // 1093
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "تكرار_مدى_عدد",
	.basicSize = sizeof(LongRangeIterObject), 
	/* methods */
	//.dealloc = (Destructor)longRangeIter_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT,
	.iter = alifObject_selfIter,
	.iterNext = (IterNextFunc)longRangeIter_next,
	//.methods = longRangeIter_methods,
};


static AlifObject* range_iter(AlifObject* _seq) { // 1126
	RangeObject* r = (RangeObject*)_seq;
	LongRangeIterObject* it{};
	long lstart{}, lstop{}, lstep{};
	unsigned long ulen{};

	lstart = alifLong_asLong(r->start);
	if (lstart == -1 /*and alifErr_occurred()*/) {
		//alifErr_clear();
		goto long_range;
	}
	lstop = alifLong_asLong(r->stop);
	if (lstop == -1 /*and alifErr_occurred()*/) {
		//alifErr_clear();
		goto long_range;
	}
	lstep = alifLong_asLong(r->step);
	if (lstep == -1 /*and alifErr_occurred()*/) {
		//alifErr_clear();
		goto long_range;
	}
	ulen = get_lenOfRange(lstart, lstop, lstep);
	if (ulen > (unsigned long)LONG_MAX) {
		goto long_range;
	}
	/* check for potential overflow of lstart + ulen * lstep */
	if (ulen) {
		if (lstep > 0) {
			if (lstop > LONG_MAX - (lstep - 1))
				goto long_range;
		}
		else {
			if (lstop < LONG_MIN + (-1 - lstep))
				goto long_range;
		}
	}
	return fast_rangeIter(lstart, lstop, lstep, (long)ulen);

long_range:
	it = (LongRangeIterObject*)alifObject_new(&_alifLongRangeIterType_);
	if (it == nullptr)
		return nullptr;

	it->start = ALIF_NEWREF(r->start);
	it->step = ALIF_NEWREF(r->step);
	it->len = ALIF_NEWREF(r->length);
	return (AlifObject*)it;
}
