#include "Lexer.h"

/*
تم تعريف متغيرات الاسماء خارج الصنف لكي لا يتم إعادة ضبطها عند
استخدام الطرفية في تنفيذ الشفرة او عند استيراد المكتبات
*/
uint32_t name = 0; // متغير اسماء على شكل ارقام
std::map<wstr, int> namesAlter{};


Lexer::Lexer(wstr _fileName, wstr* _input)
{
    this->fileName = _fileName;
    this->input_ = _input;
    this->currentChar = L'\0';
    this->advance();
}

void Lexer::advance()
{

    tokIndex++;
    tokPos++;

    if (this->currentChar == L'\n')
    {
        tokLine++;
        tokPos = 0;
    }

    if (this->tokIndex < this->input_->length())
    {
        this->currentChar = (*this->input_)[this->tokIndex];
    }
    else
    {
        this->currentChar = L'\0';
    }
}

void Lexer::make_token() 
{

        while (this->currentChar != L'\0')
        {
            /*
                يجب مراعاة ترتيب استدعاء الدوال
                لانه في حال استدعاء symbol_lex
                قبل two_sympol_lex
                سيظهر خطأ عند التحقق من المسافات التي في بداية السطر
            */
            if (!this->word_lex())
            {
                wstr detail = L" حرف غير معروف \'";
                detail.push_back(this->currentChar);
                detail += L"\' ";

                PRINT_(SyntaxError(tokPos, tokPos, tokIndex, tokLine, detail, fileName, input_).print_());
                exit(-1);
            }

        }

        tokens_.push_back(Token(this->tokLine, this->tokPos, this->tokPos, this->tokIndex, TTEndOfFile));

    }

bool Lexer::word_lex()
{
    if (this->currentChar > MIN_WEST_ARABIC_NUMBER_HEX and this->currentChar < MAX_WEST_ARABIC_NUMBER_HEX)
    {
        this->make_number();

        return true;
    }
    else if (this->currentChar > MIN_ARABIC_LETTER_HEX and this->currentChar < MAX_ARABIC_LETTER_HEX)
    {
        this->make_name();

        return true;
    }
    else if (this->currentChar == L'\"')
    {
        this->make_string();

        return true;
    }
    else
    {
        return this->two_symbol_lex();
    }
}

bool Lexer::two_symbol_lex()
{
    switch (this->currentChar)
    {
    case L'\n':
        this->make_newline(); // يجب ان يتم التحقق من السطر الجديد قبل المسافة او المسافة البادئة

        return true;
    case L'+':
        this->make_plus_equal();

        return true;
    case L'-':
        this->make_minus_equal();

        return true;
    case L'*':
        this->make_multiply_equal();

        return true;
    case L'\\':
        this->make_divide();

        return true;
    case L'^':
        this->make_power_equal();

        return true;
    case L'!':
        this->make_not_equal();

        return true;
    case L'=':
        this->make_equals();

        return true;
    case L'<':
        this->make_less_than();

        return true;
    case L'>':
        this->make_greater_than();

        return true;
    default:
        return this->symbol_lex();
    }
}

bool Lexer::symbol_lex()
{
    uint32_t posStart = this->tokPos;
    
    switch (this->currentChar)
    {
    case L'\r':
        this->advance();
        return true;
    case L' ':
        this->skip_space();

        return true;
    case L'\t':
        this->skip_space();

        return true;
    case L'.':
        this->advance();

        this->tokens_.push_back(Token(this->tokLine ,posStart, this->tokPos ,this->tokIndex ,TTDot));

        return true;
    case L')':
        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTRrightParenthesis));
        
        return true;
    case L'(':
        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTLeftParenthesis));

        return true;
    case L']':
        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTRightSquare));

        return true;
    case L'[':
        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTLeftSquare));

        return true;
    case L':':
        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTColon));

        return true;
    case L',':
        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTComma));

        return true;
    case L'#':
        this->skip_comment();

        return true;
    default:
        return false;
    }

}




void Lexer::skip_space()
{
    while (this->currentChar == L' ' or this->currentChar == L'\t')
    {
        this->advance();
    }
}

void Lexer::make_indent()
{
    uint32_t posStart = tokPos;
    int spaces = 0;

    while (this->currentChar == L'\t' or this->currentChar == L' ')
    {
        if (this->currentChar == L'\t')
        {
            this->advance();
            spaces += 4;
        }
        else
        {
            this->advance();
            spaces++;
        }
    }

    if (spaces > dedentSpec->spaces)
    {
        if (this->currentChar != L'\n') // تحقق اذا كان السطر لا يحتوي سوى مسافات بادئة >> قم بتخطيه
        {
            this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTIndent));
            DedentSpecifier* newIndent = new DedentSpecifier(*dedentSpec);
            dedentSpec->spaces = spaces;
            dedentSpec->previous = newIndent;
        }
        else
        {
            this->advance();
        }
    }
    else if (spaces < dedentSpec->spaces)
    {
        while (this->dedentSpec->spaces != spaces) {

            if (this->dedentSpec->spaces < spaces)
            {
                PRINT_(L"خطأ في المسافات البادئة - لقد خرجت عن النطاق الحالي");
                exit(-1);
            }

            if (this->dedentSpec->previous != nullptr)
            {
                this->dedentSpec = this->dedentSpec->previous;

            }
            else {
                PRINT_(L"خطأ في المسافات البادئة - لقد خرجت عن النطاق الحالي");
                exit(-1);
            }


            this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTDedent));
        }
    }
}

void Lexer::make_newline()
{
    uint32_t posStart = this->tokPos;

    this->advance();
    
    this->tokens_.push_back(Token(tokLine, posStart, this->tokPos, tokIndex, TTNewline));

    this->make_indent();
}

void Lexer::make_number() {
    wstr numberString = L"";
    uint8_t dotCount = 0;
    uint32_t posStart = this->tokPos;

    while (this->currentChar > MIN_WEST_ARABIC_NUMBER_HEX and this->currentChar < MAX_WEST_ARABIC_NUMBER_HEX or this->currentChar == L'.')
    {
        if (this->currentChar == L'.') {
            if (dotCount == 1) {
                dotCount++;
                break;
            }
            dotCount++;
        }
        numberString += this->currentChar;
        this->advance();
    }

    if (dotCount == 0)
    {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTInteger, std::stoi(numberString)));

    }
    else if (dotCount == 1) {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTFloat, std::stod(numberString)));
    }
    else
    {
        wstr detail = L"< ";
        detail.push_back(this->currentChar);
        detail += L" >";

        //PRINT_(SyntaxError(this->position_, this->position_, detail, fileName, input_).print_());
        exit(0);
    }
}

void Lexer::make_name()
{
    wstr nameString;
    uint32_t posStart = this->tokPos;

    while (this->currentChar > MIN_ARABIC_LETTER_HEX and this->currentChar < MAX_ARABIC_LETTER_HEX or this->currentChar == L'_') {
        nameString += this->currentChar;
        this->advance();
    }

    if (keywords_.find(nameString) != keywords_.end())
    {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTKeyword, keywords_.at(nameString)));
    }
    else if (buildInFunctions.find(nameString) != buildInFunctions.end())
    {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTBuildInFunc, buildInFunctions.at(nameString)));
    }
    else if (namesAlter.find(nameString) != namesAlter.end())
    {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTName, namesAlter[nameString]));
    }
    else
    {
        name++;
        namesAlter[nameString] = name;
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTName, name));
    }
}

void Lexer::make_string()
{
    wstr string_ = L"";
    uint32_t posStart = this->tokPos;
    bool ClosedString = true;
    this->advance();

    while (this->currentChar != L'\"') {
        if (this->currentChar == L'\0' or this->currentChar == L'\n') {
            ClosedString = false;
            break;
        }
        else {
            string_ += this->currentChar;
            this->advance();
        }
    }

    if (ClosedString)
    {
        this->advance();
        wstr* newString = new wstr(string_);
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTString, newString));
    }
    else {
        //PRINT_(SyntaxError(positionStart, this->position_, L"< لم يتم إغلاق النص >", fileName, input_).print_());
        exit(0);
    }
}

void Lexer::make_plus_equal() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTPlusEqual));
    }
    else {
        this->tokens_.push_back(Token(this->tokLine,posStart, this->tokPos, this->tokIndex, TTPlus));
    }
}

void Lexer::make_minus_equal() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTMinusEqual));
    }
    else {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTMinus));
    }
}

void Lexer::make_multiply_equal() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTMultiplyEqual));
    }
    else {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTMultiply));
    }
}

void Lexer::make_power_equal() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTPowerEqual));
    }
    else {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTPower));
    }
}

void Lexer::make_divide() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTDivideEqual));
    }
    else if (this->currentChar == L'\\') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTRemain));
    }
    else {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTDivide));
    }
}

void Lexer::make_not_equal() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTNotEqual));
    }
    else {
        //PRINT_(SyntaxError(this->position_, this->position_, L"< يتوقع وجود \'=\' بعد إشارة \'!\' >", fileName, input_).print_());
        exit(-1);
    }
}

void Lexer::make_equals() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTEqualEqual));
    }
    else
    {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTEqual));
    }
}

void Lexer::make_less_than() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTLessThanEqual));
    }
    else {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTLessThan));
    }

}

void Lexer::make_greater_than() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTGreaterThanEqual));
    }
    else
    {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTGreaterThan));
    }
}

void Lexer::skip_comment() 
{
    this->advance();
    while (this->currentChar != L'\n' and this->currentChar != L'\0') {
        this->advance();
    }
}
