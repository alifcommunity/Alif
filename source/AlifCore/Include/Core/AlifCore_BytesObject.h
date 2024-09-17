#pragma once



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






void alifBytesWriter_init(AlifBytesWriter*); // 96

void alifBytesWriter_dealloc(AlifBytesWriter*); // 105

void* alifBytesWriter_alloc(AlifBytesWriter*, AlifSizeT); // 110


void* alifBytesWriter_prepare(AlifBytesWriter*, void*, AlifSizeT); // 119


void* alifBytesWriter_resize(AlifBytesWriter*, void*, AlifSizeT); // 134
