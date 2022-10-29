#pragma once


// ملاحظات
// يجب إضافة رمز : النقطتين

// المعرب اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Lexer {
public:
    std::wstring fileName, input_;
    wchar_t currentChar;
    Position position;
    std::list<Token> tokens;
    Error* error;
    Lexer(std::wstring fileName, std::wstring input_) : fileName(fileName), input_(input_), position(Position()), currentChar(L'\0') {
        this->advance();
    }

    ~Lexer()
    {
        delete error;
    }

    void advance() {
        this->position.advance(this->currentChar);

        if (this->position.index < this->input_.length())
        {
            this->currentChar = this->input_[this->position.index];
        }
        else {
            this->currentChar = L'\0';
        }
    }

    void make_token() {

        while (this->currentChar != L'\0')
        {
            if (this->currentChar == L' ')
            {
                this->advance();

            }
            else if (this->currentChar == L'#')
            {
                this->skip_comment();

            }
            else if (this->currentChar == L'\n')
            {
                this->tokens.push_back(Token(newlineT, this->position));
                this->advance();

            }
            else if (this->currentChar == L'\t') {
                this->tokens.push_back(Token(tabT, this->position));
                this->advance();

            }
            else if (this->currentChar == L'.') {
                this->tokens.push_back(Token(dotT, this->position));
                this->advance();

            }
            else if (digits.find(this->currentChar) != std::wstring::npos)
            {
                this->make_number();
                if (error) {
                    return;
                }

            }
            else if (letters.find(this->currentChar) != std::wstring::npos) {
                this->make_name();

            }
            else if (this->currentChar == L'\"')
            {
                this->make_string();
                if (error)
                {
                    return;
                }

            }
            else if (this->currentChar == L'+')
            {
                this->make_plus_equal();

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
                if (error)
                {
                    return;
                }

            }
            else if (this->currentChar == L'^')
            {
                this->make_power_equal();

            }
            else if (this->currentChar == L'(')
            {
                this->tokens.push_back(Token(lParenthesisT, this->position));
                this->advance();

            }
            else if (this->currentChar == L')')
            {
                this->tokens.push_back(Token(rParenthesisT, this->position));
                this->advance();

            }
            else if (this->currentChar == L'[')
            {
                this->tokens.push_back(Token(lSquareT, this->position));
                this->advance();

            }
            else if (this->currentChar == L']')
            {
                this->tokens.push_back(Token(rSquareT, this->position));
                this->advance();

            }
            else if (this->currentChar == L'!')
            {
                this->make_not_equals();
                if (error) {
                    return;
                }

            }
            else if (this->currentChar == L'=')
            {
                this->make_equals();
                if (error)
                {
                    return;
                }

            }
            else if (this->currentChar == L'<')
            {
                this->make_less_than();
                if (error)
                {
                    return;
                }

            }
            else if (this->currentChar == L'>')
            {
                this->make_greater_than();
                if (error)
                {
                    return;
                }

            }
            else if (this->currentChar == L',') {
                tokens.push_back(Token(commaT, this->position));
                this->advance();
            }
            else
            {
                std::wstring detail = L"< حرف غير معروف \'";
                detail.push_back(this->currentChar);
                detail += L"\' >";

                error = new SyntaxError(this->position, this->position, detail);
                return;
            }
        }

        tokens.push_back(Token(endOfFileT, this->position));
        return;

    }


    void make_number() {
        std::wstring numberString = L"";
        unsigned int dotCount = 0;
        Position positionStart = this->position;

        while (this->currentChar != L'\0' and (digits + L".").find(this->currentChar) != std::wstring::npos) {
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
            this->tokens.push_back(Token(integerT, numberString, positionStart, this->position));

        }
        else if (dotCount == 1) {
            this->tokens.push_back(Token(floatT, numberString, positionStart, this->position));
        }
        else
        {
            std::wstring detail = L"\"";
            detail.push_back(this->currentChar);
            detail += L"\"";

            error = new SyntaxError(this->position, this->position, detail);
        }
    }

    void make_name()
    {
        std::wstring nameString;
        std::wstring tokenType;
        Position positionStart = this->position;

        while (this->currentChar != L'\0' and (lettersDigits + L'_').find(this->currentChar) != std::wstring::npos) {
            nameString += this->currentChar;
            this->advance();
        }
        bool keyword = (find(keywords.begin(), keywords.end(), nameString) != keywords.end());
        if (keyword) {
            tokenType = keywordT;
        }
        else {
            tokenType = nameT;
        }
        this->tokens.push_back(Token(tokenType, nameString, positionStart, this->position));
    }

    void make_string()
    {
        std::wstring string_ = L"";
        Position positionStart = this->position;
        bool ClosedString = true;
        this->advance();

        while (this->currentChar != L'\"') {
            if (this->currentChar == L'\0') {
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
            this->tokens.push_back(Token(stringT, string_, positionStart, this->position));
        }
        else {
            error = new SyntaxError(this->position, this->position, L"< لم يتم إغلاق النص >");
        }
    }

    void make_plus_equal(){
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(plusT, positionStart));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(plusEqualT, L"+=", positionStart, this->position));
            this->advance();
        }
        else {
            error = new SyntaxError(positionStart ,this->position, L"< هل تقصد += ؟ >");
        }
    }

    void make_minus_equal() {
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(minusT, positionStart));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(minusEqualT, L"-=", positionStart, this->position));
            this->advance();
        }
        else {
            error = new SyntaxError(positionStart, this->position, L"< هل تقصد -= ؟ >");
        }
    }

    void make_multiply_equal() {
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(multiplyT, positionStart));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(multiplyEqualT, L"*=", positionStart, this->position));
            this->advance();
        }
        else {
            error = new SyntaxError(positionStart, this->position, L"< هل تقصد *= ؟ >");
        }
    }

    void make_power_equal() {
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(powerT, positionStart));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(powerEqualT, L"^=", positionStart, this->position));
            this->advance();
        }
        else {
            error = new SyntaxError(positionStart, this->position, L"< هل تقصد ^= ؟ >");
        }
    }

    void make_divide() {
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(divideT, positionStart));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(divideEqualT, L"\\=", positionStart, this->position));
            this->advance();
        }
        else if (this->currentChar == L'\\') {
            this->tokens.push_back(Token(remainT, L"\\\\", positionStart, this->position));
            this->advance();
        }
        else {
            error = new SyntaxError(positionStart, this->position, L"< هل تقصد \\= ؟ >");
        }
    }

    void make_not_equals() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(notEqualT, positionStart));
        }
        else {
            error = new SyntaxError(this->position, this->position, L"< يتوقع وجود \'=\' بعد إشارة \'!\' >");
        }
    }

    void make_equals() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(equalEqualT, positionStart));
        }
        else
        {
            this->tokens.push_back(Token(equalT, this->position));
        }
    }

    void make_less_than() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(lessThanEqualT, positionStart));
        }
        else {
            this->tokens.push_back(Token(lessThanT, this->position));
        }

    }

    void make_greater_than() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(greaterThanEqualT, positionStart));
        }
        else
        {
            this->tokens.push_back(Token(greaterThanT, this->position));
        }
    }

    void skip_comment() {
        this->advance();
        while (this->currentChar != L'\n' and this->currentChar != L'\0') {
            this->advance();
        }
    }

    void print() {
        std::wstring result;

        if (error)
        {
            std::wcout << error->print_() << std::endl;
        }
        else
        {
            for (std::list<Token>::iterator tokItr = tokens.begin(); tokItr != tokens.end(); ++tokItr)
            {
                Token token = *tokItr;
                if (token.value == nullptr)
                {
                    result += L"[" + std::to_wstring(token.positionStart->index) + L"]  ->  " + *token.type + L", \n";
                }
                else
                {
                    result += L"[" + std::to_wstring(token.positionStart->index) + L", " + std::to_wstring(token.positionEnd->index) + L"]  ->  " + *token.type + L" : " + *token.value + L", \n";
                }
            }
            std::wcout << L"نتائج المعرب اللغوي : \n" << result << std::endl;
        }
    }
};
