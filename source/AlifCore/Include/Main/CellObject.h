#pragma once



class AlifCellObject { // 10
public:
	ALIFOBJECT_HEAD{};
	AlifObject* ref{};
};

extern AlifTypeObject _alifCellType_; // 16

#define ALIFCELL_CHECK(_op) ALIF_IS_TYPE(_op, &_alifCellType_) // 18

AlifObject* alifCell_new(AlifObject*); // 20
//AlifObject* alifCell_get(AlifObject*);
AlifIntT alifCell_set(AlifObject*, AlifObject*); // 22

static inline AlifObject* _alifCell_get(AlifObject* _op) {
	AlifCellObject* cell{};
	cell = ALIF_CAST(AlifCellObject*, _op);
	return cell->ref;
}
#define ALIFCELL_GET(_op) _alifCell_get(ALIFOBJECT_CAST(_op))
