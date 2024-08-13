#pragma once






















/* ----------------------------------------------------------------------------------------------------------- */




class AlifThread { // 59
public:
	AlifThread* prev{};
	AlifThread* next{};
	class AlifInterpreter* interpreter{};

	AlifIntT tracing{};

	class AlifInterpreterFrame* currentFrame{};

	AlifIntT recursionRemaining{};
	AlifIntT recursionHeadroom{};

	AlifSizeT id{};

	//AlifStackChunk* dataStackChunk{};
	//AlifObject** dataStackTop{};
	//AlifObject** dataStackLimit{};
};




#define ALIFCPP_RECURSION_LIMIT 10000 // 214


