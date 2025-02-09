#include "alif.h"

#include "AlifCore_Time.h"
#include "AlifCore_ModuleObject.h"



class TimeModuleState { // 77
public:
	AlifTypeObject* structTimeType{};
};


static inline TimeModuleState* get_timeState(AlifObject* module) { // 90
	void* state = _alifModule_getState(module);
	return (TimeModuleState*)state;
}


static AlifObject* time_sleep(AlifObject* self, AlifObject* timeout_obj) { // 391
	//if (alifSys_audit("time.sleep", "O", timeout_obj) < 0) {
	//	return nullptr;
	//}

	AlifTimeT timeout{};
	if (_alifTime_fromSecondsObject(&timeout, timeout_obj, AlifTimeRoundT::AlifTime_Round_TIMEOUT))
		return nullptr;
	if (timeout < 0) {
		//alifErr_setString(_alifExcValueError_,
		//	"sleep length must be non-negative");
		return nullptr;
	}
	if (alif_sleep(timeout) != 0) {
		return nullptr;
	}

	return ALIF_NONE;
}



static AlifStructSequenceField _structTimeTypeFields_[] = { // 418
	{0}
};

static AlifStructSequenceDesc _structTimeTypeDesc_ = { // 433
	"time.struct_time",
	"",
	_structTimeTypeFields_,
	0,
};











static AlifIntT time_exec(AlifObject* module) { // 1942
	TimeModuleState* state = get_timeState(module);
	// struct_time type
	state->structTimeType = alifStructSequence_newType(&_structTimeTypeDesc_);
	if (state->structTimeType == nullptr) {
		return -1;
	}
	if (alifModule_addType(module, state->structTimeType)) {
		return -1;
	}


	return 0;
}

static AlifMethodDef _timeMethods_[] = {
	{"غفوة", time_sleep, METHOD_O},
	{nullptr, nullptr}           /* sentinel */
};

static AlifModuleDefSlot _timeSlots_[] = {
	{ALIF_MOD_EXEC, time_exec},
	{ALIF_MOD_MULTIPLE_INTERPRETERS, ALIF_MOD_PER_INTERPRETER_GIL_SUPPORTED},
	{ALIF_MOD_GIL, ALIF_MOD_GIL_NOT_USED},
	{0, nullptr}
};

static AlifModuleDef _timeModule_ = {
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "الزمن",
	//.size = sizeof(TimeModuleState),
	.methods = _timeMethods_,
	.slots = _timeSlots_,
	//.traverse = time_module_traverse,
	//.clear = time_module_clear,
	//.free = time_module_free,
};



AlifObject* alifInit_time(void) {
	return alifModuleDef_init(&_timeModule_);
}
