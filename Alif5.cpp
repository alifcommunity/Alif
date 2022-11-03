
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
//#include "Interpreter.h"

int main()
{
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);

    std::wstring input_;
    std::wstring line;
    //VarTaple* varTaple = new VarTaple;
    //Interpreter interprete = Interpreter(varTaple);

    std::wifstream fileContent("AlifCode.txt");
    fileContent.imbue(std::locale("ar_SA.UTF-8"));


    // القراءة من ملف
    /////////////////////////////////////////////////////////////////

    while (std::getline(fileContent, line))
    {
        if (line != L"")
        {
            input_ += line;
            input_ += L"\n";
        }
        else
        {
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

        if (input_ == L"خروج") {
            exit(0);
        }

        clock_t start = clock(); // بداية حساب الوقت


        // المعرب اللغوي
        /////////////////////////////////////////////////////////////////

        std::wstring fileName = L"AlifCode.txt";
        Lexer lexer(fileName, input_);
        lexer.make_token();
        if (lexer.error)
        {
            lexer.print();
            exit(0);
        }
        //lexer.print();


        // المحلل اللغوي
        /////////////////////////////////////////////////////////////////

        Parser parser = Parser(lexer.tokens);
        parser.parse();
        //std::shared_ptr<Node> AST = parser.node;
        //parser.print_node(AST);


        // المنفذ
        /////////////////////////////////////////////////////////////////

        //interprete.Interpreter_print(AST);


        std::wcout << float(clock() - start) / CLOCKS_PER_SEC << std::endl; // طباعة نتائج الوقت

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