#include "Lexer.h"

/*
تم تعريف متغيرات الاسماء خارج الصنف لكي لا يتم إعادة ضبطها عند
استخدام الطرفية في تنفيذ الشفرة او عند استيراد المكتبات
*/
uint32_t name = 0; // متغير اسماء على شكل ارقام
std::map<wstr, int> namesAlter{};



Lexer::Lexer(wstr _fileName, wstr _input)
{
    this->fileName = _fileName;
    this->input_ = _input;
    this->currentChar = L'\0';
    this->advance();
}

void Lexer::advance() {

    tokIndex++;
    tokPos++;

    if (this->currentChar == L'\n')
    {
        tokLine++;
        tokPos = 0;
    }

    if (this->tokIndex < this->input_.length())
    {
        this->currentChar = this->input_[this->tokIndex];
    }
    else
    {
        this->currentChar = L'\0';
    }
}

void Lexer::make_token() {

        while (this->currentChar != L'\0')
        {
            if (this->word_lex())
            {
                continue;
            }
            else if (this->two_symbol_lex())
            {
                continue;
            }
            else if (this->symbol_lex()) // اذا لم ينجح في إنشاء الرمز فذلك يدل على حدوث خطأ ويجب ان يخرج من البرنامج
            {
                continue;
            }
            else
            {
                wstr detail = L"< حرف غير معروف \'";
                detail.push_back(this->currentChar);
                detail += L"\' >";

                //PRINT_(SyntaxError(this->position_, this->position_, detail, fileName, input_).print_());
                exit(-1);
            }

            else if (digits.find(this->currentChar) != wstr::npos)
            {
                this->make_number();
            }
            else if (letters.find(this->currentChar) != std::wstring::npos) {
                this->make_name();
            }
            else if (this->currentChar == L'\"')
            {
                this->make_string();
            }
            else if (this->currentChar == L'-')
            {
                this->make_minus_equal();
            }
            else if (this->currentChar == L'*')
            {
                this->make_multiply_equal();
            }
            else if (this->currentChar == L'\\')
            {
                this->make_divide();
            }
            else if (this->currentChar == L'^')
            {
                this->make_power_equal();
            }
            else if (this->currentChar == L'!')
            {
                this->make_not_equals();
            }
            else if (this->currentChar == L'=')
            {
                this->make_equals();
            }
            else if (this->currentChar == L'<')
            {
                this->make_less_than();
            }
            else if (this->currentChar == L'>')
            {
                this->make_greater_than();
            }

        }

        tokens_.push_back(Token(this->position_, this->position_, TTendOfFile));

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
    default:
        false;
    }
}

bool Lexer::symbol_lex()
{
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
        uint32_t posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine ,posStart, this->tokPos ,this->tokIndex ,TTDot));

        return true;
    case L')':
        uint32_t posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTRrightParenthesis));
        
        return true;
    case L'(':
        uint32_t posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTLeftParenthesis));

        return true;
    case L']':
        uint32_t posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTRightSquare));

        return true;
    case L'[':
        uint32_t posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTLeftSquare));

        return true;
    case L':':
        uint32_t posStart = this->tokPos;

        this->advance();

        this->tokens_.push_back(Token(this->tokLine, posStart, this->tokPos, this->tokIndex, TTColon));

        return true;
    case L',':
        uint32_t posStart = this->tokPos;

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


//void Lexer::make_token() {
//
//        while (this->currentChar != L'\0')
//        {
//            if (this->currentChar == L'\r')
//            {
//                this->advance();
//            }
//            else if (this->currentChar == L'\n') // يجب ان يتم التحقق من السطر الجديد قبل المسافة او المسافة البادئة
//            {
//                this->make_newline();
//            }
//            else if (this->currentChar == L' ' or this->currentChar == L'\t')
//            {
//                this->skip_space();
//            }
//            else if (this->currentChar == L'#')
//            {
//                this->skip_comment();
//            }
//            else if (digits.find(this->currentChar) != wstr::npos)
//            {
//                this->make_number();
//            }
//            else if (this->currentChar == L'.') {
//                Position positionStart = this->position_;
//                this->advance();
//                this->tokens_.push_back(Token(positionStart, this->position_, TTdot));
//            }
//            else if (letters.find(this->currentChar) != std::wstring::npos) {
//                this->make_name();
//            }
//            else if (this->currentChar == L'\"')
//            {
//                this->make_string();
//            }
//            else if (this->currentChar == L'+')
//            {
//                this->make_plus_equal();
//            }
//            else if (this->currentChar == L'-')
//            {
//                this->make_minus_equal();
//            }
//            else if (this->currentChar == L'*')
//            {
//                this->make_multiply_equal();
//            }
//            else if (this->currentChar == L'\\')
//            {
//                this->make_divide();
//            }
//            else if (this->currentChar == L'^')
//            {
//                this->make_power_equal();
//            }
//            else if (this->currentChar == L'(')
//            {
//                Position positionStart = this->position_;
//                this->advance();
//                this->tokens_.push_back(Token(positionStart, this->position_, TTlParenthesis));
//            }
//            else if (this->currentChar == L')')
//            {
//                Position positionStart = this->position_;
//                this->advance();
//                this->tokens_.push_back(Token(positionStart, this->position_, TTrParenthesis));
//            }
//            else if (this->currentChar == L'[')
//            {
//                Position positionStart = this->position_;
//                this->advance();
//                this->tokens_.push_back(Token(positionStart, this->position_, TTlSquare));
//            }
//            else if (this->currentChar == L']')
//            {
//                Position positionStart = this->position_;
//                this->advance();
//                this->tokens_.push_back(Token(positionStart, this->position_, TTrSquare));
//            }
//            else if (this->currentChar == L':')
//            {
//                Position positionStart = this->position_;
//                this->advance();
//                this->tokens_.push_back(Token(positionStart, this->position_, TTcolon));
//            }
//            else if (this->currentChar == L',')
//            {
//                Position positionStart = this->position_;
//                this->advance();
//                tokens_.push_back(Token(positionStart, this->position_, TTcomma));
//            }
//            else if (this->currentChar == L'!')
//            {
//                this->make_not_equals();
//            }
//            else if (this->currentChar == L'=')
//            {
//                this->make_equals();
//            }
//            else if (this->currentChar == L'<')
//            {
//                this->make_less_than();
//            }
//            else if (this->currentChar == L'>')
//            {
//                this->make_greater_than();
//            }
//            else
//            {
//                wstr detail = L"< حرف غير معروف \'";
//                detail.push_back(this->currentChar);
//                detail += L"\' >";
//
//                prnt(SyntaxError(this->position_, this->position_, detail, fileName, input_).print_());
//                exit(0);
//            }
//        }
//
//        tokens_.push_back(Token(this->position_, this->position_, TTendOfFile));
//
//    }
//

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

//void Lexer::make_number() {
//    wstr numberString = L"";
//    unsigned int dotCount = 0;
//    Position positionStart = this->position_;
//
//    while (this->currentChar != L'\0' and (digits + L".").find(this->currentChar) != wstr::npos) {
//        if (this->currentChar == L'.') {
//            if (dotCount == 1) {
//                dotCount++;
//                break;
//            }
//            dotCount++;
//        }
//        numberString += this->currentChar;
//        this->advance();
//    }
//
//    if (dotCount == 0)
//    {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTinteger, std::stoi(numberString)));
//
//    }
//    else if (dotCount == 1) {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTfloat, std::stod(numberString)));
//    }
//    else
//    {
//        wstr detail = L"< ";
//        detail.push_back(this->currentChar);
//        detail += L" >";
//
//        prnt(SyntaxError(this->position_, this->position_, detail, fileName, input_).print_());
//        exit(0);
//    }
//}
//
//void Lexer::make_name()
//{
//    wstr nameString;
//    Position positionStart = this->position_;
//
//    while (this->currentChar != L'\0' and (lettersDigits + L'_').find(this->currentChar) != wstr::npos) {
//        nameString += this->currentChar;
//        this->advance();
//    }
//
//    if (keywords_.find(nameString) != keywords_.end())
//    {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTkeyword, keywords_[nameString]));
//    }
//    else if (buildInFunctions.find(nameString) != buildInFunctions.end())
//    {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTbuildInFunc, buildInFunctions[nameString]));
//    }
//    else if (namesAlter.find(nameString) != namesAlter.end())
//    {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTname, namesAlter[nameString]));
//    }
//    else
//    {
//        name++;
//        namesAlter[nameString] = name;
//        this->tokens_.push_back(Token(positionStart, this->position_, TTname, name));
//    }
//}
//
//void Lexer::make_string()
//{
//    wstr string_ = L"";
//    Position positionStart = this->position_;
//    bool ClosedString = true;
//    this->advance();
//
//    while (this->currentChar != L'\"') {
//        if (this->currentChar == L'\0' or this->currentChar == L'\n') {
//            ClosedString = false;
//            break;
//        }
//        else {
//            string_ += this->currentChar;
//            this->advance();
//        }
//    }
//
//    if (ClosedString)
//    {
//        this->advance();
//        wstr* newString = new wstr(string_);
//        this->tokens_.push_back(Token(positionStart, this->position_, TTstring, newString));
//    }
//    else {
//        prnt(SyntaxError(positionStart, this->position_, L"< لم يتم إغلاق النص >", fileName, input_).print_());
//        exit(0);
//    }
//}

void Lexer::make_plus_equal() {
    uint32_t posStart = this->tokPos;
    this->advance();

    if (this->currentChar == L'=') {
        this->advance();
        this->tokens_.push_back(Token(tokLine, posStart, this->tokPos, this->tokIndex, TTPlusEqual));
    }
    else {
        this->tokens_.push_back(Token(tokLine,posStart, this->tokPos, this->tokIndex, TTPlus));
    }
}

//void Lexer::make_minus_equal() {
//    Position positionStart = this->position_;
//    this->advance();
//
//    if (this->currentChar == L'=') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTminusEqual));
//    }
//    else {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTminus));
//    }
//}
//
//void Lexer::make_multiply_equal() {
//    Position positionStart = this->position_;
//    this->advance();
//
//    if (this->currentChar == L'=') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTmultiplyEqual));
//    }
//    else {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTmultiply));
//    }
//}
//
//void Lexer::make_power_equal() {
//    Position positionStart = this->position_;
//    this->advance();
//
//    if (this->currentChar == L'=') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTpowerEqual));
//    }
//    else {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTpower));
//    }
//}
//
//void Lexer::make_divide() {
//    Position positionStart = this->position_;
//    this->advance();
//
//    if (this->currentChar == L'=') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTdivideEqual));
//    }
//    else if (this->currentChar == L'\\') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTremain));
//    }
//    else {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTdivide));
//    }
//}
//
//void Lexer::make_not_equals() {
//    Position positionStart = this->position_;
//    this->advance();
//
//    if (this->currentChar == L'=') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTnotEqual));
//    }
//    else {
//        prnt(SyntaxError(this->position_, this->position_, L"< يتوقع وجود \'=\' بعد إشارة \'!\' >", fileName, input_).print_());
//        exit(0);
//    }
//}
//
//void Lexer::make_equals() {
//    Position positionStart = this->position_;
//    this->advance();
//
//    if (this->currentChar == L'=') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTequalEqual));
//    }
//    else
//    {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTequal));
//    }
//}
//
//void Lexer::make_less_than() {
//    Position positionStart = this->position_;
//    this->advance();
//
//    if (this->currentChar == L'=') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTlessThanEqual));
//    }
//    else {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTlessThan));
//    }
//
//}
//
//void Lexer::make_greater_than() {
//    Position positionStart = this->position_;
//    this->advance();
//
//    if (this->currentChar == L'=') {
//        this->advance();
//        this->tokens_.push_back(Token(positionStart, this->position_, TTgreaterThanEqual));
//    }
//    else
//    {
//        this->tokens_.push_back(Token(positionStart, this->position_, TTgreaterThan));
//    }
//}

void Lexer::skip_comment() 
{
    this->advance();
    while (this->currentChar != L'\n' and this->currentChar != L'\0') {
        this->advance();
}
