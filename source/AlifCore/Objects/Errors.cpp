#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Errors.h"
#include "AlifCore_State.h"
#include "AlifCore_StructSeq.h"
#include "AlifCore_SysModule.h"
#include "AlifCore_Traceback.h"





void _alifErr_setRaisedException(AlifThread* _thread, AlifObject* _exc) { // 24
	AlifObject* old_exc = _thread->currentException;
	_thread->currentException = _exc;
	ALIF_XDECREF(old_exc);
}


static AlifObject* _alifErr_createException(AlifObject* _exceptionType,
	AlifObject* _value) { // 32
	AlifObject* exc{};

	if (_value == nullptr or _value == ALIF_NONE) {
		exc = _alifObject_callNoArgs(_exceptionType);
	}
	else if (ALIFTUPLE_CHECK(_value)) {
		exc = alifObject_call(_exceptionType, _value, nullptr);
	}
	else {
		exc = alifObject_callOneArg(_exceptionType, _value);
	}

	if (exc != nullptr and !ALIFEXCEPTIONINSTANCE_CHECK(exc)) {
		alifErr_format(nullptr /*_alifExcTypeError_*/,
			"calling %R should have returned an instance of "
			"BaseException, not %s",
			_exceptionType, ALIF_TYPE(exc)->name);
		ALIF_CLEAR(exc);
	}

	return exc;
}


void _alifErr_restore(AlifThread* _thread, AlifObject* _type, AlifObject* _value,
	AlifObject* _traceback) { // 58
	if (_type == nullptr) {
		_alifErr_setRaisedException(_thread, nullptr);
		return;
	}
	if (_value != nullptr and _type == (AlifObject*)ALIF_TYPE(_value)) {
		/* Already normalized */
	}
	else {
		AlifObject* exc = _alifErr_createException(_type, _value);
		ALIF_XDECREF(_value);
		if (exc == nullptr) {
			ALIF_DECREF(_type);
			ALIF_XDECREF(_traceback);
			return;
		}
		_value = exc;
	}

	if (_traceback != nullptr and !ALIFTRACEBACK_CHECK(_traceback)) {
		if (_traceback == ALIF_NONE) {
			ALIF_DECREF(ALIF_NONE);
			_traceback = nullptr;
		}
		else {
			//alifErr_setString(_alifExcTypeError_, "traceback must be a Traceback or None");
			ALIF_XDECREF(_value);
			ALIF_DECREF(_type);
			ALIF_XDECREF(_traceback);
			return;
		}
	}
	AlifObject* old_traceback = ((AlifBaseExceptionObject*)_value)->traceback;
	((AlifBaseExceptionObject*)_value)->traceback = _traceback;
	ALIF_XDECREF(old_traceback);
	_alifErr_setRaisedException(_thread, _value);
	ALIF_DECREF(_type);
}






void alifErr_restore(AlifObject* _type,
	AlifObject* _value, AlifObject* _traceback) { // 104
	AlifThread* tstate = _alifThread_get();
	_alifErr_restore(tstate, _type, _value, _traceback);
}


void alifErr_setRaisedException(AlifObject* _exc) { // 111
	AlifThread* thread = _alifThread_get();
	_alifErr_setRaisedException(thread, _exc);
}


AlifErrStackItem* _alifErr_getTopMostException(AlifThread* _thread) { // 118
	AlifErrStackItem* excInfo = _thread->excInfo;

	while (excInfo->excValue == nullptr
		and excInfo->previousItem != nullptr)
	{
		excInfo = excInfo->previousItem;
	}
	return excInfo;
}



static AlifObject* getNormalization_failureNote(AlifThread* tstate,
	AlifObject* exception, AlifObject* value) { // 132
	AlifObject* args = alifObject_repr(value);
	if (args == nullptr) {
		_alifErr_clear(tstate);
		args = alifUStr_fromFormat("<unknown>");
	}
	AlifObject* note{};
	const char* tpname = ((AlifTypeObject*)exception)->name;
	if (args == nullptr) {
		_alifErr_clear(tstate);
		note = alifUStr_fromFormat("Normalization failed: type=%s", tpname);
	}
	else {
		note = alifUStr_fromFormat("Normalization failed: type=%s args=%S",
			tpname, args);
		ALIF_DECREF(args);
	}
	return note;
}




void _alifErr_setObject(AlifThread* _thread,
	AlifObject* _exception, AlifObject* _value) { // 154
	AlifObject* excValue{};
	AlifObject* tb = nullptr;

	if (_exception != nullptr and
		not ALIFEXCEPTIONCLASS_CHECK(_exception)) {
		//_alifErr_format(_thread, _alifExcSystemError_,
		//	"_alifErr_setObject: "
		//	"exception %R is not a BaseException subclass",
		//	_exception);
		return;
	}
	/* Normalize the exception */
	AlifIntT isSubclass = 0;
	if (_value != nullptr and ALIFEXCEPTIONINSTANCE_CHECK(_value)) {
		isSubclass = alifObject_isSubclass((AlifObject*)ALIF_TYPE(_value), _exception);
		if (isSubclass < 0) {
			return;
		}
	}
	ALIF_XINCREF(_value);
	if (!isSubclass) {
		_alifErr_clear(_thread);

		AlifObject* fixed_value = _alifErr_createException(_exception, _value);
		if (fixed_value == nullptr) {
			AlifObject* exc = _alifErr_getRaisedException(_thread);

			AlifObject* note = getNormalization_failureNote(_thread, _exception, _value);
			ALIF_XDECREF(_value);
			if (note != nullptr) {
				_alifException_addNote(exc, note);
				ALIF_DECREF(note);
			}
			_alifErr_setRaisedException(_thread, exc);
			return;
		}
		ALIF_XSETREF(_value, fixed_value);
	}

	excValue = _alifErr_getTopMostException(_thread)->excValue;
	if (excValue != nullptr and excValue != ALIF_NONE) {
		ALIF_INCREF(excValue);
		if (excValue != _value) {
			AlifObject* o_ = excValue, * context;
			AlifObject* slowObj = o_; 
			AlifIntT slowUpdateToggle = 0;
			while ((context = alifException_getContext(o_))) {
				ALIF_DECREF(context);
				if (context == _value) {
					alifException_setContext(o_, nullptr);
					break;
				}
				o_ = context;
				if (o_ == slowObj) {
					break;
				}
				if (slowUpdateToggle) {
					slowObj = alifException_getContext(slowObj);
					ALIF_DECREF(slowObj);
				}
				slowUpdateToggle = not slowUpdateToggle;
			}
			alifException_setContext(_value, excValue);
		}
		else {
			ALIF_DECREF(excValue);
		}
	}
	if (ALIFEXCEPTIONINSTANCE_CHECK(_value)) {
		tb = alifException_getTraceback(_value);
	}
	_alifErr_restore(_thread, ALIF_NEWREF(ALIF_TYPE(_value)), _value, tb);
}







void alifErr_setObject(AlifObject* _exception, AlifObject* _value) { // 246
	AlifThread* thread = _alifThread_get();
	_alifErr_setObject(thread, _exception, _value);
}


void _alifErr_setString(AlifThread* _thread, AlifObject* _exception,
	const char* _string) { // 285
	AlifObject* value = alifUStr_fromString(_string);
	if (value != nullptr) {
		_alifErr_setObject(_thread, _exception, value);
		ALIF_DECREF(value);
	}
}


void alifErr_setString(AlifObject* _exception, const char* _string) { // 296
	AlifThread* thread = _alifThread_get();
	_alifErr_setString(thread, _exception, _string);
}


AlifObject* ALIF_HOT_FUNCTION alifErr_occurred(void) { // 304
	AlifThread* thread = _alifThread_get();
	return _alifErr_occurred(thread);
}


AlifIntT alifErr_givenExceptionMatches(AlifObject* _err, AlifObject* _exc) { // 315
	if (_err == nullptr or _exc == nullptr) {
		return 0;
	}
	if (ALIFTUPLE_CHECK(_exc)) {
		AlifSizeT i{}, n{};
		n = alifTuple_size(_exc);
		for (i = 0; i < n; i++) {
			/* Test recursively */
			if (alifErr_givenExceptionMatches(
				_err, ALIFTUPLE_GET_ITEM(_exc, i)))
			{
				return 1;
			}
		}
		return 0;
	}
	/* err might be an instance, so check its class. */
	if (ALIFEXCEPTIONINSTANCE_CHECK(_err))
		_err = ALIFEXCEPTIONINSTANCE_CLASS(_err);

	if (ALIFEXCEPTIONCLASS_CHECK(_err) and ALIFEXCEPTIONCLASS_CHECK(_exc)) {
		return alifType_isSubType((AlifTypeObject*)_err, (AlifTypeObject*)_exc);
	}

	return _err == _exc;
}

AlifIntT _alifErr_exceptionMatches(AlifThread* _thread, AlifObject* _exc) { // 347
	return alifErr_givenExceptionMatches(_alifErr_occurred(_thread), _exc);
}


AlifIntT alifErr_exceptionMatches(AlifObject* _exc) { // 354
	AlifThread* thread = _alifThread_get();
	return _alifErr_exceptionMatches(thread, _exc);
}


AlifObject* _alifErr_getRaisedException(AlifThread* _thread) { // 483
	AlifObject* exc = _thread->currentException;
	_thread->currentException = nullptr;
	return exc;
}

AlifObject* alifErr_getRaisedException(void) { // 490
	AlifThread* tstate = _alifThread_get();
	return _alifErr_getRaisedException(tstate);
}


void _alifErr_fetch(AlifThread* tstate, AlifObject** p_type, AlifObject** p_value,
	AlifObject** p_traceback) { // 497
	AlifObject* exc = _alifErr_getRaisedException(tstate);
	*p_value = exc;
	if (exc == nullptr) {
		*p_type = nullptr;
		*p_traceback = nullptr;
	}
	else {
		*p_type = ALIF_NEWREF(ALIF_TYPE(exc));
		*p_traceback = ALIF_XNEWREF(((AlifBaseExceptionObject*)exc)->traceback);
	}
}

void alifErr_fetch(AlifObject** _pType, AlifObject** _pValue,
	AlifObject** _pTraceback) { // 514
	AlifThread* tstate = _alifThread_get();
	_alifErr_fetch(tstate, _pType, _pValue, _pTraceback);
}


void _alifErr_clear(AlifThread* _thread) { // 522
	_alifErr_restore(_thread, nullptr, nullptr, nullptr);
}


void alifErr_clear(void) { // 529
	AlifThread* tstate = _alifThread_get();
	_alifErr_clear(tstate);
}


void _alifErr_setHandledException(AlifThread* _thread, AlifObject* _exc) { // 593
	ALIF_XSETREF(_thread->excInfo->excValue, ALIF_XNEWREF(_exc == ALIF_NONE ? nullptr : _exc));
}

void alifErr_setHandledException(AlifObject* _exc) { // 599
	AlifThread* thread = _alifThread_get();
	_alifErr_setHandledException(thread, _exc);
}


void _alifErr_chainExceptions1(AlifObject* _exc) { // 686
	if (_exc == nullptr) {
		return;
	}
	AlifThread* thread = _alifThread_get();
	if (_alifErr_occurred(thread)) {
		AlifObject* exc2 = _alifErr_getRaisedException(thread);
		alifException_setContext(exc2, _exc);
		_alifErr_setRaisedException(thread, exc2);
	}
	else {
		_alifErr_setRaisedException(thread, _exc);
	}
}




AlifObject* alifErr_setFromErrnoWithFilenameObject(AlifObject* _exc, AlifObject* _fileNameObject) { // 783
	return alifErr_setFromErrnoWithFilenameObjects(_exc, _fileNameObject, nullptr);
}

AlifObject* alifErr_setFromErrnoWithFilenameObjects(AlifObject* exc,
	AlifObject* filenameObject, AlifObject* filenameObject2) { // 789
	AlifThread* tstate = _alifThread_get();
	AlifObject* message{};
	AlifObject* v{}, * args{};
	AlifIntT i = errno;
#ifdef _WINDOWS
	WCHAR* s_buf = nullptr;
#endif /* Unix/Windows */

#ifdef EINTR
	//if (i == EINTR and alifErr_checkSignals())
	//	return nullptr;
#endif

#ifndef _WINDOWS
	if (i != 0) {
		const char* s = strerror(i);
		message = alifUStr_decodeLocale(s, "surrogateescape");
	}
	else {
		/* Sometimes errno didn't get set */
		message = alifUStr_fromString("Error");
	}
#else
	if (i == 0)
		message = alifUStr_fromString("Error"); /* Sometimes errno didn't get set */
	else
	{
		if (i > 0 and i < _sys_nerr) {
			message = alifUStr_fromString(_sys_errlist[i]);
		}
		else {
			int len = FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,                   /* no message source */
				i,
				MAKELANGID(LANG_NEUTRAL,
					SUBLANG_DEFAULT),
				/* Default language */
				(LPWSTR)&s_buf,
				0,                      /* size not used */
				nullptr);                  /* no args */
			if (len == 0) {
				/* Only ever seen this in out-of-mem
				   situations */
				s_buf = nullptr;
				message = alifUStr_fromFormat("Windows Error 0x%x", i);
			}
			else {
				/* remove trailing cr/lf and dots */
				while (len > 0 and (s_buf[len - 1] <= L' ' or s_buf[len - 1] == L'.'))
					s_buf[--len] = L'\0';
				message = alifUStr_fromWideChar(s_buf, len);
			}
		}
	}
#endif /* Unix/Windows */

	if (message == nullptr)
	{
#ifdef _WINDOWS
		LocalFree(s_buf);
#endif
		return nullptr;
	}

	if (filenameObject != nullptr) {
		if (filenameObject2 != nullptr)
			args = alif_buildValue("(iOOiO)", i, message, filenameObject, 0, filenameObject2);
		else
			args = alif_buildValue("(iOO)", i, message, filenameObject);
	}
	else {
		args = alif_buildValue("(iO)", i, message);
	}
	ALIF_DECREF(message);

	if (args != nullptr) {
		v = alifObject_call(exc, args, nullptr);
		ALIF_DECREF(args);
		if (v != nullptr) {
			_alifErr_setObject(tstate, (AlifObject*)ALIF_TYPE(v), v);
			ALIF_DECREF(v);
		}
	}
#ifdef _WINDOWS
	LocalFree(s_buf);
#endif
	return nullptr;
}


#ifdef _WINDOWS // 911

AlifObject* alifErr_setExcFromWindowsErrWithFilenameObject(
	AlifObject* exc, AlifIntT ierr, AlifObject* filenameObject) { // 913
	return alifErr_setExcFromWindowsErrWithFilenameObjects(exc, ierr,
		filenameObject, nullptr);
}

AlifObject* alifErr_setExcFromWindowsErrWithFilenameObjects(
	AlifObject* exc, AlifIntT ierr, AlifObject* filenameObject, AlifObject* filenameObject2) { // 922
	AlifThread* tstate = _alifThread_get();
	AlifIntT len{};
	WCHAR* s_buf = nullptr; /* Free via LocalFree */
	AlifObject* message{};
	AlifObject* args{}, * v{};

	DWORD err = (DWORD)ierr;
	if (err == 0) {
		err = GetLastError();
	}

	len = FormatMessageW(
		/* Error API error */
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,           /* no message source */
		err,
		MAKELANGID(LANG_NEUTRAL,
			SUBLANG_DEFAULT), /* Default language */
		(LPWSTR)&s_buf,
		0,              /* size not used */
		nullptr);          /* no args */
	if (len == 0) {
		/* Only seen this in out of mem situations */
		message = alifUStr_fromFormat("Windows Error 0x%x", err);
		s_buf = nullptr;
	}
	else {
		/* remove trailing cr/lf and dots */
		while (len > 0 and (s_buf[len - 1] <= L' ' || s_buf[len - 1] == L'.'))
			s_buf[--len] = L'\0';
		message = alifUStr_fromWideChar(s_buf, len);
	}

	if (message == nullptr)
	{
		LocalFree(s_buf);
		return nullptr;
	}

	if (filenameObject == nullptr) {
		filenameObject = filenameObject2 = ALIF_NONE;
	}
	else if (filenameObject2 == nullptr)
		filenameObject2 = ALIF_NONE;
	args = alif_buildValue("(iOOiO)", 0, message, filenameObject, err, filenameObject2);
	ALIF_DECREF(message);

	if (args != nullptr) {
		v = alifObject_call(exc, args, nullptr);
		ALIF_DECREF(args);
		if (v != nullptr) {
			//_alifErr_setObject(tstate, (AlifObject*)ALIF_TYPE(v), v);
			ALIF_DECREF(v);
		}
	}
	LocalFree(s_buf);
	return nullptr;
}

#endif /* _WINDOWS */ // 1046





static AlifObject* _alifErr_formatV(AlifThread* _thread, AlifObject* _exception,
	const char* _format, va_list _vargs) { // 1152
	AlifObject* string{};

	_alifErr_clear(_thread);

	string = alifUStr_fromFormatV(_format, _vargs);
	if (string != nullptr) {
		_alifErr_setObject(_thread, _exception, string);
		ALIF_DECREF(string);
	}
	return nullptr;
}



AlifObject* _alifErr_format(AlifThread* _thread, AlifObject* _exception,
	const char* _format, ...) { // 1179
	va_list vargs{};
	va_start(vargs, _format);
	_alifErr_formatV(_thread, _exception, _format, vargs);
	va_end(vargs);
	return nullptr;
}


AlifObject* alifErr_format(AlifObject* _exception,
	const char* _format, ...) { // 1191
	AlifThread* thread = _alifThread_get();
	va_list vargs{};
	va_start(vargs, _format);
	_alifErr_formatV(thread, _exception, _format, vargs);
	va_end(vargs);
	return nullptr;
}




static AlifObject* err_programText(FILE* _fp, AlifIntT _lineno, const char* _encoding) { // 1905
	char linebuf[1000]{};
	AlifUSizeT line_size = 0;

	for (AlifIntT i = 0; i < _lineno; ) {
		line_size = 0;
		if (alifUniversal_newLineFGetsWithSize(linebuf, sizeof(linebuf),
			_fp, nullptr, &line_size) == nullptr)
		{
			/* Error or EOF. */
			return nullptr;
		}
		if (i + 1 < _lineno
			and line_size == sizeof(linebuf) - 1
			and linebuf[sizeof(linebuf) - 2] != '\n')
		{
			continue;
		}
		i++;
	}

	const char* line = linebuf;
	/* Skip BOM. */
	if (_lineno == 1 and line_size >= 3 and memcmp(line, "\xef\xbb\xbf", 3) == 0) {
		line += 3;
		line_size -= 3;
	}
	AlifObject* res = alifUStr_decode(line, line_size, _encoding, "replace");
	if (res == nullptr) {
		alifErr_clear();
	}
	return res;
}

extern char* _alifTokenizer_findEncodingFilename(AlifIntT, AlifObject*); // 1964

AlifObject* _alifErr_programDecodedTextObject(AlifObject* filename, AlifIntT lineno,
	const char* encoding) { // 1966
	char* found_encoding = nullptr;
	if (filename == nullptr or lineno <= 0) {
		return nullptr;
	}

	FILE* fp = alif_fOpenObj(filename, "r");
	if (fp == nullptr) {
		alifErr_clear();
		return nullptr;
	}
	if (encoding == nullptr) {
		AlifIntT fd = fileno(fp);
		found_encoding = _alifTokenizer_findEncodingFilename(fd, filename);
		encoding = found_encoding;
		if (encoding == nullptr) {
			alifErr_clear();
			encoding = "utf-8";
		}
		/* Reset position */
		if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
			fclose(fp);
			alifMem_dataFree(found_encoding);
			return nullptr;
		}
	}
	AlifObject* res = err_programText(fp, lineno, encoding);
	fclose(fp);
	alifMem_dataFree(found_encoding);
	return res;
}
