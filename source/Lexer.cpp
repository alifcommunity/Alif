#include "Lexer.h"

Lexer::Lexer(wstr _fileName, wstr* _input) 
    : fileName(_fileName), input_(_input), currentChar(L'\0')
{
    this->advance();

    dedentSpec->spaces = 0; // تهيئة المتغير بقيمة افتراضية صفر لضمان عدم ظهور قيم عشوائية
    dedentSpec->previous = nullptr; // تهيئة المتغير بقيمة افتراضية فارغ لضمان عدم ظهور قيم عشوائية
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
            if (!this->word_lex()) // في حال عدم العثور على حرف يظهر خطأ
            {
                wstr detail = L" حرف غير معروف \'";
                detail.push_back(this->currentChar);
                detail += L"\' ";

                PRINT_(SyntaxError(this->tokPos, this->tokPos, this->tokIndex, this->tokLine, detail, this->fileName, this->input_).print_());
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
        return this->symbol_lex();
    }
}

bool Lexer::symbol_lex()
{
    uint32_t posStart{};
    
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
        posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine ,posStart, this->tokPos ,this->tokIndex ,TTDot));

        return true;
    case L')':
        posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTRrightParenthesis));
        
        return true;
    case L'(':
        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTLeftParenthesis));

        return true;
    case L']':
        posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTRightSquare));

        return true;
    case L'[':
        posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTLeftSquare));

        return true;
    case L':':
        posStart = this->tokPos;
        
        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTColon));

        return true;
    case L',':
        posStart = this->tokPos;
        
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
            DedentSpecifier* newIndent = (DedentSpecifier*)alifMemory.allocate(sizeof(DedentSpecifier)); 
            *newIndent = *dedentSpec;
            dedentSpec->spaces = spaces;
            dedentSpec->previous = newIndent;
        }
        else
        {
            this->advance();
            if (this->currentChar == L'\t') // تحقق اذا كان السطر يحتوي مسافة بادئة بعد سطر فارغ >> قم بإنشاء رمز مسافة بادئة
            {
                this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTIndent));
                DedentSpecifier* newIndent = (DedentSpecifier*)alifMemory.allocate(sizeof(DedentSpecifier));
                *newIndent = *dedentSpec;
                dedentSpec->spaces = spaces;
                dedentSpec->previous = newIndent;
            }
        }
    }
    else if (spaces < dedentSpec->spaces)
    {
        while (this->dedentSpec->spaces != spaces) {

            if (this->dedentSpec->spaces < spaces)
            {
                // يجب عمل خطأ من نوع "خطأ في النطاق" حيث يقوم بتحديد السطر الذي تم فيه الخروج عن النطاق فقط
                PRINT_(SyntaxError(posStart, posStart, this->tokIndex, this->tokLine, L"لقد خرجت عن النطاق الحالي", this->fileName, this->input_).print_());
                //PRINT_(L"خطأ في المسافات البادئة - لقد خرجت عن النطاق الحالي");
                exit(-1);
            }

            if (this->dedentSpec->previous != nullptr)
            {
                this->dedentSpec = this->dedentSpec->previous;

            }
            else {
                // يجب عمل خطأ من نوع "خطأ في النطاق" حيث يقوم بتحديد السطر الذي تم فيه الخروج عن النطاق فقط
                PRINT_(SyntaxError(posStart, posStart, this->tokIndex, this->tokLine, L"لقد خرجت عن النطاق الحالي", this->fileName, this->input_).print_());
                //PRINT_(L"خطأ في المسافات البادئة - لقد خرجت عن النطاق الحالي");
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
    
    this->tokens_.push_back(Token(tokLine, posStart, posStart, tokIndex, TTNewline));

    this->make_indent();
}

void Lexer::make_number() {
    wstr numberString{};
    uint8_t dotCount = 0;
    uint32_t posStart = this->tokPos;

    while (this->currentChar > MIN_WEST_ARABIC_NUMBER_HEX and this->currentChar < MAX_WEST_ARABIC_NUMBER_HEX or this->currentChar == L'.')
    {
        if (this->currentChar == L'.') {
            if (dotCount == 1) {
                wstr detail = L"< ";
                detail.push_back(this->currentChar);
                detail += L" >";

                PRINT_(SyntaxError(posStart, this->tokPos, this->tokIndex, this->tokLine, detail, fileName, input_).print_());
                exit(-1);
            }
            dotCount++;
        }

        numberString += this->currentChar;
        this->advance();
    }

    //wchar_t* number_ = new wchar_t[numberString.length() + 1];
    wchar_t* number_ = (wchar_t*)alifMemory.allocate(numberString.size() * 2 + 2); // .length() * sizeof(wchar_t) + terminator 
    for (uint16_t i = 0; i < numberString.length(); i++)
    {
        number_[i] = numberString[i];
    }
    number_[numberString.length()] = L'\0'; // لضمان قطع "مقاطعة" السلسلة النصية
    
    if (dotCount == 0)
    {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTInteger, number_));
    }
    else {
        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTFloat, number_));
    }
}

void Lexer::make_name()
{
    wstr nameString{};
    uint32_t posStart = this->tokPos;

    while (this->currentChar > MIN_ARABIC_LETTER_HEX and this->currentChar < MAX_ARABIC_LETTER_HEX or this->currentChar == L'_') {
        nameString += this->currentChar;
        this->advance();
    }

    wchar_t* name_ = new wchar_t[nameString.length() + 1];
    //wchar_t* name_ = (wchar_t*)alifMemory.allocate((nameString.size() * 2) + 2); // .length() * sizeof(wchar_t) + terminator 
    for (uint16_t i = 0; i < nameString.length(); i++)
    {
        name_[i] = nameString[i];
    }
    name_[nameString.length()] = L'\0'; // لضمان قطع "مقاطعة" السلسلة النصية

    this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTName, is_repeated(name_)));
}

void Lexer::make_string()
{
    wstr newString{};
    uint32_t posStart = this->tokPos;
    this->advance();

    while (this->currentChar != L'\"') {
        if (this->currentChar == L'\0' or this->currentChar == L'\n') {
            PRINT_(SyntaxError(posStart, this->tokPos, this->tokIndex, this->tokLine, L"< لم يتم إغلاق النص >", fileName, input_).print_());
            exit(-1);
        }
        else {
            newString += this->currentChar;
            this->advance();
        }
    }

    //wchar_t* string_ = new wchar_t[newString.length() + 1];
    wchar_t* string_ = (wchar_t*)alifMemory.allocate((newString.size() * 2) + 2); // .length() * sizeof(wchar_t) + terminator 
    for (uint16_t i = 0; i < newString.length(); i++)
    {
        string_[i] = newString[i];
    }
    string_[newString.length()] = L'\0'; // لضمان قطع "مقاطعة" السلسلة النصية

    this->advance();
    this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTString, string_));

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
        PRINT_(SyntaxError(posStart, this->tokPos, this->tokIndex, this->tokLine, L"< يتوقع وجود \'=\' بعد إشارة \'!\' >", fileName, input_).print_());
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
