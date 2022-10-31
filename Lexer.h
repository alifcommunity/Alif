#pragma once


// ملاحظات
// يجب إضافة رمز : النقطتين

// المعرب اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Lexer {
public:
    std::wstring fileName, input_;
    wchar_t currentChar;
    Position position, positionEnd;
    std::list<Token> tokens;
    std::shared_ptr<Error>(error);
    Lexer(std::wstring fileName, std::wstring input_) : fileName(fileName), input_(input_), position(Position()), currentChar(L'\0') {
        this->advance();
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

    Position position_end(Position position)
    {
        positionEnd = position;
        positionEnd.advance();
        return positionEnd;
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

                this->tokens.push_back(Token(this->position, position_end(this->position), newlineT));
                this->advance();

            }
            else if (this->currentChar == L'\t') {
                this->tokens.push_back(Token(this->position, position_end(this->position), tabT));
                this->advance();

            }
            else if (this->currentChar == L'.') {
                this->tokens.push_back(Token(this->position, position_end(this->position), dotT));
                this->advance();

            }
            else if (digits.find(this->currentChar) != std::wstring::npos)
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

            }
            else if (this->currentChar == L'^')
            {
                this->make_power_equal();

            }
            else if (this->currentChar == L'(')
            {
                this->tokens.push_back(Token(this->position, position_end(this->position), lParenthesisT));
                this->advance();

            }
            else if (this->currentChar == L')')
            {
                this->tokens.push_back(Token(this->position, position_end(this->position), rParenthesisT));
                this->advance();

            }
            else if (this->currentChar == L'[')
            {
                this->tokens.push_back(Token(this->position, position_end(this->position), lSquareT));
                this->advance();

            }
            else if (this->currentChar == L']')
            {
                this->tokens.push_back(Token(this->position, position_end(this->position), rSquareT));
                this->advance();

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
            else if (this->currentChar == L',') {
                tokens.push_back(Token(this->position, position_end(this->position), commaT));
                this->advance();
            }
            else
            {
                std::wstring detail = L"< حرف غير معروف \'";
                detail.push_back(this->currentChar);
                detail += L"\' >";

                error = std::make_shared<Error>(SyntaxError(this->position, this->position, detail, fileName, input_));
            }

            if (error)
            {
                return;
            }
        }

        tokens.push_back(Token(this->position, position_end(this->position), endOfFileT));

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
            this->tokens.push_back(Token(positionStart, this->position, integerT, numberString));

        }
        else if (dotCount == 1) {
            this->tokens.push_back(Token(positionStart, this->position, floatT, numberString));
        }
        else
        {
            std::wstring detail = L"\"";
            detail.push_back(this->currentChar);
            detail += L"\"";

            error = std::make_shared<Error>(SyntaxError(this->position, this->position, detail, fileName, input_));
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
        this->tokens.push_back(Token(positionStart, this->position, tokenType, nameString));
    }

    void make_string()
    {
        std::wstring string_ = L"";
        Position positionStart = this->position;
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
            this->tokens.push_back(Token(positionStart, this->position, stringT, string_));
        }
        else {
            error = std::make_shared<Error>(SyntaxError(positionStart, this->position, L"< لم يتم إغلاق النص >", fileName, input_));
        }
    }

    void make_plus_equal(){
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(positionStart, position_end(positionStart), plusT));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(positionStart, this->position, plusEqualT));
            this->advance();
        }
        else {
            error = std::make_shared<Error>(SyntaxError(positionStart ,this->position, L"< هل تقصد += ؟ >", fileName, input_));
        }
    }

    void make_minus_equal() {
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(positionStart, position_end(positionStart), minusT));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(positionStart, this->position, minusEqualT));
            this->advance();
        }
        else {
            error = std::make_shared<Error>(SyntaxError(positionStart, this->position, L"< هل تقصد -= ؟ >", fileName, input_));
        }
    }

    void make_multiply_equal() {
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(positionStart, position_end(positionStart), multiplyT));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(positionStart, this->position, multiplyEqualT));
            this->advance();
        }
        else {
            error = std::make_shared<Error>(SyntaxError(positionStart, this->position, L"< هل تقصد *= ؟ >", fileName, input_));
        }
    }

    void make_power_equal() {
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(positionStart, position_end(positionStart), powerT));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(positionStart, this->position, powerEqualT));
            this->advance();
        }
        else {
            error = std::make_shared<Error>(SyntaxError(positionStart, this->position, L"< هل تقصد ^= ؟ >", fileName, input_));
        }
    }

    void make_divide() {
        Position positionStart = this->position;
        this->advance();

        if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
            this->tokens.push_back(Token(positionStart, position_end(positionStart), divideT));
        }
        else if (this->currentChar == L'=') {
            this->tokens.push_back(Token(positionStart, this->position, divideEqualT));
            this->advance();
        }
        else if (this->currentChar == L'\\') {
            this->tokens.push_back(Token(positionStart, this->position, remainT));
            this->advance();
        }
        else {
            error = std::make_shared<Error>(SyntaxError(positionStart, this->position, L"< هل تقصد \\= ؟ >", fileName, input_));
        }
    }

    void make_not_equals() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(positionStart, position_end(positionStart), notEqualT));
        }
        else {
            error = std::make_shared<Error>(SyntaxError(this->position, this->position, L"< يتوقع وجود \'=\' بعد إشارة \'!\' >", fileName, input_));
        }
    }

    void make_equals() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(positionStart, this->position, equalEqualT));
        }
        else
        {
            this->tokens.push_back(Token(positionStart, position_end(positionStart), equalT));
        }
    }

    void make_less_than() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(positionStart, this->position, lessThanEqualT));
        }
        else {
            this->tokens.push_back(Token(positionStart, position_end(positionStart), lessThanT));
        }

    }

    void make_greater_than() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(positionStart, this->position, greaterThanEqualT));
        }
        else
        {
            this->tokens.push_back(Token(positionStart, position_end(positionStart), greaterThanT));
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
                if (!token.value)
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
