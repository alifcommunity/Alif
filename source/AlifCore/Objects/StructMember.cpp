#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_CriticalSection.h"












static inline AlifObject* member_getObject(const char* addr, const char* obj_addr, AlifMemberDef* l) { // 11
	AlifObject* v = (AlifObject*)alifAtomic_loadPtr(&(*(AlifObject**)addr)); //* alif
	if (v == nullptr) {
		//alifErr_format(_alifExcAttributeError_,
		//	"'%T' object has no attribute '%s'",
		//	(AlifObject*)obj_addr, l->name);
	}
	return v;
}



AlifObject* alifMember_getOne(const char* obj_addr, AlifMemberDef* l) { // 23
	AlifObject* v{};
	if (l->flags & ALIF_RELATIVE_OFFSET) {
		alifErr_setString(
			_alifExcSystemError_,
			"alifMember_getOne used with ALIF_RELATIVE_OFFSET");
		return nullptr;
	}

	const char* addr = obj_addr + l->offset;
	switch (l->type) {
	case ALIF_T_BOOL:
		v = alifBool_fromLong(*(char*)addr);
		break;
	case ALIF_T_BYTE:
		v = alifLong_fromLong(*(char*)addr);
		break;
	case ALIF_T_UBYTE:
		v = alifLong_fromUnsignedLong(*(unsigned char*)addr);
		break;
	case ALIF_T_SHORT:
		v = alifLong_fromLong(*(short*)addr);
		break;
	case ALIF_T_USHORT:
		v = alifLong_fromUnsignedLong(*(unsigned short*)addr);
		break;
	case ALIF_T_INT:
		v = alifLong_fromLong(*(int*)addr);
		break;
	case ALIF_T_UINT:
		v = alifLong_fromUnsignedLong(*(unsigned int*)addr);
		break;
	case ALIF_T_LONG:
		v = alifLong_fromLong(*(long*)addr);
		break;
	case ALIF_T_ULONG:
		v = alifLong_fromUnsignedLong(*(unsigned long*)addr);
		break;
	case ALIF_T_ALIFSIZET:
		v = alifLong_fromSizeT(*(AlifSizeT*)addr);
		break;
	case ALIF_T_FLOAT:
		v = alifFloat_fromDouble((double)*(float*)addr);
		break;
	case ALIF_T_DOUBLE:
		v = alifFloat_fromDouble(*(double*)addr);
		break;
	case ALIF_T_STRING:
		if (*(char**)addr == nullptr) {
			v = ALIF_NEWREF(ALIF_NONE);
		}
		else
			v = alifUStr_fromString(*(char**)addr);
		break;
	case ALIF_T_STRING_INPLACE:
		v = alifUStr_fromString((char*)addr);
		break;
	case ALIF_T_CHAR:
		v = alifUStr_fromStringAndSize((char*)addr, 1);
		break;
	case ALIF_T_OBJECT:
		v = *(AlifObject**)addr;
		if (v == nullptr)
			v = ALIF_NONE;
		ALIF_INCREF(v);
		break;
	case ALIF_T_OBJECT_EX:
		v = member_getObject(addr, obj_addr, l);
		ALIF_XINCREF(v);
		break;
	case ALIF_T_LONGLONG:
		v = alifLong_fromLongLong(*(long long*)addr);
		break;
	case ALIF_T_ULONGLONG:
		v = alifLong_fromUnsignedLongLong(*(unsigned long long*)addr);
		break;
	case _ALIF_T_NONE:
		// doesn't require free-threading code path
		v = ALIF_NEWREF(ALIF_NONE);
		break;
	default:
		alifErr_setString(_alifExcSystemError_, "bad memberdescr type");
		v = nullptr;
	}
	return v;
}



AlifIntT alifMember_setOne(char* addr, AlifMemberDef* l, AlifObject* v) { // 129
	AlifObject* oldv{};
	if (l->flags & ALIF_RELATIVE_OFFSET) {
		alifErr_setString(
			_alifExcSystemError_,
			"alifMember_setOne used with ALIF_RELATIVE_OFFSET");
		return -1;
	}

	AlifObject* obj = (AlifObject*)addr;
	addr += l->offset;

	if ((l->flags & ALIF_READONLY))
	{
		//alifErr_setString(_alifExcAttributeError_, "readonly attribute");
		return -1;
	}
	if (v == nullptr) {
		if (l->type == ALIF_T_OBJECT_EX) {
			/* Check if the attribute is set. */
			if (*(AlifObject**)addr == nullptr) {
				//alifErr_setString(_alifExcAttributeError_, l->name);
				return -1;
			}
		}
		else if (l->type != ALIF_T_OBJECT) {
			alifErr_setString(_alifExcTypeError_,
				"can't delete numeric/char attribute");
			return -1;
		}
	}
	switch (l->type) {
	case ALIF_T_BOOL: {
		if (!ALIFBOOL_CHECK(v)) {
			alifErr_setString(_alifExcTypeError_,
				"attribute value type must be bool");
			return -1;
		}
		if (v == ALIF_TRUE)
			*(char*)addr = (char)1;
		else
			*(char*)addr = (char)0;
		break;
	}
	case ALIF_T_BYTE: {
		long long_val = alifLong_asLong(v);
		if ((long_val == -1) and alifErr_occurred())
			return -1;
		*(char*)addr = (char)long_val;
		//if ((long_val > CHAR_MAX) || (long_val < CHAR_MIN))
		//	WARN("Truncation of value to char");
		break;
	}
	case ALIF_T_UBYTE: {
		long long_val = alifLong_asLong(v);
		if ((long_val == -1) and alifErr_occurred())
			return -1;
		*(unsigned char*)addr = (unsigned char)long_val;
		//if ((long_val > UCHAR_MAX) or (long_val < 0))
		//	WARN("Truncation of value to unsigned char");
		break;
	}
	case ALIF_T_SHORT: {
		long long_val = alifLong_asLong(v);
		if ((long_val == -1) and alifErr_occurred())
			return -1;
		*(short*)addr = (short)long_val;
		//if ((long_val > SHRT_MAX) or (long_val < SHRT_MIN))
		//	WARN("Truncation of value to short");
		break;
	}
	case ALIF_T_USHORT: {
		long long_val = alifLong_asLong(v);
		if ((long_val == -1) and alifErr_occurred())
			return -1;
		*(unsigned short*)addr = (unsigned short)long_val;
		//if ((long_val > USHRT_MAX) or (long_val < 0))
		//	WARN("Truncation of value to unsigned short");
		break;
	}
	case ALIF_T_INT: {
		long long_val = alifLong_asLong(v);
		if ((long_val == -1) and alifErr_occurred())
			return -1;
		*(int*)addr = (int)long_val;
		//if ((long_val > INT_MAX) or (long_val < INT_MIN))
		//	WARN("Truncation of value to int");
		break;
	}
	case ALIF_T_UINT: {
		v = _alifNumber_index(v);
		if (v == NULL) {
			return -1;
		}
		if (_alifLong_isNegative((AlifLongObject*)v)) {
			long long_val = alifLong_asLong(v);
			ALIF_DECREF(v);
			if (long_val == -1 and alifErr_occurred()) {
				return -1;
			}
			*(unsigned int*)addr = (unsigned int)(unsigned long)long_val;
			//WARN("Writing negative value into unsigned field");
		}
		else {
			unsigned long ulong_val = alifLong_asUnsignedLong(v);
			ALIF_DECREF(v);
			if (ulong_val == (unsigned long)-1 and alifErr_occurred()) {
				return -1;
			}
			*(unsigned int*)addr = (unsigned int)ulong_val;
			if (ulong_val > UINT_MAX) {
				//WARN("Truncation of value to unsigned int");
			}
		}
		break;
	}
	case ALIF_T_LONG: {
		*(long*)addr = alifLong_asLong(v);
		if ((*(long*)addr == -1) and alifErr_occurred())
			return -1;
		break;
	}
	case ALIF_T_ULONG: {
		v = _alifNumber_index(v);
		if (v == nullptr) {
			return -1;
		}
		if (_alifLong_isNegative((AlifLongObject*)v)) {
			long long_val = alifLong_asLong(v);
			ALIF_DECREF(v);
			if (long_val == -1 and alifErr_occurred()) {
				return -1;
			}
			*(unsigned long*)addr = (unsigned long)long_val;
			//WARN("Writing negative value into unsigned field");
		}
		else {
			unsigned long ulong_val = alifLong_asUnsignedLong(v);
			ALIF_DECREF(v);
			if (ulong_val == (unsigned long)-1 and alifErr_occurred()) {
				return -1;
			}
			*(unsigned long*)addr = ulong_val;
		}
		break;
	}
	case ALIF_T_ALIFSIZET: {
		*(AlifSizeT*)addr = alifLong_asSizeT(v);
		if ((*(AlifSizeT*)addr == (AlifSizeT)-1)
			and alifErr_occurred())
			return -1;
		break;
	}
	case ALIF_T_FLOAT: {
		double double_val = alifFloat_asDouble(v);
		if ((double_val == -1) and alifErr_occurred())
			return -1;
		*(float*)addr = (float)double_val;
		break;
	}
	case ALIF_T_DOUBLE:
		*(double*)addr = alifFloat_asDouble(v);
		if ((*(double*)addr == -1) and alifErr_occurred())
			return -1;
		break;
	case ALIF_T_OBJECT:
	case ALIF_T_OBJECT_EX:
		ALIF_BEGIN_CRITICAL_SECTION(obj);
		oldv = *(AlifObject**)addr;
		alifAtomic_storePtrRelease(&(*(AlifObject**)addr), ALIF_XNEWREF(v));
		ALIF_END_CRITICAL_SECTION();
		ALIF_XDECREF(oldv);
		break;
	case ALIF_T_CHAR: {
		const char* string{};
		AlifSizeT len{};

		string = alifUStr_asUTF8AndSize(v, &len);
		if (string == nullptr or len != 1) {
			//ALIFERR_BADARGUMENT();
			return -1;
		}
		*(char*)addr = string[0];
		break;
	}
	case ALIF_T_STRING:
	case ALIF_T_STRING_INPLACE:
		alifErr_setString(_alifExcTypeError_, "readonly attribute");
		return -1;
	case ALIF_T_LONGLONG: {
		long long value;
		*(long long*)addr = value = alifLong_asLongLong(v);
		if ((value == -1) and alifErr_occurred())
			return -1;
		break;
	}
	case ALIF_T_ULONGLONG: {
		v = _alifNumber_index(v);
		if (v == nullptr) {
			return -1;
		}
		if (_alifLong_isNegative((AlifLongObject*)v)) {
			long long_val = alifLong_asLong(v);
			ALIF_DECREF(v);
			if (long_val == -1 and alifErr_occurred()) {
				return -1;
			}
			*(unsigned long long*)addr = (unsigned long long)(long long)long_val;
			//WARN("Writing negative value into unsigned field");
		}
		else {
			unsigned long long ulonglong_val = alifLong_asUnsignedLongLong(v);
			ALIF_DECREF(v);
			if (ulonglong_val == (unsigned long long) - 1 and alifErr_occurred()) {
				return -1;
			}
			*(unsigned long long*)addr = ulonglong_val;
		}
		break;
	}
	default:
		alifErr_format(_alifExcSystemError_,
			"bad memberdescr type for %s", l->name);
		return -1;
	}
	return 0;
}
