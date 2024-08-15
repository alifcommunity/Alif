#pragma once






















/* ----------------------------------------------------------------------------------------------------------- */




class AlifThread { // 59
public:
	AlifThread* prev{};
	AlifThread* next{};
	class AlifInterpreter* interpreter{};


	class AlifInterpreterFrame* currentFrame{};

	AlifIntT alifRecursionRemaining{};
	AlifIntT alifRecursionLimit{};

	AlifIntT cppRecursionRemaining{};
	AlifIntT recursionHeadroom; /* Allow 50 more calls to handle any errors. */

	AlifIntT tracing{};
	AlifSizeT id{};

	//AlifStackChunk* dataStackChunk{};
	//AlifObject** dataStackTop{};
	//AlifObject** dataStackLimit{};
};




#define ALIFCPP_RECURSION_LIMIT 3000 // 214


