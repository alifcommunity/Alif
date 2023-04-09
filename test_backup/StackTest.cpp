#include <iostream>

using namespace std;

template <typename T>
class AlifStack {
public:

    AlifStack(const size_t _size) : size_(_size) {}

    const T& operator[](size_t _index)
    {
        return data_[_index];
    }

    void operator=(size_t _value)
    {
        data_[index_] = _value;
        index_++;
    }

    void push()
    {
        
    }


private:
    size_t index_ = 0;
    size_t size_;
    T* data_ = new T[size_];
};


int main()
{
    AlifStack<int> stack_ = AlifStack<int>(10);

    for (int i = 0; i < 10; i++)
    {
        stack_ = 10;
        int a = stack_[i];

    }
    //auto start = chrono::steady_clock::now();


    //auto end = chrono::steady_clock::now();
    //chrono::duration<double> elapsed_seconds = end - start;
    //cout << elapsed_seconds.count() << endl;

    return 0;
}
