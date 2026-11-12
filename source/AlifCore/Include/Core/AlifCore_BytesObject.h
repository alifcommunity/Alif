#pragma once










ALIFAPI_FUNC(AlifObject*) _alifBytes_decodeEscape(const char*, AlifSizeT, const char*, const char**); // 23



ALIFAPI_FUNC(void) _alifBytes_repeat(char*, AlifSizeT, const char*, AlifSizeT); // 59




class AlifBytesWriter { // 67
public:
	AlifObject* buffer{};

	AlifSizeT allocated{};

	AlifSizeT minSize{};

	AlifIntT useByteArray{};

	AlifIntT overAllocate{};

	AlifIntT useSmallBuffer{};
	char smallBuffer[512]{};
};






ALIFAPI_FUNC(void) _alifBytesWriter_init(AlifBytesWriter*); // 96

ALIFAPI_FUNC(AlifObject*) _alifBytesWriter_finish(AlifBytesWriter*, void*); // 101

ALIFAPI_FUNC(void) _alifBytesWriter_dealloc(AlifBytesWriter*); // 105

ALIFAPI_FUNC(void*) _alifBytesWriter_alloc(AlifBytesWriter*, AlifSizeT); // 110


ALIFAPI_FUNC(void*) _alifBytesWriter_prepare(AlifBytesWriter*, void*, AlifSizeT); // 119


ALIFAPI_FUNC(void*) _alifBytesWriter_resize(AlifBytesWriter*, void*, AlifSizeT); // 134


ALIFAPI_FUNC(void*) _alifBytesWriter_writeBytes(AlifBytesWriter*, void*, const void*, AlifSizeT); // 140
