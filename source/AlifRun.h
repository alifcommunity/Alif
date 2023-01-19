void file_run(char* _fileName) {

    bool outWText = _setmode(_fileno(stdout), _O_WTEXT);
    if (!outWText)
    {
        prnt(L"لم يتمكن من تحميل طباعة الملفات عريضة الاحرف - السطر 6 - الملف Alif5.cpp");
    }

    STR input_;
    STR line;

    std::wifstream fileContent(_fileName);
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
        prnt(L"لم يتمكن من تحميل قراءة الملفات عريضة الاحرف - السطر 39 - الملف Alif5.cpp");
    }

    while (true) {
        STR input_;

        std::wcout << L"alif -> ";
        std::getline(std::wcin, input_);

        if (input_ == L"خروج")
        {
            exit(0);
        }

        // المعرب اللغوي
        /////////////////////////////////////////////////////////////////

        STR fileName = L"<طرفية>";
        Lexer lexer(fileName, input_);
        lexer.make_token();

        // المحلل اللغوي
        /////////////////////////////////////////////////////////////////

        Parser parser = Parser(&lexer.tokens_, fileName, input_);
        parser.parse_terminal();
    }
}