#pragma once






















/* ----------------------------------------------------------------------------------------------------------- */



class AlifThread { // 59
public:
	AlifThread* prev{};
	AlifThread* next{};
	AlifInterpreter* interpreter{};

	class {
	public:
		AlifUIntT initialized : 1;

		AlifUIntT bound : 1;
		AlifUIntT active : 1;

		AlifUIntT finalizing : 1;
		AlifUIntT cleared : 1;
		AlifUIntT finalized : 1;

		/* padding to align to 4 bytes */
		AlifUIntT : 2;
	} status{};

	AlifIntT state{};

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
	uint64_t dictGlobalVersion;

};





extern void alifThread_detach(AlifThread*); // 157





#define ALIFCPP_RECURSION_LIMIT 3000 // 214


