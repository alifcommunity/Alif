#pragma once





class AlifStructSequenceField { // 10
public:
	const char* name{};
	const char* doc{};
};

class AlifStructSequenceDesc { // 15
public:
	const char* name{};
	const char* doc{};
	AlifStructSequenceField* fields{};
	AlifIntT nInSequence{};
};





AlifObject* alifStructSequence_new(AlifTypeObject*); // 32

void alifStructSequence_setItem(AlifObject*, AlifSizeT, AlifObject*); // 34
AlifObject* alifStructSequence_getItem(AlifObject*, AlifSizeT);



typedef AlifTupleObject AlifStructSequence; // 38
#define ALIFSTRUCTSEQUENCE_SET_ITEM alifStructSequence_setItem
#define ALIFSTRUCTSEQUENCE_GET_ITEM alifStructSequence_getItem
