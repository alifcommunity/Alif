#pragma once

/*
	ذاكرة ألف، هي ذاكرة خاصة بلغة ألف النسخة الخامسة 5،
	تم إنشاؤها من قبل shadow and smoke
	https://www.aliflang.org
	وتخضع لترخيص ألف 2023.

	مخططات الذاكرة متوفرة في ملف ../documents/AlifMemory

	لمزيد من المعلومات يمكنك الإنضمام إلى مجتمع ألف
	عبر تلغرام وطرح الأسئلة على الرابط التالي
	https://t.me/aliflang
*/

extern class AlifMemory _alifMem_;

ALIFAPI_FUNC(void*) alifMem_objAlloc(AlifUSizeT);
ALIFAPI_FUNC(void*) alifMem_dataAlloc(AlifUSizeT);

ALIFAPI_FUNC(void) alifMem_objFree(void*);
ALIFAPI_FUNC(void) alifMem_dataFree(void*);

ALIFAPI_FUNC(void*) alifMem_objRealloc(void*, AlifUSizeT);
ALIFAPI_FUNC(void*) alifMem_dataRealloc(void*, AlifUSizeT);

const void alif_getMemState();











/* ------------------------------------ ذاكرة المخزن ----------------------------------- */


#define ALIF_MANAGED_BUFFER_RELEASED    0x001 
#define ALIF_MANAGED_BUFFER_FREE_FORMAT 0x002  


extern AlifTypeObject _alifManagedBufferType_;
extern AlifTypeObject _alifMemoryViewType_;


AlifObject* alifMemoryView_fromMemory(char*, AlifSizeT, AlifIntT);

AlifObject* alifMemoryView_fromBuffer(const AlifBuffer*);


class AlifManagedBufferObject {
public:
	ALIFOBJECT_HEAD;
	AlifIntT flags{};
	AlifSizeT exports;
	AlifBuffer master{};
};

class AlifMemoryViewObject{
public:
	ALIFOBJECT_VAR_HEAD;
	AlifManagedBufferObject* mbuf{};
	AlifHashT hash{};
	AlifIntT flags{};
	AlifSizeT exports{};
	AlifBuffer view{};
	AlifObject* weakRefList{};
	AlifSizeT array[1]{};
};


/* memoryview state flags */
#define ALIF_MEMORYVIEW_RELEASED    0x001
#define ALIF_MEMORYVIEW_C           0x002
#define ALIF_MEMORYVIEW_FORTRAN     0x004
#define ALIF_MEMORYVIEW_SCALAR      0x008
#define ALIF_MEMORYVIEW_PIL         0x010
#define ALIF_MEMORYVIEW_RESTRICTED  0x020







/* ------------------------------------ دوال مساعدة ----------------------------------- */


#define ALIFMEM_NEW(_type, _n) /* for data only */ \
  ( ((AlifUSizeT)(_n) > ALIF_SIZET_MAX / sizeof(_type)) ? nullptr :      \
        ( (_type *) alifMem_dataAlloc((_n) * sizeof(_type)) ) )

#define ALIFMEM_RESIZE(_p, _type, _n) /* for data only */ \
  ( (_p) = ((AlifUSizeT)(_n) > ALIF_SIZET_MAX / sizeof(_type)) ? nullptr :        \
        (_type *) alifMem_dataRealloc((_p), (_n) * sizeof(_type)) )
