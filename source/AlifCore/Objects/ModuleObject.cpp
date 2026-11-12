#include "alif.h"

#include "AlifCore_Dict.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Import.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_ObjectDeferred.h" // هذا التضمين غير ضروري ويجب حذفه مع التقدم في البرنامج

#include "OSDefs.h"


// 18
#define ALIFMODULE_CAST(_op) \
    (ALIF_CAST(AlifModuleObject*, _op))

static AlifMemberDef _moduleMembers_[] = {
	{"__فهرس__", ALIF_T_OBJECT, offsetof(AlifModuleObject, dict), ALIF_READONLY},
	{0}
};


AlifTypeObject _alifModuleDefType_ = { // 24
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "تعريف-الوحدة",
	.basicSize = sizeof(AlifModuleDef),
	.itemSize = 0,
};








AlifObject* alifModuleDef_init(AlifModuleDef* _def) { // 45
	if (_def->base.index == 0) {
		ALIF_SET_REFCNT(_def, 1);
		ALIF_SET_TYPE(_def, &_alifModuleDefType_);
		_def->base.index = alifImport_getNextModuleIndex();
	}
	return (AlifObject*)_def;
}


static AlifIntT module_initDict(AlifModuleObject* mod, AlifObject* md_dict,
	AlifObject* name, AlifObject* doc) { // 57
	if (doc == nullptr)
		doc = ALIF_NONE;

	if (alifDict_setItem(md_dict, &ALIF_STR(__name__), name) != 0)
		return -1;
	if (alifDict_setItem(md_dict, &ALIF_ID(__doc__), doc) != 0)
		return -1;
	if (alifDict_setItem(md_dict, &ALIF_STR(__package__), ALIF_NONE) != 0)
		return -1;
	if (alifDict_setItem(md_dict, &ALIF_STR(__loader__), ALIF_NONE) != 0)
		return -1;
	if (alifDict_setItem(md_dict, &ALIF_STR(__spec__), ALIF_NONE) != 0)
		return -1;
	if (ALIFUSTR_CHECKEXACT(name)) {
		ALIF_XSETREF(mod->name, ALIF_NEWREF(name));
	}

	return 0;
}


static AlifModuleObject* newModule_noTrack(AlifTypeObject* mt) { // 82
	AlifModuleObject* m{};
	m = (AlifModuleObject*)alifType_allocNoTrack(mt, 0);
	if (m == nullptr) return nullptr;

	m->def = nullptr;
	m->state = nullptr;
	m->weaklist = nullptr;
	m->name = nullptr;
	m->dict = alifDict_new();
	if (m->dict == nullptr) {
		ALIF_DECREF(m);
		return nullptr;
	}
	return m;
}


static void track_module(AlifModuleObject* _m) { // 107
	_alifDict_enablePerThreadRefcounting(_m->dict);

	alifObject_setDeferredRefcount((AlifObject*)_m);
	alifObject_gcTrack(_m);
}

static AlifObject* new_module(AlifTypeObject* _mt,
	AlifObject* _args, AlifObject* _kws) { // 116
	AlifModuleObject* m = newModule_noTrack(_mt);
	if (m != nullptr) {
		track_module(m);
	}
	return (AlifObject*)m;
}

AlifObject* alifModule_newObject(AlifObject* _name) { // 121
	AlifModuleObject* m = newModule_noTrack(&_alifModuleType_);
	if (m == nullptr) return nullptr;

	if (module_initDict(m, m->dict, _name, nullptr) != 0)
		goto fail;
	track_module(m);
	return (AlifObject*)m;

fail:
	ALIF_DECREF(m);
	return nullptr;
}




AlifObject* alifModule_new(const char* name) { // 137
	AlifObject* nameObj{}, * module{};
	nameObj = alifUStr_fromString(name);
	if (nameObj == nullptr)
		return nullptr;
	module = alifModule_newObject(nameObj);
	ALIF_DECREF(nameObj);
	return module;
}



static AlifIntT addMethods_toObject(AlifObject* _module,
	AlifObject* _name, AlifMethodDef* _functions) { // 169
	AlifObject* func{};
	AlifMethodDef* fdef{};

	for (fdef = _functions; fdef->name != nullptr; fdef++) {
		if ((fdef->flags & METHOD_CLASS) or
			(fdef->flags & METHOD_STATIC)) {
			//alifErr_setString(_alifExcValueError_,
			//	"module functions cannot set"
			//	" METH_CLASS or METH_STATIC");
			return -1;
		}
		func = ALIFCPPFUNCTION_NEWEX(fdef, (AlifObject*)_module, _name);
		if (func == nullptr) {
			return -1;
		}
		alifObject_setDeferredRefcount(func);
		if (alifObject_setAttrString(_module, fdef->name, func) != 0) {
			ALIF_DECREF(func);
			return -1;
		}
		ALIF_DECREF(func);
	}

	return 0;
}


AlifObject* alifModule_createInitialized(AlifModuleDef* _module) { // 209

	const char* name{};
	AlifModuleObject* m{};

	if (!alifModuleDef_init(_module)) return nullptr;

	name = _module->name;
	if (_module->slots) {
		// error
		return nullptr;
	}
	name = alifImport_resolveNameWithPackageContext(name);

	m = (AlifModuleObject*)alifModule_new(name);
	if (m == nullptr) return nullptr;

	if (_module->size > 0) {
		m->state = alifMem_dataAlloc(_module->size);
		if (!m->state) {
			//alifErr_noMemory();
			ALIF_DECREF(m);
			return nullptr;
		}
		memset(m->state, 0, _module->size);
	}

	if (_module->methods != nullptr) {
		if (alifModule_addFunctions((AlifObject*)m, _module->methods) != 0) {
			ALIF_DECREF(m);
			return nullptr;
		}
	}
	m->def = _module;
	m->gil = ALIF_MOD_GIL_USED;

	return (AlifObject*)m;
}







AlifObject* alifModule_fromDefAndSpec2(AlifModuleDef* _def,
	AlifObject* _spec, AlifIntT _moduleApiVersion) { // 266
	AlifModuleDefSlot* curSlot{};
	AlifObject* (*create)(AlifObject*, AlifModuleDef*) = nullptr;
	AlifObject* nameobj{};
	AlifObject* m = nullptr;
	AlifIntT has_multiple_interpreters_slot = 0;
	void* multiple_interpreters = (void*)0;
	AlifIntT has_gil_slot = 0;
	void* gil_slot = ALIF_MOD_GIL_USED;
	AlifIntT has_execution_slots = 0;
	const char* name{};
	AlifIntT ret{};
	AlifInterpreter* interp = _alifInterpreter_get();

	alifModuleDef_init(_def);

	nameobj = alifObject_getAttrString(_spec, "اسم");
	if (nameobj == nullptr) {
		return nullptr;
	}
	name = alifUStr_asUTF8(nameobj);
	if (name == nullptr) {
		goto error;
	}

	//if (!check_apiVersion(name, module_api_version)) {
	//	goto error;
	//}

	if (_def->size < 0) {
		alifErr_format(
			_alifExcSystemError_,
			"الوحدة %s: الحجم يجب أن لا يكون ذو قيمة سالبة للتهيئة متعددة المراحل",
			name);
		goto error;
	}

	for (curSlot = _def->slots; curSlot and curSlot->slot; curSlot++) {
		switch (curSlot->slot) {
		case ALIF_MOD_CREATE:
			if (create) {
				alifErr_format(
					_alifExcSystemError_,
					"الوحدة %s لديها عدة خانات إنشاء",
					name);
				goto error;
			}
			create = (AlifObject * (*)(AlifObject*, AlifModuleDef*))curSlot->value; //* alif

			break;
		case ALIF_MOD_EXEC:
			has_execution_slots = 1;
			break;
		case ALIF_MOD_MULTIPLE_INTERPRETERS:
			if (has_multiple_interpreters_slot) {
				alifErr_format(
					_alifExcSystemError_,
					"الوحدة %s تمتلك أكثر من خانة 'مفسرات متعددة'",
					name);
				goto error;
			}
			multiple_interpreters = curSlot->value;
			has_multiple_interpreters_slot = 1;
			break;
		case ALIF_MOD_GIL:
			if (has_gil_slot) {
				alifErr_format(
					_alifExcSystemError_,
					"الوحدة %s تمتلك أكثر من خانة 'قفل عام'"
					"(ميزة ملغاة - راجع فريق ألف)",
					name);
				goto error;
			}
			gil_slot = curSlot->value;
			has_gil_slot = 1;
			break;
		default:
			alifErr_format(
				_alifExcSystemError_,
				"الوحدة %s تستخدم رقم تعريف خانة غير معروفة %i",
				name, curSlot->slot);
			goto error;
		}
	}

	if (!has_multiple_interpreters_slot) {
		multiple_interpreters = ALIF_MOD_MULTIPLE_INTERPRETERS_SUPPORTED;
	}
	//if (multiple_interpreters == ALIF_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED) {
	//	if (!alif_isMainInterpreter(interp)
	//		and _alifImport_checkSubinterpIncompatibleExtensionAllowed(name) < 0)
	//	{
	//		goto error;
	//	}
	//}
	//else if (multiple_interpreters != ALIF_MOD_PER_INTERPRETER_GIL_SUPPORTED
	//	and interp->eval.ownGil
	//	and !alif_isMainInterpreter(interp)
	//	and _alifImport_checkSubinterpIncompatibleExtensionAllowed(name) < 0)
	//{
	//	goto error;
	//}

	if (create) {
		m = create(_spec, _def);
		if (m == nullptr) {
			if (!alifErr_occurred()) {
				alifErr_format(
					_alifExcSystemError_,
					"فشل إنشاء الوحدة %s بدون ضبط خطأ",
					name);
			}
			goto error;
		}
		else {
			if (alifErr_occurred()) {
				//_alifErr_formatFromCause(
				//	_alifExcSystemError_,
				//	"creation of module %s raised unreported exception",
				//	name);
				goto error;
			}
		}
	}
	else {
		m = alifModule_newObject(nameobj);
		if (m == nullptr) {
			goto error;
		}
	}

	if (ALIFMODULE_CHECK(m)) {
		((AlifModuleObject*)m)->state = nullptr;
		((AlifModuleObject*)m)->def = _def;
		((AlifModuleObject*)m)->gil = gil_slot;
	}
	else {
		if (_def->size > 0 or _def->traverse or _def->clear or _def->free) {
			alifErr_format(
				_alifExcSystemError_,
				"الوحدة %s ليست كائن وحدة, ولكن تقوم بطلب حالة الوحدة",
				name);
			goto error;
		}
		if (has_execution_slots) {
			alifErr_format(
				_alifExcSystemError_,
				"الوحدة %s تحدد خانات التنفيذ, ولكنها لم تنشئ "
				"نسخة لـ ModuleType",
				name);
			goto error;
		}
	}

	if (_def->methods != nullptr) {
		ret = addMethods_toObject(m, nameobj, _def->methods);
		if (ret != 0) {
			goto error;
		}
	}

	//if (def->doc != nullptr) {
	//	ret = alifModule_setDocString(m, def->doc);
	//	if (ret != 0) {
	//		goto error;
	//	}
	//}

	ALIF_DECREF(nameobj);
	return m;

error:
	ALIF_DECREF(nameobj);
	ALIF_XDECREF(m);
	return nullptr;
}










AlifIntT alifUnstable_moduleSetGIL(AlifObject* _module, void* _gil) { // 442
	if (!ALIFMODULE_CHECK(_module)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	((AlifModuleObject*)_module)->gil = _gil;
	return 0;
}


AlifIntT alifModule_execDef(AlifObject* _module, AlifModuleDef* _def) { // 460
	AlifModuleDefSlot* curSlot{};
	const char* name{};
	AlifIntT ret{};

	name = alifModule_getName(_module);
	if (name == nullptr) {
		return -1;
	}

	if (_def->size >= 0) {
		AlifModuleObject* md = (AlifModuleObject*)_module;
		if (md->state == nullptr) {
			md->state = alifMem_dataAlloc(_def->size ? _def->size : 1); //* alif
			if (!md->state) {
				//alifErr_noMemory();
				return -1;
			}
			memset(md->state, 0, _def->size);
		}
	}

	if (_def->slots == nullptr) {
		return 0;
	}

	for (curSlot = _def->slots; curSlot and curSlot->slot; curSlot++) {
		switch (curSlot->slot) {
		case ALIF_MOD_CREATE:
			/* handled in alifModule_fromDefAndSpec2 */
			break;
		case ALIF_MOD_EXEC:
			ret = ((AlifIntT(*)(AlifObject*))curSlot->value)(_module);
			if (ret != 0) {
				if (!alifErr_occurred()) {
					alifErr_format(
						_alifExcSystemError_,
						"فشل تنفيذ الوحدة %s بدون ضبط خطأ",
						name);
				}
				return -1;
			}
			if (alifErr_occurred()) {
				//_alifErr_formatFromCause(
				//	_alifExcSystemError_,
				//	"execution of module %s raised unreported exception",
				//	name);
				return -1;
			}
			break;
		case ALIF_MOD_MULTIPLE_INTERPRETERS:
		case ALIF_MOD_GIL:
			/* handled in alifModule_fromDefAndSpec2 */
			break;
		default:
			alifErr_format(
				_alifExcSystemError_,
				"الوحدة %s تمت تهيئتها بخانة غير معروفة %i",
				name, curSlot->slot);
			return -1;
		}
	}
	return 0;
}




AlifIntT alifModule_addFunctions(AlifObject* _m,
	AlifMethodDef* _functions) { // 523
	AlifIntT res{};
	AlifObject* name = alifModule_getNameObject(_m);
	if (name == nullptr) {
		return -1;
	}

	res = addMethods_toObject(_m, name, _functions);
	ALIF_DECREF(name);
	return res;
}


AlifObject* alifModule_getDict(AlifObject* _m) { // 551
	if (!ALIFMODULE_CHECK(_m)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	return _alifModule_getDict(_m);  // borrowed reference
}


AlifObject* alifModule_getNameObject(AlifObject* _mod) { // 561
	AlifObject* name{};

	if (!ALIFMODULE_CHECK(_mod)) {
		//alifErr_badArgument();
		return nullptr;
	}
	AlifObject* dict = ((AlifModuleObject*)_mod)->dict;  // borrowed reference
	if (dict == nullptr or !ALIFDICT_CHECK(dict)) {
		goto error;
	}
	// name
	if (alifDict_getItemRef(dict, &ALIF_STR(__name__), &name) <= 0) {
		// error or not found
		goto error;
	}
	if (!ALIFUSTR_CHECK(name)) {
		ALIF_DECREF(name);
		goto error;
	}
	return name;

error:
	if (!alifErr_occurred()) {
		alifErr_setString(_alifExcSystemError_, "وحدة بدون اسم");
	}
	return nullptr;
}


const char* alifModule_getName(AlifObject* _m) { // 596
	AlifObject* name = alifModule_getNameObject(_m);
	if (name == nullptr) {
		return nullptr;
	}
	ALIF_DECREF(name);   /* module dict has still a reference */
	return alifUStr_asUTF8(name);
}


AlifObject* alifModule_getFilenameObject(AlifObject* _mod) { // 602
	AlifObject* fileobj{};

	if (!ALIFMODULE_CHECK(_mod)) {
		//ALIFERR_BADARGUMENT();
		return nullptr;
	}
	AlifObject* dict = ((AlifModuleObject*)_mod)->dict;  // borrowed reference
	if (dict == nullptr) {
		goto error;
	}
	if (alifDict_getItemRef(dict, &ALIF_ID(__file__), &fileobj) <= 0) {
		// error or not found
		goto error;
	}
	if (!ALIFUSTR_CHECK(fileobj)) {
		ALIF_DECREF(fileobj);
		goto error;
	}
	return fileobj;

error:
	if (!alifErr_occurred()) {
		alifErr_setString(_alifExcSystemError_, "اسم ملف الوحدة مفقود");
	}
	return nullptr;
}



#include "clinic/ModuleObject.cpp.h"



AlifModuleDef* alifModule_getDef(AlifObject* m) { // 650
	if (!ALIFMODULE_CHECK(m)) {
		//ALIFERR_BADARGUMENT();
		return nullptr;
	}
	return _alifModule_getDef(m);
}

void* alifModule_getState(AlifObject* m) { // 660
	if (!ALIFMODULE_CHECK(m)) {
		//ALIFERR_BADARGUMENT();
		return nullptr;
	}
	return _alifModule_getState(m);
}



static AlifIntT module___init__Impl(AlifModuleObject* _self,
	AlifObject* _name, AlifObject* _doc) { // 760
	return module_initDict(_self, _self->dict, _name, _doc);
}


static void module_dealloc(AlifObject* self) { // 767
	AlifModuleObject* m = ALIFMODULE_CAST(self);

	alifObject_gcUnTrack(m);

	AlifIntT verbose = _alif_getConfig()->verbose;
	if (verbose and m->name) {
		//alifSys_formatStderr("# destroy %U\n", m->name);
	}
	if (m->weaklist != nullptr)
		alifObject_clearWeakRefs((AlifObject*)m);

	if (m->def and m->def->free
		and (m->def->size <= 0 or m->state != nullptr)) {
		m->def->free(m);
	}

	ALIF_XDECREF(m->dict);
	ALIF_XDECREF(m->name);
	if (m->state != nullptr)
		alifMem_dataFree(m->state);
	ALIF_TYPE(m)->free((AlifObject*)m);
}


static AlifObject* module_repr(AlifObject* self) {
	AlifModuleObject* m = ALIFMODULE_CAST(self);
	AlifInterpreter* interp = _alifInterpreter_get();
	//return _alifImport_importLibModuleRepr(interp, (AlifObject*)m);
}


AlifIntT _alifModuleSpec_isInitializing(AlifObject* _spec) { // 793
	if (_spec == nullptr) {
		return 0;
	}
	AlifObject* value{};
	AlifIntT rc = alifObject_getOptionalAttr(_spec, &ALIF_STR(_initializing), &value);
	if (rc > 0) {
		rc = alifObject_isTrue(value);
		ALIF_DECREF(value);
	}
	return rc;
}



AlifIntT _alifModuleSpec_getFileOrigin(AlifObject* _spec, AlifObject** _pOrigin) { // 839
	AlifObject* hasLocation = nullptr;
	AlifIntT rc_ = alifObject_getOptionalAttr(_spec, &ALIF_ID(HasLocation), &hasLocation);
	if (rc_ <= 0) {
		return rc_;
	}
	rc_ = alifObject_isTrue(hasLocation);
	ALIF_DECREF(hasLocation);
	if (rc_ <= 0) {
		return rc_;
	}
	AlifObject* origin = nullptr;
	rc_ = alifObject_getOptionalAttr(_spec, &ALIF_ID(Origin), &origin);
	if (rc_ <= 0) {
		return rc_;
	}
	if (!ALIFUSTR_CHECK(origin)) {
		ALIF_DECREF(origin);
		return 0;
	}
	*_pOrigin = origin;
	return 1;
}

AlifIntT _alifModule_isPossiblyShadowing(AlifObject* _origin) { // 870
	if (_origin == nullptr) {
		return 0;
	}

	const AlifConfig* config = _alif_getConfig();
	if (config->safePath) {
		return 0;
	}

	wchar_t root[MAXPATHLEN + 1];
	AlifSizeT size = alifUStr_asWideChar(_origin, root, MAXPATHLEN);
	if (size < 0) {
		return -1;
	}
	root[size] = L'\0';

	wchar_t* sep_ = wcsrchr(root, SEP);
	if (sep_ == nullptr) {
		return 0;
	}

	if (wcscmp(sep_ + 1, L"__init__.alif") == 0) {
		*sep_ = L'\0';
		sep_ = wcsrchr(root, SEP);
		if (sep_ == nullptr) {
			return 0;
		}
	}
	*sep_ = L'\0';

	wchar_t* sysPath0 = config->sysPath0;
	if (!sysPath0) {
		return 0;
	}

	wchar_t sysPath0Buf[MAXPATHLEN];
	if (sysPath0[0] == L'\0') {
		if (!_alif_wGetCWD(sysPath0Buf, MAXPATHLEN)) {
			return -1;
		}
		sysPath0 = sysPath0Buf;
	}

	AlifIntT result = wcscmp(sysPath0, root) == 0;
	return result;
}

AlifObject* alifModule_getAttroImpl(AlifModuleObject* _m,
	AlifObject* _name, AlifIntT _suppress) { // 921
	AlifObject* attr{}, * modName{}, * getAttr{};
	AlifIntT isPossiblyShadowing{};
	AlifIntT isPossiblyShadowingStdLib = 0;

	attr = _alifObject_genericGetAttrWithDict((AlifObject*)_m, _name, nullptr, _suppress);
	if (attr) {
		return attr;
	}
	if (_suppress == 1) {
		if (alifErr_occurred()) {
			// pass up non-AttributeError exception
			return nullptr;
		}
	}
	else {
		if (!alifErr_exceptionMatches(_alifExcAttributeError_)) {
			// pass up non-AttributeError exception
			return nullptr;
		}
		alifErr_clear();
	}
	if (alifDict_getItemRef(_m->dict, &ALIF_ID(__getAttr__), &getAttr) < 0) {
		return nullptr;
	}
	if (getAttr) {
		AlifObject* result = alifObject_callOneArg(getAttr, _name);
		//if (result == nullptr and _suppress == 1
		//	and alifErr_exceptionMatches(exception)
		//	) {
		//	alifErr_clear();
		//	return nullptr;
		//}
		ALIF_DECREF(getAttr);
		return result;
	}
	if (_suppress == 1) {
		return nullptr;
	}
	if (alifDict_getItemRef(_m->dict, &ALIF_STR(__name__), &modName) < 0) {
		return nullptr;
	}
	if (!modName or !ALIFUSTR_CHECK(modName)) {
		ALIF_XDECREF(modName);
		alifErr_format(_alifExcAttributeError_,
			"الوحدة لا تملك الصفة '%U'", _name);
		return nullptr;
	}
	AlifObject* spec{};
	if (alifDict_getItemRef(_m->dict, &ALIF_STR(__spec__), &spec) < 0) {
		ALIF_DECREF(modName);
		return nullptr;
	}
	if (spec == nullptr) {
		alifErr_format(_alifExcAttributeError_,
			"الوحدة '%U' لا تملك هذه الصفة '%U'",
			modName, _name);
		ALIF_DECREF(modName);
		return nullptr;
	}

	AlifObject* origin = nullptr;
	if (_alifModuleSpec_getFileOrigin(spec, &origin) < 0) {
		goto done;
	}

	isPossiblyShadowing = _alifModule_isPossiblyShadowing(origin);
	if (isPossiblyShadowing < 0) {
		goto done;
	}
	isPossiblyShadowingStdLib = 0;
	if (isPossiblyShadowing) {
		AlifObject* stdlibModules = alifSys_getObject("stdlib_module_names");
		if (stdlibModules and ALIFANYSET_CHECK(stdlibModules)) {
			isPossiblyShadowingStdLib = alifSet_contains(stdlibModules, modName);
			if (isPossiblyShadowingStdLib < 0) {
				goto done;
			}
		}
	}

	if (isPossiblyShadowingStdLib) {
		alifErr_format(_alifExcAttributeError_,
			"الوحدة '%U' لا تحتوي على الصفة '%U' "
			"(يُنصح بإعادة تسمية '%U' لأنها تحمل نفس "
			"اسم الوحدة النمطية للمكتبة القياسية المسماة '%U' "
		  "وتمنع استيراد تلك المكتبة القياسية)",
			modName, _name, origin, modName);
	}
	else {
		AlifIntT rc_ = _alifModuleSpec_isInitializing(spec);
		if (rc_ < 0) {
			goto done;
		}
		else if (rc_ > 0) {
			if (isPossiblyShadowing) {
				alifErr_format(_alifExcAttributeError_,
						"الوحدة '%U' لا تحتوي على الصفة '%U' "
							"(يُنصح بإعادة تسمية '%U' إذا كانت تحمل نفس الاسم "
							"المكتبة التي تحاول استيرادها)",
							modName, _name, origin);
			}
			else if (origin) {
				alifErr_format(_alifExcAttributeError_,
				"مهيئ بشكل جزئي "
					"الوحدة '%U' من '%U' لا تملك الصفة '%U' "
					"(على الأرجح بسبب استيراد دائري)",
					modName, origin, _name);
			}
			else {
					alifErr_format(_alifExcAttributeError_,
					"مهيئ بشكل جزئي "
						"الوحدة '%U' لا تملك الصفة '%U' "
						"(على الأرجح بسبب استيراد دائري)",
						modName, _name);
			}
		}
		else {
			//rc_ = _alifModuleSpec_isUninitializedSubModule(spec, _name);
			if (rc_ > 0) {
				alifErr_format(_alifExcAttributeError_,
				"لا يمكن الوصول إلى الوحدة الفرعية '%U' من الوحدة '%U' "
					"(على الأرجح بسبب استيراد دائري)",
					_name, modName);
			}
			else if (rc_ == 0) {
				alifErr_format(_alifExcAttributeError_,
				"الوحدة '%U' لا تملك الصفة '%U'",
					modName, _name);
			}
		}
	}

done:
	ALIF_XDECREF(origin);
	ALIF_DECREF(spec);
	ALIF_DECREF(modName);
	return nullptr;
}


AlifObject* alifModule_getAttro(AlifObject* _self, AlifObject* _name) { // 1065
	AlifModuleObject* m = ALIFMODULE_CAST(_self);
	return alifModule_getAttroImpl(m, _name, 0);
}


static AlifIntT module_traverse(AlifObject* _self,
	VisitProc visit, void* arg) { // 1070
	AlifModuleObject* m = ALIFMODULE_CAST(_self);
	if (m->def and m->def->traverse
		and (m->def->size <= 0 or m->state != nullptr)) {
		AlifIntT res = m->def->traverse((AlifObject*)m, visit, arg);
		if (res)
			return res;
	}
	ALIF_VISIT(m->dict);
	return 0;
}



AlifTypeObject _alifModuleType_ = { // 1320
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "وحدة",
	.basicSize = sizeof(AlifModuleObject),
	.dealloc = (Destructor)module_dealloc,
	//.repr = module_repr,
	.getAttro = (GetAttroFunc)alifModule_getAttro,
	.setAttro = alifObject_genericSetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE,
	.traverse = (TraverseProc)module_traverse,
	.weakListOffset = offsetof(AlifModuleObject, weaklist),

	.members = _moduleMembers_,

	.dictOffset = offsetof(AlifModuleObject, dict),
	.init = module___init__,
	.new_ = new_module,
	.free = alifObject_gcDel,
};
