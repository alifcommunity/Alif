#include "alif.h"
#include "AlifCore_Memory.h"


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
static inline AlifSizeT size_alignUp(AlifSizeT);


/* ----------------------------------------------------------------------------------- */
static void alifMemError_noMemory() {
	std::wcout << L"لا يوجد ذاكرة كافية \n" << std::endl;
	//exit(-1); // replaced with return nullptr
}
static void alifMemError_tryAllocZero() {
	std::wcout << L"محاولة حجز 0 في الذاكرة" << std::endl;
	//exit(-2); // replaced with return nullptr
}
static void alifMemError_reallocLessThan() {
	std::wcout << L"لا يمكن إعادة حجز حجم أقل من حجم المصدر " << std::endl;
	//exit(-3); // replaced with return nullptr
}


template<typename T>
static inline T* alif_new(AlifSizeT _count = 1) {
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
	s_->ptr_ = _element;
	s_->next_ = nullptr;

	s_->next_ = arr_;
	arr_ = s_;
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
void* FreeSegments::try_alloc(AlifSizeT _size) {
	/* ------------------------------------
		حاول جلب قطعة مفرغة بنفس الحجم وذلك بحسب المؤشر
	------------------------------------ */
	AlifSizeT index_ = (_size / ALIGNMENT) - 2;
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
	AlifSizeT size_ = *((AlifSizeT*)_ptr - 1);

	if (size_ <= BLOCK_SIZE) {
		AlifSizeT index_ = (size_ / ALIGNMENT) - 2;
		memset(_ptr, 0, (size_ - ALIGNMENT));
		freeSegs[index_].push_(_ptr);
	}
	else {
		ALIFMEM_RAWALLOC_SIZE -= size_;
		delete[] ((AlifSizeT*)_ptr - 1);
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

void alif_memoryInit()
{
	/* --- تهيئة ذاكرة الشظايا --- */
	ALIFMEM_FRAGMEM = FragsBlock();
	char* fSegs = alif_new<char>(FSEGS_SIZE);
	if (fSegs == nullptr) {
		std::wcout << L"لا يمكن إكمال تهيئة الذاكرة \n" << std::endl;
		exit(-1);
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
		std::wcout << L"لا يمكن إكمال تهيئة الذاكرة \n" << std::endl;
		exit(-1);
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
			std::wcout << L"لا يمكن إكمال تهيئة الذاكرة \n" << std::endl;
			exit(-1);
		}

		b_->segments_ = s_;
		prev_ = b_;
	}
}



/* ---------------------------------- ذاكرة النظام ----------------------------------- */

inline void* alif_rawAlloc(AlifSizeT _size) {
	/* ------------------------------------
		قم بحجز من النظام
		من ثم اضف الحجم المحجوز للذاكرة
	------------------------------------ */
	AlifSizeT size_ = size_alignUp(_size + ALIGNMENT);
	void* ptr_ = alif_new<char>(size_);
	if (ptr_ == nullptr) return nullptr;

	ALIFMEM_RAWALLOC_SIZE += size_;

	*(AlifSizeT*)ptr_ = size_;

	return ((AlifSizeT*)ptr_ + 1);
}

inline void alif_rawDelete(void* _ptr) {
	AlifSizeT* ptr_ = ((AlifSizeT*)_ptr - 1);
	ALIFMEM_RAWALLOC_SIZE -= *(AlifSizeT*)ptr_;
	delete[] ptr_;
}

inline void* alif_rawRealloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);
	AlifSizeT distSize = size_alignUp(_distSize + ALIGNMENT);
	
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

		*(AlifSizeT*)newPtr = distSize;

		memcpy(((AlifSizeT*)newPtr + 1), _sourcePtr, (sourceSize - ALIGNMENT));
		delete[] ((AlifSizeT*)_sourcePtr - 1);

		ALIFMEM_RAWALLOC_SIZE += distSize - sourceSize;

		return ((AlifSizeT*)newPtr + 1);
	}
	else {
		alifMemError_reallocLessThan();
	}
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

static inline void* fSeg_newBlock() {
	ALIFMEM_FRAGIDX = 0;
	ALIFMEM_FRAGMEM.fSegs = alif_new<char>(FSEGS_SIZE);
	if (ALIFMEM_FRAGMEM.fSegs == nullptr) return nullptr;
	ALIFMEM_FRAGMEM.freeSize = FSEGS_SIZE;
}

#define SEGMENT (ALIGNMENT + ALIGNMENT)
static void* freeSeg_alloc() {
	
	if (void* a = ALIFMEM_FREEDSEGMS->try_allocFreeSeg()) { return a; }

	if (ALIFMEM_FRAGMEM.freeSize >= sizeof(Frag)) {
		return fSeg_alloc();
	}

	if (!fSeg_newBlock()) return nullptr;

	return fSeg_alloc();
}
/* ----------------------------------------------------------------------------------- */


static inline const void toNext_block() {
	ALIFMEM_CURRENTSEG_INDEX = 0;
	ALIFMEM_CURRENTBLOCK = ALIFMEM_CURRENTBLOCK->next_;
	ALIFMEM_FREEBLOCKS_NUM--;
}


static inline void* alifMem_newBlock() {
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
			return nullptr;
		}

		b_ = alif_new<AlifMemBlock>();
		if (b_ == nullptr) return nullptr;

		prev_->next_ = b_;
		b_->freeSize = BLOCK_SIZE;
		b_->segments_ = s_;
		prev_ = b_;
	}
}

#define ALIGN (ALIGNMENT - 1)
static inline AlifSizeT size_alignUp(AlifSizeT _size) {
	/* -- align _size to ALIGNMENT -- */
	return (_size + ALIGN) & ~ALIGN;
}

static inline void* alif_allocSeg(AlifSizeT _size) {
	ALIFMEM_CURRENTBLOCK->freeSize -= _size;
	void* ptr_ = ALIFMEM_CURRENTBLOCK->segments_ + ALIFMEM_CURRENTSEG_INDEX;
	*(AlifSizeT*)ptr_ = _size;
	ALIFMEM_CURRENTSEG_INDEX += _size;
	return ((AlifSizeT*)ptr_ + 1);
}

static inline void alif_freeLast() {
	*(AlifSizeT*)((char*)ALIFMEM_CURRENTBLOCK->segments_ + ALIFMEM_CURRENTSEG_INDEX)
		= ALIFMEM_CURRENTBLOCK->freeSize;
	ALIFMEM_CURRENTBLOCK->freeSize = 0;
	void* s = ALIFMEM_CURRENTBLOCK->segments_ + ALIFMEM_CURRENTSEG_INDEX + ALIGNMENT;
	ALIFMEM_FREEDSEGMS->dealloc_(s);
}

static inline void* alifMem_alloc(AlifSizeT _size)
{
	if (_size == 0) {
		alifMemError_tryAllocZero();
		return nullptr;
	}

	AlifSizeT size_ = size_alignUp(_size + ALIGNMENT);

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
	{ return a_; }

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
		if (!alifMem_newBlock()) return nullptr;
	}

	toNext_block();

	return alif_allocSeg(size_);
}


/* --------------------------------- ذاكرة ألف --------------------------------------- */
void* alifMem_dataAlloc(AlifSizeT _size) {
	return alifMem_alloc(_size);
}

void* alifMem_objAlloc(AlifSizeT _size) {
	ALIFMEM_OBJNUMS++;

	return alifMem_alloc(_size);;
}


inline void alifMem_dataFree(void* _ptr) {
	ALIFMEM_FREEDSEGMS->dealloc_(_ptr);
}

inline void alifMem_objFree(void* _ptr) {
	ALIFMEM_FREEDSEGMS->dealloc_(_ptr);
	ALIFMEM_OBJNUMS--;
}


/* ----------------------------------------------------------------------------------- */
	/* ------------------------------------
		هذه المنطقة خاصة بالدوال الفرعية
		الخاصة بإعادة الحجز
	------------------------------------ */
static inline void* alif_dataToAlifMemAlloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);

	void* s_ = alifMem_alloc(_distSize);
	if (s_ == nullptr) return nullptr;

	/* انسخ المصدر الى الهدف */
	memcpy(s_, _sourcePtr, (sourceSize - ALIGNMENT));

	return s_;
}
static inline void* alif_dataToSysAlloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);

	void* r_ = alif_rawAlloc(_distSize);
	if (r_ == nullptr) return nullptr;

	/* انسخ المصدر الى الهدف */
	memcpy(r_, _sourcePtr, (sourceSize - ALIGNMENT));

	return r_;
}

static inline void* alif_objToAlifMemAlloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);

	void* s_ = alifMem_alloc(_distSize);
	if (s_ == nullptr) return nullptr;

	/* انسخ المصدر الى الهدف */
	memcpy(s_, _sourcePtr, (sourceSize - ALIGNMENT));

	ALIFMEM_OBJNUMS++;

	return s_;
}
static inline void* alif_objToSysAlloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);

	void* r_ = alif_rawAlloc(_distSize);
	if (r_ == nullptr) return nullptr;

	/* انسخ المصدر الى الهدف */
	memcpy(r_, _sourcePtr, (sourceSize - ALIGNMENT));

	ALIFMEM_OBJNUMS++;

	return r_;
}
/* ----------------------------------------------------------------------------------- */

void* alifMem_dataRealloc(void* _ptr, AlifSizeT _size) {
	void* sourcePtr = _ptr;
	AlifSizeT sourceSize = *((AlifSizeT*)_ptr - 1);
	void* distPtr{};
	AlifSizeT distSize = size_alignUp(_size + ALIGNMENT);

	if (sourcePtr == nullptr) { std::wcout << __FUNCTION__ << std::endl; exit(-1); } // temp - need to correct

	if (distSize == 0) alifMemError_reallocLessThan();

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

		delete[] ((AlifSizeT*)sourcePtr - 1);
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

void* alifMem_objRealloc(void* _ptr, AlifSizeT _size)
{
	void* sourcePtr = _ptr;
	AlifSizeT sourceSize = *((AlifSizeT*)_ptr - 1);
	void* distPtr{};
	AlifSizeT distSize = size_alignUp(_size + ALIGNMENT);

	if (sourcePtr == nullptr) { std::wcout << __FUNCTION__ << std::endl; exit(-1); } // temp - need to correct

	if (distSize == 0) alifMemError_reallocLessThan();

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

		delete[]((AlifSizeT*)sourcePtr - 1);
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



/* -------------------------------- Memory API --------------------------------------- */
Frag* AlifArray::get_arr()
{
	return arr_;
}

AlifArray FreeSegments::return_freeSegs(AlifSizeT i)
{
	return freeSegs[i];
}

static std::pair<AlifSizeT, const wchar_t*> convert_(AlifSizeT _size, const wchar_t* _unit) {
	if (_size > 1073741823) {
		_size /= 1000000000;
		_unit = L"غيغابايت";
	}
	else if (_size > 1048575)
	{
		_size /= 1000000;
		_unit = L"ميغابايت";
	}
	else if (_size > 1023)
	{
		_size /= 1000;
		_unit = L"كيلوبايت";
	}

	return std::pair<AlifSizeT, const wchar_t*>(_size, _unit);
}

static void rawMem_sizeAllocated() 
{
	AlifSizeT sysMemSize = ALIFMEM_RAWALLOC_SIZE;
	const wchar_t* sysMemSizeUnit = L"بايت";
	std::pair<AlifSizeT, const wchar_t*> sysMemPair = 
		std::pair<AlifSizeT, const wchar_t*>(sysMemSize, sysMemSizeUnit);

	sysMemPair = convert_(sysMemSize, sysMemSizeUnit);

	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                       : ذاكرة النظام المحجوزة |I\n");
#if defined(_OS64)
	wprintf(L"I| %9lsa %9llu                          |I\n",
		sysMemPair.second, sysMemPair.first);
#elif defined(_OS32)
	wprintf(L"I| %9lsa %9u                          |I\n",
		sysMemPair.second, sysMemPair.first);
#endif
}

static void alifMem_sizeAllocated() 
{
	AlifSizeT alifMemSize{};
	AlifSizeT alifAllocSize{};
	AlifSizeT wasteSize{};
	const wchar_t* alifMemSizeUnit = L"بايت";
	const wchar_t* alifAllocSizeUnit = L"بايت";
	const wchar_t* wasteSizeUnit = L"بايت";
	std::pair<AlifSizeT, const wchar_t*> alifMemPair =
		std::pair<AlifSizeT, const wchar_t*>(alifMemSize, alifMemSizeUnit);
	std::pair<AlifSizeT, const wchar_t*> alifAllocPair =
		std::pair<AlifSizeT, const wchar_t*>(alifAllocSize, alifAllocSizeUnit);
	std::pair<AlifSizeT, const wchar_t*> wastePair =
		std::pair<AlifSizeT, const wchar_t*>(wasteSize, wasteSizeUnit);
	
	AlifMemBlock* currentBlock = ALIFMEM_HEADBLOCK;
	while (currentBlock->next_) {
		if (currentBlock->freeSize < SEGMENT) {
			wasteSize += currentBlock->freeSize;
			alifAllocSize += BLOCK_SIZE - currentBlock->freeSize;
		}
		else if (currentBlock->freeSize < BLOCK_SIZE) {
			alifAllocSize += BLOCK_SIZE - currentBlock->freeSize;
		}
		alifMemSize += BLOCK_SIZE;
		currentBlock = currentBlock->next_;
	}
	if (currentBlock->freeSize < SEGMENT) {
		wasteSize += currentBlock->freeSize;
		alifAllocSize += BLOCK_SIZE - currentBlock->freeSize;
	}
	else if (currentBlock->freeSize < BLOCK_SIZE) {
		alifAllocSize += BLOCK_SIZE - currentBlock->freeSize;
	}
	alifMemSize += BLOCK_SIZE;


	alifMemPair = convert_(alifMemSize, alifMemSizeUnit);
	alifAllocPair = convert_(alifAllocSize, alifAllocSizeUnit);
	wastePair = convert_(wasteSize, wasteSizeUnit);

	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                          : ذاكرة ألف المحجوزة |I\n");
#if defined(_OS64)
	wprintf(L"I| %9lsa %9llu                          |I\n",
		alifMemPair.second, alifMemPair.first);
	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                         : ذاكرة ألف المستخدمة |I\n");
	wprintf(L"I| %9lsa %9llu                          |I\n",
		alifAllocPair.second, alifAllocPair.first);
	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                          : الهدر في ذاكرة ألف |I\n");
	wprintf(L"I| %9lsa %9llu                          |I\n",
		wastePair.second, wastePair.first);
#elif defined(_OS32)
	wprintf(L"I| %9lsa %9u                          |I\n",
		alifMemPair.second, alifMemPair.first);
	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                         : ذاكرة ألف المستخدمة |I\n");
	wprintf(L"I| %9lsa %9u                          |I\n",
		alifAllocPair.second, alifAllocPair.first);
	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                          : الهدر في ذاكرة ألف |I\n");
	wprintf(L"I| %9lsa %9u                          |I\n",
		wastePair.second, wastePair.first);
#endif
}

static void fragment_sizeAllocated() 
{
	AlifSizeT freedMemSize{};
	const wchar_t* freedMemSizeUnit = L"بايت";
	std::pair<AlifSizeT, const wchar_t*> freedMemPair = 
		std::pair<AlifSizeT, const wchar_t*>(freedMemSize, freedMemSizeUnit);

	for (int i_ = 0; i_ < FRAGS_NUM; i_++) {
		Frag* f_ = ALIFMEM_FREEDSEGMS->return_freeSegs(i_).get_arr();
		if (f_) {
			while (f_->next_) {
				freedMemSize += *((AlifSizeT*)f_->ptr_ - 1);
				f_ = f_->next_;
			}
			freedMemSize += *((AlifSizeT*)f_->ptr_ - 1);
		}
	}
	freedMemPair = convert_(freedMemSize, freedMemSizeUnit);

	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                             : المساحة المحررة |I\n");
#if defined(_OS64)
	wprintf(L"I| %9lsa %9llu                          |I\n",
		freedMemPair.second, freedMemPair.first);
#elif defined(_OS32)
	wprintf(L"I| %9lsa %9u                          |I\n",
		freedMemPair.second, freedMemPair.first);
#endif
}

static void objects_count() 
{
	AlifSizeT objsNum = ALIFMEM_OBJNUMS;

	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                                : عدد الكائنات |I\n");
#if defined(_OS64)
	wprintf(L"I| %9lsa %9llu                          |I\n", L"كائن", objsNum);
#elif defined(_OS32)
	wprintf(L"I| %9lsa %9u                          |I\n", L"كائن", objsNum);
#endif
}

static void freeBlocks_count() 
{
	AlifSizeT freeBlocks = ALIFMEM_FREEBLOCKS_NUM;

	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                             : عدد الكتل الحرة |I\n");
#if defined(_OS64)
	wprintf(L"I| %9lsa %4i%1ls%4llu                          |I\n",
		L"كتلة", BLOCK_NUMS, L"\\", freeBlocks);
#elif defined(_OS32)
	wprintf(L"I| %9lsa %4i%1ls%4u                          |I\n",
		L"كتلة", BLOCK_NUMS, L"\\", freeBlocks);
#endif
}

static void currentSeg_size() 
{
	AlifSizeT currSegSize = ALIFMEM_CURRENTBLOCK->freeSize;

	wprintf(L"I| --------------------------------------------- |I\n");
	wprintf(L"I|                    : حجم القطعة الحرة الحالية |I\n");
#if defined(_OS64)
	wprintf(L"I| %9lsa %4i%1ls%4llu                          |I\n",
		L"بايت", BLOCK_SIZE, L"\\", currSegSize);
#elif defined(_OS32)
	wprintf(L"I| %9lsa %4i%1ls%4u                          |I\n",
		L"بايت", BLOCK_SIZE, L"\\", currSegSize);
#endif
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

	wprintf(L"I| --------------------------------------------- |I\n");
	/* ملاحظة: حرف ال "أ" الإنكليزي المستخدم في الطباعة فقط لموازنة الطباعة في الطرفية التي لا تدعم العربية */
}

/* ----------------------------------------------------------------------------------- */
















/* -------------------------------- ذاكرة شجرة المحلل -------------------------------- */

AlifASTBlock* block_new(AlifSizeT _size) {
	AlifASTBlock* b_ = (AlifASTBlock*)alifMem_dataAlloc(sizeof(AlifASTBlock) + _size);
	if (b_ == nullptr) return nullptr;
	b_->size_ = _size;
	b_->mem_ = (void*)(b_ + 1);
	b_->next_ = nullptr;
	//#define ALIF_ALIGNUP(p, a) ((void*)(((uintptr_t)(p) + (uintptr_t)((a) - 1)) & ~(uintptr_t)((a) - 1))) // for delete
		//b->offset_ = (char*)ALIF_ALIGNUP(b->mem_, ALIGNMENT) - (char*)(b->mem_); // for delete
	b_->offset_ = (char*)size_alignUp((AlifSizeT)b_->mem_) - (char*)(b_->mem_);

	return b_;
}


static void* block_alloc(AlifASTBlock* _b, AlifSizeT _size) {
	void* p_{};
	_size = size_alignUp(_size);
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


AlifASTMem* alifASTMem_new() {
	AlifASTMem* astMem = (AlifASTMem*)alifMem_dataAlloc(sizeof(AlifASTMem));
	if (astMem == nullptr) return nullptr;

	astMem->head_ = block_new(ASTMEM_BLOCKSIZE);
	if (astMem->head_ == nullptr) return nullptr;

	astMem->current_ = astMem->head_;

	astMem->objects_ = alifNew_list(0);

	return astMem;
}

void* alifArena_malloc(AlifASTMem* _arena, AlifSizeT _size) {
	void* p_ = block_alloc(_arena->current_, _size);
	if (p_ == nullptr) return nullptr;

	if (_arena->current_->next_) {
		_arena->current_ = _arena->current_->next_;
	}

	return p_;
}

int alifArena_listAddAlifObj(AlifASTMem* _arena, AlifObj* _obj) {
	int r_ = list_append(_arena->objects_, _obj);
	if (r_ >= 0) {
		//ALIF_DECREF(_obj);
	}
	return r_;
}

