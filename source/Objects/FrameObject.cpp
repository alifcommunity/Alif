#include "alif.h"

#include "AlifCore_AlifEval.h"
#include "AlifCore_Code.h"
#include "AlifCore_Function.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpCodeData.h"
#include "AlifCore_Frame.h"

//#include "FrameObject.h"
#include "OpCode.h"


























AlifObject* alifEval_builtinsFromGlobals(AlifThread* _thread, AlifObject* _globals) { 
	//AlifObject* builtins = alifDict_getItemWithError(_globals, &ALIF_ID(__builtins__));
	AlifObject* name = alifUStr_decodeStringToUTF8(L"__builtins__");
	AlifObject* builtins = alifDict_getItem(_globals, name);
	if (builtins) {
		if (ALIFMODULE_CHECK(builtins)) {
			builtins = alifModule_getDict(builtins);
		}
		return builtins;
	}
	//if (alifErr_occurred()) {
	//	return nullptr;
	//}

	return alifEval_getBuiltins(_thread);
}
