#include "alif.h"

#ifndef PLATFORM
#define PLATFORM "غير_معروف"
#endif // PLATFORM



const char* alif_getPlatform(void) {
	return PLATFORM;
}
