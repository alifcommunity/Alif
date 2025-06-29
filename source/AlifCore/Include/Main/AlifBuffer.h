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
    AlifSizeT* subOffsets{};
    void* internal{};
};

typedef AlifIntT (*GetBufferProc)(AlifObject*, AlifBuffer*, AlifIntT); // 35
typedef void (*ReleaseBufferProc)(AlifObject*, AlifBuffer*);


AlifIntT alifObject_getBuffer(AlifObject* , AlifBuffer* , AlifIntT); // 46



AlifIntT alifBuffer_fillInfo(AlifBuffer*, AlifObject*, void*,
	AlifSizeT, AlifIntT, AlifIntT); // 97


void alifBuffer_release(AlifBuffer*); // 102


 // 108
#define ALIFBUF_SIMPLE 0
#define ALIFBUF_WRITABLE 0x0001




#define ALIFBUF_FORMAT 0x0004
#define ALIFBUF_ND 0x0008
#define ALIFBUF_STRIDES (0x0010 | ALIFBUF_ND)
#define ALIFBUF_C_CONTIGUOUS (0x0020 | ALIFBUF_STRIDES)
#define ALIFBUF_F_CONTIGUOUS (0x0040 | ALIFBUF_STRIDES)
#define ALIFBUF_ANY_CONTIGUOUS (0x0080 | ALIFBUF_STRIDES)
#define ALIFBUF_INDIRECT (0x0100 | ALIFBUF_STRIDES)

#define ALIFBUF_CONTIG (ALIFBUF_ND | ALIFBUF_WRITABLE) // 124




#define ALIFBUF_FULL_RO (ALIFBUF_INDIRECT | ALIFBUF_FORMAT) // 134




 // 137
#define ALIFBUF_READ  0x100
#define ALIFBUF_WRITE 0x200
