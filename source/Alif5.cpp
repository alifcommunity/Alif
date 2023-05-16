#define PRINT_(a){std::wcout << a << std::endl;}

#include<iostream>
#include<string>

#ifndef _WIN32
#include<codecvt>
#include<locale>
#else
#include<Windows.h>
#include<fcntl.h> //لقبول ادخال الاحرف العربية من الكونسل
#include<io.h> //لقبول ادخال الاحرف العربية من الكونسل
#endif

#include "AlifRun.h"
#include "initConfig.h"

#ifndef _WIN32
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

int wmain(int argc, wchar_t** argv)
{
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

    //if (argc > 1)
    if (argc > 0)
    {
        if (argc > 2)
        {
            PRINT_(L"يجب ان يتم تمرير اسم الملف فقط");
            exit(-1);
        }
      
        //file_run(argv[1]);
        file_run(L"../code.alif5");

    }
    else
    {
        terminal_run();
    }
}

#endif
