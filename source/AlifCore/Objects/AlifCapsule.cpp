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


static AlifIntT _is_legalCapsule(AlifObject* _op,
	const char* _invalidCapsule) { // 22

	AlifCapsule* capsule{};
	if (!_op or !ALIFCAPSULE_CHECKEXACT(_op)) {
		goto error;
	}
	capsule = (AlifCapsule*)_op;

	if (capsule->pointer == nullptr) {
		goto error;
	}
	return 1;

error:
	//alifErr_setString(_alifExcValueError_, _invalidCapsule);
	return 0;
}

 // 40
#define IS_LEGAL_CAPSULE(_capsule, _name) \
    (_is_legalCapsule(_capsule, _name " called with invalid AlifCapsule object"))


static AlifIntT name_matches(const char* _name1, const char* _name2) { // 45
	if (!_name1 or !_name2) {
		return _name1 == _name2;
	}
	return !strcmp(_name1, _name2);
}


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



void* alifCapsule_getPointer(AlifObject* _op, const char* _name) { // 96
	if (!IS_LEGAL_CAPSULE(_op, "alifCapsule_getPointer")) {
		return nullptr;
	}
	AlifCapsule* capsule = (AlifCapsule*)_op;

	if (!name_matches(_name, capsule->name)) {
		//alifErr_setString(_alifExcValueError_, "alifCapsule_getPointer called with incorrect name");
		return nullptr;
	}

	return capsule->pointer;
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
