#pragma once




class AlifBackoffCounter { // 17
public:
	uint16_t valueAndBackoff{};
};



#define BACKOFF_BITS 4 // 40
#define MAX_BACKOFF 12
#define UNREACHABLE_BACKOFF 15

static inline AlifBackoffCounter make_backoffCounter(uint16_t value,
	uint16_t backoff) { // 50
	AlifBackoffCounter result{};
	result.valueAndBackoff = (value << BACKOFF_BITS) | backoff;
	return result;
}

static inline AlifBackoffCounter forge_backoffCounter(uint16_t _counter) { // 60
	AlifBackoffCounter result{};
	result.valueAndBackoff = _counter;
	return result;
}


static inline AlifBackoffCounter restart_backoffCounter(AlifBackoffCounter _counter) { // 68
	AlifIntT backoff = _counter.valueAndBackoff & 15;
	if (backoff < MAX_BACKOFF) {
		return make_backoffCounter((1 << (backoff + 1)) - 1, backoff + 1);
	}
	else {
		return make_backoffCounter((1 << MAX_BACKOFF) - 1, MAX_BACKOFF);
	}
}




static inline AlifBackoffCounter advance_backoffCounter(AlifBackoffCounter _counter) { // 89
	AlifBackoffCounter result{};
	result.valueAndBackoff = _counter.valueAndBackoff - (1 << BACKOFF_BITS);
	return result;
}



static inline bool backoff_counterTriggers(AlifBackoffCounter _counter) { // 97
	return _counter.valueAndBackoff < UNREACHABLE_BACKOFF;
}




/* Initial JUMP_BACKWARD counter.
* This determines when we create a trace for a loop.
* Backoff sequence 16, 32, 64, 128, 256, 512, 1024, 2048, 4096. */
#define JUMP_BACKWARD_INITIAL_VALUE 4095
#define JUMP_BACKWARD_INITIAL_BACKOFF 12
static inline AlifBackoffCounter initial_jumpBackoffCounter(void) { // 109
	return make_backoffCounter(JUMP_BACKWARD_INITIAL_VALUE,
		JUMP_BACKWARD_INITIAL_BACKOFF);
}



static inline AlifBackoffCounter initial_unreachableBackoffCounter(void) { // 132
	return make_backoffCounter(0, UNREACHABLE_BACKOFF);
}
