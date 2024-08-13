#include "alif.h"

//#include "AlifCore_Import.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Memory.h"






extern InitTable alifImportInitTab[];

InitTable* alifImportInitTable = alifImportInitTab;

#define INITTABLE _alifDureRun_.imports.initTable


#define MODULES(interp) \
    (interp)->imports.modules_


AlifObject* alifImport_initModules(AlifInterpreter* _interp)
{
	MODULES(_interp) = alifNew_dict();
	if (MODULES(_interp) == nullptr) {
		return nullptr;
	}
	return MODULES(_interp);
}





static AlifIntT initBuildin_modulesTable() {

	AlifUSizeT size_{};

	for (size_ = 0; alifImportInitTable[size_].name != nullptr; size_++)
		;
	size_++;

	InitTable* tableCopy = (InitTable*)alifMem_dataAlloc(size_ * sizeof(InitTable));
	if (tableCopy == nullptr) return -1;

	memcpy(tableCopy, alifImportInitTable, size_ * sizeof(InitTable));
	INITTABLE = tableCopy;
	return 0;
}



static AlifObject* import_addModule(AlifThread* _tstate, AlifObject* _name)
{
	AlifObject* modules_ = _tstate->interpreter->imports.modules_;
	if (modules_ == nullptr) {
		return nullptr;
	}

	AlifObject* m_;
	if (alifMapping_getOptionalItem(modules_, _name, &m_) < 0) {
		return nullptr;
	}
	if (m_ != nullptr and ALIFMODULE_CHECK(m_)) {
		return m_;
	}
	ALIF_XDECREF(m_);
	m_ = alifModule_newObject(_name);
	if (m_ == nullptr)
		return nullptr;
	if (alifObject_setItem(modules_, _name, m_) != 0) {
		ALIF_DECREF(m_);
		return nullptr;
	}

	return m_;
}

AlifObject* alifImport_addModuleRef(const wchar_t* _name)
{
	AlifObject* nameObj = alifUStr_fromString(_name);
	if (nameObj == nullptr) {
		return nullptr;
	}
	AlifThread* tstate = alifThread_get();
	AlifObject* module_ = import_addModule(tstate, nameObj);
	ALIF_DECREF(nameObj);
	return module_;
}


AlifIntT alifImport_init() {

	if (INITTABLE != nullptr) {
		// error
	}

	AlifIntT status = 1;

	if (initBuildin_modulesTable() != 0) {
		status = -1;
		return status;
	}

	return status;
}
