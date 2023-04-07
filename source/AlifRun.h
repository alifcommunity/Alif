#include<fstream>
#include<chrono> /////////////////////// for test only


#include "Lexer.h"
#include "Parser.h"
#include "Compiler.h"
#include "Interpreter.h"

#ifndef _WIN64

void file_run(char* _fileName) {

    /*
    تم تعريف متغيرين للمدخلات
    الاول يستقبل الشفرة بترميز utf8
    وهو u8input
    والثاني يستقبل الشفرة بترميز utf16
    وهو input
    ولك بعد عملية تحويل ترميز الشفرة عبر دالة convert.from_bytes()
    */
    wstr input_;
    std::string u8input;
    std::string line;

    std::ifstream fileContent(_fileName);
    if (!fileContent.is_open()) {
        PRINT_(L"لا يمكن فتح الملف او انه غير موجود - تاكد من اسم الملف -");
        exit(-1);
    }

    while (std::getline(fileContent, line))
    {
        if (line != "" and line != "\r")
        {
            u8input += line;
            u8input += "\n";
        }
        else
        {
            u8input += '\n';
        }
    }
    fileContent.close();

    /*
        هنا يجري تحويل fileName
        من char
        الى wchar_t
    */
    int fnLength = sizeof(_fileName) / sizeof(char) + 6;
    wstr fileName(&_fileName[0], &_fileName[fnLength]); // تحويل من char الى wchar_t

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    /*
    تحويل utf-8
    الى utf-16
    لكي يقوم بحساب الحرف الواحد على انه 2 بايت وليس 1 بايت
    حيث نظام utf-8 يأخذ اول بايت من الحرف فقط ويترك البايت الثاني
    ** يجب مراجعة جدول بايتات نظام utf-8
    */
    input_ = convert.from_bytes(u8input);

    input_.shrink_to_fit(); // بعد التأكد ان المدخلات لم تعد قابلة للتغيير يتم عمل تحجيم لها لتصبح كأنها ثابته فيسهل التعامل معها من قبل البرنامج

    // المركب اللغوي
    /////////////////////////////////////////////////////////////////

    Lexer lexer(fileName, &input_);
    lexer.make_token();

    // المحلل اللغوي
    /////////////////////////////////////////////////////////////////

    Parser parser = Parser(&lexer.tokens_, fileName, &input_);
    parser.parse_file();
}

void terminal_run() {

    wstr fileName = L"<طرفية>";

    const wstr about_ = L"ألف نـ5.0.0";
    wstr input_;
    
    PRINT_(about_);

    while (true) {

        std::wcout << L"ألف -> ";
        std::getline(std::wcin, input_);
        input_ += L'\n';

        if (input_ == L"خروج\n")
        {
            exit(0);
        }

        // المركب اللغوي
        /////////////////////////////////////////////////////////////////

        Lexer lexer(fileName, &input_);
        lexer.make_token();

        // المحلل اللغوي
        /////////////////////////////////////////////////////////////////

        Parser parser = Parser(&lexer.tokens_, fileName, &input_);
        parser.parse_terminal();

        // std::wcin.ignore(); // لمنع ارسال قيمة فارغة في المتغير input_ ** يجب إضافة شرط في حال كان المدخل غير فارغ يجب ان يقوم بعمل تجاهل له

    }
}

#else

std::wstring utf8_decode(const std::string& str)
{
    /*
        تحويل utf-8 
        الى utf-16
        لكي يقوم بحساب الحرف الواحد على انه 2 بايت وليس 1 بايت
        حيث نظام utf-8
        يأخذ اول بايت من الحرف فقط ويترك البايت الثاني
        -> يجب مراجعة جدول بايتات نظام utf-8
    */
    int size_ = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring strToWstr(size_, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &strToWstr[0], size_);
    return strToWstr;
}

void file_run(const wchar_t* _fileName) {

    /*
        تم تعريف متغيرين للمدخلات 
        الاول يستقبل الشفرة بترميز utf8
        وهو u8input
        والثاني يستقبل الشفرة بترميز utf16
        وهو input
        ولك بعد عملية تحويل ترميز الشفرة عبر دالة utf8_decode()
    */
    wstr input_;
    std::string u8input;
    std::string line;

    std::ifstream fileContent(_fileName);
    if (!fileContent.is_open()) {
        PRINT_(L"لا يمكن فتح الملف او انه غير موجود - تاكد من اسم الملف -");
        exit(-1);
    }

    while (std::getline(fileContent, line))
    {
        if (line != "")
        {
            u8input += line;
            u8input += "\n";
        }
        else
        {
            u8input += '\n'; // ليتم حساب عدد الاسطر الحقيقي والذي سيتم استعماله في محدد الخطأ
        }
    }
    fileContent.close();

    input_ = utf8_decode(u8input);

    input_.shrink_to_fit();

    // المعرب اللغوي
    /////////////////////////////////////////////////////////////////

    Lexer lexer(_fileName, &input_);
    lexer.make_token();

    // المحلل اللغوي
    /////////////////////////////////////////////////////////////////

    Parser parser = Parser(&lexer.tokens_, _fileName, &input_, &lexer.alifMemory);
    parser.parse_file();

    //// المترجم اللغوي
    ///////////////////////////////////////////////////////////////////

    Compiler compiler = Compiler(&parser.statements_, &lexer.alifMemory);
    compiler.compile_file();

    //// المفسر اللغوي
    ///////////////////////////////////////////////////////////////////

    Interpreter interpreter = Interpreter(&compiler.containers_, &lexer.alifMemory);
    interpreter.run_code();
}

void terminal_run() 
{

    wstr fileName = L"<طرفية>";
    const wstr about_ = L"ألف نـ5.0.0";

    wstr input_;

    PRINT_(about_);

    while (true) {

        std::wcout << L"ألف -> ";

        std::getline(std::wcin, input_);
        input_ += L'\n';

        if (input_ == L"خروج\n")
        {
            exit(0);
        }

        // المعرب اللغوي
        /////////////////////////////////////////////////////////////////

        Lexer lexer(fileName, &input_);
        lexer.make_token();

        // المحلل اللغوي
        /////////////////////////////////////////////////////////////////

        Parser parser = Parser(&lexer.tokens_, fileName, &input_, &lexer.alifMemory);
        parser.parse_terminal();
        
        // المترجم اللغوي
        /////////////////////////////////////////////////////////////////

        Compiler compiler = Compiler(&parser.statements_, &lexer.alifMemory);
        compiler.compile_file();

        // المفسر اللغوي
        /////////////////////////////////////////////////////////////////

        auto start = std::chrono::high_resolution_clock::now();
        
        Interpreter interpreter = Interpreter(&compiler.containers_, &lexer.alifMemory);
        interpreter.run_code();

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed_seconds = end - start;
        std::wcout << elapsed_seconds << std::endl;

        // std::wcin.ignore(); // لمنع ارسال قيمة فارغة في المتغير input_ ** يجب إضافة شرط في حال كان المدخل غير فارغ يجب ان يقوم بعمل تجاهل له
    }
}
#endif