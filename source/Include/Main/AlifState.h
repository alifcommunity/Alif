#pragma once


class AlifStackChunk {
public:
	AlifStackChunk* previous;
	AlifUSizeT size;
	AlifUSizeT top;
	AlifObject* data[1]; /* Variable sized */
};


class AlifThread {
public:
	AlifThread* prev{};
	AlifThread* next{};
	class AlifInterpreter* interpreter{};

	AlifIntT tracing{};

	class AlifInterpreterFrame* currentFrame{};

	AlifIntT recursionRemaining{};
	AlifIntT recursionHeadroom{};

	AlifSizeT id{};

	AlifStackChunk* dataStackChunk{};
	AlifObject** dataStackTop{};
	AlifObject** dataStackLimit{};
};




#define ALIFCPP_RECURSION_LIMIT 10000



typedef AlifObject* (*AlifFrameEvalFunction)(AlifThread*, AlifInterpreterFrame*, AlifIntT);
