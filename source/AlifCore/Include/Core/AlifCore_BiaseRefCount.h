#pragma once

#include "AlifCore_LList.h"
#include "AlifCore_ObjectStack.h"


#define ALIF_BRC_NUM_BUCKETS 257 // 20

class BRCBucket { // 23
public:
	AlifMutex mutex{};
	LListNode root{};
};

class BRCState { // 32
public:
	BRCBucket table[ALIF_BRC_NUM_BUCKETS]{};
};

class BRCThreadState { // 39
public:
	LListNode bucketNode{};
	uintptr_t threadID{};
	AlifObjectStack objectsToMerge{};
	AlifObjectStack localObjectsToMerge{};
};



void alifBRC_initThread(AlifThread*); // 54


void alifBRC_initState(AlifInterpreter*); // 58

void alifBRC_queueObject(AlifObject*); // 64
