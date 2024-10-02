#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_Array.h"







AlifPArray::AlifPArray() {
	capacity = 4;
	size = 0;
	data = (void**)alifMem_dataAlloc(capacity * sizeof(void**));
}

AlifPArray::~AlifPArray() {
	alifMem_dataFree(data);
}

void AlifPArray::push_back(void*& value) {
	if (size == capacity) {
		capacity *= 2;
		data = (void**)alifMem_dataRealloc(data, capacity * sizeof(void**));
		if (data == nullptr) return;
	}
	data[size++] = value;
}
