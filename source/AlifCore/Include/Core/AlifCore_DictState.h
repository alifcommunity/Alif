#pragma once





// 11
#define DICT_MAX_WATCHERS 8
#define DICT_WATCHED_MUTATION_BITS 4


class AlifDictState { // 14
public:
	uint32_t nextKeysVersion{};
	AlifDictWatchCallback watchers[DICT_MAX_WATCHERS]{};
};

#define DICT_STATE_INIT \
    { \
        .nextKeysVersion = 2, \
    }
