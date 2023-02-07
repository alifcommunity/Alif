#define NUM long double
#define STR std::wstring
#define prnt(a){std::wcout << a << std::endl;}

// استيراد
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include<iostream>
#include<fstream>
#include<codecvt> //
#include<string>
#include<vector>
#include<map>
#include<math.h>
#include<algorithm> // لعمل تتالي على المصفوفات
#include<fcntl.h> //لقبول ادخال الاحرف العربية من الكونسل
#include<io.h> //لقبول ادخال الاحرف العربية من الكونسل
#include "MemoryBlock.h"
#include "Constants.h"
#include "Position.h"
#include "Tokens.h"
#include "Errors.h"
#include "Lexer.h"
#include "Parser.h"
#include "AlifRun.h"

int wmain(int argc, wchar_t* argv[])
{
    if (argc > 1)
    {
        if (argc > 2)
        {
            prnt("يجب ان يتم تمرير اسم الملف فقط");
            exit(-1);
        }

        file_run(argv[1]);
    }
    else
    {
        terminal_run();
    }
}