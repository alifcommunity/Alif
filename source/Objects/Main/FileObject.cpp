#include "alif.h"
#include "AlifCore_Call.h"
//#include "AlifCore_runtime.h"



// هذا الملف لا يحتوي AlifCore_FileObject.h

wchar_t* alifUniversal_newLineFGetsWithSize(wchar_t* _buf, int _n, FILE* _stream, AlifSizeT* _size) {
	wchar_t* p = _buf;
	int c{};
	while (--_n > 0 and (c = getwc(_stream)) != WEOF) {
		if (c == L'\r') {
			c = getwc(_stream);
			if (c != L'\n') {
				ungetwc(c, _stream);
				c = L'\n';
			}
		}
		*p++ = c;
		if (c == L'\n') {
			break;
		}
	}

	*p = L'\0';
	if (p == _buf) {
		return nullptr;
	}
	*_size = p - _buf;
	return _buf;

}