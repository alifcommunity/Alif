#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_CriticalSection.h"
















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
	case _ALIF_T_OBJECT:
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
