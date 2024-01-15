#include "alif.h"
#include "AlifCore_Memory.h"


#define ALIFMEM_FRAGIDX (alifMem.fragIdx)
#define ALIFMEM_FRAGMEM (alifMem.fragMem)
#define ALIFMEM_CURRSEGIDX (alifMem.curSegIdx)
#define ALIFMEM_FREEDSEGMS (alifMem.freedSegms)
#define ALIFMEM_FREEBLOCKSNUM (alifMem.freeBlocksNum)
#define ALIFMEM_HEADBLOCK (alifMem.headBlock)
#define ALIFMEM_CURRENTBLOCK (alifMem.currentBlock)
#define ALIFMEM_RAWALLOCSIZE (alifMem.rawAllocSize)
#define ALIFMEM_OBJNUMS (alifMem.objNums)

AlifMemory alifMem{};

/* forward decleration */
static void* freeSeg_alloc();
static inline AlifSizeT size_alignment(AlifSizeT);


/* ----------------------------------------------------------------------------------- */
static void alifMemError_noMemory() {
	std::wcout << L"لا يوجد ذاكرة كافية \n" << std::endl;
	exit(-1);
}
static void alifMemError_tryAllocZero() {
	std::wcout << L"محاولة حجز 0 في الذاكرة" << std::endl;
	exit(-2);
}
static void alifMemError_reallocLessThan() {
	std::wcout << L"لا يمكن إعادة حجز أقل من حجم المصدر " << std::endl;
	exit(-3);
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
	}
	return nullptr;
}
/* ----------------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------------- */
void AlifArray::push(void* _element) {
	/* ------------------------------------
		قم بحجز شظية وقم بتهيئتها 
		ومن ثم إسنادها الى مصفوفة القطع المفرغة 
	------------------------------------ */
	Frag* s = (Frag*)freeSeg_alloc();
	s->ptr = _element;
	s->next = nullptr;

	s->next = arr;
	arr = s;
}
const inline Frag* AlifArray::not_empty() const {
	return arr;
}
inline void* AlifArray::get() {
	/* ------------------------------------
		قم بقطع الشظية الحالية من مصفوفة القطع الفارغة
		وارسلها لجعلها ضمن الشظايا الفارغة
		من ثم ارجع عنوان القطعة الفارغة
	------------------------------------ */
	Frag* temp = arr;
	void* ptr = arr->ptr;

	arr = arr->next;

	ALIFMEM_FREEDSEGMS->freeSeg_dealloc(temp);

	return ptr;
}
/* ----------------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------------- */
void* FreeSegments::try_alloc(AlifSizeT _size) {
	/* ------------------------------------
		حاول جلب قطعة مفرغة بنفس الحجم وذلك بحسب المؤشر
	------------------------------------ */
	AlifSizeT index = (_size / ALIGNMENT) - 2;
	if (freeSegs[index].not_empty()) {
		void* ptr = freeSegs[index].get();
		return ptr;
	}
	return nullptr;
}
void* FreeSegments::try_allocFreeSeg() {
	/* ------------------------------------
		حاول قطع شظية
	------------------------------------ */
	if (fSegs) 
	{
		Frag* arr = fSegs;

		fSegs = fSegs->next;	

		return arr;
	}
	return nullptr;
}
void FreeSegments::dealloc(void* _ptr) {
	/* ------------------------------------
		في حال كان حجم القطعة اقل من حجم الكتلة،
		قم بتصفيرها وارسالها الى مصفوفة القطع بحسب المؤشر،
		وإلا فقم بخذفها بإستخدام حذف النظام
	------------------------------------ */
	AlifSizeT size = *((AlifSizeT*)_ptr - 1);

	if (size <= BLOCK_SIZE) {
		AlifSizeT index = (size / ALIGNMENT) - 2;
		memset(_ptr, 0, (size - ALIGNMENT));
		freeSegs[index].push(_ptr);
	}
	else {
		ALIFMEM_RAWALLOCSIZE -= size;
		delete[] ((AlifSizeT*)_ptr - 1);
	}
}
void FreeSegments::freeSeg_dealloc(Frag* _seg) {
	/* ------------------------------------
		قم بتحرير الشظية
	------------------------------------ */
	_seg->ptr = nullptr;
	_seg->next = nullptr;

	_seg->next = fSegs;
	fSegs = _seg;
}
/* ----------------------------------------------------------------------------------- */







/* ------------------------------------ ذاكرة ألف ------------------------------------ */

void alif_memoryInit()
{
	/* --- تهيئة ذاكرة الشظايا --- */
	ALIFMEM_FRAGMEM = FragsBlock();
	ALIFMEM_FRAGMEM.fSegs = alif_new<char>(FSEGS_SIZE);
	ALIFMEM_FRAGMEM.freeSize = FSEGS_SIZE;

	/* --- تهيئة الذاكرة --- */
	ALIFMEM_HEADBLOCK = new AlifMemBlock();
	ALIFMEM_CURRENTBLOCK = ALIFMEM_HEADBLOCK;
	ALIFMEM_CURRENTBLOCK->freeSize = BLOCK_SIZE;
	ALIFMEM_FREEBLOCKSNUM = BLOCK_NUMS;
	ALIFMEM_FREEDSEGMS = new FreeSegments();

	char* s = alif_new<char>(BLOCK_SIZE);

	ALIFMEM_CURRENTBLOCK->segments = s;

	AlifMemBlock* b{};
	AlifMemBlock* prev = ALIFMEM_CURRENTBLOCK;
	for (int i = 1; i <= BLOCK_NUMS; i++) {
		b = new AlifMemBlock();
		prev->next = b;
		b->freeSize = BLOCK_SIZE;
		
		s = alif_new<char>(BLOCK_SIZE);

		b->segments = s;
		prev = b;
	}
}



/* ---------------------------------- ذاكرة النظام ----------------------------------- */

inline void* alif_rawAlloc(AlifSizeT _size) {
	/* ------------------------------------
		قم بحجز من النظام
		من ثم اضف الحجم المحجوز للذاكرة
	------------------------------------ */
	AlifSizeT size = size_alignment(_size + ALIGNMENT);
	void* ptr = alif_new<char>(size);

	ALIFMEM_RAWALLOCSIZE += size;

	*(AlifSizeT*)ptr = size;

	return ((AlifSizeT*)ptr + 1);
}

inline void alif_rawDelete(void* _ptr) {
	AlifSizeT* ptr = ((AlifSizeT*)_ptr - 1);
	ALIFMEM_RAWALLOCSIZE -= *(AlifSizeT*)ptr;
	delete[] ptr;
}

inline void* alif_rawRealloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);
	AlifSizeT distSize = size_alignment(_distSize + ALIGNMENT);
	
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
		*(AlifSizeT*)newPtr = distSize;

		memcpy(((AlifSizeT*)newPtr + 1), _sourcePtr, (sourceSize - ALIGNMENT));
		delete[] ((AlifSizeT*)_sourcePtr - 1);

		ALIFMEM_RAWALLOCSIZE += distSize - sourceSize;

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

static inline void fSeg_newBlock() {
	ALIFMEM_FRAGIDX = 0;
	ALIFMEM_FRAGMEM.fSegs = alif_new<char>(FSEGS_SIZE);
	ALIFMEM_FRAGMEM.freeSize = FSEGS_SIZE;
}

#define SEGMENT (ALIGNMENT + ALIGNMENT)
static void* freeSeg_alloc() {
	void* a = ALIFMEM_FREEDSEGMS->try_allocFreeSeg();
	if (a) { return a; }

	if (ALIFMEM_FRAGMEM.freeSize >= sizeof(Frag)) {
		return fSeg_alloc();
	}

	fSeg_newBlock();

	return fSeg_alloc();
}
/* ----------------------------------------------------------------------------------- */


static inline const void toNext_Block() {
	ALIFMEM_CURRSEGIDX = 0;
	ALIFMEM_CURRENTBLOCK = ALIFMEM_CURRENTBLOCK->next;
	ALIFMEM_FREEBLOCKSNUM--;
}


static inline void alifMem_newBlock() {
	ALIFMEM_FREEBLOCKSNUM = BLOCK_NUMS;

	char* s{};
	AlifMemBlock* b{};
	AlifMemBlock* prev = ALIFMEM_CURRENTBLOCK;
	for (int i = 0; i <= BLOCK_NUMS; i++)
	{
		s = (char*)malloc(BLOCK_SIZE);
		if (!s)
		{
			if (i > 0) {
				ALIFMEM_FREEBLOCKSNUM = i;
				break;
			} 	/* ------------------------------------
					في حال تم حجز اكثر من كتلة واحدة
					ولم يعد هناك ذاكرة، 
					قم بإرجاع ما تم حجزه بدون إظهار خطأ
				------------------------------------ */
			alifMemError_noMemory();
		}

		b = alif_new<AlifMemBlock>();
		prev->next = b;
		b->freeSize = BLOCK_SIZE;
		b->segments = s;
		prev = b;
	}
}

#define ALIGN (ALIGNMENT - 1)
static inline AlifSizeT size_alignment(AlifSizeT _size) {
	/* -- align _size to ALIGNMENT -- */
	return (_size + ALIGN) & ~ALIGN;
}

static inline void* alif_allocSeg(AlifSizeT _size) {
	ALIFMEM_CURRENTBLOCK->freeSize -= _size;
	void* ptr = ALIFMEM_CURRENTBLOCK->segments + ALIFMEM_CURRSEGIDX;
	*(AlifSizeT*)ptr = _size;
	ALIFMEM_CURRSEGIDX += _size;
	return ((AlifSizeT*)ptr + 1);
}

static inline void alif_freeLast() {
	*(AlifSizeT*)((char*)ALIFMEM_CURRENTBLOCK->segments + ALIFMEM_CURRSEGIDX)
		= ALIFMEM_CURRENTBLOCK->freeSize;
	ALIFMEM_CURRENTBLOCK->freeSize = 0;
	void* s = ALIFMEM_CURRENTBLOCK->segments + ALIFMEM_CURRSEGIDX + ALIGNMENT;
	ALIFMEM_FREEDSEGMS->dealloc(s);
}

static inline void* alifMem_alloc(AlifSizeT _size)
{
	if (_size == 0) {
		alifMemError_tryAllocZero();
	}

	AlifSizeT size = size_alignment(_size + ALIGNMENT);

	/* ------------------------------------
		إذا كان الحجم اكبر من حجم الكتلة
		قم بالحجز بإستخدام النظام
	------------------------------------ */
	if (size > BLOCK_SIZE) {
		return alif_rawAlloc(_size);
	}

	/* ------------------------------------
		حاول حجز قطعة محررة
	------------------------------------ */
	void* a = ALIFMEM_FREEDSEGMS->try_alloc(size);
	if (a) { return a; }

	/* ------------------------------------
		في حال توفر مساحة في الكتلة
		قم بالحجز
	------------------------------------ */
	if (ALIFMEM_CURRENTBLOCK->freeSize >= size) {
		return alif_allocSeg(size);
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
	if (ALIFMEM_FREEBLOCKSNUM < 1)
	{
		alifMem_newBlock();
	}

	toNext_Block();

	return alif_allocSeg(size);
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
	ALIFMEM_FREEDSEGMS->dealloc(_ptr);
}

inline void alifMem_objFree(void* _ptr) {
	ALIFMEM_FREEDSEGMS->dealloc(_ptr);
	ALIFMEM_OBJNUMS--;
}


/* ----------------------------------------------------------------------------------- */
	/* ------------------------------------
		هذه المنطقة خاصة بالدوال الفرعية
		الخاصة بإعادة الحجز
	------------------------------------ */
static inline void* alif_dataToAlifMemAlloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);

	void* s = alifMem_alloc(_distSize);

	/* انسخ المصدر الى الهدف */
	memcpy(s, _sourcePtr, (sourceSize - ALIGNMENT));

	return s;
}
static inline void* alif_dataToSysAlloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);

	void* r = alif_rawAlloc(_distSize);

	/* انسخ المصدر الى الهدف */
	memcpy(r, _sourcePtr, (sourceSize - ALIGNMENT));

	return r;
}

static inline void* alif_objToAlifMemAlloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);

	void* s = alifMem_alloc(_distSize);

	/* انسخ المصدر الى الهدف */
	memcpy(s, _sourcePtr, (sourceSize - ALIGNMENT));

	ALIFMEM_OBJNUMS++;

	return s;
}
static inline void* alif_objToSysAlloc(void* _sourcePtr, AlifSizeT _distSize) {
	AlifSizeT sourceSize = *((AlifSizeT*)_sourcePtr - 1);

	void* r = alif_rawAlloc(_distSize);

	/* انسخ المصدر الى الهدف */
	memcpy(r, _sourcePtr, (sourceSize - ALIGNMENT));

	ALIFMEM_OBJNUMS++;

	return r;
}
/* ----------------------------------------------------------------------------------- */

void* alifMem_dataRealloc(void* _ptr, AlifSizeT _size) {
	void* sourcePtr = _ptr;
	AlifSizeT sourceSize = *((AlifSizeT*)_ptr - 1);
	void* distPtr{};
	AlifSizeT distSize = size_alignment(_size + ALIGNMENT);

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
		ALIFMEM_RAWALLOCSIZE -= sourceSize;

		return distPtr;
	}

	if (sourceSize > BLOCK_SIZE and
		distSize <= BLOCK_SIZE) { // sysMem to alifMem
		alifMemError_reallocLessThan();
	}

	return sourcePtr;
}

void* alifMem_objRealloc(void* _ptr, AlifSizeT _size)
{
	void* sourcePtr = _ptr;
	AlifSizeT sourceSize = *((AlifSizeT*)_ptr - 1);
	void* distPtr{};
	AlifSizeT distSize = size_alignment(_size + ALIGNMENT);

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
		ALIFMEM_RAWALLOCSIZE -= sourceSize;
		ALIFMEM_OBJNUMS--;

		return distPtr;
	}

	if (sourceSize > BLOCK_SIZE and
		distSize <= BLOCK_SIZE) { // sysMem to alifMem
		alifMemError_reallocLessThan();
	}

	return sourcePtr;
}
/* ----------------------------------------------------------------------------------- */



/* -------------------------------- Memory API --------------------------------------- */
Frag* AlifArray::get_arr()
{
	return arr;
}

AlifArray FreeSegments::return_freeSegs(AlifSizeT i)
{
	return freeSegs[i];
}

static std::pair<AlifSizeT, const wchar_t*> convert(AlifSizeT _size, const wchar_t* _unit) {
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
	AlifSizeT sysMemSize = ALIFMEM_RAWALLOCSIZE;
	const wchar_t* sysMemSizeUnit = L"بايت";
	std::pair<AlifSizeT, const wchar_t*> sysMemPair = 
		std::pair<AlifSizeT, const wchar_t*>(sysMemSize, sysMemSizeUnit);

	sysMemPair = convert(sysMemSize, sysMemSizeUnit);

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
	while (currentBlock->next) {
		if (currentBlock->freeSize < SEGMENT) {
			wasteSize += currentBlock->freeSize;
			alifAllocSize += BLOCK_SIZE - currentBlock->freeSize;
		}
		else if (currentBlock->freeSize < BLOCK_SIZE) {
			alifAllocSize += BLOCK_SIZE - currentBlock->freeSize;
		}
		alifMemSize += BLOCK_SIZE;
		currentBlock = currentBlock->next;
	}
	if (currentBlock->freeSize < SEGMENT) {
		wasteSize += currentBlock->freeSize;
		alifAllocSize += BLOCK_SIZE - currentBlock->freeSize;
	}
	else if (currentBlock->freeSize < BLOCK_SIZE) {
		alifAllocSize += BLOCK_SIZE - currentBlock->freeSize;
	}
	alifMemSize += BLOCK_SIZE;


	alifMemPair = convert(alifMemSize, alifMemSizeUnit);
	alifAllocPair = convert(alifAllocSize, alifAllocSizeUnit);
	wastePair = convert(wasteSize, wasteSizeUnit);

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

	for (int i = 0; i < FRAGS_NUM; i++) {
		Frag* f = ALIFMEM_FREEDSEGMS->return_freeSegs(i).get_arr();
		if (f) {
			while (f->next) {
				freedMemSize += *((AlifSizeT*)f->ptr - 1);
				f = f->next;
			}
			freedMemSize += *((AlifSizeT*)f->ptr - 1);
		}
	}
	freedMemPair = convert(freedMemSize, freedMemSizeUnit);

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
	AlifSizeT freeBlocks = ALIFMEM_FREEBLOCKSNUM;

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
