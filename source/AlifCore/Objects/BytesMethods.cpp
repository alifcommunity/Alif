#include "alif.h"
#include "AlifCore_Abstract.h"





















void _alifBytes_lower(char* _result,
	const char* _cptr, AlifSizeT _len) { // 250
	AlifSizeT i{};

	for (i = 0; i < _len; i++) {
		_result[i] = ALIF_TOLOWER((unsigned char)_cptr[i]);
	}
}


void _alifBytes_upper(char* _result,
	const char* _cptr, AlifSizeT _len) {
	AlifSizeT i;

	for (i = 0; i < _len; i++) {
		_result[i] = ALIF_TOUPPER((unsigned char)_cptr[i]);
	}
}
