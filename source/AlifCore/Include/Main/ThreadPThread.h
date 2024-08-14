#pragma once


#include "AlifCore_Thread.h"

























AlifUSizeT alifThread_getThreadID() { // 365
	volatile pthread_t threadID{};

	if (!INITIALIZED) INITIALIZED = 1;

	threadID = pthread_self();
	return (AlifUSizeT)threadID;
}








