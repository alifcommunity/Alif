#ifndef _WIN64

void file_run(char* _fileName) {

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
    }
    fileContent.close();

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

    // المعرب اللغوي
    /////////////////////////////////////////////////////////////////

    Lexer lexer(fileName, input_);
    lexer.make_token();

    // المحلل اللغوي
    /////////////////////////////////////////////////////////////////

    Parser parser = Parser(&lexer.tokens_, fileName, input_);
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

        if (input_ == L"خروج")
        {
            exit(0);
        }

        // المعرب اللغوي
        /////////////////////////////////////////////////////////////////

        Lexer lexer(fileName, input_);
        lexer.make_token();

        // المحلل اللغوي
        /////////////////////////////////////////////////////////////////

        Parser parser = Parser(&lexer.tokens_, fileName, input_);
        parser.parse_terminal();

        // std::wcin.ignore(); // لمنع ارسال قيمة فارغة في المتغير input_

    }
}

#else

std::wstring utf8_decode(const std::string& str)
{
/*
    تحويل utf-8 
    الى utf-16
    لكي يقوم بحساب الحرف الواحد على انه 2 بايت وليس 1 بايت
    حيث نظام utf-8 يأخذ اول بايت من الحرف فقط ويترك البايت الثاني
    ** يجب مراجعة جدول بايتات نظام utf-8
*/
    int size_ = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring strToWstr(size_, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &strToWstr[0], size_);
    return strToWstr;
}

void file_run(wchar_t* _fileName) {

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
    }
    fileContent.close();

    input_ = utf8_decode(u8input);

    // المعرب اللغوي
    /////////////////////////////////////////////////////////////////

    //Lexer lexer(_fileName, input_);
    //lexer.make_token();

    // المحلل اللغوي
    /////////////////////////////////////////////////////////////////

    //Parser parser = Parser(&lexer.tokens_, _fileName, input_);
    //parser.parse_file();
}

void terminal_run() {

    wstr fileName = L"<طرفية>";
    const wstr about_ = L"ألف نـ5.0.0";
    wstr input_;
    PRINT_(about_);

    while (true) {

        std::wcout << L"ألف -> ";
        std::getline(std::wcin, input_);

        if (input_ == L"خروج")
        {
            exit(0);
        }

        // المعرب اللغوي
        /////////////////////////////////////////////////////////////////

        Lexer lexer(fileName, input_);
        //lexer.make_token();

        // المحلل اللغوي
        /////////////////////////////////////////////////////////////////

        //Parser parser = Parser(&lexer.tokens_, fileName, input_);
        //parser.parse_terminal();

    }
}
#endif // !_WIN64