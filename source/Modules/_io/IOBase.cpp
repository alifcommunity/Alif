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






static AlifIntT ioBase_traverse(IOBase* self, VisitProc visit, void* arg) { // 344
	ALIF_VISIT(ALIF_TYPE(self));
	ALIF_VISIT(self->dict);
	return 0;
}




AlifObject* _alifIOBase_checkReadable(AlifIOState* state, AlifObject* self, AlifObject* args) { // 437
	AlifObject* res = alifObject_callMethodNoArgs(self, &ALIF_ID(readable));
	if (res == nullptr)
		return nullptr;
	if (res != ALIF_TRUE) {
		ALIF_CLEAR(res);
		//iobase_unsupported(state, "File or stream is not readable.");
		return nullptr;
	}
	if (args == ALIF_TRUE) {
		ALIF_DECREF(res);
	}
	return res;
}



static AlifGetSetDef _ioBaseGetSet_[] = { // 853
	{"__dict__", alifObject_genericGetDict, nullptr, nullptr},
	{nullptr},
};

static AlifMemberDef _ioBaseMembers_[] = { // 859
	{"__weaklistoffset__", ALIF_T_ALIFSIZET, offsetof(IOBase, weakRefList), ALIF_READONLY},
	{"__dictoffset__", ALIF_T_ALIFSIZET, offsetof(IOBase, dict), ALIF_READONLY},
	{nullptr},
};




static AlifTypeSlot _ioBaseSlots_[] = { // 866
	{ALIF_TP_TRAVERSE, ioBase_traverse},
	{ALIF_TP_MEMBERS, _ioBaseMembers_},
	{ALIF_TP_GETSET, _ioBaseGetSet_},
	{0, nullptr},
};
AlifTypeSpec _ioBaseSpec_ = { // 880
	.name = "تبادل.قاعدة",
	.basicsize = sizeof(IOBase),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _ioBaseSlots_,
};




static AlifTypeSlot _rawIOBaseSlots_[] = { // 1029
	{0, nullptr},
};


AlifTypeSpec _rawIOBaseSpec_ = { // 1036
	.name = "تبادل.خام",
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _rawIOBaseSlots_,
};
