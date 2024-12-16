#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Call.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_Frame.h"
#include "AlifCore_Interpreter.h"

#include "AlifCore_OpcodeMetaData.h"



 // 41
#define LOCK_CODE(_code)                                             \
    ALIF_BEGIN_CRITICAL_SECTION(_code)

#define UNLOCK_CODE()   ALIF_END_CRITICAL_SECTION()






static AlifIntT instrument_lockHeld(AlifCodeObject* _code, AlifInterpreter* _interp) { // 1852
	//if (isVersion_upToDate(_code, _interp)) {
	//	return 0;
	//}

	//return forceInstrument_lockHeld(_code, _interp);
	return 1; // alif
}

AlifIntT _alif_instrument(AlifCodeObject* _code, AlifInterpreter* _interp) { // 1868
	AlifIntT res{};
	LOCK_CODE(_code);
	res = instrument_lockHeld(_code, _interp);
	UNLOCK_CODE();
	return res;
}
