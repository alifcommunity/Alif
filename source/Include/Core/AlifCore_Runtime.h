#pragma once


class AlifRuntime {
public:
	short selfInitializing{};
	short selfInitialized{};

	short configInitializing{};
	short configInitialized{};

	uint32_t mainThread{};
};

