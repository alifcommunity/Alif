void file_run(wchar_t* _fileName) {

    bool outWText = _setmode(_fileno(stdout), _O_WTEXT);
    bool inWText = _setmode(_fileno(stdin), _O_WTEXT);

    if (!outWText and !inWText)
    {
        prnt(L"لم يتمكن من تحميل طباعة الملفات عريضة الاحرف - الملف Alif5.cpp");
    }

    STR input_;
    STR line;

    std::wifstream fileContent(_fileName);
    if (!fileContent.is_open()) {
        prnt(L"لا يمكن فتح الملف او انه غير موجود - تاكد من اسم الملف -");
        exit(-1);
    }
    //std::wifstream fileContent(L"../source/AlifCode.alif5"); // للتجربة فقط
    fileContent.imbue(std::locale("ar_SA.UTF-8"));
    //fileContent.imbue(std::locale(std::locale(""), new std::codecvt_utf8<wchar_t>));

    while (std::getline(fileContent, line))
    {
        if (line != L"")
        {
            input_ += line;
            input_ += L"\n";
        }
    }
    fileContent.close();

    // المعرب اللغوي
    /////////////////////////////////////////////////////////////////

    Lexer lexer(_fileName, input_);
    lexer.make_token();

    // المحلل اللغوي
    /////////////////////////////////////////////////////////////////

    Parser parser = Parser(&lexer.tokens_, _fileName, input_);
    parser.parse_file();
}

void terminal_run() {

    bool outWText = _setmode(_fileno(stdout), _O_WTEXT);
    bool inWText = _setmode(_fileno(stdin), _O_WTEXT);

    if (!outWText and !inWText)
    {
        prnt(L"لم يتمكن من تحميل قراءة الملفات عريضة الاحرف - الملف Alif5.cpp");
    }

    STR fileName = L"<طرفية>";
    STR input_;
    const STR about_ = L"ألف نـ5.0.0";
    prnt(about_);

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

        std::wcin.ignore(); // لمنع ارسال قيمة فارغة في المتغير input_

    }
}