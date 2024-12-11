#include "alif.h"

#include "AlifCore_Import.h"
#include "AlifCore_State.h"
//#include "AlifCore_InitConfig.h"






extern InitTable _alifImportInitTab_[]; // 55

InitTable* _alifImportInitTable_ = _alifImportInitTab_; // 59

#define INITTABLE _alifDureRun_.imports.initTable // 69
#define LAST_MODULE_INDEX _alifDureRun_.imports.lastModuleIndex // 70
//#define EXTENSIONS _alifDureRun_.imports.extensions // 71

// 80
#define MODULES(interp) \
    (interp)->imports.modules


AlifObject* alifImport_initModules(AlifInterpreter* _interp) { // 129
	MODULES(_interp) = alifDict_new();
	if (MODULES(_interp) == nullptr) {
		return nullptr;
	}
	return MODULES(_interp);
}


static inline AlifObject* get_modulesDict(AlifThread* tstate, bool fatal) { // 152
	AlifObject* modules = MODULES(tstate->interpreter);
	if (modules == nullptr) {
		if (fatal) {
			//alif_fatalError("interpreter has no modules dictionary");
		}
		//alifErr_setString(tstate, _alifExcRuntimeError_, "unable to get sys.modules");
		return nullptr;
	}
	return modules;
}

static AlifObject* import_addModule(AlifThread* _thread,
	AlifObject* _name) { // 261

	AlifObject* modules_ = get_modulesDict(_thread, false);
	if (modules_ == nullptr) {
		return nullptr;
	}

	AlifObject* m_{};
	if (alifMapping_getOptionalItem(modules_, _name, &m_) < 0) {
		return nullptr;
	}
	if (m_ != nullptr and ALIFMODULE_CHECK(m_)) {
		return m_;
	}
	ALIF_XDECREF(m_);
	m_ = alifModule_newObject(_name);
	if (m_ == nullptr) return nullptr;

	if (alifObject_setItem(modules_, _name, m_) != 0) {
		ALIF_DECREF(m_);
		return nullptr;
	}

	return m_;
}


AlifObject* alifImport_addModuleRef(const char* _name) { // 288
	AlifObject* nameObj = alifUStr_fromString(_name);
	if (nameObj == nullptr) {
		return nullptr;
	}
	AlifThread* thread = _alifThread_get();
	AlifObject* module = import_addModule(thread, nameObj);
	ALIF_DECREF(nameObj);
	return module;
}



AlifSizeT alifImport_getNextModuleIndex() { // 381
	return alifAtomic_addSize(&LAST_MODULE_INDEX, 1) + 1;
}


#ifdef HAVE_LOCAL_THREAD
ALIF_LOCAL_THREAD const char* _pkgcontext_ = nullptr;
# undef PKGCONTEXT
# define PKGCONTEXT _pkgcontext_
#endif

const char* alifImport_resolveNameWithPackageContext(const char* name) { // 740
#ifndef HAVE_LOCAL_THREAD
	alifThread_acquireLock(EXTENSIONS.mutex, WAIT_LOCK);
#endif
	if (PKGCONTEXT != nullptr) {
		const char* p = strrchr(PKGCONTEXT, '.');
		if (p != nullptr and strcmp(name, p + 1) == 0) {
			name = PKGCONTEXT;
			PKGCONTEXT = nullptr;
		}
	}
#ifndef HAVE_LOCAL_THREAD
	alifThread_releaseLock(EXTENSIONS.mutex);
#endif
	return name;
}





/* the internal table */

static AlifIntT initBuildin_modulesTable() { // 2419

	AlifUSizeT size_{};
	for (size_ = 0; _alifImportInitTable_[size_].name != nullptr; size_++)
		;
	size_++;

	InitTable* tableCopy = (InitTable*)alifMem_dataAlloc(size_ * sizeof(InitTable));
	if (tableCopy == nullptr) return -1;

	memcpy(tableCopy, _alifImportInitTable_, size_ * sizeof(InitTable));
	INITTABLE = tableCopy;
	return 0;
}





// alif
AlifIntT alifImport_init() { // 3954

	if (INITTABLE != nullptr) {
		// error
		return -1; // alif
	}

	if (initBuildin_modulesTable() != 0) {
		return -1;
	}

	return 1;
}
