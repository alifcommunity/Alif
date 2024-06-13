#include "alif.h"
#include "OpCode.h"

#include "AlifCore_Code.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpCodeData.h"
#include "AlifCore_OpCodeUtils.h"
#include "AlifCore_AlifState.h"





























static void init_code(AlifCodeObject* _co, AlifCodeConstructor* _cons) { // 450

	//AlifIntT nLocalsPlus = ALIFTYPLE_GET_SIZE(_cons->localsPlusNames);
	//AlifIntT nLocals{}, nCellVars{}, nFreeVars{};
	//getLocalPlus_counts(_cons->localsPlusNames, _cons->localsPlusKinds, &nLocals, &nCellVars, &nFreeVars);
	if (_cons->stacksize == 0) {
		_cons->stacksize = 1;
	}

	_co->fileName = ALIF_NEWREF(_cons->fileName);
	_co->name = ALIF_NEWREF(_cons->name);
	_co->qualName = ALIF_NEWREF(_cons->qualName);
	


	_co->firstLineNo = _cons->firstlineno;
	_co->lineTable = ALIF_NEWREF(_cons->lineTable);

	_co->consts = ALIF_NEWREF(_cons->consts);
	_co->names = ALIF_NEWREF(_cons->names);
	
	/* code here */
}

static AlifIntT internCode_constants(AlifCodeConstructor* _cons) { // 614
	
	//if (intern_string(_cons->names) < 0) goto error;
	//if (intern_constants(_cons->consts, nullptr) < 0) goto error;
	//if (intern_string(_cons->localsPlusNames) < 0) goto error;
	
	return 0;

error:
	return -1;
}


AlifCodeObject* alifCode_new(AlifCodeConstructor* _cons) { // 645

	if (internCode_constants(_cons) < 0) return nullptr;

	AlifObject* replacementLocations = nullptr;

	AlifSizeT size = ALIFWBYTES_GET_SIZE(_cons->code) / sizeof(AlifCodeUnit);
	AlifCodeObject* co = ALIFOBJECT_NEW_VAR(AlifCodeObject, &_alifCodeType_, size);
	if (co == nullptr) {
		ALIF_XDECREF(replacementLocations);
		// memory error
		return nullptr;
	}
	init_code(co, _cons);

	ALIF_XDECREF(replacementLocations);
	return co;
}











AlifTypeObject _alifCodeType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	L"شفرة",
	offsetof(AlifCodeObject, codeAdaptive),
	sizeof(AlifCodeUnit),






};