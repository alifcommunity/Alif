#include "alif.h"

#include "AlifCore_ModuleObject.h"






































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










AlifTypeObject _alifModuleType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	L"module",                                  
	sizeof(AlifModuleObject),                   
	0,                                          
	0,                
	0,
	/*
	.
	.
	.
	*/
};
