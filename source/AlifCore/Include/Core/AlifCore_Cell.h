#pragma once









static inline AlifObject* alifCell_swapTakeRef(AlifCellObject* _cell, AlifObject* _value) { // 16
	AlifObject* oldValue{};
	ALIF_BEGIN_CRITICAL_SECTION(_cell);
	oldValue = _cell->ref;
	_cell->ref = _value;
	ALIF_END_CRITICAL_SECTION();
	return oldValue;
}


static inline void alifCell_setTakeRef(AlifCellObject* _cell, AlifObject* _value) { // 27
	AlifObject* oldValue = alifCell_swapTakeRef(_cell, _value);
	ALIF_XDECREF(oldValue);
}


// Gets the cell contents. Returns a new reference.
static inline AlifObject* alifCell_getRef(AlifCellObject* _cell) { // 34
	AlifObject* res{};
	ALIF_BEGIN_CRITICAL_SECTION(_cell);
	res = ALIF_XNEWREF(_cell->ref);
	ALIF_END_CRITICAL_SECTION();
	return res;
}
