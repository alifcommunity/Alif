#pragma once

template<typename T>
class AlifArray {
public:

    AlifArray() : size_(0), capacity_(0), data_(nullptr) {}

    // Destructor
    //~AlifArray() {
    //    if (data_ != nullptr) {
    //        delete[] data_;
    //    }
    //}

    //// Move constructor
    //AlifArray(AlifArray&& other) noexcept : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
    //    other.size_ = 0;
    //    other.capacity_ = 0;
    //    other.data_ = nullptr;
    //}

    //// Move assignment operator
    //AlifArray& operator=(AlifArray&& other) noexcept {
    //    if (this != &other) {
    //        size_ = other.size_;
    //        capacity_ = other.capacity_;
    //        data_ = other.data_;
    //        other.size_ = 0;
    //        other.capacity_ = 0;
    //        other.data_ = nullptr;
    //    }
    //    return *this;
    //}

    // Getters and setters
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    //T& operator[](size_t index) { return data_[index]; }
    //const T& operator[](size_t index) const { return data_[index]; }

    T& get(size_t _index) { return data_[_index]; }

    void push_back(const T& value) {
        if (size_ >= capacity_) {
            reserve(capacity_ == 0 ? 16 : 2 * capacity_);
        }
        data_[size_++] = value;
    }

    //void pop_back() {
    //    if (!empty()) {
    //        size_--;
    //    }
    //}

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

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};