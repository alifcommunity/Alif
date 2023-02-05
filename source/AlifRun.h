void file_run(char* _fileName) {

    bool outWText = _setmode(_fileno(stdout), _O_WTEXT);
    bool inWText = _setmode(_fileno(stdin), _O_WTEXT);

    if (!outWText and !inWText)
    {
        prnt(L"لم يتمكن من تحميل طباعة الملفات عريضة الاحرف - الملف Alif5.cpp");
    }

    STR input_;
    STR line;

    std::wifstream fileContent(_fileName);
    //std::wifstream fileContent(L"../source/AlifCode.alif5"); // للتجربة فقط
    fileContent.imbue(std::locale("ar_SA.UTF-8"));

    while (std::getline(fileContent, line))
    {
        if (line != L"")
        {
            input_ += line;
            input_ += L"\n";
        }
    }
    fileContent.close();

    int fileNameLength = sizeof(_fileName) / sizeof(char) + 6; // حساب طول اسم الملف لنتمكن من تحويله الى نص ذو احرف عريضة الترميز
    std::wstring fileName__(&_fileName[0], &_fileName[fileNameLength]); // تحويل اسم الملف الى نص ذو احرف عريضة الترميز
    STR fileName = fileName__;
    //STR fileName = L"fileName__"; // للتجربة فقط

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