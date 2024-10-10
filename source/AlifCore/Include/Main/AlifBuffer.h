#pragma once


class AlifBuffer { // 20
public:
    void* buf{};
    AlifObject* obj{};
    AlifSizeT len{};
    AlifSizeT itemSize{};
    AlifIntT readonly{};
    AlifIntT nDim{};
    char* format{};
    AlifSizeT* shape{};
    AlifSizeT* strides{};
    AlifSizeT* subOffSets{};
    void* internal{};
};

typedef AlifIntT (*GetBufferProc)(AlifObject*, AlifBuffer*, AlifIntT); // 35
typedef void (*ReleaseBufferProc)(AlifObject*, AlifBuffer*);


AlifIntT alifObject_getBuffer(AlifObject* , AlifBuffer* , AlifIntT); // 46

void alifBuffer_release(AlifBuffer*); // 102


 // 108
#define ALIFBUF_SIMPLE 0
#define ALIFBUF_WRITABLE 0x0001
