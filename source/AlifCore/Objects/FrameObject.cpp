#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Code.h"
#include "AlifCore_Function.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpcodeMetaData.h"

#include "FrameObject.h"
#include "AlifCore_Frame.h"
#include "Opcode.h"
























AlifObject* _alifEval_builtinsFromGlobals(AlifThread* _thread, AlifObject* _globals) { // 2131
	AlifObject* builtins = alifDict_getItemWithError(_globals, &ALIF_ID(__builtins__));
	if (builtins) {
		if (ALIFMODULE_CHECK(builtins)) {
			builtins = alifModule_getDict(builtins);
		}
		return builtins;
	}
	//if (alifErr_occurred()) {
	//	return nullptr;
	//}

	return _alifEval_getBuiltins(_thread);
}

