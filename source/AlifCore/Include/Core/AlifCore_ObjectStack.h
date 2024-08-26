#pragma once

















#define ALIFOBJECT_STACK_CHUNK_SIZE 254 // 16

class AlifObjectStackChunk { // 18
public:
	AlifObjectStackChunk* prev{};
	AlifSizeT n_{};
	AlifObject* objs[ALIFOBJECT_STACK_CHUNK_SIZE];
};

class AlifObjectStack { // 24
public:
	AlifObjectStackChunk* head{};
};
