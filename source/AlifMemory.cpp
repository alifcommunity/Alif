#include "AlifMemory.h"
//#include <memoryapi.h>


AlifMemory::AlifMemory(size_t _segmentSize)
    : segmentSize(_segmentSize), currentIndex(0)
{
    currentSegment = new wchar_t[segmentSize]; // احجز المساحة الممررة وهي 8192 بايت مع قيمة افتراضية 0
    segments_.push_back(currentSegment); // قم بإضافة عنوان القطعة الحالية الى مصفوفة القطع
    fragmentCounts = (int*)(currentSegment); // قم بحجز اربع بايت خاصة بعداد الوحدات
    *fragmentCounts = 0;
    currentIndex += 4; // قم بالتقدم بمقدار اربع بايت لتفادي المساحة التي حجزت للعدد الذي سيعد الوحدات في هذه القطعة
}

void* AlifMemory::allocate(size_t _size)
{
    /*
        للتاكد من ان حجم _size
        يساوي عدد صحيح من مضاعفات العدد 2
        مثال:
        في حال تم حجز 17 بايت
        ستقوم هذه العملية بالتحقق حيث ستقوم بحجز 32 بايت في الذاكرة - 16 للنصف الاول و 16 للواحد المتبقي -ا
    */
    //_size = (_size + 15) & ~15;

    if (currentIndex + _size > segmentSize) // هل يوجد مساحة كافية في القطعة؟
    {
        for (wchar_t* seg : segments_) // لاجل كل عنوان من عناوين القطع في مصفوفة القطع
        {
            if (*seg <= 0) // *seg هي قيمة العنوان الاول وهو عدد الوحدات في هذه القطعة
            {
                currentSegment = seg; // قم بإسناد عنوان القطعة في العنوان الحالي الذي يتم التعامل معه الان
                fragmentCounts = (int*)currentSegment; // قم بحجز اول اربع بايتات لعداد الوحدات
                currentIndex = 4; // قم بإضافة اربعة الى محدد العنوان الحالي لتفادي الحجز مكان عداد الوحدات
                goto returnPtr; // اذهب مباشرة الى خطوات ارجاع العنوان
            }
        }

        segmentSize = segmentSize * 1.1; // تتبع التعقيد الزمني لوغارتم(ت) ولكن لتفادي حجز كميات كبيرة من الذاكرة بلا حاجة تم تقليل نسبة المضاعفة الى 1.1
        try {
            currentSegment = new wchar_t[segmentSize];  // قم بحجز قطعة جديدة مع قيمة افتراضية
        }
        catch (const std::bad_alloc& e)
        {
            std::wcout << L"لا يوجد ذاكرة كافية" << std::endl;
            exit(-2);                
        }

        segments_.push_back(currentSegment); // قم بإضافة اول عنوان من القطعة الى مصفوفة القطع
        currentIndex = 0; // تاكد ان المؤشر الحالي يؤشر الى العنوان الاول من القطعة
        fragmentCounts = (int*)(currentSegment); // قم بحجز اربع بايت خاصة بعداد الوحدات
        *fragmentCounts = 0; // تاكد ان عدد الوحدات المحجوزة هو صفر
        currentIndex += 4; // تخطى عداد الوحدات
    }

    returnPtr:
        void* ptr_ = currentSegment + currentIndex; // قم بضبط العنوان الى عنوان المتغير القطعة الحالية مضاف اليها مزيح العناوين
        currentIndex += _size; // قم بإزاحة المؤشر بمقدار الحجم الذي تم حجزه
        *fragmentCounts += 1; // قم بإضافة واحد الى عداد الوحدات لانه تم حجز وحدة جديدة
        return ptr_; // ارجع عنوان المكان الذي تم حجزه
}

void AlifMemory::deallocate(void* _ptr)
{
    for (wchar_t* seg : segments_) // لاجل كل عنوان من عناوين القطع في مصفوفة القطع
    {
        if (_ptr < seg + segmentSize) // اذا كان العنوان الممرر ضمن القطعة الحالية
        {
            *seg -= 1; // قم بإنقاص عداد الوحدات
            break;
        }
    }

}









/*

    هناك نوعان من الذاكرة الاول المسمى raw والثاني المسممى mem
    raw يستخدم ذاكرة منخفضة المستوى مباشرة من لغة c
    mem يسخدم الذاكرة رفيعة المستوى من مميزاتها
       - تتبع التخصيصات و الغاء التخصيصات
       - وايضا يتحقق من الاخطاء
       - ويدعم تخصيصات الذاكرة المختلفة

*/

//
//////////////////////flags/////////////////////
//
//bool rawOrMem = false;
//bool objectOrMemDelete = false; // false for mem , true for object
//
///////////////////////////////////////////////
//
//
//
//void* mem_debug_raw_alloc(MemoryState* _state, size_t useCalloc, size_t nByte);
//void* mem_debug_raw_realloc(MemoryState* _state, void* ptr, size_t nByte);
//void mem_debug_raw_free(MemoryState* _state, void* ptr);
//
//
//void* object_malloc(MemoryState* _state, size_t numberByte);
//void* object_calloc(MemoryState* _state, size_t nElement, size_t elSize);
//
//void
//write_size_t(void* p, size_t n)
//{
//    uint8_t* q = (uint8_t*)p + 8 - 1;
//    int i;
//
//    for (i = 8; --i >= 0; --q) {
//        *q = (uint8_t)(n & 0xff);
//        n >>= 8;
//    }
//}
//
//size_t
//read_size_t(const void* p)
//{
//    const uint8_t* q = (const uint8_t*)p;
//    size_t result = *q++;
//    size_t i;
//
//    for (i = 8; --i > 0; ++q)
//        result = (result << 8) | *q;
//    return result;
//}
//
//void* mem_raw_malloc(size_t size) {
//
//    if (size == 0) {
//        size = 1;
//    }
//    return malloc(size);
//}
//
//void* mem_raw_calloc(size_t nElement, size_t elSize) {
//
//
//    if (nElement == 0 || elSize == 0) {
//        nElement = 1;
//        elSize = 1;
//    }
//
//    return calloc(nElement, elSize);
//
//}
//
//void* mem_raw_realloc(void* ptr, size_t size) {
//
//    if (size == 0) {
//        size = 1;
//    }
//    return realloc(ptr, size);
//}
//
//void mem_raw_free(void* ptr) {
//    free(ptr);
//}
//
//
//// raw memory
//
//void* raw_malloc(size_t size) {
//
//    if (size > 9223372036854775807i64) {
//        return NULL;
//    }
//
//    return mem_raw_malloc(size);
//}
//
//void* raw_calloc(MemoryState* _state, size_t nElement, size_t elSize) {
//
//    rawOrMem = false;
//
//    return mem_debug_raw_alloc(_state, 1, nElement * elSize);
//
//}
//
//void* raw_realloc(MemoryState* _state, void* ptr, size_t newSize) {
//
//    rawOrMem = false;
//
//    if (newSize > 9223372036854775807i64) {
//        return NULL;
//    }
//
//    return mem_debug_raw_realloc(_state, ptr, newSize);
//
//}
//
//void raw_free(MemoryState* _state, void* ptr) {
//
//    objectOrMemDelete = false;
//
//    mem_debug_raw_free(_state, ptr);
//
//}
//
//
//// mem memory
//
//void* mem_malloc(MemoryState* _state, size_t size) {
//
//    rawOrMem = true;
//
//    return mem_debug_raw_alloc(_state, 0, size);
//}
//
//void* mem_calloc(MemoryState* _state, size_t nElement, size_t elSize) {
//
//    rawOrMem = true;
//
//    return mem_debug_raw_alloc(_state, 1, nElement * elSize);
//}
//
//void* mem_realloc(MemoryState* _state, void* ptr, size_t newSize) {
//
//    rawOrMem = true;
//
//    return mem_debug_raw_realloc(_state, ptr, newSize);
//}
//
//void mem_free(MemoryState* _state, void* ptr) {
//
//    objectOrMemDelete = false;
//
//    mem_debug_raw_free(_state, ptr);
//}
//
//inline SegmentMapDown* Segment_map_get(MemoryState* _state, uint8_t* ptr, int create) {
//
//    int i1 = (((uint8_t)(ptr)) >> ((((64 - 0) - 20 + 2) / 3) + (((64 - 0) - 20 + 2 + 20))) & ((1 << (((64 - 0) - 20 + 2) / 3)) - 1));
//
//    if (_state->usage.SegmentMapRoot.ptrs[i1] == NULL) {
//
//        if (!create) {
//            return NULL;
//        }
//        SegmentMapMid* mid = (SegmentMapMid*)mem_debug_raw_alloc(_state, 1, sizeof(SegmentMapMid));
//
//        _state->usage.SegmentMapRoot.ptrs[i1] = mid;
//        _state->usage.SegmentMapMidCount++;
//
//    }
//    int i2 = ((((uintptr_t)(ptr)) >> (((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3)) + 20)) & ((1 << (((64 - 0) - 20 + 2) / 3)) - 1));
//
//    if (_state->usage.SegmentMapRoot.ptrs[i1]->ptrs[i2] == NULL) {
//
//        if (!create) {
//            return NULL;
//        }
//        SegmentMapDown* bot = (SegmentMapDown*)mem_debug_raw_alloc(_state, 1, sizeof(SegmentMapDown));
//
//        _state->usage.SegmentMapRoot.ptrs[i1]->ptrs[i2] = bot;
//        _state->usage.SegmentMapMidCount++;
//    }
//    return _state->usage.SegmentMapRoot.ptrs[i1]->ptrs[i2];
//
//}
//
//void Segment_map_mark_used(MemoryState* _state, uintptr_t SegmentBase, int isUsed) {
//
//
//    SegmentMapDown* bot = Segment_map_get(_state, (uint8_t*)SegmentBase, isUsed);
//
//    int i3 = ((((uintptr_t)((uint8_t*)SegmentBase)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));
//    int32_t tail = (int32_t)(SegmentBase & ((1 << 20) - 1));
//    if (tail == 0) {
//        bot->Segment[i3].tailHi = isUsed ? -1 : 0;
//    }
//    else {
//        bot->Segment[i3].tailHi = isUsed ? tail : 0;
//        uintptr_t SegmentBaseNext = SegmentBase + (1 << 20);
//        SegmentMapDown* botLo = Segment_map_get(_state, (uint8_t*)SegmentBaseNext, isUsed);
//        if (botLo == NULL) {
//            bot->Segment[i3].tailHi = 0;
//            return;
//        }
//        int i3Next = ((((uintptr_t)(SegmentBaseNext)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));
//        botLo->Segment[i3Next].tailLo = isUsed ? tail : 0;
//    }
//
//}
//
//int Segment_map_is_used(MemoryState* _state, uint8_t* ptr) {
//
//    SegmentMapDown* mid = Segment_map_get(_state, ptr, 0);
//
//    int i3 = ((((uintptr_t)((uint8_t*)ptr)) >> 20) & ((1 << ((64 - 0) - 20 - 2 * (((64 - 0) - 20 + 2) / 3))) - 1));
//
//    int32_t hi = mid->Segment[i3].tailHi;
//    int32_t lo = mid->Segment[i3].tailLo;
//    int32_t tail = (int32_t)((uintptr_t)(ptr) & ((1 << 20) - 1));
//    return (tail < lo) || (tail >= hi && hi != 0);
//
//}
//
//void insert_to_used_Alignment(MemoryState* _state, AlignmentHeader* Alignment) {
//
//    unsigned int size = Alignment->sizeIndex;
//    AlignmentHeader* next = _state->alignments.used[size * 2];
//    AlignmentHeader* prev = next->prevAlignment;
//
//    Alignment->nextAlignment = next;
//    Alignment->prevAlignment = prev;
//    next->prevAlignment = Alignment;
//    prev->nextAlignment = Alignment;
//
//}
//
//void insert_to_free_Alignment(MemoryState* _state, AlignmentHeader* Alignment) {
//
//    AlignmentHeader* next = Alignment->nextAlignment;
//    AlignmentHeader* prev = Alignment->prevAlignment;
//    next->prevAlignment = prev;
//    prev->nextAlignment = next;
//
//    SegmentObject* SegmentObj = _state->mGmt.Segments;
//
//    Alignment->nextAlignment = SegmentObj->freeAlignments;
//    SegmentObj->freeAlignments = Alignment;
//    unsigned int numberFree = SegmentObj->numberFreeAlignments;
//
//    SegmentObject* lastNumberFree = state.mGmt.numberfreeAlignment[numberFree];
//    if (lastNumberFree == SegmentObj) {
//        SegmentObject* Segment = SegmentObj->prevSegment;
//        state.mGmt.numberfreeAlignment[numberFree] = (Segment != NULL && Segment->numberFreeAlignments == numberFree) ? Segment : NULL;
//    }
//    SegmentObj->numberFreeAlignments = ++numberFree;
//
//    if (numberFree == SegmentObj->numberTotalAlignments && SegmentObj->nextSegment != NULL) {
//        if (SegmentObj->prevSegment == NULL) {
//            _state->mGmt.usableSegment = SegmentObj->nextSegment;
//        }
//        else {
//            SegmentObj->prevSegment->nextSegment = SegmentObj->nextSegment;
//        }
//
//        if (SegmentObj->nextSegment != NULL) {
//            SegmentObj->nextSegment->prevSegment = SegmentObj->prevSegment;
//        }
//
//        SegmentObj->nextSegment = _state->mGmt.unusedSegment;
//        _state->mGmt.unusedSegment = SegmentObj;
//
//
//        Segment_map_mark_used(_state, SegmentObj->address, 0);
//
//        SegmentObj->address = 0;
//        --_state->mGmt.numberSegmentCurrentlyAllocate;
//
//        return;
//    }
//
//    if (numberFree == 1) {
//        SegmentObj->nextSegment = _state->mGmt.usableSegment;
//        SegmentObj->prevSegment = NULL;
//        if (_state->mGmt.usableSegment) {
//            _state->mGmt.usableSegment->prevSegment = SegmentObj;
//        }
//        _state->mGmt.usableSegment = SegmentObj;
//        if (_state->mGmt.numberfreeAlignment[1] == NULL) {
//            _state->mGmt.numberfreeAlignment[1] = SegmentObj;
//        }
//        return;
//    }
//
//
//    if (_state->mGmt.numberfreeAlignment[numberFree] == NULL) {
//        _state->mGmt.numberfreeAlignment[numberFree] = SegmentObj;
//    }
//
//    if (SegmentObj == lastNumberFree) {
//        return;
//    }
//
//    if (SegmentObj->prevSegment != NULL) {
//        SegmentObj->prevSegment->nextSegment = SegmentObj->nextSegment;
//    }
//    else {
//        _state->mGmt.usableSegment = SegmentObj->nextSegment;
//    }
//
//    SegmentObj->nextSegment->prevSegment = SegmentObj->prevSegment;
//
//    SegmentObj->prevSegment = lastNumberFree;
//    SegmentObj->nextSegment = lastNumberFree->nextSegment;
//    if (SegmentObj->nextSegment != NULL) {
//        SegmentObj->nextSegment->prevSegment = SegmentObj;
//    }
//    lastNumberFree->nextSegment = SegmentObj;
//
//}
//
//bool address_in_range(MemoryState* _state, void* ptr) {
//    return Segment_map_is_used(_state, (uint8_t*)ptr);
//}
//
//int malloc_free(MemoryState* _state, void* ptr) {
//
//    AlignmentHeader* Alignment = ((AlignmentHeader*)((void*)((uintptr_t)((ptr)) & ~(uintptr_t)(((1 << 14)) - 1))));
//
//    if (!address_in_range(_state, ptr)) {
//        return 0;
//    }
//
//    uint8_t* lastFree = Alignment->freeBlock;
//    *(uint8_t**)ptr = lastFree;
//    Alignment->freeBlock = (uint8_t*)ptr;
//    Alignment->ref.count--;
//
//    if (lastFree == NULL) {
//        insert_to_used_Alignment(_state, Alignment);
//        return 1;
//    }
//
//    if (Alignment->ref.count != 0) {
//        return 1;
//    }
//
//    insert_to_free_Alignment(_state, Alignment);
//    return 1;
//
//}
//
//void object_raw_free(MemoryState* _state, void* ptr) {
//
//    objectOrMemDelete = true;
//
//    mem_debug_raw_free(_state, ptr);
//}
//
//void object_free(MemoryState* _state, void* ptr) {
//
//
//
//    if (ptr == NULL) {
//        return;
//    }
//
//    if (!malloc_free(_state, ptr)) {
//        mem_debug_raw_free(_state, ptr);
//        _state->mGmt.rawAllocatedBlocks--;
//    }
//
//}
//
//void* mem_debug_raw_alloc(MemoryState* _state, size_t useCalloc, size_t nByte) {
//
//    uint8_t* p;
//    uint8_t* data;
//    uint8_t* tail;
//    size_t total;
//
//    total = nByte + 24;
//
//    if (useCalloc) {
//        if (rawOrMem) {
//            p = (uint8_t*)object_calloc(_state, 1, total);
//        }
//        else {
//            p = (uint8_t*)mem_raw_calloc(1, total);
//        }
//
//    }
//    else {
//
//        if (rawOrMem) {
//            p = (uint8_t*)object_malloc(_state, total);
//        }
//        else {
//            p = (uint8_t*)mem_raw_malloc(total);
//        }
//
//    }
//    if (p == NULL) {
//        return NULL;
//    }
//    data = p + 2 * 8;
//
//    write_size_t(p, nByte);
//
//    memset(p + 8 + 1, 0xFD, 8 - 1);
//
//    if (nByte > 0 && !useCalloc) {
//        memset(data, 0xCD, nByte);
//    }
//
//    tail = data + nByte;
//    memset(tail, 0xFD, 8);
//
//    return data;
//}
//
//void mem_debug_raw_free(MemoryState* _state, void* ptr) {
//
//    if (ptr == NULL) {
//        return;
//    }
//
//    uint8_t* q = (uint8_t*)ptr - 2 * 8;
//    size_t numberByte;
//    numberByte = read_size_t(q);
//    numberByte += 24;
//    memset(q, 0xDD, numberByte);
//    if (objectOrMemDelete) {
//        object_free(_state, q);
//    }
//    else {
//        mem_raw_free(q);
//    }
//}
//
//void* mem_debug_raw_realloc(MemoryState* _state, void* ptr, size_t nByte) {
//
//    if (ptr == NULL) {
//        return mem_debug_raw_alloc(_state, 0, nByte);
//    }
//
//    uint8_t* head;
//    uint8_t* data;
//    uint8_t* r;
//    uint8_t* tail;
//    size_t total;
//    size_t originalNByte;
//    uint8_t save[2 * 64];
//
//    data = (uint8_t*)ptr;
//    head = data - 2 * 8;
//    originalNByte = read_size_t(head);
//    total = nByte + 24;
//
//    tail = data + originalNByte;
//    if (originalNByte <= sizeof(save)) {
//        memcpy(save, data, originalNByte);
//        memset(data - 2 * 8, 0xDD, originalNByte + 24);
//    }
//    else {
//        memcpy(save, data, 64);
//        memset(head, 0xDD, 64 + 2 * 8);
//        memcpy(&save[2 * 64], tail - 2 * 64, 2 * 64);
//        memset(tail - 64, 0xDD, 64 + 24 - 2 * 8);
//    }
//    r = (uint8_t*)mem_raw_realloc(head, total);
//
//    if (r == NULL) {
//        nByte = originalNByte;
//    }
//    else {
//        head = r;
//    }
//
//    data = head + 2 * 8;
//
//    write_size_t(head, nByte);
//    memset(head + 8 + 1, 0xFD, 8 - 1);
//
//    if (originalNByte <= sizeof(save)) {
//        memcpy(data, save, nByte > originalNByte ? originalNByte : nByte);
//    }
//    else {
//        size_t i = originalNByte - 64;
//        memcpy(data, save, nByte > 64 ? 64 : nByte);
//        if (nByte > i) {
//            memcpy(data + i, &save[64], nByte - i > 64 ? 64 : nByte);
//        }
//    }
//
//    if (r == NULL) {
//        return NULL;
//    }
//    if (nByte > originalNByte) {
//        memset(data + originalNByte, 0xCD, nByte - originalNByte);
//    }
//
//    return data;
//
//}
//
//SegmentObject* new_Segment(MemoryState* _state) {
//
//
//    SegmentObject* SegmentObj = NULL;
//    unsigned int excess;
//    void* address = nullptr;
//
//    if ((_state)->mGmt.unusedSegment == NULL) {
//
//        unsigned int i;
//        unsigned int numberSegment;
//        size_t nByte;
//
//        numberSegment = (_state)->mGmt.maxSegment ? (_state)->mGmt.maxSegment << 1 : 16;
//
//        nByte = numberSegment * sizeof(*(_state)->mGmt.Segments);
//        SegmentObj = (SegmentObject*)raw_realloc(_state, _state->mGmt.Segments, nByte);
//        (_state)->mGmt.Segments = SegmentObj;
//
//        for (i = (_state)->mGmt.maxSegment; i < numberSegment; ++i)
//        {
//            (_state)->mGmt.Segments[i].address = 0;
//            (_state)->mGmt.Segments[i].nextSegment = i < numberSegment - 1 ? &(_state)->mGmt.Segments[i + 1] : NULL;
//
//        }
//
//        (_state)->mGmt.unusedSegment = &(_state)->mGmt.Segments[(_state)->mGmt.maxSegment];
//        (_state)->mGmt.maxSegment = numberSegment;
//    }
//
//    SegmentObj = _state->mGmt.unusedSegment;
//    address = (void*)VirtualAlloc(NULL, (1 << 20), 0x00001000 | 0x00002000, 0x04);
//
//    if (address != nullptr) {
//        Segment_map_mark_used(_state, (uintptr_t)address, 1);
//    }
//
//    if (address == NULL) {
//        SegmentObj->nextSegment = (_state)->mGmt.unusedSegment;
//        (_state)->mGmt.unusedSegment = SegmentObj;
//        return NULL;
//    }
//
//    SegmentObj->address = (uintptr_t)address;
//
//    ++(_state)->mGmt.numberSegmentCurrentlyAllocate;
//    ++(_state)->mGmt.numberSegmentAllocated;
//    if ((_state)->mGmt.numberSegmentCurrentlyAllocate > (_state)->mGmt.numberSegmentHighWater) {
//        (_state)->mGmt.numberSegmentHighWater = (_state)->mGmt.numberSegmentCurrentlyAllocate;
//    }
//    SegmentObj->freeAlignments = NULL;
//
//
//    SegmentObj->alignmentAddress = (uint8_t*)SegmentObj->address;
//    SegmentObj->numberFreeAlignments = ((1 << 20) / (1 << 14));
//    excess = (unsigned int)(SegmentObj->address & ((1 << 14) - 1));
//    if (excess != 0) {
//        --SegmentObj->numberFreeAlignments;
//        SegmentObj->alignmentAddress += ((1 << 14) - excess);
//    }
//    SegmentObj->numberTotalAlignments = SegmentObj->numberFreeAlignments;
//    return SegmentObj;
//
//}
//
//void malloc_Alignment_extend(AlignmentHeader* Alignment, unsigned int size) {
//
//    if (Alignment->nextOffset <= Alignment->maxNextOffset) {
//
//        Alignment->freeBlock = (uint8_t*)Alignment + Alignment->nextOffset;
//        Alignment->nextOffset += (((unsigned int)(size)+1) << 4);
//        *(uint8_t**)(Alignment->freeBlock) = NULL;
//        return;
//    }
//
//    AlignmentHeader* next;
//    next = Alignment->nextAlignment;
//    Alignment = Alignment->prevAlignment;
//    next->prevAlignment = Alignment;
//    Alignment->nextAlignment = next;
//
//}
//
//void* allocate_from_new_Alignment(MemoryState* _state, unsigned int size) {
//
//    if ((_state)->mGmt.usableSegment == NULL) {
//
//        (_state)->mGmt.usableSegment = new_Segment(_state);
//        if ((_state)->mGmt.usableSegment == NULL) {
//            return NULL;
//        }
//        (_state)->mGmt.usableSegment->nextSegment = (_state)->mGmt.usableSegment->prevSegment = NULL;
//        (_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableSegment->numberFreeAlignments] = (_state)->mGmt.usableSegment;
//    }
//    if ((_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableSegment->numberFreeAlignments] == (_state)->mGmt.usableSegment) {
//        (_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableSegment->numberFreeAlignments] = NULL;
//    }
//    if ((_state)->mGmt.usableSegment->numberFreeAlignments > 1) {
//        (_state)->mGmt.numberfreeAlignment[(_state)->mGmt.usableSegment->numberFreeAlignments - 1] = (_state)->mGmt.usableSegment;
//    }
//
//    AlignmentHeader* Alignment = (_state)->mGmt.usableSegment->freeAlignments;
//    if (Alignment != NULL) {
//        (_state)->mGmt.usableSegment->freeAlignments = Alignment->nextAlignment;
//        (_state)->mGmt.usableSegment->numberFreeAlignments--;
//        if ((_state)->mGmt.usableSegment->numberFreeAlignments == 0) {
//            (_state)->mGmt.usableSegment = (_state)->mGmt.usableSegment->nextSegment;
//            if ((_state)->mGmt.usableSegment != NULL) {
//                (_state)->mGmt.usableSegment->prevSegment = NULL;
//            }
//        }
//    }
//    else {
//        Alignment = (AlignmentHeader*)_state->mGmt.usableSegment->alignmentAddress;
//        Alignment->SegmentIndex = (unsigned int)(_state->mGmt.usableSegment - _state->mGmt.Segments);
//        Alignment->sizeIndex = 0xffff;
//        (_state)->mGmt.usableSegment->alignmentAddress += (1 << 14);
//        --(_state)->mGmt.usableSegment->numberFreeAlignments;
//
//        if ((_state)->mGmt.usableSegment->numberFreeAlignments == 0) {
//            (_state)->mGmt.usableSegment = (_state)->mGmt.usableSegment->nextSegment;
//            if ((_state)->mGmt.usableSegment != NULL) {
//                (_state)->mGmt.usableSegment->prevSegment = NULL;
//            }
//        }
//    }
//
//    uint8_t* bp;
//    AlignmentHeader* next = (_state)->alignments.used[size + size];
//    Alignment->nextAlignment = next;
//    Alignment->prevAlignment = next;
//    next->nextAlignment = Alignment;
//    next->prevAlignment = Alignment;
//    Alignment->ref.count = 1;
//    if (Alignment->sizeIndex == size) {
//        bp = Alignment->freeBlock;
//        Alignment->freeBlock = *(uint8_t**)bp;
//        return bp;
//    }
//
//    Alignment->sizeIndex = size;
//    size = (((unsigned int)(size)+1) << 4);
//    bp = (uint8_t*)Alignment + SIZE_ROUND_UP(sizeof(AlignmentHeader), 16));
//    Alignment->nextOffset = SIZE_ROUND_UP(sizeof(AlignmentHeader), 16)) + (size << 1);
//    Alignment->maxNextOffset = (1 << 14) - size;
//    Alignment->freeBlock = bp + size;
//    *(uint8_t**)(Alignment->freeBlock) = NULL;
//    return bp;
//
//}
//
//void* malloc_alloc(MemoryState* _state, size_t nByte) {
//
//    unsigned int size = (nByte - 1) >> 4;
//    AlignmentHeader* Alignment = _state->alignments.used[size + size];
//    uint8_t* bp;
//
//    if (Alignment != Alignment->nextAlignment) {
//
//        ++Alignment->ref.count;
//        bp = Alignment->freeBlock;
//
//        if ((Alignment->freeBlock = *(uint8_t**)bp) == NULL) {
//            malloc_Alignment_extend(Alignment, size);
//        }
//
//    }
//    else {
//        bp = (uint8_t*)allocate_from_new_Alignment(_state, size);
//    }
//    return (void*)bp;
//}
//
//void* object_malloc(MemoryState* _state, size_t numberByte) {
//
//    void* ptr = malloc_alloc(_state, numberByte);
//    if (ptr != NULL) {
//        return ptr;
//    }
//    ptr = mem_raw_realloc(ptr, numberByte);
//    if (ptr != NULL) {
//        (&state)->mGmt.rawAllocatedBlocks++;
//    }
//    return ptr;
//
//}
//
//void* object_calloc(MemoryState* _state, size_t nElement, size_t elSize) {
//
//    size_t numberByte = nElement * elSize;
//
//    void* ptr = malloc_alloc(_state, numberByte);
//    if (ptr != NULL) {
//        memset(ptr, 0, numberByte);
//        return ptr;
//    }
//
//    ptr = mem_debug_raw_alloc(_state, 1, nElement * elSize);
//    if (ptr != NULL) {
//        _state->mGmt.rawAllocatedBlocks++;
//    }
//    return ptr;
//
//}
//
//int malloc_realloc(MemoryState* _state, void** newPtr, void* ptr, size_t numberByte) {
//
//    void* bp;
//    AlignmentHeader* Alignment;
//    size_t size;
//
//    Alignment = ((AlignmentHeader*)((void*)((uintptr_t)((ptr)) & ~(uintptr_t)(((1 << 14)) - 1))));
//
//    if (!address_in_range(_state, ptr)) {
//        return 0;
//    }
//
//    size = (((unsigned int)(Alignment->sizeIndex) + 1) << 4);
//
//    if (numberByte <= size) {
//        if (4 * numberByte > 3 * size) {
//            *newPtr = ptr;
//            return 1;
//        }
//        size = numberByte;
//    }
//
//    bp = object_malloc(_state, numberByte);
//
//    if (bp != NULL) {
//        memcpy(bp, ptr, size);
//        object_free(_state, ptr);
//    }
//    *newPtr = bp;
//    return 1;
//}
//
//void* object_realloc(MemoryState* _state, void* ptr, size_t numberByte) {
//
//    void* ptr2;
//    if (ptr == NULL) {
//        object_malloc(_state, numberByte);
//    }
//
//    if (malloc_realloc(_state, &ptr2, ptr, numberByte)) {
//        return ptr2;
//    }
//    return mem_debug_raw_realloc(_state, ptr, numberByte);
//
//}
