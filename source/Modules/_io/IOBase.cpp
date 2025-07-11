#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"     

#include <stddef.h>               // offsetof()
#include "_IOModule.h"



class IOBase { // 31
public:
	ALIFOBJECT_HEAD;
	AlifObject* dict{};
	AlifObject* weakRefList{};
};










static AlifTypeSlot _ioBaseSlots_[] = { // 866

	{0, nullptr},
};
AlifTypeSpec _ioBaseSpec_ = { // 880
	.name = "تبادل.قاعدة",
	.basicsize = sizeof(IOBase),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _ioBaseSlots_,
};







AlifTypeSpec _rawIOBaseSpec_ = { // 1036
	.name = "تبادل.خام",
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	//.slots = _rawIOBaseSlots_,
};
