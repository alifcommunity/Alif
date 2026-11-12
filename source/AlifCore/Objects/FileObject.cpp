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
				"ملف فارغ في alifFile_writeString");
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

AlifIntT alifObject_asFileDescriptor(AlifObject* _o) { // 164
	AlifIntT fd{};
	AlifObject *meth{};

	if (ALIFLONG_CHECK(_o)) {
		if (ALIFBOOL_CHECK(_o)) {
			//if (alifErr_warnEx(_alifExcRuntimeWarning_,
			//	"نوع منطق تم استخدامه ك واصف ملف", 1))
			//{
			//	return -1;
			//}
		}
		fd = alifLong_asInt(_o);
	}
	else if (alifObject_getOptionalAttr(_o, &ALIF_ID(Fileno), &meth) < 0) {
		return -1;
	}
	else if (meth != nullptr) {
		AlifObject *fno = _alifObject_callNoArgs(meth);
		ALIF_DECREF(meth);
		if (fno == nullptr)
			return -1;

		if (ALIFLONG_CHECK(fno)) {
			fd = alifLong_asInt(fno);
			ALIF_DECREF(fno);
		}
		else {
			alifErr_setString(_alifExcTypeError_,
				"fileno() قام بإرجاع نوع ليس-صحيح");
			ALIF_DECREF(fno);
			return -1;
		}
	}
	else {
		alifErr_setString(_alifExcTypeError_,
			"المعاملات يجب أن تكون نوع صحيح, او تملك دالة fileno().");
		return -1;
	}

	if (fd == -1 and alifErr_occurred())
		return -1;
	if (fd < 0) {
		alifErr_format(_alifExcValueError_,
			"واصف الملف لا يمكن أن يكون عدد صحيح سالب (%i)",
			fd);
		return -1;
	}
	return fd;
}

char* _alifUniversal_newLineFGetsWithSize(char* _buf,
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




class AlifStdPrinterObject { // 282
public:
	ALIFOBJECT_HEAD;
	int fd{};
};

AlifObject* alifFile_newStdPrinter(int fd) { // 287
	AlifStdPrinterObject* self{};

	if (fd != fileno(stdout) and fd != fileno(stderr)) {
		return nullptr;
	}

	self = ALIFOBJECT_NEW(AlifStdPrinterObject,
		&_alifStdPrinterType_);
	if (self != nullptr) {
		self->fd = fd;
	}
	return (AlifObject*)self;
}



static AlifObject* stdPrinter_write(AlifStdPrinterObject* _self,
	AlifObject* _args) { // 305
	AlifObject* unicode{};
	AlifObject* bytes = nullptr;
	const char* str;
	AlifSizeT n{};
	AlifIntT err{};

	if (_self->fd < 0) {
		return ALIF_NONE;
	}

	if (!alifArg_parseTuple(_args, "U", &unicode)) {
		return nullptr;
	}

	/* Encode Unicode to UTF-8/backslashreplace */
	str = alifUStr_asUTF8AndSize(unicode, &n);
	if (str == nullptr) {
		alifErr_clear();
		bytes = _alifUStr_asUTF8String(unicode, "backslashreplace");
		if (bytes == nullptr)
			return nullptr;
		str = ALIFBYTES_AS_STRING(bytes);
		n = ALIFBYTES_GET_SIZE(bytes);
	}

	n = _alif_write(_self->fd, str, n);
	err = errno;

	ALIF_XDECREF(bytes);

	if (n == -1) {
		if (err == EAGAIN) {
			alifErr_clear();
			return ALIF_NONE;
		}
		return NULL;
	}

	return alifLong_fromSizeT(n);
}



static AlifMethodDef _stdPrinterMethods_[] = { // 391
	{"اكتب",           (AlifCPPFunction)stdPrinter_write, METHOD_VARARGS},
	{nullptr,              nullptr}  /*sentinel */
};







AlifTypeObject _alifStdPrinterType_ = { // 425
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "طابع_الخطأ",
	.basicSize = sizeof(AlifStdPrinterObject),
	/* methods */
	//.repr = (ReprFunc)stdPrinter_repr,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_DISALLOW_INSTANTIATION,
	.methods = _stdPrinterMethods_,
	//.getSet = _stdPrinterGetSetList_,
	.alloc = alifType_genericAlloc,
	.free = alifMem_objFree,
};




AlifObject* alifFile_openCodeObject(AlifObject* _path) { // 494
	AlifObject* f = nullptr;

	if (!ALIFUSTR_CHECK(_path)) {
		alifErr_format(_alifExcTypeError_, "'مسار' يجب أن يكون 'نص', وليس '%.200s'",
			ALIF_TYPE(_path)->name);
		return NULL;
	}

	AlifOpenCodeHookFunction hook = _alifRuntime_.openCodeHook;
	if (hook) {
		f = hook(_path, _alifRuntime_.openCodeUserdata);
	}
	else {
		AlifObject* open = _alifImport_getModuleAttrString("تبادل", "افتح");
		if (open) {
			f = alifObject_callFunction(open, "Os", _path, "rb");
			ALIF_DECREF(open);
		}
	}

	return f;
}



AlifIntT _alifFile_flush(AlifObject* _file) { // 533
	AlifObject* tmp = alifObject_callMethodNoArgs(_file, &ALIF_STR(Flush));
	if (tmp == nullptr) {
		return -1;
	}
	ALIF_DECREF(tmp);
	return 0;
}
