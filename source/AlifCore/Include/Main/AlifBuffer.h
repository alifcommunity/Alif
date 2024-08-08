#pragma once


class AlifBuffer {
public:
    void* buf{};
    AlifObject* obj{};
    int64_t len{};
    int64_t itemSize{};
    int readonly{};
    int nunberDim{};
    wchar_t* format{};
    int64_t* shape{};
    int64_t* strides{};
    int64_t* subOffSets{};
    void* internal{};
};

typedef int (*GetBufferProc)(AlifObject*, AlifBuffer*, int);
typedef void (*ReleaseBufferProc)(AlifObject*, AlifBuffer*);


int alifObject_getBuffer(AlifObject* , AlifBuffer* , int );

void alifBuffer_release(AlifBuffer* );
