#include <iostream>
#include <chrono>
#include <stack>

using namespace std;

template <typename T>
class AlifStack {
public:
    
    AlifStack(const size_t _size = 131072) : size_(_size) {}

    //const T& operator[](size_t _index)
    //{
    //    return data_[_index];
    //}

    //void operator=(size_t _value)
    //{
    //    data_[index_] = _value;
    //    index_++;
    //}

    void push(T _value)
    {
        data_[index_++] = _value;
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

private:
    unsigned int index_ = 0;
    size_t size_;
    T* data_ = new T[size_];
};




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

    // Move constructor
    AlifArray(AlifArray&& other) noexcept : size_(other.size_), capacity_(other.capacity_), data_(other.data_) {
        other.size_ = 0;
        other.capacity_ = 0;
        other.data_ = nullptr;
    }

    // Move assignment operator
    AlifArray& operator=(AlifArray&& other) noexcept {
        if (this != &other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = other.data_;
            other.size_ = 0;
            other.capacity_ = 0;
            other.data_ = nullptr;
        }
        return *this;
    }

    // Getters and setters
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }

    void push_back(const T& value) {
        if (size_ >= capacity_) {
            reserve(capacity_ == 0 ? 1 : 2 * capacity_);
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

    //void resize(size_t new_size, const T& value = T()) {
    //    if (new_size > size_) {
    //        reserve(new_size);
    //        for (size_t i = size_; i < new_size; i++) {
    //            data_[i] = value;
    //        }
    //    }
    //    size_ = new_size;
    //}

    void clear() {
        size_ = 0;
    }

    // Iterator support
    //T* begin() { return data_; }
    //const T* begin() const { return data_; }
    //T* end() { return data_ + size_; }
    //const T* end() const { return data_ + size_; }

    // Swap function for copy and move operations
    //void swap(Vector& other) {
    //    std::swap(size_, other.size_);
    //    std::swap(capacity_, other.capacity_);
    //    std::swap(data_, other.data_);
    //}

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};



int main()
{
    //AlifStack<int> stack_ = AlifStack<int>(4);
    //stack<int> stack2_;

    AlifArray<int> alifArr;

    
    for (int i = 0; i < 1000; i++)
    {
        alifArr.push_back(9);
    //    stack_.push(10);
    //    //stack2_.push(10);
    //    //stack_ = 10;
    //    //int a = stack_[i];

    }

    auto start = chrono::steady_clock::now();


    for (int i = 0; i < 1000000000; i++)
    {
        int a = alifArr[999];

    }

    //stack_.push(9);
    //stack_.push(3);
    //stack_.push(7);

    //int c = stack_.pop();

    //stack_.swap();

    //int d = stack_.pop();

    //stack_.push(10);

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << elapsed_seconds.count() << endl;

    int s;
    cin >> s;
    return 0;
}
