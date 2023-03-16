#include "MemoryBlock.h"


MemoryBlock::MemoryBlock(size_t _segmentSize)
    : segmentSize(_segmentSize), currentIndex(0)
{
    currentSegment = new char[segmentSize]; // احجز القيمة الافتراضية الممررة وهي 8192 بايت
    blocks_.push_back(currentSegment);
}

void* MemoryBlock::allocate(size_t _size)
{
    /*
        لعمل إزاحة للذاكرة بمقدار ثابت 
        مثال:
        في حال تم حجز 17 بايت
        ستقوم هذه العملية بالتحقق حيث ستقوم بحجز 32 بايت في الذاكرة - 16 للنصف الاول و 16 للواحد المتبقي -ا
    */
    //_size = (_size + 15) & ~5;

    //if (!freeSegments.empty())
    //{
    //    for (std::pair _ptr : freeSegments)
    //    {
    //        if (_ptr.second >= _size)
    //        {
    //            void* ptr_ = _ptr.first;
    //            freeSegments.erase(_ptr.first);
    //            return ptr_;
    //        }
    //    }
    //}

    if (currentIndex + _size > segmentSize)
    {
        currentSegment = new char[segmentSize];
        blocks_.push_back(currentSegment);
        currentIndex = 0;
    }

    void* ptr_ = currentSegment + currentIndex;
    currentIndex += _size;
    return ptr_;
}

void MemoryBlock::deallocate(void* _ptr, size_t _size) 
{
    freeSegments.insert({ _ptr, _size });
}

