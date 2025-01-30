#pragma once




#  define ALIFTUPLE_MAXSAVESIZE 20     // Largest tuple to save on freelist
#  define ALIFTUPLE_MAXFREELIST 2000  // Maximum number of tuples of each size to save
#  define ALIFLISTS_MAXFREELIST 80
#  define ALIFDICTS_MAXFREELIST 80
#  define ALIFDICTKEYS_MAXFREELIST 80
#  define ALIFFLOATS_MAXFREELIST 100
#  define ALIFSLICES_MAXFREELIST 1
#  define ALIFCONTEXTS_MAXFREELIST 255
#  define ALIFASYNC_GENS_MAXFREELIST 80
#  define ALIFASYNC_GEN_ASENDS_MAXFREELIST 80
#  define ALIFFUTUREITERS_MAXFREELIST 255
#  define ALIFOBJECT_STACK_CHUNKS_MAXFREELIST 4


class AlifFreeList { // 25
public:
	void* freeList{};
	AlifSizeT size{};
};



class AlifFreeLists { // 40
public:
	AlifFreeList floats{};
	AlifFreeList tuples[ALIFTUPLE_MAXSAVESIZE];
	AlifFreeList lists{};
	AlifFreeList dicts{};
	AlifFreeList dictKeys{};
	AlifFreeList slices{};
	AlifFreeList contexts{};
	AlifFreeList asyncGens{};
	AlifFreeList asyncGenAsends{};
	AlifFreeList futureIters{};
	AlifFreeList objectStackChunks{};
};

