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











static AlifIntT initBuildin_modulesTable() {

	AlifUSizeT size{};

	for (size = 0; alifImportInitTable[size].name != nullptr; size++)
		;
	size++;

	InitTable* tableCopy = (InitTable*)alifMem_dataAlloc(size * sizeof(InitTable));
	if (tableCopy == nullptr) return -1;

	memcpy(tableCopy, alifImportInitTable, size * sizeof(InitTable));
	INITTABLE = tableCopy;
	return 0;
}



static AlifObject* importAdd_module(AlifThread* tstate, AlifObject* name)
{
	AlifObject* modules = tstate->interpreter->imports.modules_;
	if (modules == NULL) {
		return NULL;
	}

	AlifObject* m;
	if (alifMapping_getOptionalItem(modules, name, &m) < 0) {
		return NULL;
	}
	if (m != NULL && (m)) {
		return m;
	}
	ALIF_XDECREF(m);
	m = alifModule_newObject(name);
	if (m == NULL)
		return NULL;
	//if (alifObject_setItem(modules, name, m) != 0) {
		//ALIF_DECREF(m);
		//return NULL;
	//}

	return m;
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
