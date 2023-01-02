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
#include<algorithm> // لعمل تتالي على المصفوفات
#include<fcntl.h> //لقبول ادخال الاحرف العربية من الكونسل
#include<io.h> //لقبول ادخال الاحرف العربية من الكونسل
#include "Constants.h"
#include "Position.h"
#include "Tokens.h"
#include "Errors.h"
#include "Lexer.h"
#include "Parser.h"

int main(int argc, char* argv[])
{

    //bool outWText = _setmode(_fileno(stdout), _O_WTEXT);
    //bool inWText = _setmode(_fileno(stdin), _O_WTEXT);

    //if (not outWText and not inWText)
    //{
    //    std::wcout << L"error" << std::endl;
    //} 

    std::wstring input_;
    std::wstring line;

    std::wifstream fileContent("AlifCode.alif5");
    //std::wifstream fileContent(argv[1]);
    fileContent.imbue(std::locale("ar_SA.UTF-8"));

    // القراءة من ملف
    /////////////////////////////////////////////////////////////////

    while (std::getline(fileContent, line))
    {
        if (line != L"" and line != L"\t")
        {
            input_ += line;
            input_ += L"\n";
        }
    }
    fileContent.close();

    // القراءة من الكونسول
    /////////////////////////////////////////////////////////////////

    //while (true) {
        //std::wcout << L"alif -> ";
        //std::getline(std::wcin, input_);
        
    //std::wcout << input_ << std::endl;


            // المعرب اللغوي
            /////////////////////////////////////////////////////////////////

            std::wstring fileName = L"AlifCode.txt";
            Lexer lexer(fileName, input_);
            lexer.make_token();
            //lexer.print();


            // المحلل اللغوي
            /////////////////////////////////////////////////////////////////
            

            Parser parser = Parser(&lexer.tokens_, fileName, input_);

            clock_t start = clock(); // بداية حساب الوقت
            
            parser.parse();

            std::wcout << float(clock() - start) / CLOCKS_PER_SEC << std::endl; // طباعة نتائج الوقت
            
            for (void* address : lexer.deleteAddresses)
            {
                delete address;
            }

    //}

}


// vectro [2.54, 2.54, 2.42] second in release
// list [2.93, 3.05, 3.15] second in release

// 1598401 line of this ->

//#السلام عليكم
//.
//12345
//اسم
//"نص طويل الاجل"
//ب += 4
//ي -= 2
//234 + 567 - 89 * 1 \ 2
//(12 - 4, 43 * 5)