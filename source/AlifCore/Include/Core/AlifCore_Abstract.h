#pragma once



static inline AlifIntT alifIndex_check(AlifObject* _obj) { // 13
	AlifNumberMethods* asNumber = ALIF_TYPE(_obj)->asNumber;
	return (asNumber != nullptr and asNumber->index != nullptr);
}



AlifObject* _alifNumber_powerNoMod(AlifObject*, AlifObject*); // 19
AlifObject* _alifNumber_inPlacePowerNoMod(AlifObject*, AlifObject*); // 20


extern AlifIntT alifObject_hasLen(AlifObject*); // 22
AlifObject* _alifNumber_index(AlifObject*); // 56
