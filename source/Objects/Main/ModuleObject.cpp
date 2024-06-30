#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_AlifState.h"




AlifObject* alifModule_getNameObject(AlifObject* mod) { // 560

	AlifObject* name{};
	AlifObject* name1{};

	if (!ALIFMODULE_CHECK(mod)) {
		//alifErr_badArgument();
		return nullptr;
	}
	AlifObject* dict = ((AlifModuleObject*)mod)->dict;
	if (dict == nullptr or !ALIFDICT_CHECK(dict)) {
		goto error;
	}
	name1 = alifUStr_decodeStringToUTF8(L"__name__");
	if (alifDict_getItemRef(dict, name1, &name) <= 0) {
		goto error;
	}
	if (!ALIFUSTR_CHECK(name)) {
		ALIF_DECREF(name);
		goto error;
	}
	return name;

error:
	//if (!alifErr_occurred()) {
	//	alifErr_setString(alifExcSystemError, "nameless module");
	//}
	return nullptr;
}










AlifObject* alifModule_getAttroImpl(AlifModuleObject* m, AlifObject* name, AlifIntT suppress) { // 919
	AlifObject* attr, * modName, * getattr;

	AlifIntT isPossiblyShadowing{};
	AlifIntT isPossiblyShadowingStdlib = 0;
	AlifObject* origin{};
	AlifObject* spec{};


	attr = alifSubObject_genericGetAttrWithDict((AlifObject*)m, name, nullptr, suppress);
	if (attr) {
		return attr;
	}
	if (suppress == 1) {
		//if (alifErr_occurred()) {
		//	return nullptr;
		//}
	}
	else {
		//if (!alifErr_exceptionMatches(alifExcAttributeError)) {
		//	return nullptr;
		//}
		//alifErr_clear();
	}

	AlifObject* name1 = alifUStr_decodeStringToUTF8(L"__getattr__");
	if (alifDict_getItemRef(m->dict, name1, &getattr) < 0) {
		return nullptr;
	}
	if (getattr) {
		AlifObject* result = alifObject_callOneArg(getattr, name);
		//if (result == nullptr and suppress == 1 and alifErr_exceptionMatches(alifExcAttributeError)) {
		//	alifErr_clear();
		//}
		ALIF_DECREF(getattr);
		return result;
	}

	if (suppress == 1) {
		return nullptr;
	}
	AlifObject* name2 = alifUStr_decodeStringToUTF8(L"__name__");
	if (alifDict_getItemRef(m->dict, name2, &modName) < 0) {
		return nullptr;
	}
	if (!modName or !ALIFUSTR_CHECK(modName)) {
		ALIF_XDECREF(modName);
		//alifErr_format(alifExcAttributeError,
		//	"module has no attribute '%U'", name);
		return nullptr;
	}

	AlifObject* name3 = alifUStr_decodeStringToUTF8(L"__spec__");
	if (alifDict_getItemRef(m->dict, name3, &spec) < 0) {
		ALIF_DECREF(modName);
		return nullptr;
	}
	if (spec == nullptr) {
		//alifErr_format(alifExcAttributeError,
		//	"module '%U' has no attribute '%U'",
		//	modName, name);
		ALIF_DECREF(modName);
		return nullptr;
	}

	origin = nullptr;
	//if (getFileOrigin_fromSpec(spec, &origin) < 0) {
	//	goto done;
	//}

	//isPossiblyShadowing = isModule_possiblyShadowing(origin);
	//if (isPossiblyShadowing < 0) {
	//	goto done;
	//}

	if (isPossiblyShadowing) {
		//AlifObject* stdlibModules = alifSys_getObject(L"stdlib_module_names");
		//if (stdlibModules and ALIFANYSET_CHECK(stdlibModules)) {
		//	isPossiblyShadowingStdlib = alifSet_contains(stdlibModules, modName);
		//	if (isPossiblyShadowingStdlib < 0) {
		//		goto done;
		//	}
		//}
	}

	if (isPossiblyShadowingStdlib) {
		//alifErr_format(alifExcAttributeError,
		//	"module '%U' has no attribute '%U' "
		//	"(consider renaming '%U' since it has the same "
		//	"name as the standard library module named '%U' "
		//	"and the import system gives it precedence)",
		//	modName, name, origin, modName);
	}
	else {
		//int rc = alifModuleSpec_isInitializing(spec);
		//if (rc > 0) {
		//	if (is_possibly_shadowing) {
		//		alifErr_format(alifExcAttributeError,
		//			"module '%U' has no attribute '%U' "
		//			"(consider renaming '%U' if it has the same name "
		//			"as a third-party module you intended to import)",
		//			modName, name, origin);
		//	}
		//	else if (origin) {
		//		alifErr_format(alifExcAttributeError,
		//			"partially initialized "
		//			"module '%U' from '%U' has no attribute '%U' "
		//			"(most likely due to a circular import)",
		//			modName, origin, name);
		//	}
		//	else {
		//		alifErr_format(alifExcAttributeError,
		//			"partially initialized "
		//			"module '%U' has no attribute '%U' "
		//			"(most likely due to a circular import)",
		//			modName, name);
		//	}
		//}
		//else if (rc == 0) {
		//	rc = alifModuleSpec_isUninitializedSubmodule(spec, name);
		//	if (rc > 0) {
		//		alifErr_format(alifExcAttributeError,
		//			"cannot access submodule '%U' of module '%U' "
		//			"(most likely due to a circular import)",
		//			name, modName);
		//	}
		//	else if (rc == 0) {
		//		alifErr_format(alifExcAttributeError,
		//			"module '%U' has no attribute '%U'",
		//			modName, name);
		//	}
		//}
	}

done:
	ALIF_XDECREF(origin);
	ALIF_DECREF(spec);
	ALIF_DECREF(modName);
	return nullptr;
}


AlifObject* alifModule_getAttro(AlifModuleObject* m, AlifObject* name) { // 1063
	return alifModule_getAttroImpl(m, name, 0);
}









static int moduleInit_dict(AlifModuleObject* _mod, AlifObject* _dict,
	AlifObject* _name, AlifObject* _doc)
{
	if (_doc == nullptr)
		_doc = ALIF_NONE;
	AlifObject* nameName = alifUStr_fromString(L"name");
	if (alifDict_setItem(_dict, nameName, _name) != 0)
		return -1;
	AlifObject* nameDoc = alifUStr_fromString(L"doc");
	if (alifDict_setItem(_dict, nameDoc, _doc) != 0)
		return -1;
	AlifObject* namePackage = alifUStr_fromString(L"package");
	if (alifDict_setItem(_dict, namePackage, ALIF_NONE) != 0)
		return -1;
	AlifObject* nameLoader = alifUStr_fromString(L"loader");
	if (alifDict_setItem(_dict, nameLoader, ALIF_NONE) != 0)
		return -1;
	AlifObject* nameSpac = alifUStr_fromString(L"spac");
	if (alifDict_setItem(_dict, nameSpac, ALIF_NONE) != 0)
		return -1;
	if (ALIFUSTR_CHECK(_name)) {
		ALIF_XSETREF(_mod->name, ALIF_NEWREF(_name));
	}

	return 0;
}

static AlifModuleObject* newModule_noTrack(AlifTypeObject* _moduleType)
{
	AlifModuleObject* m_;
	m_ = (AlifModuleObject*)alifType_allocNoTrack(_moduleType, 0);
	if (m_ == nullptr)
		return nullptr;
	m_->def = nullptr;
	m_->state = nullptr;
	m_->weakList = nullptr;
	m_->name = nullptr;
	m_->dict = alifNew_dict();
	if (m_->dict != nullptr) {
		return m_;
	}
	ALIF_DECREF(m_);
	return nullptr;
}

static AlifObject* new_module(AlifTypeObject* mt, AlifObject* args, AlifObject* kws)
{
	AlifObject* m = (AlifObject*)newModule_noTrack(mt);
	if (m != nullptr) {
		alifObject_gc_track(m);
	}
	return m;
}

AlifObject* alifModule_newObject(AlifObject* _name)
{
	AlifModuleObject* m_ = newModule_noTrack(&_alifModuleType_);
	if (m_ == nullptr)
		return nullptr;
	if (moduleInit_dict(m_, m_->dict, _name, nullptr) != 0)
		goto fail;
	alifObject_gc_track(m_);
	return (AlifObject*)m_;

fail:
	ALIF_DECREF(m_);
	return nullptr;
}

AlifObject* alifNew_module(const wchar_t* _name)
{
	AlifObject* nameObj, * module;
	nameObj = alifUStr_fromString(_name);
	if (nameObj == nullptr)
		return nullptr;
	module = alifModule_newObject(nameObj);
	ALIF_DECREF(nameObj);
	return module;
}

AlifObject* alifModule_getDict(AlifObject* _m)
{
	if (!(_m->type_ == &_alifModuleType_)) {
		return NULL;
	}
	return alifSubModule_getDict(_m);  // borrowed reference
}

AlifTypeObject _alifModuleType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	L"module",                                   /* tp_name */
	0, //sizeof(AlifModuleObject),                     /* tp_basicsize */
	0,                                          /* tp_itemsize */
	0, //(destructor)module_dealloc,                 /* tp_dealloc */
	0,                                          /* tp_vectorcall_offset */
	0,                                          /* tp_getattr */
	0,                                          /* tp_setattr */
	0, //(reprfunc)module_repr,                      /* tp_repr */
	0,                                          /* tp_as_number */
	0,                                          /* tp_as_sequence */
	0,                                          /* tp_as_mapping */
	0,                                          /* tp_hash */
	0,                                          /* tp_call */
	0,                                          /* tp_str */
	0, //(getattrofunc)alif_module_getattro,          /* tp_getattro */
	0, //alifObject_GenericSetAttr,                    /* tp_setattro */
	0,                                          /* tp_as_buffer */
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC |
		ALIFTPFLAGS_BASETYPE,                    /* tp_flags */
	0, //(traverseproc)module_traverse,              /* tp_traverse */
	0, //(inquiry)module_clear,                      /* tp_clear */
	0,                                          /* tp_richcompare */
	0,                                          /* tp_iter */
	offsetof(AlifModuleObject, weakList),      /* tp_weaklistoffset */
	0,                                          /* tp_iternext */
	0, //module_methods,                             /* tp_methods */
	0, //module_members,                             /* tp_members */
	0,//module_getsets,                             /* tp_getset */
	0,                                          /* tp_base */
	0,                                          /* tp_dict */
	0,                                          /* tp_descr_get */
	0,                                          /* tp_descr_set */
	0, //offsetof(alifModuleObject, md_dict),          /* tp_dictoffset */
	0, //module___init__,                            /* tp_init */
	0,                                          /* tp_alloc */
	0,
	new_module,                                 /* tp_new */
	alifObject_gcDel,                            /* tp_free */
};
