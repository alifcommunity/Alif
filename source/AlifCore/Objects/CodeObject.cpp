
#include "alif.h"
#include "AlifCore_SetObject.h"





AlifTypeObject _alifCodeType_ = { // 2276
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "code",
};


AlifObject* alifCode_constantKey(AlifObject* _op) { // 2331
	AlifObject* key_{};

	if (_op == ALIF_NONE or _op == ALIF_ELLIPSIS
		or ALIFLONG_CHECKEXACT(_op)
		or ALIFUSTR_CHECKEXACT(_op)
		or ALIFCODE_CHECK(_op))
	{
		key_ = ALIF_NEWREF(_op);
	}
	else if (ALIFBOOL_CHECK(_op) or ALIFBYTES_CHECKEXACT(_op)) {
		key_ = alifTuple_pack(2, ALIF_TYPE(_op), _op);
	}
	else if (ALIFFLOAT_CHECKEXACT(_op)) {
		double d_ = ALIFFLOAT_AS_DOUBLE(_op);
		if (d_ == 0.0 and copysign(1.0, d_) < 0.0)
			key_ = alifTuple_pack(3, ALIF_TYPE(_op), _op, ALIF_NONE);
		else
			key_ = alifTuple_pack(2, ALIF_TYPE(_op), _op);
	}
	else if (ALIFCOMPLEX_CHECKEXACT(_op)) {
		AlifComplex z_{};
		AlifIntT realNegZero{}, imagNegZero{};
		z_ = alifComplex_asComplex(_op);
		realNegZero = z_.real == 0.0 and copysign(1.0, z_.real) < 0.0;
		imagNegZero = z_.imag == 0.0 and copysign(1.0, z_.imag) < 0.0;
		if (realNegZero and imagNegZero) {
			key_ = alifTuple_pack(3, ALIF_TYPE(_op), _op, ALIF_TRUE);
		}
		else if (imagNegZero) {
			key_ = alifTuple_pack(3, ALIF_TYPE(_op), _op, ALIF_FALSE);
		}
		else if (realNegZero) {
			key_ = alifTuple_pack(3, ALIF_TYPE(_op), _op, ALIF_NONE);
		}
		else {
			key_ = alifTuple_pack(2, ALIF_TYPE(_op), _op);
		}
	}
	else if (ALIFTUPLE_CHECKEXACT(_op)) {
		AlifSizeT i_{}, len_{};
		AlifObject* tuple{};

		len_ = ALIFTUPLE_GET_SIZE(_op);
		tuple = alifTuple_new(len_);
		if (tuple == nullptr)
			return nullptr;

		for (i_ = 0; i_ < len_; i_++) {
			AlifObject* item{}, * itemKey{};

			item = ALIFTUPLE_GET_ITEM(_op, i_);
			itemKey = alifCode_constantKey(item);
			if (itemKey == nullptr) {
				ALIF_DECREF(tuple);
				return nullptr;
			}

			ALIFTUPLE_SET_ITEM(tuple, i_, itemKey);
		}

		key_ = alifTuple_pack(2, tuple, _op);
		ALIF_DECREF(tuple);
	}
	else if (ALIFFROZENSET_CHECKEXACT(_op)) {
		AlifSizeT pos_ = 0;
		AlifObject* item{};
		AlifHashT hash{};
		AlifSizeT i_{}, len_{};
		AlifObject* tuple{}, * set_{};

		len_ = ALIFSET_GET_SIZE(_op);
		tuple = alifTuple_new(len_);
		if (tuple == nullptr)
			return nullptr;

		i_ = 0;
		while (alifSet_nextEntry(_op, &pos_, &item, &hash)) {
			AlifObject* item_key;

			item_key = alifCode_constantKey(item);
			if (item_key == nullptr) {
				ALIF_DECREF(tuple);
				return nullptr;
			}
			ALIFTUPLE_SET_ITEM(tuple, i_, item_key);
			i_++;
		}
		set_ = alifFrozenSet_new(tuple);
		ALIF_DECREF(tuple);
		if (set_ == nullptr)
			return nullptr;

		key_ = alifTuple_pack(2, set_, _op);
		ALIF_DECREF(set_);
		return key_;
	}
	else {
		AlifObject* objID = alifLong_fromVoidPtr(_op);
		if (objID == nullptr)
			return nullptr;

		key_ = alifTuple_pack(2, objID, _op);
		ALIF_DECREF(objID);
	}
	return key_;
}
