#include "alif.h"

#include "AlifCore_State.h"




#define HANDLERS _alifDureRun_.signals.handlers // 105


typedef SignalsDureRunState SignalStateT; // 110
#define SIGNAL_GLOBAL_STATE _alifDureRun_.signals















static AlifIntT signal_installHandlers(void) { // 1909
#ifdef SIGPIPE
	alifOS_setSig(SIGPIPE, SIG_IGN);
#endif
#ifdef SIGXFZ
	alifOS_setSig(SIGXFZ, SIG_IGN);
#endif
#ifdef SIGXFSZ
	alifOS_setSig(SIGXFSZ, SIG_IGN);
#endif

	//AlifObject* module = alifImport_importModule("_signal");
	//if (!module) {
	//	return -1;
	//}
	//ALIF_DECREF(module);

	return 0;
}






AlifIntT _alifSignal_init(AlifIntT _installSignalHandlers) { // 1956
	SignalStateT* state = &SIGNAL_GLOBAL_STATE;

	state->defaultHandler = alifLong_fromVoidPtr((void*)SIG_DFL);
	if (state->defaultHandler == nullptr) {
		return -1;
	}

	state->ignoreHandler = alifLong_fromVoidPtr((void*)SIG_IGN);
	if (state->ignoreHandler == nullptr) {
		return -1;
	}

#ifdef _WINDOWS
	/* Create manual-reset event, initially unset */
	state->sigintEvent = (void*)CreateEvent(nullptr, TRUE, FALSE, FALSE);
	if (state->sigintEvent == nullptr) {
		//alifErr_setFromWindowsErr(0);
		return -1;
	}
#endif

	for (AlifIntT signum = 1; signum < ALIF_NSIG; signum++) {
		alifAtomic_storeIntRelaxed(&HANDLERS[signum].tripped, 0);
	}

	if (_installSignalHandlers) {
		if (signal_installHandlers() < 0) {
			return -1;
		}
	}

	return 0;
}









AlifIntT _alifOS_isMainThread(void) { // 2046
	AlifInterpreter* interp = _alifInterpreter_get();
	return alif_threadCanHandleSignals(interp);
}


#ifdef _WINDOWS
void* _alifOS_sigintEvent() { // 2059
	SignalStateT* state = &SIGNAL_GLOBAL_STATE;
	return state->sigintEvent;
}
#endif
