#include "MemoryBlock.h"


MemoryBlock::MemoryBlock(size_t _segmentSize)
    : segmentSize(_segmentSize), currentIndex(0)
{
    currentSegment = new char[segmentSize]; // احجز القيمة الافتراضية الممررة وهي 8192 بايت
    segments_.push_back(currentSegment);
    segCounts = (int*)(currentSegment + currentIndex);
    *segCounts = 0;
    currentIndex = 4;
}

void* MemoryBlock::allocate(size_t _size)
{
    /*
        لعمل إزاحة للذاكرة بمقدار ثابت 
        مثال:
        في حال تم حجز 17 بايت
        ستقوم هذه العملية بالتحقق حيث ستقوم بحجز 32 بايت في الذاكرة - 16 للنصف الاول و 16 للواحد المتبقي -ا
    */
    //_size = (_size + 15) & ~15;
    if (currentIndex + _size > segmentSize)
    {
        currentSegment = new char[segmentSize];
        segments_.push_back(currentSegment);
        currentIndex = 0;
        segCounts = (int*)(currentSegment + currentIndex);
        *segCounts = 0;
        currentIndex = 4;
    }


    void* ptr_ = currentSegment + currentIndex;
    currentIndex += _size;
    *segCounts += 1;
    return ptr_;
}

void MemoryBlock::deallocate(void* _ptr) 
{
    for (char* seg : segments_)
    {
        if (seg <= _ptr and _ptr < seg + segmentSize)
        {
            *seg -= 1;
            if (*seg <= 0)
            {
                currentSegment = seg;
                currentIndex = 4;
                break;
            }
        }



        //if (it->first <= _ptr)
        //{
        //    it++;
        //    if (it != segments_.end())
        //    {
        //        if (it->first > _ptr)
        //        { 
        //            it--;
        //            it->second--;

        //            if (it->second == 0)
        //            {
        //                segments_.erase(it);
        //                currentIndex = 0;
        //                break;
        //            }
        //            break;
        //        }
        //    }
        //    else
        //    {
        //        it--;
        //        it->second--;

        //        if (it->second == 0)
        //        {
        //            segments_.erase(it);
        //            currentIndex = 0;
        //            break;
        //        }
        //        break;
        //    }
        //}
    }

}

