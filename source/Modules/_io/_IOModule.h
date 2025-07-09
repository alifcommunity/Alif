#pragma once












typedef class IOState AlifIOState; // 35





















class IOState { // 145
public:
	AlifIntT initialized{};
	AlifObject* unsupportedOperation{};

	/* Types */
	AlifTypeObject* alifIOBaseType{};
	AlifTypeObject* alifIncrementalNewlineDecoderType{};
	AlifTypeObject* alifRawIOBaseType{};
	AlifTypeObject* alifBufferedIOBaseType{};
	AlifTypeObject* alifBufferedRWPairType{};
	AlifTypeObject* alifBufferedRandomType{};
	AlifTypeObject* alifBufferedReaderType{};
	AlifTypeObject* alifBufferedWriterType{};
	AlifTypeObject* alifBytesIOBufferType{};
	AlifTypeObject* alifBytesIOType{};
	AlifTypeObject* alifFileIOType{};
	AlifTypeObject* alifStringIOType{};
	AlifTypeObject* alifTextIOBaseType{};
	AlifTypeObject* alifTextIOWrapperType{};
#ifdef HAVE_WINDOWS_CONSOLE_IO
	AlifTypeObject* alifWindowsConsoleIOType{};
#endif
};



static inline AlifIOState* get_ioState(AlifObject* _module) { // 169
	void* state = _alifModule_getState(_module);
	return (AlifIOState*)state;
}
