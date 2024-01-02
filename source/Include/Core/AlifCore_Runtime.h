#pragma once


class AlifRuntime {
public:
	short selfInitializing{};
	short selfInitialized{};

	short configInitializing{};
	short configInitialized{};

	uint32_t mainThread{};

	AlifWStringList origArgv{};

	AlifConfig config{};
};


void alifRuntime_init();
