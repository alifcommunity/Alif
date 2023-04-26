#pragma once

#include <iostream>
#include <vector>

/*
الكتلة: هي الجزء الاكبر وهي الذاكرة الافتراضية نفسها
القطعة: هي الجزء الذي يتم حجزه وهنا يكون بحجم 8192 بايت ويتم إسناد عنوان اول بايت فيها في متغير segments_
واول اربعة بايت فيها مخصصة لعد الوحدات
الوحدة: هي الكمية المحجوزة في القطعة سواء كانت 64 بايت او اكثر او اقل
*/


class AlifMemory {
private:
    std::vector<wchar_t*> segments_;
    wchar_t* currentSegment;
    size_t segmentSize;
    size_t currentIndex;
    int* fragmentCounts;

public:
    AlifMemory(size_t _segmentSize = 128); // تتبع التعقيد الزمني لوغارتم(ت) ا

    void* allocate(size_t);
    void deallocate(void*);
};
