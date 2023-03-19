#include "MemoryBlock.h"


MemoryBlock::MemoryBlock(size_t _segmentSize)
    : segmentSize(_segmentSize), currentIndex(0)
{
    currentSegment = new char[segmentSize](0); // احجز المساحة الممررة وهي 8192 بايت مع قيمة افتراضية 0
    segments_.push_back(currentSegment); // قم بإضافة عنوان القطعة الحالية الى مصفوفة القطع
    fragmentCounts = (int*)currentSegment; // قم بحجز اول اربع بايت ك متغير عدد صحيح ليستخدم فيما بعد لعد الوحدات التي سيتم حجزها في هذه القطعة
    *fragmentCounts = 0; // تأكد ان قيمة الوحدات المحجوزة في القطع هو 0
    currentIndex += 4; // قم بالتقدم بمقدار اربع بايت لتفادي المساحة التي حجزت للعدد الذي سيعد الوحدات في هذه القطعة
}

void* MemoryBlock::allocate(size_t _size)
{
    /*
        للتاكد من ان حجم _size
        يساوي عدد صحيح من مضاعفات العدد 2
        مثال:
        في حال تم حجز 17 بايت
        ستقوم هذه العملية بالتحقق حيث ستقوم بحجز 32 بايت في الذاكرة - 16 للنصف الاول و 16 للواحد المتبقي -ا
    */
    _size = (_size + 15) & ~15;

    if (currentIndex + _size > segmentSize) // هل يوجد مساحة كافية في القطعة؟
    {
        for (char* seg : segments_) // لاجل كل عنوان من عناوين القطع في مصفوفة القطع
        {
            if (*seg <= 0) // *seg هي قيمة العنوان الاول وهو عدد الوحدات في هذه القطعة
            {
                currentSegment = seg; // قم بإسناد عنوان القطعة في العنوان الحالي الذي يتم التعامل معه الان
                fragmentCounts = (int*)currentSegment; // قم بحجز اول اربع بايتات لعداد الوحدات
                currentIndex = 4; // قم بإضافة اربعة الى محدد العنوان الحالي لتفادي الحجز مكان عداد الوحدات
                goto returnPtr; // اذهب مباشرة الى خطوات ارجاع العنوان
            }
        }
        currentSegment = new char[segmentSize](0); // قم بحجز قطعة جديدة مع قيمة افتراضية 0
        segments_.push_back(currentSegment); // قم بإضافة اول عنوان من القطعة الى مصفوفة القطع
        currentIndex = 0; // تاكد ان المؤشر الحالي يؤشر الى العنوان الاول من القطعة
        fragmentCounts = (int*)currentSegment; // قم بحجز اربع بايت خاصة بعداد الوحدات
        *fragmentCounts = 0; // تاكد ان عدد الوحدات المحجوزة هو صفر
        currentIndex += 4; // تخطى عداد الوحدات
    }

    returnPtr:
        void* ptr_ = currentSegment + currentIndex; // قم بضبط العنوان الى عنوان المتغير القطعة الحالية مضاف اليها مزيح العناوين
        currentIndex += _size; // قم بإزاحة المؤشر بمقدار الحجم الذي تم حجزه
        *fragmentCounts += 1; // قم بإضافة واحد الى عداد الوحدات لانه تم حجز وحدة جديدة
        return ptr_; // ارجع عنوان المكان الذي تم حجزه
}

void MemoryBlock::deallocate(void* _ptr) 
{
    for (char* seg : segments_) // لاجل كل عنوان من عناوين القطع في مصفوفة القطع
    {
        if (_ptr < seg + segmentSize) // اذا كان العنوان الممرر ضمن القطعة الحالية
        {
            *seg -= 1; // قم بإنقاص عداد الوحدات
            break;
        }
    }

}

