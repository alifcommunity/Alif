#pragma once



static inline AlifIntT alifIndex_check(AlifObject* _obj) { // 13
	AlifNumberMethods* asNumber = ALIF_TYPE(_obj)->asNumber;
	return (asNumber != nullptr and asNumber->index != nullptr);
}






extern AlifIntT alifObject_hasLen(AlifObject*); // 22
