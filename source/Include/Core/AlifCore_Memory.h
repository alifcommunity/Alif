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


/* ------------------------------------ التعريفات ------------------------------------ */


#define ALIGNMENT		sizeof(AlifUSizeT)
#define ALIGN_UP(_size) ((_size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))


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

#define FRAGS_NUM              (BLOCK_SIZE / ALIGNMENT - 1)


/* -------------------------------------- القطع -------------------------------------- */
class Frag {
public:
	void* ptr_{};
	Frag* next_{};
};

class FragsBlock {
public:
	AlifUSizeT freeSize{};
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
	void* try_alloc(AlifUSizeT);
	void* try_allocFreeSeg();
	void dealloc_(void*);
	void freeSeg_dealloc(Frag*);

	AlifArray return_freeSegs(AlifUSizeT);
};

/* -------------------------------------- الكتلة ------------------------------------- */

class AlifMemBlock {
public:
	AlifUSizeT freeSize{};
	AlifMemBlock* next_{};

	char* segments_{};
};

/* ---------------------------------- إدارة الذاكرة --------------------------------- */

class AlifMemory
{
public:
	AlifUSizeT fragIdx{};
	FragsBlock fragMem{};


	AlifUSizeT curSegIdx{};

	FreeSegments* freedSegms{};

	AlifUSizeT freeBlocksNum{};

	AlifMemBlock* headBlock{};
	AlifMemBlock* currentBlock{};

	AlifUSizeT rawAllocSize{};
	AlifUSizeT objNums{};
};

/*
	يجب نقل الذاكرة الى المفسر وعدم تركها ذاكرة عامة
	لأنه سيتم عمل ذاكرة خاصة بكل مسار او "ثريد" لمنع تداهل البيانات
*/
extern AlifMemory _alifMem_;


void* alif_memoryInit();



inline void* alif_rawAlloc(AlifUSizeT);
inline void alif_rawDelete(void*);
inline void* alif_rawRealloc(void*, AlifUSizeT);

void* alifMem_objAlloc(AlifUSizeT);
void* alifMem_dataAlloc(AlifUSizeT);

inline void alifMem_objFree(void*);
inline void alifMem_dataFree(void*);

void* alifMem_objRealloc(void*, AlifUSizeT);
void* alifMem_dataRealloc(void*, AlifUSizeT);





const void alif_getMemState();





/* -------------------------------- ذاكرة شجرة المحلل -------------------------------- */

/*
	ذاكرة الشجرة يتم إنشاؤها في مرحلة المحلل وتمريرها مع كل دالة
	لانه سيتم تفريغها وحذفها بعد الإنتهاء من مرحلة التحليل اللغوي
*/

#define ASTMEM_BLOCKSIZE 8192

class AlifASTBlock {
public:
	AlifUSizeT size_{};
	AlifUSizeT offset_{};
	AlifASTBlock* next_{};
	void* mem_{};
};

class AlifASTMem {
public:
	AlifASTBlock* head_{};
	AlifASTBlock* current_{};
	AlifObject* objects_{};
};


AlifASTBlock* block_new(AlifUSizeT);
AlifASTMem* alifASTMem_new();
void* alifASTMem_malloc(AlifASTMem*, AlifUSizeT);
int alifASTMem_listAddAlifObj(AlifASTMem*, AlifObject*);
