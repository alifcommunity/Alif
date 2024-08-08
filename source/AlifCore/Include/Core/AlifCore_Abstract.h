#pragma once




static inline AlifIntT alifIndex_check(AlifObject* obj) {
	AlifNumberMethods* asNumber = ALIF_TYPE(obj)->asNumber;
	return (asNumber != nullptr and asNumber->index_ != nullptr);
}
