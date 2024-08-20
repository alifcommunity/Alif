#pragma once






















/* ----------------------------------------------------------------------------------------------------------- */




class AlifThread { // 59
public:
	AlifThread* prev{};
	AlifThread* next{};
	class AlifInterpreter* interpreter{};

	class {
	public:
		AlifUIntT initialized : 1;

		AlifUIntT bound : 1;
		AlifUIntT active : 1;

		AlifUIntT finalizing : 1;
		AlifUIntT cleared : 1;
		AlifUIntT finalized : 1;

		/* padding to align to 4 bytes */
		AlifUIntT : 26;
	} status;

	class AlifInterpreterFrame* currentFrame{};

	AlifIntT alifRecursionRemaining{};
	AlifIntT alifRecursionLimit{};

	AlifIntT cppRecursionRemaining{};
	AlifIntT recursionHeadroom{}; /* Allow 50 more calls to handle any errors. */

	AlifIntT tracing{};

	AlifUSizeT threadID;

	AlifSizeT id{};

	//AlifStackChunk* dataStackChunk{};
	//AlifObject** dataStackTop{};
	//AlifObject** dataStackLimit{};
};




#define ALIFCPP_RECURSION_LIMIT 3000 // 214


