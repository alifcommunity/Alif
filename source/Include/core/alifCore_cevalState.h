#pragma once










#include "alifCore_atomic.h"
#include "alifCore_gil.h"


class PendingCalls {
public:
	int busy;
	AlifThreadTypeLock lock;
	AlifAtomicInt callsToDo;
	int asyncExc;
#define NPENDINGCALLS 32
	struct PendingCall {
		int (*func)(void*);
		void* arg;
	} calls[NPENDINGCALLS];
	int first;
	int last;
};























































class CevalState {
public:
	AlifAtomicInt evalBreaker;
	AlifAtomicInt gilDropRequest;
	int recursionLimit;
	GilRuntimeState* gil;
	int ownGil;
	AlifAtomicInt gcScheduled;
	PendingCalls pending;
};
