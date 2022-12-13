#pragma once

// المعرب اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Lexer {
public:
    STR fileName{}, input_{};
    wchar_t currentChar{};
    Position position_{}, positionEnd{};
    std::vector<Token> tokens_{};

    Lexer(STR _fileName, STR _input)
    {
        this->fileName = _fileName;
        this->input_ = _input;
        this->position_ = Position(-1, 0, -1);
        this->currentChar = L'\0';
        this->advance();
    }

    void advance() {
        this->position_.advance(this->currentChar);

        if (this->position_.index_ < this->input_.length())
        {
            this->currentChar = this->input_[this->position_.index_];
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
                this->make_space();
            }
            else if (this->currentChar == L'#')
            {
                this->skip_comment();
            }
            else if (this->currentChar == L'\n')
            {
                Position positionStart = this->position_;
                this->advance();
                this->tokens_.push_back(Token(positionStart, this->position_, TTnewline));
            }
            else if (this->currentChar == L'\t') {
                this->make_tab();
            }
            else if (this->currentChar == L'.') {
                Position positionStart = this->position_;
                this->advance();
                this->tokens_.push_back(Token(positionStart, this->position_, TTdot));
            }
            else if (digits.find(this->currentChar) != STR::npos)
            {
                this->make_number();
            }
            //else if (letters.find(this->currentChar) != std::wstring::npos) {
            //    this->make_name();

            //}
            //else if (this->currentChar == L'\"')
            //{
            //    this->make_string();

            //}
            //else if (this->currentChar == L'+')
            //{
            //    this->make_plus_equal();

            //}
            //else if (this->currentChar == L'-')
            //{
            //    this->make_minus_equal();

            //}
            //else if (this->currentChar == L'*')
            //{
            //    this->make_multiply_equal();

            //}
            //else if (this->currentChar == L'\\')
            //{
            //    this->make_divide();

            //}
            //else if (this->currentChar == L'^')
            //{
            //    this->make_power_equal();

            //}
            //else if (this->currentChar == L'(')
            //{
            //    this->tokens_.push_back(Token(this->position_, position_end(this->position_), lParenthesisT));
            //    this->advance();

            //}
            //else if (this->currentChar == L')')
            //{
            //    this->tokens_.push_back(Token(this->position_, position_end(this->position_), rParenthesisT));
            //    this->advance();

            //}
            //else if (this->currentChar == L'[')
            //{
            //    this->tokens_.push_back(Token(this->position_, position_end(this->position_), lSquareT));
            //    this->advance();

            //}
            //else if (this->currentChar == L']')
            //{
            //    this->tokens_.push_back(Token(this->position_, position_end(this->position_), rSquareT));
            //    this->advance();

            //}
            //else if (this->currentChar == L':')
            //{
            //    this->tokens_.push_back(Token(this->position_, position_end(this->position_), colonT));
            //    this->advance();

            //}
            //else if (this->currentChar == L'!')
            //{
            //    this->make_not_equals();

            //}
            //else if (this->currentChar == L'=')
            //{
            //    this->make_equals();

            //}
            //else if (this->currentChar == L'<')
            //{
            //    this->make_less_than();

            //}
            //else if (this->currentChar == L'>')
            //{
            //    this->make_greater_than();

            //}
            //else if (this->currentChar == L',') {
            //    tokens_.push_back(Token(this->position_, position_end(this->position_), commaT));
            //    this->advance();
            //}
            //else
            //{
            //    std::wstring detail = L"< حرف غير معروف \'";
            //    detail.push_back(this->currentChar);
            //    detail += L"\' >";

            //    error = std::make_shared<Error>(SyntaxError(this->position_, this->position_, detail, fileName, input_));
            //}

            //if (error)
            //{
            //    return;
            //}
        }

        //tokens_.push_back(Token(this->position_, position_end(this->position_), endOfFileT));

    }

    void make_space()
    {
        if (this->position_.column_ == 0)
        {
            Position positionStart = this->position_;
            int spaces = 0;

            while (this->currentChar == L' ')
            {
                this->advance();
                spaces++;
            }

            this->tokens_.push_back(Token(positionStart, this->position_, TTtab, spaces));
        }
        else
        {
            this->advance();
        }
    }

    void make_tab()
    {
        if (this->position_.column_ == 0)
        {
            Position positionStart = this->position_;
            int spaces = 0;

            while (this->currentChar == L'\t')
            {
                this->advance();
                spaces += 4;
            }

            this->tokens_.push_back(Token(positionStart, this->position_, TTtab, spaces));
        }
        else
        {
            this->advance();
        }
    }

    void make_number() {
        STR numberString = L"";
        unsigned int dotCount = 0;
        Position positionStart = this->position_;

        while (this->currentChar != L'\0' and (digits + L".").find(this->currentChar) != STR::npos) {
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
            this->tokens_.push_back(Token(positionStart, this->position_, TTinteger, std::stoi(numberString)));

        }
        else if (dotCount == 1) {
            this->tokens_.push_back(Token(positionStart, this->position_, TTfloat, std::stod(numberString)));
        }
        else
        {
            std::wstring detail = L"\"";
            detail.push_back(this->currentChar);
            detail += L"\"";

            error = std::make_shared<Error>(SyntaxError(this->position_, this->position_, detail, fileName, input_));
        }
    }

    //void make_name()
    //{
    //    std::wstring nameString;
    //    Position positionStart = this->position_;

    //    while (this->currentChar != L'\0' and (lettersDigits + L'_').find(this->currentChar) != std::wstring::npos) {
    //        nameString += this->currentChar;
    //        this->advance();
    //    }
    //    this->tokens_.push_back(Token(positionStart, this->position_, nameT, nameString));
    //}

    //void make_string()
    //{
    //    std::wstring string_ = L"";
    //    Position positionStart = this->position_;
    //    bool ClosedString = true;
    //    this->advance();

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

    //    if (ClosedString)
    //    {
    //        this->advance();
    //        this->tokens_.push_back(Token(positionStart, this->position_, stringT, string_));
    //    }
    //    else {
    //        error = std::make_shared<Error>(SyntaxError(positionStart, this->position_, L"< لم يتم إغلاق النص >", fileName, input_));
    //    }
    //}

    //void make_plus_equal(){
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), plusT));
    //    }
    //    else if (this->currentChar == L'=') {
    //        this->tokens_.push_back(Token(positionStart, this->position_, plusEqualT));
    //        this->advance();
    //    }
    //    else {
    //        error = std::make_shared<Error>(SyntaxError(positionStart ,this->position_, L"< هل تقصد += ؟ >", fileName, input_));
    //    }
    //}

    //void make_minus_equal() {
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), minusT));
    //    }
    //    else if (this->currentChar == L'=') {
    //        this->tokens_.push_back(Token(positionStart, this->position_, minusEqualT));
    //        this->advance();
    //    }
    //    else {
    //        error = std::make_shared<Error>(SyntaxError(positionStart, this->position_, L"< هل تقصد -= ؟ >", fileName, input_));
    //    }
    //}

    //void make_multiply_equal() {
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), multiplyT));
    //    }
    //    else if (this->currentChar == L'=') {
    //        this->tokens_.push_back(Token(positionStart, this->position_, multiplyEqualT));
    //        this->advance();
    //    }
    //    else {
    //        error = std::make_shared<Error>(SyntaxError(positionStart, this->position_, L"< هل تقصد *= ؟ >", fileName, input_));
    //    }
    //}

    //void make_power_equal() {
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), powerT));
    //    }
    //    else if (this->currentChar == L'=') {
    //        this->tokens_.push_back(Token(positionStart, this->position_, powerEqualT));
    //        this->advance();
    //    }
    //    else {
    //        error = std::make_shared<Error>(SyntaxError(positionStart, this->position_, L"< هل تقصد ^= ؟ >", fileName, input_));
    //    }
    //}

    //void make_divide() {
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if ((lettersDigits + L' ').find(this->currentChar) != std::wstring::npos) {
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), divideT));
    //    }
    //    else if (this->currentChar == L'=') {
    //        this->tokens_.push_back(Token(positionStart, this->position_, divideEqualT));
    //        this->advance();
    //    }
    //    else if (this->currentChar == L'\\') {
    //        this->tokens_.push_back(Token(positionStart, this->position_, remainT));
    //        this->advance();
    //    }
    //    else {
    //        error = std::make_shared<Error>(SyntaxError(positionStart, this->position_, L"< هل تقصد \\= ؟ >", fileName, input_));
    //    }
    //}

    //void make_not_equals() {
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if (this->currentChar == L'=') {
    //        this->advance();
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), notEqualT));
    //    }
    //    else {
    //        error = std::make_shared<Error>(SyntaxError(this->position_, this->position_, L"< يتوقع وجود \'=\' بعد إشارة \'!\' >", fileName, input_));
    //    }
    //}

    //void make_equals() {
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if (this->currentChar == L'=') {
    //        this->advance();
    //        this->tokens_.push_back(Token(positionStart, this->position_, equalEqualT));
    //    }
    //    else
    //    {
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), equalT));
    //    }
    //}

    //void make_less_than() {
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if (this->currentChar == L'=') {
    //        this->advance();
    //        this->tokens_.push_back(Token(positionStart, this->position_, lessThanEqualT));
    //    }
    //    else {
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), lessThanT));
    //    }

    //}

    //void make_greater_than() {
    //    Position positionStart = this->position_;
    //    this->advance();

    //    if (this->currentChar == L'=') {
    //        this->advance();
    //        this->tokens_.push_back(Token(positionStart, this->position_, greaterThanEqualT));
    //    }
    //    else
    //    {
    //        this->tokens_.push_back(Token(positionStart, position_end(positionStart), greaterThanT));
    //    }
    //}

    void skip_comment() {
        this->advance();
        while (this->currentChar != L'\n' and this->currentChar != L'\0') {
            this->advance();
        }
    }

    //void print() {
    //    std::wstring result;

    //    if (error)
    //    {
    //        std::wcout << error->print_() << std::endl;
    //    }
    //    else
    //    {
    //        for (std::vector<Token>::iterator tokItr = tokens.begin(); tokItr != tokens.end(); ++tokItr)
    //        {
    //            Token token = *tokItr;
    //            if (token.value == L"")
    //            {
    //                result += L"[" + std::to_wstring(token.positionStart.index) + L"]  ->  " + std::to_wstring(token.type) + L", \n";
    //            }
    //            else
    //            {
    //                result += L"[" + std::to_wstring(token.positionStart.index) + L", " + std::to_wstring(token.positionEnd.index) + L"]  ->  " + std::to_wstring(token.type) + L" : " + token.value + L", \n";
    //            }
    //        }
    //        std::wcout << L"نتائج المعرب اللغوي : \n" << result << std::endl;
    //    }
    //}
};
