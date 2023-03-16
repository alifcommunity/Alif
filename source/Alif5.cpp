#define PRINT_(a){std::wcout << a << std::endl;}

#include<iostream>
#include<string>

#ifndef _WIN64
#include<codecvt>
#include<locale>
#else
#include<Windows.h>
#include<fcntl.h> //لقبول ادخال الاحرف العربية من الكونسل
#include<io.h> //لقبول ادخال الاحرف العربية من الكونسل
#endif

#include "AlifRun.h"

#ifndef _WIN64
int main(int argc, char** argv)
{
    setlocale(LC_ALL, "en_US.UTF-8");

    if (argc > 1)
    {
        if (argc > 2)
        {
            PRINT_(L"يجب ان يتم تمرير اسم الملف فقط");
            exit(-1);
        }

        file_run(argv[1]);
    }
    else
    {
        terminal_run();
    }
}
#else
#include "MemoryBlock.h"
int wmain(int argc, const wchar_t** argv)
{
    MemoryBlock mb(32);
    char* a = (char*)mb.allocate(sizeof(char[2]) + 1);

    for (int i = 0; i < strlen("97"); i++)
    {
        (a)[i] = "97"[i];
    }
    a[strlen("97")] = '\0';
    std::cout << (const char*)a << std::endl;

    int* b = (int*)mb.allocate(sizeof(int));
    *b = 7;

    mb.deallocate(a, strlen(a) + 1);

    int* c = (int*)mb.allocate(sizeof(int));
    *c = 5;

    std::cout << (const char*)a << std::endl;

    a = nullptr;
    b = nullptr;
    exit(0);

    /*
        _setmode
        تسمح للطرفية في نظام ويندوز بقراءة وكتابة الاحرف عريضة الترميز
    */
    bool outWText = _setmode(_fileno(stdout), _O_WTEXT);
    bool inWText = _setmode(_fileno(stdin), _O_WTEXT);

    if (!outWText and !inWText)
    {
        PRINT_(L"لم يتمكن من تحميل طباعة الملفات عريضة الاحرف - الملف Alif5.cpp");
    }

    if (argc > 1)
    {
        if (argc > 2)
        {
            PRINT_(L"يجب ان يتم تمرير اسم الملف فقط");
            exit(-1);
        }

        file_run(argv[1]);
        //file_run(L"C:/Users/Shadow/Desktop/GitHub/code.alif5");
    }
    else
    {
        terminal_run();
    }
}

#endif // !_WIN64
