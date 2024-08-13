#pragma once

#include "AlifCore_AlifThread.h"



























AlifThreadIdentT alifThread_getThreadID() {
	volatile pthread_t threadID{};

	//if (!initialized)
	//	alifThread_initThread();

	threadID = pthread_self();
	return (AlifThreadIdentT)threadID;
}
