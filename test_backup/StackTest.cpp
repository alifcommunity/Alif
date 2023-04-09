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


int main()
{
    AlifStack<int> stack_ = AlifStack<int>(4);
    stack<int> stack2_;

    auto start = chrono::steady_clock::now();
    
    //for (int i = 0; i < 100000000; i++)
    //{
    //    stack_.push(10);
    //    //stack2_.push(10);
    //    //stack_ = 10;
    //    //int a = stack_[i];

    //}

    stack_.push(9);
    stack_.push(3);
    stack_.push(7);

    int c = stack_.pop();

    stack_.swap();

    int d = stack_.pop();

    stack_.push(10);

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << elapsed_seconds.count() << endl;

    int s;
    cin >> s;
    return 0;
}
