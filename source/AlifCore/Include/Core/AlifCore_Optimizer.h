#pragma once





class AlifUOpInstruction { // 49
public:
	uint16_t opcode : 15;
	uint16_t format : 1;
	uint16_t oparg{};
	union {
		uint32_t target;
		class {
		public:
			uint16_t jumpTarget;
			uint16_t errorTarget;
		};
	};
	uint64_t operand{};  // A cache entry
};


class AlifExitData { // 63
public:
	uint32_t target{};
	AlifBackoffCounter temperature{};
	const class AlifExecutorObject* executor{};
};

class AlifExecutorObject { // 69
public:
	ALIFOBJECT_VAR_HEAD;
	const AlifUOpInstruction* trace{};
	//AlifVMData data{}; /* Used by the VM, but opaque to the optimizer */
	uint32_t exitCount{};
	uint32_t codeSize{};
	AlifUSizeT size{};
	void* code{};
	void* sideEntry{};
	AlifExitData exits[1];
};
