#pragma once


















class AlifBuffer {
public:
	void* buf;
	AlifObject* obj;        
	AlifSizeT len;
	AlifSizeT itemsize;
	int readonly;
	int ndim;
	char* format;
	AlifSizeT* shape;
	AlifSizeT* strides;
	AlifSizeT* suboffsets;
	void* internal;
} ;

typedef int (*GetBufferProc)(AlifObject*, AlifBuffer*, int);
typedef void (*ReleaseBufferProc)(AlifObject*, AlifBuffer*);
