#pragma once


class AlifExecutorLinkListNode {
public:
	AlifExecutorObject* next{};
	AlifExecutorObject* previous{};
};

#define ALIF_BLOOM_FILTER_WORDS 8

class AlifBloomFilter { // 25
public:
	uint32_t bits[ALIF_BLOOM_FILTER_WORDS];
};

class AlifVMData { // 29
public:
	uint8_t opcode;
	uint8_t oparg;
	uint8_t valid : 1;
	uint8_t linked : 1;
	uint8_t chain_depth : 6;  // Must be big enough for MAX_CHAIN_DEPTH - 1.
	bool warm;
	AlifIntT index;           // Index of ENTER_EXECUTOR (if code isn't NULL, below).
	AlifBloomFilter bloom;
	AlifExecutorLinkListNode links;
	AlifCodeObject* code;  // Weak (NULL if no corresponding ENTER_EXECUTOR).
};

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
	AlifVMData data{}; /* Used by the VM, but opaque to the optimizer */
	uint32_t exitCount{};
	uint32_t codeSize{};
	AlifUSizeT size{};
	void* code{};
	void* sideEntry{};
	AlifExitData exits[1];
};
