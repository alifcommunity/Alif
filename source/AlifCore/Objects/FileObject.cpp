#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Runtime.h"



AlifObject* alifFile_getLine(AlifObject* _f, AlifIntT _n) { // 51
	AlifObject* result{};

	if (_f == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	if (_n <= 0) {
		result = alifObject_callMethodNoArgs(_f, &ALIF_STR(ReadLine));
	}
	else {
		result = _alifObject_callMethod(_f, &ALIF_STR(ReadLine), "i", _n);
	}
	if (result != nullptr and !ALIFBYTES_CHECK(result) and
		!ALIFUSTR_CHECK(result)) {
		ALIF_SETREF(result, nullptr);
		alifErr_setString(_alifExcTypeError_,
			"object.readline() returned non-string");
	}

	if (_n < 0 and result != nullptr and ALIFBYTES_CHECK(result)) {
		const char* s = ALIFBYTES_AS_STRING(result);
		AlifSizeT len = ALIFBYTES_GET_SIZE(result);
		if (len == 0) {
			ALIF_SETREF(result, nullptr);
			//alifErr_setString(_alifExcEOFError_,
			//	"EOF when reading a line");
		}
		else if (s[len - 1] == '\n') {
			(void)_alifBytes_resize(&result, len - 1);
		}
	}
	if (_n < 0 and result != nullptr and ALIFUSTR_CHECK(result)) {
		AlifSizeT len = ALIFUSTR_GET_LENGTH(result);
		if (len == 0) {
			ALIF_SETREF(result, nullptr);
			//alifErr_setString(_alifExcEOFError_,
			//	"EOF when reading a line");
		}
		else if (ALIFUSTR_READ_CHAR(result, len - 1) == '\n') {
			AlifObject* v{};
			v = alifUStr_subString(result, 0, len - 1);
			ALIF_SETREF(result, v);
		}
	}
	return result;
}


AlifIntT alifFile_writeObject(AlifObject* _v, AlifObject* _f, AlifIntT _flags) { // 104
	AlifObject* writer{}, * value{}, * result{};

	if (_f == nullptr) {
		alifErr_setString(_alifExcTypeError_, "writeobject with nullptr file");
		return -1;
	}
	writer = alifObject_getAttr(_f, &ALIF_STR(Write));
	if (writer == nullptr)
		return -1;
	if (_flags & ALIF_PRINT_RAW) {
		value = alifObject_str(_v);
	}
	else
		value = alifObject_repr(_v);
	if (value == nullptr) {
		ALIF_DECREF(writer);
		return -1;
	}
	result = alifObject_callOneArg(writer, value);
	ALIF_DECREF(value);
	ALIF_DECREF(writer);
	if (result == nullptr)
		return -1;
	ALIF_DECREF(result);
	return 0;
}


AlifIntT alifFile_writeString(const char* _s, AlifObject* _f) { // 134
	if (_f == nullptr) {
		/* Should be caused by a pre-existing error */
		if (!alifErr_occurred())
			alifErr_setString(_alifExcSystemError_,
				"null file for alifFile_writeString");
		return -1;
	}
	else if (!alifErr_occurred()) {
		AlifObject* v = alifUStr_fromString(_s);
		AlifIntT err{};
		if (v == nullptr)
			return -1;
		err = alifFile_writeObject(v, _f, ALIF_PRINT_RAW);
		ALIF_DECREF(v);
		return err;
	}
	else
		return -1;
}



char* alifUniversal_newLineFGetsWithSize(char* _buf,
	AlifIntT _n, FILE* _stream, AlifObject* _fObj, AlifUSizeT* _size) { // 228
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




AlifIntT _alifFile_flush(AlifObject* _file) { // 533
	AlifObject* tmp = alifObject_callMethodNoArgs(_file, &ALIF_ID(Flush));
	if (tmp == nullptr) {
		return -1;
	}
	ALIF_DECREF(tmp);
	return 0;
}
