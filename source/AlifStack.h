#pragma once

#include<iostream>
#define PRINT_(a){std::wcout << a << std::endl;}

template <typename T>
class AlifStack {
public:
    unsigned int index_ = 0;

    AlifStack(const size_t _size = 131072) : size_(_size) {}

    void push(T _value)
    {
        data_[index_++] = _value;

        if (index_ > size_)
        {
            PRINT_(L"نفدت ذاكرة المكدس");
            exit(-1);
        }
    }

    T pop()
    {
        return data_[--index_];
    }

    void swap()
    {
        T a = data_[--index_];
        T b = data_[--index_];
        data_[index_++] = a;
        data_[index_++] = b;
    }

    void reset()
    {
        index_ = 0;
    }

    T get() { 
        index_--;
        return data_[index_++]; 
    } // للمراجعة**

private:
    size_t size_;
    T* data_ = new T[size_];
};