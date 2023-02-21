#define NUM long double
#define STR std::wstring
#define prnt(a){std::wcout << a << std::endl;}

// استيراد
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include<math.h>
#include<algorithm> // لعمل تتالي على المصفوفات

#ifndef _WIN64
#include<codecvt>
#include<locale>
#else
#include<Windows.h>
#include<fcntl.h> //لقبول ادخال الاحرف العربية من الكونسل
#include<io.h> //لقبول ادخال الاحرف العربية من الكونسل
#endif

#include "Constants.h"
#include "Position.h"
#include "Tokens.h"
#include "Errors.h"
#include "Lexer.h"
#include "Parser.h"
#include "AlifRun.h"

#ifndef _WIN64
int main(int argc, char** argv)
{
    setlocale(LC_ALL, "en_US.UTF-8");
    if (argc > 1)
    {
        if (argc > 2)
        {
            prnt(L"يجب ان يتم تمرير اسم الملف فقط");
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
    bool outWText = _setmode(_fileno(stdout), _O_WTEXT);
    bool inWText = _setmode(_fileno(stdin), _O_WTEXT);

    if (!outWText and !inWText)
    {
        prnt(L"لم يتمكن من تحميل طباعة الملفات عريضة الاحرف - الملف Alif5.cpp");
    }

    if (argc > 1)
    {
        if (argc > 2)
        {
            prnt(L"يجب ان يتم تمرير اسم الملف فقط");
            exit(-1);
        }

        file_run(argv[1]);
    }
    else
    {
        terminal_run();
    }
}

#endif // !_WIN64
