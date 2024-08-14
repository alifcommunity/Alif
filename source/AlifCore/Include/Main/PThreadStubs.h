#pragma once





#ifndef POSIX_THREADS // 8
#  define POSIX_THREADS 1
#endif






typedef class { unsigned __attr; } PThreadCondAttrT; // 50
typedef unsigned PThreadKeyT; // 53







#ifndef PTHREAD_KEYS_MAX // 95
#  define PTHREAD_KEYS_MAX 128
#endif










AlifIntT alifThreadTSS_create(AlifTssT* _key) { // 935
	/* If the key has been created, function is silently skipped. */
	if (_key->isInitialized) {
		return 0;
	}

	AlifIntT fail = pthreadKey_create(&(key->_key), nullptr);
	if (fail) {
		return -1;
	}
	_key->isInitialized = 1;
	return 0;
}





void alifThreadTSS_delete(AlifTssT* _key) { // 952
	/* If the key has not been created, function is silently skipped. */
	if (!_key->isInitialized) {
		return;
	}

	pthread_key_delete(_key->key);
	/* pthread has not provided the defined invalid value for the key. */
	_key->isInitialized = 0;
}
