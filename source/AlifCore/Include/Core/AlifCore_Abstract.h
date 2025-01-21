#pragma once



static inline AlifIntT alifIndex_check(AlifObject* _obj) { // 13
	AlifNumberMethods* asNumber = ALIF_TYPE(_obj)->asNumber;
	return (asNumber != nullptr and asNumber->index != nullptr);
}



AlifObject* _alifNumber_powerNoMod(AlifObject*, AlifObject*); // 19
AlifObject* _alifNumber_inPlacePowerNoMod(AlifObject*, AlifObject*); // 20


extern AlifIntT alifObject_hasLen(AlifObject*); // 22






#define ALIF_ITERSEARCH_COUNT    1
#define ALIF_ITERSEARCH_INDEX    2
#define ALIF_ITERSEARCH_CONTAINS 3




AlifObject* _alifNumber_index(AlifObject*); // 56
