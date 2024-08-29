#pragma once


//#include "AlifCore_Interpreter.h"
//#include "AlifCore_State.h"













#define ALIF_DEFAULT_RECURSION_LIMIT 1000 // 40















extern void alifEval_acquireLock(AlifThread*); // 134







static inline AlifIntT alifEval_isGILEnabled(AlifThread* _thread) { // 145
	GILDureRunState* gil = _thread->interpreter->eval.gil_;
	return alifAtomic_loadIntRelaxed(&gil->enabled) != 0;
}
