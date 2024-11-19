#include "alif.h"



#include "AlifCore_Object.h"



class AlifCapsule { // 10
public:
	ALIFOBJECT_HEAD;
	void* pointer{};
	const char* name{};
	void* context{};
	AlifCapsuleDestructor destructor{};
	TraverseProc traverseFunc{};
	Inquiry clearFunc{};
};






AlifObject* alifCapsule_new(void* pointer, const char* name,
	AlifCapsuleDestructor destructor) { // 57

	AlifCapsule* capsule{};

	if (!pointer) {
		//alifErr_setString(_alifExcValueError_, "alifCapsule_new called with null pointer");
		return nullptr;
	}

	capsule = ALIFOBJECT_GC_NEW(AlifCapsule, &_alifCapsuleType_);
	if (capsule == nullptr) {
		return nullptr;
	}

	capsule->pointer = pointer;
	capsule->name = name;
	capsule->context = nullptr;
	capsule->destructor = destructor;
	capsule->traverseFunc = nullptr;
	capsule->clearFunc = nullptr;

	return (AlifObject*)capsule;
}











AlifTypeObject _alifCapsuleType_ = { // 349
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "كبسولة",
	.basicSize = sizeof(AlifCapsule),
	//.dealloc = capsule_dealloc,
	//.repr = capsule_repr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = (TraverseProc)capsule_traverse,
	//.clear = (Inquiry)capsule_clear,
};
