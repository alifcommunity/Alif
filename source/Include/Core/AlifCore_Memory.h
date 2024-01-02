#pragma once

/* 
	ذاكرة ألف، هي ذاكرة خاصة بلغة ألف النسخة الخامسة 5،
	تم إنشاؤها من قبل مجتمع ألف
	https://www.aliflang.org
	وتخضع لترخيص ألف 2023.

	مخططات الذاكرة متوفرة في ملف ../documents/AlifMemory

	لمزيد من المعلومات يمكنك الإنضمام إلى مجتمع ألف
	عبر تلغرام وطرح الأسئلة على الرابط التالي
	https://t.me/aliflang


*/


#if defined(_OS64)
	#define BLOCK_SIZE         1024
	#define BLOCK_NUMS         1024

	#define FSEGS_SIZE         2048
#elif defined(_OS32)
	#define BLOCK_SIZE         512
	#define BLOCK_NUMS         64

	#define FSEGS_SIZE         512
#endif

#define ALIGNMENT              sizeof(AlifSizeT)
#define FRAGS_NUM              (BLOCK_SIZE / ALIGNMENT - 1)


/* ----------------------------------- ذاكرة القطع ----------------------------------- */
class Frag {
public:
	void* ptr{};
	Frag* next{};
};

class FragsBlock {
public:
	AlifSizeT freeSize{};
	char* fSegs{};
};
/* ----------------------------------------------------------------------------------- */

class AlifArray {
	Frag* arr{};

public:
	void push(void*);
	const inline Frag* not_empty() const;
	inline void* get();
	Frag* get_arr();
};

class FreeSegments
{
	AlifArray freeSegs[FRAGS_NUM]{};
	Frag* fSegs{};

public:
	void* try_alloc(AlifSizeT);
	void* try_allocFreeSeg();
	void dealloc(void*);
	void freeSeg_dealloc(Frag*);

	AlifArray return_freeSegs(AlifSizeT);
};

/* --------------------------------- كتلة من الذاكرة --------------------------------- */

class AlifMemBlock {
public:
	AlifSizeT freeSize{};
	AlifMemBlock* next{};

	char* segments{};
};

/* ------------------------------------ ذاكرة ألف ------------------------------------ */

class AlifMemory
{
public:
	AlifSizeT fragIdx{};
	FragsBlock fragMem{};


	AlifSizeT curSegIdx{};

	FreeSegments* freedSegms{};

	AlifSizeT freeBlocksNum{};

	AlifMemBlock* headBlock{};
	AlifMemBlock* currentBlock{};

	AlifSizeT rawAllocSize{};
	AlifSizeT objNums{};
};

extern AlifMemory alifMem;


void alif_memoryInit();

inline void* alif_rawAlloc(AlifSizeT);
inline void alif_rawDelete(void*);
inline void* alif_rawRealloc(void*, AlifSizeT);

void* alifMem_objAlloc(AlifSizeT);
void* alifMem_dataAlloc(AlifSizeT);

inline void alifMem_objFree(void*);
inline void alifMem_dataFree(void*);

void* alifMem_objRealloc(void*, AlifSizeT);
void* alifMem_dataRealloc(void*, AlifSizeT);


const void alif_getMemState();
