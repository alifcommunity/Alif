
// استيراد
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include<iostream>
#include<fstream>
#include<string>
#include<list>
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
#include "Interpreter.h"

int main()
{
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);

    std::wstring input_;
    VarTaple* varTaple = new VarTaple;
    Interpreter interprete = Interpreter(varTaple);

    //std::wstring file;
    //std::wifstream fileContent("AlifCode.txt");
    //fileContent.imbue(std::locale("ar_SA.UTF-8"));

    while (true) {
        std::wcout << L"alif -> ";
        std::getline(std::wcin, input_);


        if (input_ == L"خروج") {
            exit(0);
        }

        // المعرب اللغوي
        /////////////////////////////////////////////////////////////////

        clock_t start = clock(); // بداية حساب الوقت

        std::wstring fileName = L"الملف_الرئيسي";
        Lexer lexer(fileName, input_);
        lexer.make_token();
        lexer.print();


        // المحلل اللغوي
        /////////////////////////////////////////////////////////////////

        Parser parser = Parser(lexer.tokens);
        parser.parse();
        Node* AST = parser.node;

        parser.print_node(AST);


        // المنفذ
        /////////////////////////////////////////////////////////////////

        interprete.Interpreter_print(AST);

        //Node* res = interprete.visit(AST);
        //std::wcout << res->token.value_ << std::endl;

        std::wcout << float(clock() - start) / CLOCKS_PER_SEC << std::endl; // طباعة نتائج الوقت

    }

    //fileContent.close();
}
