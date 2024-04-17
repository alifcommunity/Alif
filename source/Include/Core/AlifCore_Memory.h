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


/* ------------------------------------ ذاكرة ألف ------------------------------------ */

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


/* -------------------------------------- القطع -------------------------------------- */
class Frag {
public:
	void* ptr_{};
	Frag* next_{};
};

class FragsBlock {
public:
	AlifSizeT freeSize{};
	char* fSegs{};
};
/* ----------------------------------------------------------------------------------- */

class AlifArray {
	Frag* arr_{};

public:
	void push_(void*);
	const inline Frag* not_empty() const;
	inline void* get_();
	Frag* get_arr();
};

class FreeSegments
{
	AlifArray freeSegs[FRAGS_NUM]{};
	Frag* fSegs_{};

public:
	void* try_alloc(AlifSizeT);
	void* try_allocFreeSeg();
	void dealloc_(void*);
	void freeSeg_dealloc(Frag*);

	AlifArray return_freeSegs(AlifSizeT);
};

/* -------------------------------------- الكتلة ------------------------------------- */

class AlifMemBlock {
public:
	AlifSizeT freeSize{};
	AlifMemBlock* next_{};

	char* segments_{};
};

/* ---------------------------------- إدارة الذاكرة --------------------------------- */

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

/*
	يجب نقل الذاكرة الى المفسر وعدم تركها ذاكرة عامة
	لأنه سيتم عمل ذاكرة خاصة بكل مسار او "ثريد" لمنع تداهل البيانات
*/ 
extern AlifMemory _alifMem_;


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








/* -------------------------------- ذاكرة شجرة المحلل -------------------------------- */

/*
	ذاكرة الشجرة يتم إنشاؤها في مرحلة المحلل وتمريرها مع كل دالة
	لانه سيتم تفريغها وحذفها بعد الإنتهاء من مرحلة التحليل اللغوي
*/

#define ASTMEM_BLOCKSIZE 8192

class AlifASTBlock {
public:
	AlifSizeT size_{};
	AlifSizeT offset_{};
	AlifASTBlock* next_{};
	void* mem_{};
};

class AlifASTMem {
public:
	AlifASTBlock* head_{};
	AlifASTBlock* current_{};
	AlifObj* objects_{};
};


AlifASTBlock* block_new(AlifSizeT _size);
AlifASTMem* alifASTMem_new();
void* alifArena_malloc(AlifASTMem* _arena, AlifSizeT _size);
int alifArena_listAddAlifObj(AlifASTMem* _arena, AlifObj* _obj);
