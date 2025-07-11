#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_InitConfig.h"

#include "AlifCore_State.h"


#include "_IOModule.h"





static AlifObject* _io_openImpl(AlifObject* _module, AlifObject* _file, const char* _mode,
	AlifIntT _buffering, const char* _encoding, const char* _errors,
	const char* _newline, AlifIntT _closefd, AlifObject* _opener) { // 199
	AlifUSizeT i{};

	AlifIntT creating = 0, reading = 0, writing = 0, appending = 0, updating = 0;
	AlifIntT text = 0, binary = 0;

	char rawmode[6]{}, * m{};
	AlifIntT lineBuffering{}, isNumber{}, isatty = 0;

	AlifObject* raw{}, * modeobj = nullptr, * buffer{}, * wrapper{}, * result = nullptr, * pathOrFD = nullptr;

	isNumber = alifNumber_check(_file);

	if (isNumber) {
		pathOrFD = ALIF_NEWREF(_file);
	}
	else {
		pathOrFD = alifOS_fsPath(_file);
		if (pathOrFD == nullptr) {
			return nullptr;
		}
	}

	if (!isNumber and
		!ALIFUSTR_CHECK(pathOrFD) and
		!ALIFBYTES_CHECK(pathOrFD)) {
		alifErr_format(_alifExcTypeError_, "invalid file: %R", _file);
		goto error;
	}

	/* Decode mode */
	for (i = 0; i < strlen(_mode); i++) {
		char c = _mode[i];

		switch (c) {
		case 'x':
			creating = 1;
			break;
		case 'r':
			reading = 1;
			break;
		case 'w':
			writing = 1;
			break;
		case 'a':
			appending = 1;
			break;
		case '+':
			updating = 1;
			break;
		case 't':
			text = 1;
			break;
		case 'b':
			binary = 1;
			break;
		default:
			goto invalid_mode;
		}

		/* c must not be duplicated */
		if (strchr(_mode + i + 1, c)) {
		invalid_mode:
			alifErr_format(_alifExcValueError_, "invalid mode: '%s'", _mode);
			goto error;
		}

	}

	m = rawmode;
	if (creating)  *(m++) = 'x';
	if (reading)   *(m++) = 'r';
	if (writing)   *(m++) = 'w';
	if (appending) *(m++) = 'a';
	if (updating)  *(m++) = '+';
	*m = '\0';

	/* Parameters validation */
	if (text and binary) {
		alifErr_setString(_alifExcValueError_,
			"can't have text and binary mode at once");
		goto error;
	}

	if (creating + reading + writing + appending > 1) {
		alifErr_setString(_alifExcValueError_,
			"must have exactly one of create/read/write/append mode");
		goto error;
	}

	if (binary and _encoding != nullptr) {
		alifErr_setString(_alifExcValueError_,
			"binary mode doesn't take an encoding argument");
		goto error;
	}

	if (binary and _errors != nullptr) {
		alifErr_setString(_alifExcValueError_,
			"binary mode doesn't take an errors argument");
		goto error;
	}

	if (binary and _newline != nullptr) {
		alifErr_setString(_alifExcValueError_,
			"binary mode doesn't take a newline argument");
		goto error;
	}

	if (binary and _buffering == 1) {
		//if (alifErr_warnEx(_alifExcRuntimeWarning_,
		//	"line buffering (buffering=1) isn't supported in "
		//	"binary mode, the default buffer size will be used",
		//	1) < 0) {
		//	goto error;
		//}
	}

	/* Create the Raw file stream */
	AlifIOState* state; state = get_ioState(_module); // because of "goto" error
	{
		AlifObject* RawIOClass = (AlifObject*)state->alifFileIOType;
#ifdef HAVE_WINDOWS_CONSOLE_IO
		const AlifConfig* config = alif_getConfig();
		if (!config->legacyWindowsStdio and _alifIO_getConsoleType(pathOrFD) != '\0') {
			RawIOClass = (AlifObject*)state->alifWindowsConsoleIOType;
			_encoding = "utf-8";
		}
#endif
		raw = alifObject_callFunction(RawIOClass, "OsOO",
			pathOrFD, rawmode,
			_closefd ? ALIF_TRUE : ALIF_FALSE,
			_opener);
	}

	if (raw == nullptr)
		goto error;
	result = raw;

	ALIF_SETREF(pathOrFD, nullptr);

	modeobj = alifUStr_fromString(_mode);
	if (modeobj == nullptr)
		goto error;

	/* buffering */
	if (_buffering < 0) {
		AlifObject* res = alifObject_callMethodNoArgs(raw, &ALIF_ID(_isAttyOpenOnly));
		if (res == nullptr)
			goto error;
		isatty = alifObject_isTrue(res);
		ALIF_DECREF(res);
		if (isatty < 0)
			goto error;
	}

	if (_buffering == 1 or isatty) {
		_buffering = -1;
		lineBuffering = 1;
	}
	else
		lineBuffering = 0;

	if (_buffering < 0) {
		AlifObject* blkSizeObj{};
		blkSizeObj = alifObject_getAttr(raw, &ALIF_ID(_blkSize));
		if (blkSizeObj == nullptr)
			goto error;
		_buffering = alifLong_asLong(blkSizeObj);
		ALIF_DECREF(blkSizeObj);
		if (_buffering == -1 and alifErr_occurred())
			goto error;
	}
	if (_buffering < 0) {
		alifErr_setString(_alifExcValueError_,
			"invalid buffering size");
		goto error;
	}

	/* if not buffering, returns the raw file object */
	if (_buffering == 0) {
		if (!binary) {
			alifErr_setString(_alifExcValueError_,
				"can't have unbuffered text I/O");
			goto error;
		}

		ALIF_DECREF(modeobj);
		return result;
	}

	/* wraps into a buffered file */
	{
		AlifObject* BufferedClass{};

		if (updating) {
			BufferedClass = (AlifObject*)state->alifBufferedRandomType;
		}
		else if (creating or writing or appending) {
			BufferedClass = (AlifObject*)state->alifBufferedWriterType;
		}
		else if (reading) {
			BufferedClass = (AlifObject*)state->alifBufferedReaderType;
		}
		else {
			alifErr_format(_alifExcValueError_,
				"unknown mode: '%s'", _mode);
			goto error;
		}

		buffer = alifObject_callFunction(BufferedClass, "Oi", raw, _buffering);
	}
	if (buffer == nullptr)
		goto error;
	result = buffer;
	ALIF_DECREF(raw);


	/* if binary, returns the buffered file */
	if (binary) {
		ALIF_DECREF(modeobj);
		return result;
	}

	/* wraps into a TextIOWrapper */
	wrapper = alifObject_callFunction((AlifObject*)state->alifTextIOWrapperType,
		"OsssO",
		buffer,
		_encoding, _errors, _newline,
		lineBuffering ? ALIF_TRUE : ALIF_FALSE);
	if (wrapper == nullptr)
		goto error;
	result = wrapper;
	ALIF_DECREF(buffer);

	if (alifObject_setAttr(wrapper, &ALIF_ID(Mode), modeobj) < 0)
		goto error;
	ALIF_DECREF(modeobj);
	return result;

error:
	if (result != nullptr) {
		AlifObject* exc = alifErr_getRaisedException();
		AlifObject* close_result = alifObject_callMethodNoArgs(result, &ALIF_ID(Close));
		_alifErr_chainExceptions1(exc);
		ALIF_XDECREF(close_result);
		ALIF_DECREF(result);
	}
	ALIF_XDECREF(pathOrFD);
	ALIF_XDECREF(modeobj);
	return nullptr;
}









#include "clinic/_IOModule.cpp.h" // 627





static AlifMethodDef _moduleMethods_[] = { // 630
	_IO_OPEN_METHODDEF,
	//_IO_TEXT_ENCODING_METHODDEF
	//_IO_OPEN_CODE_METHODDEF
	{nullptr, nullptr}
};

// 637
#define ADD_TYPE(_module, _type, _spec, _base)                               \
do {                                                                     \
    _type = (AlifTypeObject *)alifType_fromModuleAndSpec(_module, _spec,        \
                                                    (AlifObject *)_base);   \
    if (_type == nullptr) {                                                  \
        return -1;                                                       \
    }                                                                    \
    if (alifModule_addType(_module, _type) < 0) {                            \
        return -1;                                                       \
    }                                                                    \
} while (0)



static AlifIntT iomodule_exec(AlifObject* m) { // 650
	AlifIOState* state = get_ioState(m);

	/* DEFAULT_BUFFER_SIZE */
	if (ALIFMODULE_ADDINTMACRO(m, DEFAULT_BUFFER_SIZE) < 0)
		return -1;

	state->unsupportedOperation = alifObject_callFunction(
		(AlifObject*)&_alifTypeType_, "s(OO){}",
		"UnsupportedOperation", _alifExcOSError_, _alifExcValueError_);
	if (state->unsupportedOperation == nullptr)
		return -1;
	if (alifModule_addObjectRef(m, "UnsupportedOperation",
		state->unsupportedOperation) < 0)
	{
		return -1;
	}

	/* BlockingIOError, for compatibility */
	if (alifModule_addObjectRef(m, "BlockingIOError",
		(AlifObject*)_alifExcBlockingIOError_) < 0) {
		return -1;
	}

	// Base classes
	//ADD_TYPE(m, state->alifIncrementalNewlineDecoderType, &_nlDecoderSpec_, nullptr);
	//ADD_TYPE(m, state->alifBytesIOBufferType, &_bytesIOBufSpec_, nullptr);
	ADD_TYPE(m, state->alifIOBaseType, &_ioBaseSpec_, nullptr);

	// alifIOBaseType subclasses
	//ADD_TYPE(m, state->alifTextIOBaseType, &_textIOBaseSpec_,
	//	state->alifIOBaseType);
	//ADD_TYPE(m, state->alifBufferedIOBaseType, &_bufferedIOBaseSpec_,
	//	state->alifIOBaseType);
	ADD_TYPE(m, state->alifRawIOBaseType, &_rawIOBaseSpec_,
		state->alifIOBaseType);

	// alifBufferedIOBaseType(alifIOBaseType) subclasses
	//ADD_TYPE(m, state->alifBytesIOType, &_bytesIOSpec_, state->alifBufferedIOBaseType);
	//ADD_TYPE(m, state->alifBufferedWriterType, &_bufferedWriterSpec_,
	//	state->alifBufferedIOBaseType);
	//ADD_TYPE(m, state->alifBufferedReaderType, &_bufferedReaderSpec_,
	//	state->alifBufferedIOBaseType);
	//ADD_TYPE(m, state->alifBufferedRWPairType, &_bufferedRWPairSpec_,
	//	state->alifBufferedIOBaseType);
	//ADD_TYPE(m, state->alifBufferedRandomType, &_bufferedRandomSpec_,
	//	state->alifBufferedIOBaseType);

	// alifRawIOBaseType(alifIOBaseType) subclasses
	ADD_TYPE(m, state->alifFileIOType, &_fileIOSpec_, state->alifRawIOBaseType);

#ifdef HAVE_WINDOWS_CONSOLE_IO
	//ADD_TYPE(m, state->alifWindowsConsoleIOType, &_winConsoleIOSpec_,
	//	state->alifRawIOBaseType);
#endif

	// alifTextIOBaseType(alifIOBaseType) subclasses
	//ADD_TYPE(m, state->alifStringIOType, &_stringIOSpec_, state->alifTextIOBaseType);
	//ADD_TYPE(m, state->alifTextIOWrapperType, &_textIOWrapperSpec_,
	//	state->alifTextIOBaseType);

#undef ADD_TYPE
	return 0;
}



static AlifModuleDefSlot _ioModuleSlots_[] = { // 717
	{ALIF_MOD_EXEC, iomodule_exec},
	{ALIF_MOD_MULTIPLE_INTERPRETERS, ALIF_MOD_PER_INTERPRETER_GIL_SUPPORTED},
	{ALIF_MOD_GIL, ALIF_MOD_GIL_NOT_USED},
	{0, nullptr},
};

AlifModuleDef _alifIOModule_ = { // 724
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "تبادل",
	//.doc = module_doc,
	.size = sizeof(AlifIOState),
	.methods = _moduleMethods_,
	//.traverse = iomodule_traverse,
	//.clear = iomodule_clear,
	//.free = iomodule_free,
	.slots = _ioModuleSlots_,
};

AlifObject* alifInit__io(void) { // 736
	return alifModuleDef_init(&_alifIOModule_);
}
