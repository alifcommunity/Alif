#pragma once





AlifSizeT alifObject_size(AlifObject*); // 328


#undef ALIFOBJECT_LENGTH
AlifSizeT alifObject_length(AlifObject*); // 342
#define ALIFOBJECT_LENGTH alifObject_size


AlifObject* alifObject_getIter(AlifObject*); // 383

AlifObject* alifIter_next(AlifObject* ); // 417


AlifIntT alifSequence_check(AlifObject*); // 664


AlifObject* alifSequence_tuple(AlifObject*); // 723



AlifIntT alifMapping_check(AlifObject*); // 806

AlifSizeT alifMapping_size(AlifObject*); // 810






/* ------------------------------------------------------------------------------------- */











AlifSizeT alifObject_lengthHint(AlifObject*, AlifSizeT); // 80
