#pragma once














#define HAS_JUMP_FLAG (8) // 961






#define OPCODE_HAS_JUMP(_op) (_alifOpcodeOpcodeMetadata[_op].flags & (HAS_JUMP_FLAG)) // 976









struct OpcodeMetadata {
	uint8_t validEntry{};
	int8_t instrFormat{};
	int16_t flags{};
};

extern const OpcodeMetadata _alifOpcodeOpcodeMetadata[264]; // 1004
