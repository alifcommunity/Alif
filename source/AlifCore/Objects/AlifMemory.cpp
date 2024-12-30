#include "alif.h"

#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"
#include "AlifCore_Memory.h"
#include "AlifCore_QSBR.h" // for class QsbrThreadState


#define ALIFMEM_FRAGIDX (_alifMem_.fragIdx)
#define ALIFMEM_FRAGMEM (_alifMem_.fragMem)
#define ALIFMEM_CURRENTSEG_INDEX (_alifMem_.curSegIdx)
#define ALIFMEM_FREEDSEGMS (_alifMem_.freedSegms)
#define ALIFMEM_FREEBLOCKS_NUM (_alifMem_.freeBlocksNum)
#define ALIFMEM_HEADBLOCK (_alifMem_.headBlock)
#define ALIFMEM_CURRENTBLOCK (_alifMem_.currentBlock)
#define ALIFMEM_RAWALLOC_SIZE (_alifMem_.rawAllocSize)
#define ALIFMEM_OBJNUMS (_alifMem_.objNums)

AlifMemory _alifMem_{};

/* forward decleration */
static void* freeSeg_alloc();
//static inline AlifUSizeT size_alignUp(AlifUSizeT);


/* ----------------------------------------------------------------------------------- */
static void alifMemError_noMemory() {
	std::cout << "لا يوجد ذاكرة كافية \n" << std::endl;
	//exit(-1); // replaced with return nullptr
}
static void alifMemError_tryAllocZero() {
	std::cout << "محاولة حجز 0 في الذاكرة" << std::endl;
	//exit(-2); // replaced with return nullptr
}
static void alifMemError_reallocLessThan() {
	std::cout << "لا يمكن إعادة حجز حجم أقل من حجم المصدر " << std::endl;
	//exit(-3); // replaced with return nullptr
}


template<typename T>
static inline T* alif_new(AlifUSizeT _count = 1) {
	try
	{
		return new T[_count]{};
	}
	catch (const std::bad_alloc& e)
	{
		std::wcout << e.what() << std::endl;
		alifMemError_noMemory();
		return nullptr;
	}
	return nullptr;
}
/* ----------------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------------- */
void AlifArray::push_(void* _element) {
	/* ------------------------------------
		قم بحجز شظية وقم بتهيئتها
		ومن ثم إسنادها الى مصفوفة القطع المفرغة
	------------------------------------ */
	Frag* s_ = (Frag*)freeSeg_alloc();

	/* ------------------------------------
		في حال لم يتمكن من حجز شظية
		بسبب عدم توفر ذاكرة
		لا تقم بتفريغ القطعة حالية
	------------------------------------ */
	if (s_) {
		s_->ptr_ = _element;
		s_->next_ = nullptr;

		s_->next_ = arr_;
		arr_ = s_;
	}
}
const inline Frag* AlifArray::not_empty() const {
	return arr_;
}
inline void* AlifArray::get_() {
	/* ------------------------------------
		قم بقطع الشظية الحالية من مصفوفة القطع الفارغة
		وارسلها لجعلها ضمن الشظايا الفارغة
		من ثم ارجع عنوان القطعة الفارغة
	------------------------------------ */
	Frag* temp_ = arr_;
	void* ptr_ = arr_->ptr_;

	arr_ = arr_->next_;

	ALIFMEM_FREEDSEGMS->freeSeg_dealloc(temp_);

	return ptr_;
}
/* ----------------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------------- */
void* FreeSegments::try_alloc(AlifUSizeT _size) {
	/* ------------------------------------
		حاول جلب قطعة مفرغة بنفس الحجم وذلك بحسب المؤشر
	------------------------------------ */
	AlifUSizeT index_ = (_size / ALIGNMENT) - 2;
	if (freeSegs[index_].not_empty()) {
		void* ptr_ = freeSegs[index_].get_();
		return ptr_;
	}
	return nullptr;
}
void* FreeSegments::try_allocFreeSeg() {
	/* ------------------------------------
		حاول قطع شظية
	------------------------------------ */
	if (fSegs_)
	{
		Frag* arr_ = fSegs_;

		fSegs_ = fSegs_->next_;

		return arr_;
	}
	return nullptr;
}
void FreeSegments::dealloc_(void* _ptr) {
	/* ------------------------------------
		في حال كان حجم القطعة اقل من حجم الكتلة،
		قم بتصفيرها وارسالها الى مصفوفة القطع بحسب المؤشر،
		وإلا فقم بخذفها بإستخدام حذف النظام
	------------------------------------ */
	AlifUSizeT size_ = *((AlifUSizeT*)_ptr - 1);

	if (size_ <= BLOCK_SIZE) {
		AlifUSizeT index_ = (size_ / ALIGNMENT) - 2;
		memset(_ptr, 0, (size_ - ALIGNMENT));
		freeSegs[index_].push_(_ptr);
	}
	else {
		ALIFMEM_RAWALLOC_SIZE -= size_;
		delete[]((AlifUSizeT*)_ptr - 1);
	}
}
void FreeSegments::freeSeg_dealloc(Frag* _seg) {
	/* ------------------------------------
		قم بتحرير الشظية
	------------------------------------ */
	_seg->ptr_ = nullptr;
	_seg->next_ = nullptr;

	_seg->next_ = fSegs_;
	fSegs_ = _seg;
}
/* ----------------------------------------------------------------------------------- */







/* ------------------------------------ ذاكرة ألف ------------------------------------ */

/* --- ذاكرة المفسر الرئيسي --- */
AlifIntT alif_mainMemoryInit()
{
	/* --- تهيئة ذاكرة الشظايا --- */
	ALIFMEM_FRAGMEM = FragsBlock();
	char* fSegs = alif_new<char>(FSEGS_SIZE);
	if (fSegs == nullptr) {
		std::cout << "لا يمكن إكمال تهيئة الذاكرة \n لم ينجح في حجز قطعة شظايا جديدة" << std::endl;
		return -1;
	}
	ALIFMEM_FRAGMEM.fSegs = fSegs;
	ALIFMEM_FRAGMEM.freeSize = FSEGS_SIZE;

	/* --- تهيئة الذاكرة --- */
	ALIFMEM_HEADBLOCK = new AlifMemBlock();
	ALIFMEM_CURRENTBLOCK = ALIFMEM_HEADBLOCK;
	ALIFMEM_CURRENTBLOCK->freeSize = BLOCK_SIZE;
	ALIFMEM_FREEBLOCKS_NUM = BLOCK_NUMS;
	ALIFMEM_FREEDSEGMS = new FreeSegments();

	char* s_ = alif_new<char>(BLOCK_SIZE);
	if (s_ == nullptr) {
		std::cout << "لا يمكن إكمال تهيئة الذاكرة \n لم ينجح في حجز قطعة جديدة" << std::endl;
		return -1;
	}

	ALIFMEM_CURRENTBLOCK->segments_ = s_;

	AlifMemBlock* b_{};
	AlifMemBlock* prev_ = ALIFMEM_CURRENTBLOCK;
	for (int i_ = 1; i_ <= BLOCK_NUMS; i_++) {
		b_ = new AlifMemBlock();
		prev_->next_ = b_;
		b_->freeSize = BLOCK_SIZE;

		s_ = alif_new<char>(BLOCK_SIZE);
		if (s_ == nullptr) {
			std::cout << "لا يمكن إكمال تهيئة الذاكرة \n لم ينجح في حجز قطعة جديدة" << std::endl;
			return -1;
		}

		b_->segments_ = s_;
		prev_ = b_;
	}

	return 1;
}

/* --- ذاكرة المفسرات الفرعية --- */
AlifMemory* alif_memoryInit()
{

	AlifMemory* memory_ = new AlifMemory();

	/* --- تهيئة ذاكرة الشظايا --- */
	memory_->fragMem = FragsBlock();
	char* fSegs = alif_new<char>(FSEGS_SIZE);
	if (fSegs == nullptr) {
		std::cout << "لا يمكن إكمال تهيئة الذاكرة \n لم ينجح في حجز قطعة شظايا جديدة" << std::endl;
		return nullptr;
	}
	memory_->fragMem.fSegs = fSegs;
	memory_->fragMem.freeSize = FSEGS_SIZE;

	/* --- تهيئة الذاكرة --- */
	memory_->headBlock = new AlifMemBlock();
	memory_->currentBlock = ALIFMEM_HEADBLOCK;
	memory_->currentBlock->freeSize = BLOCK_SIZE;
	memory_->freeBlocksNum = BLOCK_NUMS;
	memory_->freedSegms = new FreeSegments();

	char* s_ = alif_new<char>(BLOCK_SIZE);
	if (s_ == nullptr) {
		std::cout << "لا يمكن إكمال تهيئة الذاكرة \n لم ينجح في حجز قطعة جديدة" << std::endl;
		return nullptr;
	}

	memory_->currentBlock->segments_ = s_;

	AlifMemBlock* b_{};
	AlifMemBlock* prev_ = memory_->currentBlock;
	for (int i_ = 1; i_ <= BLOCK_NUMS; i_++) {
		b_ = new AlifMemBlock();
		prev_->next_ = b_;
		b_->freeSize = BLOCK_SIZE;

		s_ = alif_new<char>(BLOCK_SIZE);
		if (s_ == nullptr) {
			std::cout << "لا يمكن إكمال تهيئة الذاكرة \n لم ينجح في حجز قطعة جديدة" << std::endl;
			return nullptr;
		}

		b_->segments_ = s_;
		prev_ = b_;
	}

	return memory_;
}



/* ---------------------------------- ذاكرة النظام ----------------------------------- */

inline void* alif_rawAlloc(AlifUSizeT _size) {
	/* ------------------------------------
		قم بحجز من النظام
		من ثم اضف الحجم المحجوز للذاكرة
	------------------------------------ */
	AlifUSizeT size_ = MEMORY_ALIGN_UP(_size + ALIGNMENT);
	void* ptr_ = alif_new<char>(size_);
	if (ptr_ == nullptr) return nullptr;

	ALIFMEM_RAWALLOC_SIZE += size_;

	*(AlifUSizeT*)ptr_ = size_;

	return ((AlifUSizeT*)ptr_ + 1);
}

inline void alif_rawDelete(void* _ptr) {
	AlifUSizeT* ptr_ = ((AlifUSizeT*)_ptr - 1);
	ALIFMEM_RAWALLOC_SIZE -= *(AlifUSizeT*)ptr_;
	delete[] ptr_;
}

inline void* alif_rawRealloc(void* _sourcePtr, AlifUSizeT _distSize) {
	AlifUSizeT sourceSize = *((AlifUSizeT*)_sourcePtr - 1);
	AlifUSizeT distSize = MEMORY_ALIGN_UP(_distSize + ALIGNMENT);

	// إذا كان حجم المصدر مساوي لحجم الخدف ارجع المصدر
	if (sourceSize == distSize) {
		return _sourcePtr;
	}

	/* ------------------------------------
		في حال كان حجم المصدر اقل من حجم الهدف
		قم بحجز متغير جديد
		وانقل البيانات من المصدر اليه واحذف المصدر
		وإلا اظهر خطأ منع حجز قيمة اقل من قيمة المصدر
	------------------------------------ */
	if (sourceSize < distSize) {
		void* newPtr = alif_new<char>(distSize);
		if (newPtr == nullptr) return nullptr;

		*(AlifUSizeT*)newPtr = distSize;

		memcpy(((AlifUSizeT*)newPtr + 1), _sourcePtr, (sourceSize - ALIGNMENT));
		delete[]((AlifUSizeT*)_sourcePtr - 1);

		ALIFMEM_RAWALLOC_SIZE += distSize - sourceSize;

		return ((AlifUSizeT*)newPtr + 1);
	}
	else {
		alifMemError_reallocLessThan();
	}

	return nullptr;
}
/* ----------------------------------------------------------------------------------- */



/* ----------------------------------------------------------------------------------- */
	/* ------------------------------------
		هذه المنطقة خاصة بحجز
		شظية جديدة من ذاكرة الشظايا
	------------------------------------ */
static inline void* fSeg_alloc() {
	ALIFMEM_FRAGMEM.freeSize -= sizeof(Frag);
	void* ptr = (char*)ALIFMEM_FRAGMEM.fSegs + ALIFMEM_FRAGIDX;
	ALIFMEM_FRAGIDX += sizeof(Frag);
	return ptr;
}

static inline AlifIntT fSeg_newBlock() {
	ALIFMEM_FRAGIDX = 0;
	ALIFMEM_FRAGMEM.fSegs = alif_new<char>(FSEGS_SIZE);
	if (ALIFMEM_FRAGMEM.fSegs == nullptr) return -1;
	ALIFMEM_FRAGMEM.freeSize = FSEGS_SIZE;

	return 1;
}

#define SEGMENT (ALIGNMENT + ALIGNMENT)
static void* freeSeg_alloc() {

	if (void* a = ALIFMEM_FREEDSEGMS->try_allocFreeSeg())
		return a;

	if (ALIFMEM_FRAGMEM.freeSize >= sizeof(Frag)) {
		return fSeg_alloc();
	}

	if (fSeg_newBlock() < 1) return nullptr;

	return fSeg_alloc();
}
/* ----------------------------------------------------------------------------------- */


static inline const void toNext_block() {
	ALIFMEM_CURRENTSEG_INDEX = 0;
	ALIFMEM_CURRENTBLOCK = ALIFMEM_CURRENTBLOCK->next_;
	ALIFMEM_FREEBLOCKS_NUM--;
}


static inline AlifIntT alifMem_newBlock() {
	ALIFMEM_FREEBLOCKS_NUM = BLOCK_NUMS;

	char* s_{};
	AlifMemBlock* b_{};
	AlifMemBlock* prev_ = ALIFMEM_CURRENTBLOCK;
	for (int i_ = 0; i_ <= BLOCK_NUMS; i_++)
	{
		s_ = alif_new<char>(BLOCK_SIZE);
		if (s_ == nullptr)
		{
			if (i_ > 0) {
				ALIFMEM_FREEBLOCKS_NUM = i_;
				break;
			} 	/* ------------------------------------
					في حال تم حجز اكثر من كتلة واحدة
					ولم يعد هناك ذاكرة،
					قم بإرجاع ما تم حجزه بدون إظهار خطأ
				------------------------------------ */
			alifMemError_noMemory();
			return -1;
		}

		b_ = alif_new<AlifMemBlock>();
		if (b_ == nullptr) return -1;

		prev_->next_ = b_;
		b_->freeSize = BLOCK_SIZE;
		b_->segments_ = s_;
		prev_ = b_;
	}

	return 1;
}

// not needed
//#define ALIGN (ALIGNMENT - 1)
//static inline AlifUSizeT size_alignUp(AlifUSizeT _size) {
//	/* -- align _size to ALIGNMENT -- */
//	return (_size + ALIGN) & ~ALIGN;
//}
//

static inline void* alif_allocSeg(AlifUSizeT _size) {
	ALIFMEM_CURRENTBLOCK->freeSize -= _size;
	void* ptr_ = ALIFMEM_CURRENTBLOCK->segments_ + ALIFMEM_CURRENTSEG_INDEX;
	*(AlifUSizeT*)ptr_ = _size;
	ALIFMEM_CURRENTSEG_INDEX += _size;
	return ((AlifUSizeT*)ptr_ + 1);
}

static inline void alif_freeLast() {
	*(AlifUSizeT*)((char*)ALIFMEM_CURRENTBLOCK->segments_ + ALIFMEM_CURRENTSEG_INDEX)
		= ALIFMEM_CURRENTBLOCK->freeSize;
	ALIFMEM_CURRENTBLOCK->freeSize = 0;
	void* s = ALIFMEM_CURRENTBLOCK->segments_ + ALIFMEM_CURRENTSEG_INDEX + ALIGNMENT;
	ALIFMEM_FREEDSEGMS->dealloc_(s);
}

static inline void* alifMem_alloc(AlifUSizeT _size)
{
	if (_size == 0) {
		alifMemError_tryAllocZero();
		return nullptr;
	}

	AlifUSizeT size_ = MEMORY_ALIGN_UP(_size + ALIGNMENT);

	/* ------------------------------------
		إذا كان الحجم اكبر من حجم الكتلة
		قم بالحجز بإستخدام النظام
	------------------------------------ */
	if (size_ > BLOCK_SIZE) {
		return alif_rawAlloc(_size);
	}

	/* ------------------------------------
		حاول حجز قطعة محررة
	------------------------------------ */
	if (void* a_ = ALIFMEM_FREEDSEGMS->try_alloc(size_))
		return a_;

	/* ------------------------------------
		في حال توفر مساحة في الكتلة
		قم بالحجز
	------------------------------------ */
	if (ALIFMEM_CURRENTBLOCK->freeSize >= size_) {
		return alif_allocSeg(size_);
	}

	/* ------------------------------------
		قم بتحرير اخر قطعة من الكتلة في حال وجودها
	------------------------------------ */
	if (ALIFMEM_CURRENTBLOCK->freeSize >= SEGMENT) {
		alif_freeLast();
	}

	/* ------------------------------------
		في حال نفاذ الكتل
		قم بحجز مجموعة كتل جديدة
		من ثم الانتقال الى الكتلة الجديدة
	------------------------------------ */
	if (ALIFMEM_FREEBLOCKS_NUM < 1)
	{
		if (alifMem_newBlock() < 1) return nullptr;
	}

	toNext_block();

	return alif_allocSeg(size_);
}


/* --------------------------------- ذاكرة ألف --------------------------------------- */
void* alifMem_dataAlloc(AlifUSizeT _size) {
	return alifMem_alloc(_size);
}

void* alifMem_objAlloc(AlifUSizeT _size) {
	ALIFMEM_OBJNUMS++;

	return alifMem_alloc(_size);
}


inline void alifMem_dataFree(void* _ptr) {
	/*
		يجب إضافة تحذير يظهر عند وضع التصحيح Debug
		حيث أنه لا يجب أن يتم عمل تحرير لمؤشر فارغ
	*/
	if (_ptr == nullptr) return;
	ALIFMEM_FREEDSEGMS->dealloc_(_ptr);
}

inline void alifMem_objFree(void* _ptr) {
	/*
		يجب إضافة تحذير يظهر عند وضع التصحيح Debug
		حيث أنه لا يجب أن يتم عمل تحرير لمؤشر فارغ
	*/
	if (_ptr == nullptr) return;
	ALIFMEM_FREEDSEGMS->dealloc_(_ptr);
	ALIFMEM_OBJNUMS--;
}

/* ----------------------------------------------------------------------------------- */
	/* ------------------------------------
		هذه المنطقة خاصة بالدوال الفرعية
		الخاصة بإعادة الحجز
	------------------------------------ */
static inline void* alif_dataToAlifMemAlloc(void* _sourcePtr, AlifUSizeT _distSize) {
	AlifUSizeT sourceSize = *((AlifUSizeT*)_sourcePtr - 1);

	void* s_ = alifMem_alloc(_distSize);
	if (s_ == nullptr) return nullptr;

	/* انسخ المصدر الى الهدف */
	memcpy(s_, _sourcePtr, (sourceSize - ALIGNMENT));

	return s_;
}
static inline void* alif_dataToSysAlloc(void* _sourcePtr, AlifUSizeT _distSize) {
	AlifUSizeT sourceSize = *((AlifUSizeT*)_sourcePtr - 1);

	void* r_ = alif_rawAlloc(_distSize);
	if (r_ == nullptr) return nullptr;

	/* انسخ المصدر الى الهدف */
	memcpy(r_, _sourcePtr, (sourceSize - ALIGNMENT));

	return r_;
}

static inline void* alif_objToAlifMemAlloc(void* _sourcePtr, AlifUSizeT _distSize) {
	AlifUSizeT sourceSize = *((AlifUSizeT*)_sourcePtr - 1);

	void* s_ = alifMem_alloc(_distSize);
	if (s_ == nullptr) return nullptr;

	/* انسخ المصدر الى الهدف */
	memcpy(s_, _sourcePtr, (sourceSize - ALIGNMENT));

	ALIFMEM_OBJNUMS++;

	return s_;
}
static inline void* alif_objToSysAlloc(void* _sourcePtr, AlifUSizeT _distSize) {
	AlifUSizeT sourceSize = *((AlifUSizeT*)_sourcePtr - 1);

	void* r_ = alif_rawAlloc(_distSize);
	if (r_ == nullptr) return nullptr;

	/* انسخ المصدر الى الهدف */
	memcpy(r_, _sourcePtr, (sourceSize - ALIGNMENT));

	ALIFMEM_OBJNUMS++;

	return r_;
}
/* ----------------------------------------------------------------------------------- */

void* alifMem_dataRealloc(void* _ptr, AlifUSizeT _size)
{
	void* sourcePtr = _ptr;

	/*
		في حال كان مؤشر المصدر فارغ
		قم بحجز متغير جديد
	*/
	if (sourcePtr == nullptr)
	{
		return alifMem_alloc(_size);
	}

	AlifUSizeT sourceSize = *((AlifUSizeT*)_ptr - 1);
	void* distPtr{};
	AlifUSizeT distSize = MEMORY_ALIGN_UP(_size + ALIGNMENT);

	//if (distSize < sourceSize) alifMemError_reallocLessThan();
	if (distSize < sourceSize) {
		distPtr = alifMem_dataAlloc(distSize);
		if (distPtr == nullptr) return nullptr;

		/* انسخ المصدر الى الهدف */
		memcpy(distPtr, sourcePtr, distSize);

		alifMem_dataFree(sourcePtr);

		return distPtr;
	}

	if (sourceSize <= BLOCK_SIZE and
		distSize <= BLOCK_SIZE) { // alifMem to alifMem

		distPtr = alif_dataToAlifMemAlloc(sourcePtr, _size);

		alifMem_dataFree(sourcePtr);

		return distPtr;
	}

	if (sourceSize <= BLOCK_SIZE and
		distSize > BLOCK_SIZE) { // alifMem to sysMem

		distPtr = alif_dataToSysAlloc(sourcePtr, _size);

		alifMem_dataFree(sourcePtr);

		return distPtr;
	}

	if (sourceSize > BLOCK_SIZE and
		distSize > BLOCK_SIZE) { // sysAlloc to sysAlloc

		distPtr = alif_dataToSysAlloc(sourcePtr, _size);

		delete[]((AlifUSizeT*)sourcePtr - 1);
		ALIFMEM_RAWALLOC_SIZE -= sourceSize;

		return distPtr;
	}

	if (sourceSize > BLOCK_SIZE and
		distSize <= BLOCK_SIZE) { // sysMem to alifMem
		alifMemError_reallocLessThan();
		return nullptr;
	}

	return sourcePtr;
}

void* alifMem_objRealloc(void* _ptr, AlifUSizeT _size)
{
	void* sourcePtr = _ptr;

	/*
		في حال كان مؤشر المصدر فارغ
		قم بحجز متغير جديد
	*/
	if (sourcePtr == nullptr)
	{
		return alifMem_alloc(_size);
	}

	AlifUSizeT sourceSize = *((AlifUSizeT*)_ptr - 1);
	void* distPtr{};
	AlifUSizeT distSize = MEMORY_ALIGN_UP(_size + ALIGNMENT);

	//if (distSize < sourceSize) alifMemError_reallocLessThan();
	if (distSize < sourceSize) {
		distPtr = alifMem_objAlloc(distSize);
		if (distPtr == nullptr) return nullptr;

		/* انسخ المصدر الى الهدف */
		memcpy(distPtr, sourcePtr, distSize);

		alifMem_objFree(sourcePtr);

		return distPtr;
	}

	if (sourceSize <= BLOCK_SIZE and
		distSize <= BLOCK_SIZE) { // alifMem to alifMem

		distPtr = alif_objToAlifMemAlloc(sourcePtr, _size);

		alifMem_objFree(sourcePtr);

		return distPtr;
	}

	if (sourceSize <= BLOCK_SIZE and
		distSize > BLOCK_SIZE) { // alifMem to sysMem

		distPtr = alif_objToSysAlloc(sourcePtr, _size);

		alifMem_objFree(sourcePtr);

		return distPtr;
	}

	if (sourceSize > BLOCK_SIZE and
		distSize > BLOCK_SIZE) { // sysAlloc to sysAlloc

		distPtr = alif_objToSysAlloc(sourcePtr, _size);

		delete[]((AlifUSizeT*)sourcePtr - 1);
		ALIFMEM_RAWALLOC_SIZE -= sourceSize;
		ALIFMEM_OBJNUMS--;

		return distPtr;
	}

	if (sourceSize > BLOCK_SIZE and
		distSize <= BLOCK_SIZE) { // sysMem to alifMem
		alifMemError_reallocLessThan();
		return nullptr;
	}

	return sourcePtr;
}
/* ----------------------------------------------------------------------------------- */











/* -------------------------------- ذاكرة شجرة المحلل -------------------------------- */

AlifASTBlock* block_new(AlifUSizeT _size) {
	AlifASTBlock* b_ = (AlifASTBlock*)alifMem_dataAlloc(sizeof(AlifASTBlock) + _size);
	if (b_ == nullptr) return nullptr;
	b_->size_ = _size;
	b_->mem_ = (void*)(b_ + 1);
	b_->next_ = nullptr;
	b_->offset_ = (char*)MEMORY_ALIGN_UP((AlifUSizeT)b_->mem_) - (char*)(b_->mem_);

	return b_;
}


static void* block_alloc(AlifASTBlock* _b, AlifUSizeT _size) {
	void* p_{};
	_size = MEMORY_ALIGN_UP(_size);
	if (_b->offset_ + _size > _b->size_) {
		AlifASTBlock* newBlock = block_new(_size < ASTMEM_BLOCKSIZE ? ASTMEM_BLOCKSIZE : _size);
		if (newBlock == nullptr) return nullptr;
		_b->next_ = newBlock;
		_b = newBlock;
	}

	p_ = (void*)(((char*)_b->mem_) + _b->offset_);
	_b->offset_ += _size;
	return p_;
}

static void block_free(AlifASTBlock* _b) {
	while (_b) {
		AlifASTBlock* next = _b->next_;
		alifMem_dataFree(_b);
		_b = next;
	}
}

AlifASTMem* alifASTMem_new() {
	AlifASTMem* astMem = (AlifASTMem*)alifMem_dataAlloc(sizeof(AlifASTMem));
	if (astMem == nullptr) return nullptr;

	astMem->head = block_new(ASTMEM_BLOCKSIZE);
	if (astMem->head == nullptr) return nullptr;

	astMem->current = astMem->head;
	astMem->objects = alifList_new(0);

	return astMem;
}

void* alifASTMem_malloc(AlifASTMem* _arena, AlifUSizeT _size) {
	void* p_ = block_alloc(_arena->current, _size);
	if (p_ == nullptr) return nullptr;

	if (_arena->current->next_) {
		_arena->current = _arena->current->next_;
	}

	return p_;
}

AlifIntT alifASTMem_listAddAlifObj(AlifASTMem* _astMem, AlifObject* _obj) {
	int r_ = alifList_append(_astMem->objects, _obj);
	if (r_ >= 0) {
		ALIF_DECREF(_obj);
	}
	return r_;
}


void alifASTMem_free(AlifASTMem* _astMem) { // 153
	block_free(_astMem->head);
	ALIF_DECREF(_astMem->objects);
	alifMem_dataFree(_astMem);
}




/* ------------------------------- إسناد ذاكرة المفسر ------------------------------- */
AlifIntT alifInterpreterMem_init(AlifInterpreter* _interpreter) {

	if (_interpreter == _alifDureRun_.interpreters.main) {
		_interpreter->memory_ = &_alifMem_;
	}
	else {
		_interpreter->memory_ = alif_memoryInit();
		if (_interpreter->memory_ == nullptr) {
			return -1;
		}
	}

	return 1;
}





/* --------------------------------- أدوات ذاكرة ألف --------------------------------- */

wchar_t* alifMem_wcsDup(const wchar_t* _str) {

	AlifUSizeT len = wcslen(_str);
	if (len > (AlifUSizeT)ALIF_SIZET_MAX / sizeof(wchar_t) - 1) {
		return nullptr;
	}

	size_t size = (len + 1) * sizeof(wchar_t);
	wchar_t* str2 = (wchar_t*)alifMem_dataAlloc(size);
	if (str2 == nullptr) {
		return nullptr;
	}

	memcpy(str2, _str, size);
	return str2;
}








/* ----------------------------------- حالة الذاكرة ----------------------------------- */
Frag* AlifArray::get_arr()
{
	return arr_;
}

AlifArray FreeSegments::return_freeSegs(unsigned long i)
{
	return freeSegs[i];
}

static std::pair<unsigned long, const char*> convert_(unsigned long _size, const char* _unit) {
	if (_size > 1073741823) {
		_size /= 1000000000;
		_unit = "غـ.ـب";
	}
	else if (_size > 1048575)
	{
		_size /= 1000000;
		_unit = "مـ.ـب";
	}
	else if (_size > 1023)
	{
		_size /= 1000;
		_unit = "كـ.ـب";
	}

	return std::pair<unsigned long, const char*>(_size, _unit);
}

static void rawMem_sizeAllocated()
{
	unsigned long sysMemSize = (unsigned long)ALIFMEM_RAWALLOC_SIZE;
	const char* sysMemSizeUnit = "بايت";
	std::pair<unsigned long, const char*> sysMemPair =
		std::pair<unsigned long, const char*>(sysMemSize, sysMemSizeUnit);

	sysMemPair = convert_(sysMemSize, sysMemSizeUnit);

	printf("I| --------------------------------------------- |I\n");
	printf("I|                       : ذاكرة النظام المحجوزة |I\n");
	printf("I| %9sa %9lu                          |I\n",
		sysMemPair.second, sysMemPair.first);
}

static void alifMem_sizeAllocated()
{
	unsigned long alifMemSize{};
	unsigned long alifAllocSize{};
	unsigned long wasteSize{};
	const char* alifMemSizeUnit = "بايت";
	const char* alifAllocSizeUnit = "بايت";
	const char* wasteSizeUnit = "بايت";
	std::pair<unsigned long, const char*> alifMemPair =
		std::pair<unsigned long, const char*>(alifMemSize, alifMemSizeUnit);
	std::pair<unsigned long, const char*> alifAllocPair =
		std::pair<unsigned long, const char*>(alifAllocSize, alifAllocSizeUnit);
	std::pair<unsigned long, const char*> wastePair =
		std::pair<unsigned long, const char*>(wasteSize, wasteSizeUnit);

	AlifMemBlock* currentBlock = ALIFMEM_HEADBLOCK;
	while (currentBlock->next_) {
		if (currentBlock->freeSize < SEGMENT) {
			wasteSize += (unsigned long)currentBlock->freeSize;
			alifAllocSize += BLOCK_SIZE - (unsigned long)currentBlock->freeSize;
		}
		else if (currentBlock->freeSize < BLOCK_SIZE) {
			alifAllocSize += BLOCK_SIZE - (unsigned long)currentBlock->freeSize;
		}
		alifMemSize += BLOCK_SIZE;
		currentBlock = currentBlock->next_;
	}
	if (currentBlock->freeSize < SEGMENT) {
		wasteSize += (unsigned long)currentBlock->freeSize;
		alifAllocSize += BLOCK_SIZE - (unsigned long)currentBlock->freeSize;
	}
	else if (currentBlock->freeSize < BLOCK_SIZE) {
		alifAllocSize += BLOCK_SIZE - (unsigned long)currentBlock->freeSize;
	}
	alifMemSize += BLOCK_SIZE;


	alifMemPair = convert_(alifMemSize, alifMemSizeUnit);
	alifAllocPair = convert_(alifAllocSize, alifAllocSizeUnit);
	wastePair = convert_(wasteSize, wasteSizeUnit);

	printf("I| --------------------------------------------- |I\n");
	printf("I|                          : ذاكرة ألف المحجوزة |I\n");
	printf("I| %9sa %9lu                          |I\n",
		alifMemPair.second, alifMemPair.first);
	printf("I| --------------------------------------------- |I\n");
	printf("I|                         : ذاكرة ألف المستخدمة |I\n");
	printf("I| %9sa %9lu                          |I\n",
		alifAllocPair.second, alifAllocPair.first);
	printf("I| --------------------------------------------- |I\n");
	printf("I|                          : الهدر في ذاكرة ألف |I\n");
	printf("I| %9sa %9lu                          |I\n",
		wastePair.second, wastePair.first);
}

static void fragment_sizeAllocated()
{
	unsigned long freedMemSize{};
	const char* freedMemSizeUnit = "بايت";
	std::pair<unsigned long, const char*> freedMemPair =
		std::pair<unsigned long, const char*>(freedMemSize, freedMemSizeUnit);

	for (unsigned long i_ = 0; i_ < FRAGS_NUM; i_++) {
		Frag* f_ = ALIFMEM_FREEDSEGMS->return_freeSegs(i_).get_arr();
		if (f_) {
			while (f_->next_) {
				freedMemSize += *((unsigned long*)f_->ptr_ - 1);
				f_ = f_->next_;
			}
			freedMemSize += *((unsigned long*)f_->ptr_ - 1);
		}
	}
	freedMemPair = convert_(freedMemSize, freedMemSizeUnit);

	printf("I| --------------------------------------------- |I\n");
	printf("I|                             : المساحة المحررة |I\n");
	printf("I| %9sa %9lu                          |I\n",
		freedMemPair.second, freedMemPair.first);
}

static void objects_count()
{
	unsigned long objsNum = (unsigned long)ALIFMEM_OBJNUMS;

	printf("I| --------------------------------------------- |I\n");
	printf("I|                                : عدد الكائنات |I\n");
	printf("I| %9sa %9lu                          |I\n", "كائن", objsNum);
}

static void freeBlocks_count()
{
	unsigned long freeBlocks = (unsigned long)ALIFMEM_FREEBLOCKS_NUM;

	printf("I| --------------------------------------------- |I\n");
	printf("I|                             : عدد الكتل الحرة |I\n");
	printf("I| %9sa %4i%1s%4lu                          |I\n",
		"كتلة", BLOCK_NUMS, "\\", freeBlocks);
}

static void currentSeg_size()
{
	unsigned long currSegSize = (unsigned long)ALIFMEM_CURRENTBLOCK->freeSize;

	printf("I| --------------------------------------------- |I\n");
	printf("I|                    : حجم القطعة الحرة الحالية |I\n");
	printf("I| %9sa %4i%1s%4lu                          |I\n",
		"بايت", BLOCK_SIZE, "\\", currSegSize);
}

const void alif_getMemState()
{
	/* --- sysMemSize --- */
	rawMem_sizeAllocated();
	/* --- alifMemSize --- */
	alifMem_sizeAllocated();
	/* --- fragmentSize --- */
	fragment_sizeAllocated();
	objects_count();
	freeBlocks_count();
	currentSeg_size();

	printf("I| --------------------------------------------- |I\n");
	/* ملاحظة: حرف ال "أ" الإنكليزي المستخدم في الطباعة فقط لموازنة الطباعة في الطرفية التي لا تدعم العربية */
}

/* ----------------------------------------------------------------------------------- */



