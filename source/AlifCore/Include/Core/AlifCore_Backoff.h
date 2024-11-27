#pragma once




class AlifBackoffCounter {
public:
	union {
		class {
		public:
			uint16_t backoff : 4;
			uint16_t value : 12;
		};
		uint16_t asCounter{};  // For printf("%#x", ...)
	};
};
