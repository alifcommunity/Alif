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












typedef AlifTupleObject AlifStructSequence; // 38
