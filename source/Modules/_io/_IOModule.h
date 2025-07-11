#pragma once





extern AlifTypeSpec _fileIOSpec_;
extern AlifTypeSpec _ioBaseSpec_; // 20

extern AlifTypeSpec _rawIOBaseSpec_;




typedef class IOState AlifIOState; // 35






// 81
#define DEFAULT_BUFFER_SIZE (8 * 1024)  /* bytes */













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




#ifdef HAVE_WINDOWS_CONSOLE_IO
extern char _alifIO_getConsoleType(AlifObject*); // 195
#endif
