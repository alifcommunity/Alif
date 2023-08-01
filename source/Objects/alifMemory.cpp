#include "alif.h"

// raw memory ///////////////////////////////////////////////////////////////////////////////

/***************************************/
/* low-level allocator implementations */
/***************************************/

void* AlifMem_raw_malloc(size_t size) {

	if (size == 0) {
		size = 1;
	}
	return malloc(size);
}

void* AlifMem_raw_calloc(size_t nElement, size_t elSize) {


	if (nElement == 0 || elSize == 0) {
		nElement = 1;
		elSize = 1;
	}

	return calloc(nElement, elSize);

}

void* AlifMem_raw_realloc(void* ptr, size_t size) {

	if (size == 0) {
		size = 1;
	}
	return realloc(ptr, size);
}

void AlifMem_raw_free(void* ptr) {
	free(ptr);
}




