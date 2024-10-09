#include "alif.h"

//#include "AlifCore_Call.h"
#include "AlifCore_DureRun.h"





char* alifUniversal_newLineFGetsWithSize(char* _buf,
	AlifIntT _n, FILE* _stream, AlifObject* _fObj,  AlifUSizeT* _size) { // 228
	char* p = _buf;
	AlifIntT c_{};

	if (_fObj) {
		errno = ENXIO;          /* What can you do... */
		return nullptr;
	}

	while (--_n > 0 and (c_ = getc(_stream)) != EOF) {
		if (c_ == '\r') {
			c_ = getc(_stream);
			if (c_ != '\n') {
				ungetc(c_, _stream);
				c_ = '\n';
			}
		}
		*p++ = c_;
		if (c_ == '\n') {
			break;
		}
	}

	*p = '\0';
	if (p == _buf) {
		return nullptr;
	}
	*_size = p - _buf;
	return _buf;

}
