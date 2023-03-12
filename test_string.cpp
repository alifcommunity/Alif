/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <string>
#include <chrono>
#include <string.h>

using namespace std;

const inline bool comp(const wchar_t* a, const wchar_t* b) {
    for (int i = 0; i < 6; i++)
    {
        if (a[i] == b[i])
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}

class Test {
public:
    union {
        char* h;
    }V;

    Test(char* s) {
        this->V.h = s;
    }

};

int main()
{
    string a = "1621212121212";
    string b = "1621212121212";
    // string a = "abcd";
    // uint32_t b = (a.at(0) << 24) | (a.at(1) << 16) | (a.at(2) << 8) | a.at(3) ;
    // uint32_t c[6] = {{(a.at(0) << 24) | (a.at(1) << 16) | (a.at(2) << 8) | a.at(3) },  {(a.at(4) << 24) | (a.at(5) << 16) | (a.at(6) << 8) | a.at(7)},  {(a.at(8) << 24) | (a.at(9) << 16) | (a.at(10) << 8) | a.at(11)}, 0x31323132, 0x31323132, 0x31323132 };
    const wchar_t f[6] = { L'ت', L'ب', L'ب', L'ب', L'ب', L'ب' };
    const wchar_t g[7] = { L'ش', L'ب', L'ب', L'ب', L'ب', L'س', L'ض' };
    char z[2] = { 'a', 'b' };

    // int e = 1;
    // Test* tst = new Test(z);

    // tst->V.h = new char[a.length()];
    // for (int i = 0; i < a.length(); i++)
    // {
    //     tst->V.h[i] = a[i];
    //     cout << (char)tst->V.h[i] << endl;
    // }
    // cout << size(tst->V.h) << endl;

    const char* x = a.c_str();
    const char* v = b.c_str();
    cout << x[0] << endl;

    auto start = chrono::steady_clock::now();

    for (int i = 0; i < 100000000; i++)
    {
        // if (comp(f,g))
        //if (!wcscmp(f,g))
        if (!strcmp(x, v))
        {
             //cout << "tm" << endl;
        }

        // for (uint32_t d : c)
        // for (uint16_t d : f)
        // {
            // if (d == 0x31323132)
            // if (d == L'ب')
            // if (a == "121212121212")
            // if (e == 1)
            // {
                // cout << "tm" << endl;
            // }

        // }
    }

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << elapsed_seconds.count() << endl;

    return 0;
}
