#include "alif.h"

#include "AlifCore_Memory.h"



void alif_incRef(AlifObj* object) {

	object->ref++;
}

void alif_decRef(AlifObj* object) {

	if (object->ref < 0) {
		return;
	}
	if (--object->ref == 0) {
		alif_dealloc(object);
	}

}

void alif_setImmortal(AlifObj* object)
{
	if (object) {
		object->ref = 0xffff;
	}
}

AlifObj* alifObject_init(AlifObj* object, AlifInitObject* type) {

	if (object == nullptr) {
		// error
	}

	object->type = type;

	new_reference(object);

	return object;
}

AlifObjVar* alifObject_varInit(AlifObjVar* object, AlifInitObject* type, size_t size) {

	if (object == nullptr) {
		// error
	}

	object->object.type = type;
	object->size = size;

	new_reference((AlifObj*)object);

	return object;
}

AlifObj* alifNew_object(AlifInitObject* type) {

	AlifObj* object = (AlifObj*)alifMem_objAlloc(type->basicSize);
	if (object == nullptr) {
		// error
	}
	alifObject_init(object, type);
	return object;
}

AlifObjVar* alifNew_varObject(AlifInitObject* type, size_t numberItem) {

	size_t totalSize = type->basicSize + (numberItem * type->itemsSize);
	AlifObjVar* object = (AlifObjVar*)alifMem_objAlloc(ALIF_SIZE_ROUND_UP(totalSize, POINTER_SIZE));
	if (object == nullptr) {
		// error
	}
	alifObject_varInit(object, type, numberItem);
	return object;
}

AlifObj* richcompare(AlifObj* v, AlifObj* w, int op) {

	RichCmpFunc func;
	AlifObj* res = nullptr;

	//if (!(value->type == w->type) && ) {}
	if ((func = v->type->richCompare) != nullptr) {
		res = (*func)(v, w, op);
		return res;
	}

	switch (op)
	{
	case ALIF_EQ:
		//res = (v == w) ? ALIF_TRUE : ALIF_FALSE;
		break;
	case ALIF_NE:
		//res = (v != w) ? ALIF_TRUE : ALIF_FALSE;
		break;
	default:
		// error
		break;
	}

	return res;
}

AlifObj* alifObject_richCompare(AlifObj* v, AlifObj* w, int op) {

	if (v == nullptr || w == nullptr) {
		// error
	}

	return richcompare(v, w, op);

}

int alifObject_richCompareBool(AlifObj* v, AlifObj* w, int op) {

	AlifObj* res;
	int ok{};

	if (v == w) {
		if (op == ALIF_EQ) {
			return 1;
		}
		else if (op == ALIF_NE) {
			return 0;
		}
	}

	res = alifObject_richCompare(v, w, op);
	if (res == nullptr) {
		return -1;
	}
	//if (res->type == &typeBool) {
	//	ok = (res == ALIF_TRUE);
	//}

	return ok;
}

size_t alifObject_hashNotImplemented(AlifObj* object) {

	// show error
	return false;

}

AlifObj* alifObject_getAttr(AlifObj* object, AlifObj* name)
{
	AlifInitObject* type = object->type;
	if (!(name->type != &typeUnicode)) {
		// error
	}

	AlifObj* result = nullptr;
	if (type->getAttro != nullptr) {
		result = (*type->getAttro)(object, name);
	}
	else if (type->getAttr != nullptr) {
		const wchar_t* nameStr = (const wchar_t*)(((AlifUStrObject*)name)->UTF);
		if (nameStr == nullptr) {
			return nullptr;
		}
		result = (*type->getAttr)(object, (wchar_t*)nameStr);
	}
	else {
		// error
	}

	if (result == nullptr) {
		//set_attribute_error_context(value, name);
	}
	return result;
}

AlifObj* alifObject_getAttrString(AlifObj* object, const wchar_t* name)
{
	AlifObj* str, * result;

	if (object->type->getAttr != nullptr)
		return (*object->type->getAttr)(object, (wchar_t*)name);
	str = (AlifObj*)alifUnicode_decodeStringToUTF(name);
	if (str == nullptr)
		return nullptr;
	result = alifObject_getAttr(object, str);
	return result;
}

int alifObject_setAttr(AlifObj* object, AlifObj* name, AlifObj* value)
{
	AlifInitObject* type = object->type;
	int error;
	if (!(name->type != &typeUnicode)) {
		// error
	}

	if (type->setAttro != nullptr) {
		error = (*type->setAttro)(value, name, value);
		return error;
	}
	if (type->setAttr != nullptr) {
		const wchar_t* nameStr = (const wchar_t*)(((AlifUStrObject*)name)->UTF);
		if (nameStr == nullptr) {
			// error
		}
		error = (*type->setAttr)(value, (wchar_t*)nameStr, value);
		return error;
	}
	if (type->getAttr == nullptr && type->getAttro == nullptr) {
		// error
	}
	return -1;
}

int alifObj_delAttr(AlifObj* v, AlifObj* name)
{
	return alifObject_setAttr(v, name, nullptr);
}

//AlifObj* alifObject_genericGetAttrWithDict(AlifObj* object, AlifObj* name, AlifObj* dict) {
//
//	AlifInitObject* type = object->type;
//	AlifObj* descr = nullptr;
//	AlifObj* res = nullptr;
//	DescrGetFunc f;
//
//	if (name->type != &typeUnicode) {
//		// error
//	}
//
//	//descr = ;
//
//
//}

size_t alifObject_hash(AlifObj* value) {

	AlifInitObject* type = value->type;

	if (type->hash != nullptr) {
		return (*type->hash)(value);
	}
	else {
		// error the object can not be hashed
		return 0;
	}
	return 0;
}

void alif_dealloc(AlifObj* object) {

	AlifInitObject* type = object->type;
	Destructor dealloc = type->dealloc;

	(*dealloc)(object);

}

void new_reference(AlifObj* object) {
	
	object->ref = 1;
}

void none_dealloc(AlifObj* none)
{
	alif_setImmortal(none);
}

//AlifObj* alifNew_none(AlifInitObject* type, AlifObj* args, AlifObj* kwargs)
//{
//	if (PyTuple_GET_SIZE(args) || (kwargs && PyDict_GET_SIZE(kwargs))) {
//		PyErr_SetString(PyExc_TypeError, "NoneType takes no arguments");
//		return NULL;
//	}
//	Py_RETURN_NONE;
//}

int none_bool(AlifObj* v)
{
	return 0;
}

size_t none_hash(AlifObj* v)
{
	return 3;
}

AlifNumberMethods NoneMethod = {
	0,                          /* nb_add */
	0,                          /* nb_subtract */
	0,                          /* nb_multiply */
	0,                          /* nb_remainder */
	0,                          /* nb_divmod */
	0,                          /* nb_power */
	0,                          /* nb_negative */
	0,                          /* nb_positive */
	0,                          /* nb_absolute */
	(Inquiry)none_bool,         /* nb_bool */
	0,                          /* nb_invert */
	0,                          /* nb_lshift */
	0,                          /* nb_rshift */
	0,                          /* nb_and */
	0,                          /* nb_xor */
	0,                          /* nb_or */
	0,                          /* nb_int */
	0,                          /* nb_reserved */
	0,                          /* nb_float */
	0,                          /* nb_inplace_add */
	0,                          /* nb_inplace_subtract */
	0,                          /* nb_inplace_multiply */
	0,                          /* nb_inplace_remainder */
	0,                          /* nb_inplace_power */
	0,                          /* nb_inplace_lshift */
	0,                          /* nb_inplace_rshift */
	0,                          /* nb_inplace_and */
	0,                          /* nb_inplace_xor */
	0,                          /* nb_inplace_or */
	0,                          /* nb_floor_divide */
	0,                          /* nb_true_divide */
	0,                          /* nb_inplace_floor_divide */
	0,                          /* nb_inplace_true_divide */
	0,                          /* nb_index */
};

AlifInitObject typeNone = {
	//PyVarObject_HEAD_INIT(&PyType_Type, 0)
	L"NoneType",
	0,
	0,
	none_dealloc,       /*tp_dealloc*/
	0,                  /*tp_vectorcall_offset*/
	0,                  /*tp_getattr*/
	0,                  /*tp_setattr*/
	0,          /*tp_repr*/
	&NoneMethod,    /*tp_as_number*/
	0,                  /*tp_as_sequence*/
	0,                  /*tp_as_mapping*/
	(HashFunc)none_hash,/*tp_hash */
	0,                  /*tp_call */
	0,                  /*tp_str */
	0,                  /*tp_getattro */
	0,                  /*tp_setattro */
	0,                  /*tp_as_buffer */
	0, /*tp_flags */
	0,                  /*tp_doc */
	0,                  /*tp_traverse */
	0,                  /*tp_clear */
	0,                  /*tp_richcompare */
	0,                  /*tp_weaklistoffset */
	0,                  /*tp_iter */
	0,                  /*tp_iternext */
	0,                  /*tp_methods */
	0,                  /*tp_members */
	0,                  /*tp_getset */
	0,                  /*tp_base */
	0,                  /*tp_dict */
	0,                  /*tp_descr_get */
	0,                  /*tp_descr_set */
	0,                  /*tp_dictoffset */
	0,                  /*tp_init */
	0,                  /*tp_alloc */
	//none_new,           /*tp_new */
};

AlifObj alifNone = {
	0xffff,
	0,
	&typeNone
};

int alif_Is(AlifObj* x, AlifObj* y)
{
	return (x == y);
}

int alif_IsNone(AlifObj* x)
{
	return alif_Is(x, ALIF_NONE);
}

int alif_isTrue(AlifObj* x)
{
	//return alif_Is(x, ALIF_TRUE);
	return 0;
}

int alif_isFalse(AlifObj* x)
{
	//return alif_Is(x, ALIF_FALSE);
	return 0;
}
