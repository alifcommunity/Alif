#pragma once

template<typename T>
class AlifArray 
{
    T* data_;
    size_t size_;
    size_t capacity_;

public:

    AlifArray() : size_(0), capacity_(0), data_(nullptr) {}

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    T& get(size_t _index) { return data_[_index]; }

    void push_back(const T& value) {
        if (size_ >= capacity_) {
            reserve(capacity_ == 0 ? 8 : 2 * capacity_);
        }
        data_[size_++] = value;
    }

    void reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            T* new_data = new T[new_capacity];
            for (size_t i = 0; i < size_; i++) {
                new_data[i] = data_[i];
            }
            delete[] data_;
            data_ = new_data;
            capacity_ = new_capacity;
        }
    }

    void clear() {
        size_ = 0;
    }

    // void fit(); لعمل مصفوفة تناسب عدد العناصر تماماً وذلك لحفظ بعض المساحة
};