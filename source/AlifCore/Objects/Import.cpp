#include "alif.h"

#include "AlifCore_Import.h"
#include "AlifCore_State.h"
//#include "AlifCore_InitConfig.h"






extern InitTable _alifImportInitTab_[]; // 55

InitTable* _alifImportInitTable_ = _alifImportInitTab_; // 59

#define INITTABLE _alifDureRun_.imports.initTable // 69
#define LAST_MODULE_INDEX _alifDureRun_.imports.lastModuleIndex // 70

// 80
#define MODULES(interp) \
    (interp)->imports.modules_


//AlifObject* alifImport_initModules(AlifInterpreter* _interp) { // 129
//	MODULES(_interp) = alifNew_dict();
//	if (MODULES(_interp) == nullptr) {
//		return nullptr;
//	}
//	return MODULES(_interp);
//}








AlifSizeT alifImport_getNextModuleIndex() { // 381
	return alifAtomic_addSize(&LAST_MODULE_INDEX, 1) + 1;
}







/* the internal table */

//static AlifIntT initBuildin_modulesTable() { // 2419
//
//	AlifUSizeT size_{};
//
//	for (size_ = 0; _alifImportInitTable_[size_].name != nullptr; size_++)
//		;
//	size_++;
//
//	InitTable* tableCopy = (InitTable*)alifMem_dataAlloc(size_ * sizeof(InitTable));
//	if (tableCopy == nullptr) return -1;
//
//	memcpy(tableCopy, _alifImportInitTable_, size_ * sizeof(InitTable));
//	INITTABLE = tableCopy;
//	return 0;
//}



//static AlifObject* import_addModule(AlifThread* _tstate, AlifObject* _name)
//{
//	AlifObject* modules_ = _tstate->interpreter->imports.modules_;
//	if (modules_ == nullptr) {
//		return nullptr;
//	}
//
//	AlifObject* m_;
//	if (alifMapping_getOptionalItem(modules_, _name, &m_) < 0) {
//		return nullptr;
//	}
//	if (m_ != nullptr and ALIFMODULE_CHECK(m_)) {
//		return m_;
//	}
//	ALIF_XDECREF(m_);
//	m_ = alifModule_newObject(_name);
//	if (m_ == nullptr)
//		return nullptr;
//	if (alifObject_setItem(modules_, _name, m_) != 0) {
//		ALIF_DECREF(m_);
//		return nullptr;
//	}
//
//	return m_;
//}

//AlifObject* alifImport_addModuleRef(const wchar_t* _name) { // 288
//	AlifObject* nameObj = alifUStr_fromString(_name);
//	if (nameObj == nullptr) {
//		return nullptr;
//	}
//	AlifThread* tstate = alifThread_get();
//	AlifObject* module_ = import_addModule(tstate, nameObj);
//	ALIF_DECREF(nameObj);
//	return module_;
//}


//AlifIntT alifImport_init() { // 3954
//
//	if (INITTABLE != nullptr) {
//		// error
//		return -1;
//	}
//
//	if (initBuildin_modulesTable() != 0) {
//		return -1;
//	}
//
//	return 1;
//}
