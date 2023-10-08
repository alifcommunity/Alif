#pragma once









#include "alifCore_gil.h"


class PendingCalls {
public:
	int busy;
	void* lock; // type should be AlifThreadTypeLock 
	AlifAtomicInt callsToDo;



	int asyncExc;
#define NPENDINGCALLS 32
	class PendingCall {
	public:
		int (*func)(void*);
		void* arg;
	} calls[NPENDINGCALLS];
	int first;
	int last;
};



















































class CEvalState {
public:



	AlifAtomicInt evalBreaker;

	AlifAtomicInt gilDropRequest;
	int recursionLimit;
	class GilRuntimeState* gil;
	int ownGil;

	AlifAtomicInt gcScheduled;
	class PendingCalls pending;
};
