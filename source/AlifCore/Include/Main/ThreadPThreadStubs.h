#pragma once

#include "PThreadStubs.h"












#define ALIF_TLS_ENTRIES (_alifDureRun_.threads.stubs.tlsEntries) // 139


AlifIntT pthread_key_create(pthread_key_t* _key, void (*destr_function)(void*)) { // 141
	if (!_key) {
		return EINVAL;
	}
	if (destr_function != nullptr) {
		// destructor is not supported
		return -1;
	}
	for (pthread_key_t idx = 0; idx < PTHREAD_KEYS_MAX; idx++) {
		if (!ALIF_TLS_ENTRIES[idx].inUse) {
			ALIF_TLS_ENTRIES[idx].inUse = true;
			*_key = idx;
			return 0;
		}
	}
	return EAGAIN;
}

AlifIntT pthread_key_delete(pthread_key_t _key) { // 160
	if (_key < 0 or _key >= PTHREAD_KEYS_MAX or !ALIF_TLS_ENTRIES[_key].inUse) {
		return EINVAL;
	}
	ALIF_TLS_ENTRIES[_key].inUse = false;
	ALIF_TLS_ENTRIES[_key].value = nullptr;
	return 0;
}
