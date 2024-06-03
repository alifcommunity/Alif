#pragma once

#include "AlifCore_DureRun.h"



#if defined(HAVE_LOCAL_THREAD)
extern ALIF_LOCAL_THREAD AlifThread* _alifTSSThread_;
#endif


static inline AlifThread* alifThread_get() {
#ifdef HAVE_LOCAL_THREAD
	return _alifTSSThread_;
#endif // HAVE_LOCAL_THREAD
}

extern void alifThread_attach(AlifThread*);

static inline AlifInterpreter* alifInterpreter_get() {
	AlifThread* thread_ = alifThread_get();

	return thread_->interpreter;
	//return nullptr;
}


extern AlifIntT alifInterpreter_enable(AlifDureRun*);


const AlifConfig* alifConfig_get();
