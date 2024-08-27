#include "alif.h"

#include "AlifCore_State.h"





void alifEval_acquireThread(AlifThread* _thread) { // 591
	ALIF_ENSURETHREADNOTNULL(_thread);
	alifThread_attach(_thread);
}


void alifEval_releaseThread(AlifThread* _thread) { // 598
	alifThread_detach(_thread);
}
