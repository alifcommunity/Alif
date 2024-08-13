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

void* alifMem_objAlloc(AlifUSizeT);
void* alifMem_dataAlloc(AlifUSizeT);

inline void alifMem_objFree(void*);
inline void alifMem_dataFree(void*);

void* alifMem_objRealloc(void*, AlifUSizeT);
void* alifMem_dataRealloc(void*, AlifUSizeT);

const void alif_getMemState();

