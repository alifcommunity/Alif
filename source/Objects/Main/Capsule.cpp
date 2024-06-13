#include "alif.h"

//#include "AlifCore_Capsule.h"
#include "AlifCore_GC.h"
#include "AlifCore_Object.h"


class AlifCapsule {
public:
	ALIFOBJECT_HEAD;
	void* pointer{};
	const wchar_t* name{};
	void* context{};
	AlifCapsuleDestructor destructor{};
	TraverseProc traverseFunc{};
	Inquiry clearFunc{};
};

static AlifIntT isLegal_capsule(AlifObject* _op, const wchar_t* _invalidCapsule) { // 22
	
	AlifCapsule* capsule{};

	if (!_op 
		//or ALIFCAPSULE_CHECKEXAXT(_op)
		)
	{
		goto error;
	}

	capsule = (AlifCapsule*)_op;

	if (capsule->pointer == nullptr) {
		goto error;
	}

	return 1;

error:
	// error
	return 0;
}

#define IS_LEGAL_CAPSULE(_cap, _name) (isLegal_capsule(_cap, _name L" called with invalid AlifCapsule object"))

static AlifIntT name_matches(const wchar_t* _name1, const wchar_t* _name2) { // 45
	if (!_name1 or !_name2) {
		return _name1 == _name2;
	}

	return !wcscmp(_name1, _name2);
}

AlifObject* alifCapsule_new(void* _ptr, const wchar_t* _name, AlifCapsuleDestructor _destructor) { // 57

	AlifCapsule* capsule{};
	if (!_ptr) {
		// error
		return nullptr;
	}

	capsule = ALIFOBJECT_GC_NEW(AlifCapsule, &_alifCapsuleType_);
	if (capsule == nullptr) return nullptr;

	capsule->pointer = _ptr;
	capsule->name = _name;
	capsule->context = nullptr;
	capsule->destructor = _destructor;
	capsule->traverseFunc = nullptr;
	capsule->clearFunc = nullptr;

	return (AlifObject*)capsule;
}



void* alifCapsule_getPointer(AlifObject* _op, const wchar_t* _name) { // 96
	if (IS_LEGAL_CAPSULE(_op, L"alifCapsule_getPointer")) {
		return nullptr;
	}
	AlifCapsule* capsule = (AlifCapsule*)_op;

	if (!name_matches(_name, capsule->name)) {
		// error
		return nullptr;
	}

	return capsule->pointer;
}





















// 349 // need fix
AlifTypeObject _alifCapsuleType_ = {
	//ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	ALIFVAROBJECT_HEAD_INIT(&_alifCapsuleType_, 0),
	L"AlifCapsule",
	16,
	0,
	(Destructor)0, /*tp_dealloc*/
	0,                  /*tp_vectorcall_offset*/
	0,                  /*tp_getattr*/
	0,                  /*tp_setattr*/
	0,                  /*tp_as_async*/
	0,                  /*tp_repr*/
	0,                  /*tp_as_number*/
	0,                  /*tp_as_sequence*/
	0,                  /*tp_as_mapping*/
	0,                  /* tp_hash */
	0,                  /* tp_call */
	0,                  /* tp_str */
	0,  /* tp_getattro */
	0,                  /* tp_setattro */
	0,                  /* tp_as_buffer */
	L".",/* tp_flags */
	0,                    /* tp_doc */
	nullptr,        /* tp_traverse */
	nullptr,                /* tp_clear */
	0,                                      /* tp_richcompare */
	0,                                      /* tp_weaklistoffset */
	0,                                      /* tp_iter */
	0,                                      /* tp_iternext */
	0,                       /* tp_methods */
	0,                    /* tp_members */
	0,                    /* tp_getset */
	0,                                      /* tp_base */
	0,                                      /* tp_dict */
	0,                                      /* tp_descr_get */
	0,                                      /* tp_descr_set */
	0,                                      /* tp_dictoffset */
	0,                                      /* tp_init */
	0,                                      /* tp_alloc */
	0,                           /* tp_new */
};