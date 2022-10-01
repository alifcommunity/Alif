// استيراد
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include<iostream>
#include<string>
#include<list>
#include<algorithm> // لعمل تتالي على المصفوفات
#include<fcntl.h> //لقبول ادخال الاحرف العربية من الكونسل
#include<io.h> //لقبول ادخال الاحرف العربية من الكونسل
#include "constants.h"
#include "position.h"


// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::wstring
integerT = L"Integer",
floatT = L"Float",
stringT = L"String",
nameT = L"Name",
keywordT = L"Keyword",
plusT = L"Plus",
plusEqualT = L"Plus_equal",
minusT = L"Minus",
minusEqualT = L"Minus_equal",
multiplyT = L"Multiply",
multiplyEqualT = L"Multiply_equal",
divideT = L"Divide",
divideEqualT = L"Divide_equal",
powerT = L"Power",
powerEqualT = L"Power_equal",
remainT = L"Remain",
remainEqualT = L"Remain_equal",
equalT = L"Equal",
lParenthesisT = L"L_Parenthesis",
rParenthesisT = L"R_Parenthesis",
lSquareT = L"L_Square",
rSquareT = L"R_Square",
lCurlyBraceT = L"L_curly_brace",
rCurlyBraceT = L"R_curly_brace",
equalEqualT = L"Equal_equal",
notEqualT = L"Not_equal",
lessThanT = L"Less_than",
greaterThanT = L"Greater_than",
lessThanEqualT = L"Less_than_equal",
greaterThanEqualT = L"Greater_than_equal",
commaT = L"Comma",
colonT = L"Colon",
arrowT = L"Arrow",
newlineT = L"Newline",
tabT = L"Tab",
dotT = L"Dot",
endOfFileT = L"End_Of_File";

const std::list<std::wstring> keywords = { L"مرر", L"توقف", L"استمر", L"حذف", L"استورد", L"من", L"اذا", L"بينما", L"لاجل", L"ارجع", L"دالة", L"صنف", L"والا", L"او", L"و", L"ليس", L"صح", L"خطا", L"عدم", L"اطبع", L"في" };


class Token {
public:
    std::wstring type_, value_;
    Position positionStart, positionEnd;
    Token() {}
    Token(std::wstring type_, Position positionStart) : type_(type_), positionStart(positionStart)
    {
        this->positionStart = positionStart;
        this->positionEnd = positionStart;
        this->positionEnd.advance();
    }
    Token(std::wstring type_, std::wstring value_, Position positionStart, Position positionEnd) : type_(type_), value_(value_), positionStart(positionStart), positionEnd(positionEnd)
    {
        this->positionStart = positionStart;
        this->positionEnd = positionEnd;
    }
};


// محدد الخطأ
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ErrorArrow {
public:
    std::wstring error_arrow(std::wstring input_, Position positionStart, Position positionEnd) {
        std::wstring result;
        int columnStart, columnEnd;

        int newlineIndex = input_.rfind(L"\n", positionStart.index);
        int indexStart = std::max(newlineIndex, 0);
        int indexEnd = input_.find(L"\n", indexStart + 1);
        if (indexEnd < 0) {
            indexEnd = input_.length();
        }

        int lineCount = positionEnd.lineNumber - positionStart.lineNumber + 1;
        for (int i = 0; i < lineCount; i++) {
            std::wstring line = input_.substr(indexStart, indexEnd);
            if (i == 0) { columnStart = positionStart.columnNumber; }
            else { columnStart = 0; }
            if (i == lineCount - 1) { columnEnd = positionEnd.columnNumber + 1; }
            else { columnEnd = line.length() - 1; }

            result += line + L"\n" + std::wstring(columnStart, ' ') + std::wstring(columnEnd - columnStart, '^');

            indexStart = indexEnd;
            indexEnd = input_.find(L"\n", indexStart + 1);
            if (indexEnd < 0) { indexEnd = input_.length(); }
        }

        replace(result.begin(), result.end(), '\t', ' ');
        return result;
    }
};

// أخطاء
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Error {
public:
    Position positionStart, positionEnd;
    std::wstring errorName;
    wchar_t details;
    Error() {}
    Error(Position positionStart, Position positionEnd, std::wstring errorName, wchar_t details) : positionStart(positionStart), positionEnd(positionEnd), errorName(errorName), details(details) {}

    std::wstring print_() {
        std::wstring result = this->errorName + L": " + this->details + L"\n";
        result += L"الملف " + this->positionStart.fileName + L", السطر " + std::to_wstring(this->positionStart.lineNumber + 1);
        result += L"\n\n" + ErrorArrow().error_arrow(this->positionStart.input_, this->positionStart, this->positionEnd);

        return result;
    }
};

class SyntaxError : public Error {
public:
    SyntaxError(Position positionStart, Position positionEnd, wchar_t details) : Error(positionStart, positionEnd, L"خطأ في النسق", details) {
    }
};


// المعرب اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Lexer {
public:
    std::wstring fileName, input_;
    wchar_t currentChar;
    Position position;
    std::list<Token> tokens;
    Error* error;
    Lexer(std::wstring fileName, std::wstring input_) : fileName(fileName), input_(input_), position(Position(-1, 0, -1, fileName, input_)), currentChar(L'\0') {
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

    void reverse() {
        this->position.reverse(this->currentChar);
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
                this->tokens.push_back(Token(plusT, this->position));
                this->advance();

            }
            else if (this->currentChar == L'-')
            {
                this->tokens.push_back(Token(minusT, this->position));
                this->advance();

            }
            else if (this->currentChar == L'*')
            {
                this->tokens.push_back(Token(multiplyT, this->position));
                this->advance();

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
                this->tokens.push_back(Token(powerT, this->position));
                this->advance();

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
                wchar_t character = this->currentChar;
                error = new SyntaxError(this->position, this->position, L'< حرف غير معروف \'' + character + L'\' >');
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
            error = new SyntaxError(this->position, this->position, L'\'' + this->currentChar + L'\'');
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
        this->advance();
        if (ClosedString)
        {
            this->tokens.push_back(Token(stringT, string_, positionStart, this->position));
        }
        else {
            this->reverse();
            error = new SyntaxError(this->position, this->position, L'< لم يتم إغلاق النص >');
        }
    }

    void make_divide() {
        std::wstring divide_ = L"";
        unsigned int slashCount = 0;
        wchar_t char_;
        Position positionStart = this->position;

        while (this->currentChar != L'\0' and this->currentChar == L'\\') {
            if (this->currentChar == L'\\') {
                slashCount += 1;
            }
            divide_ += this->currentChar;
            char_ = this->currentChar;
            this->advance();
        }

        if (slashCount == 1) {
            this->tokens.push_back(Token(divideT, this->position));
        }
        else if (slashCount == 2) {
            this->tokens.push_back(Token(remainT, this->position));
        }
        else {
            this->reverse();
            error = new SyntaxError(this->position, this->position, L'< حرف زائد \'' + char_ + L'\' >');
        }
    }

    void make_not_equals() {
        Position positionStart = this->position;
        this->advance();

        if (this->currentChar == L'=') {
            this->advance();
            this->tokens.push_back(Token(notEqualT, positionStart));
        }

        error = new SyntaxError(this->position, this->position, L'< يتوقع وجود \'=\' بعد إشارة \'!\' >');
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
        while (this->currentChar != L'\n') {
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
                if (token.value_ == L"")
                {
                    result += L"pos start: " + std::to_wstring(token.positionStart.index) + L", " + token.type_ + L", \n";
                }
                else
                {
                    result += L"pos start: " + std::to_wstring(token.positionStart.index) + L" ,pos end: " + std::to_wstring(token.positionEnd.index) + L", " + token.type_ + L" : " + token.value_ + L", \n";
                }
            }
            std::wcout << L"نتائج المعرب اللغوي : \n" << result << std::endl;
        }
    }
};

// نتائج المحلل اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum NodeType {
    Undefined,
    NumberNode,
    StringNode,
    UnaryOpNode,
    BinOpNode,
    VarAccessNode,
    VarAssignNode,
    StatementCondationNode,
    ListNode,

};

class Node {
public:
    Token token;
    NodeType type;
    Node* left;
    Node* right;
    std::list<Node*> list_;

    Node(Node* left, Token token, std::list<Node*> list_ ,Node* right, NodeType nodeType) {
        this->left = left;
        this->token = token;
        this->right = right;
        this->type = nodeType;
        this->list_ = list_;
    }

    ~Node() {
        delete left;
        delete right;
    }

};


class Parser {
public:
    std::list<Token> tokens;
    int tokenIndex;
    Token currentToken;
    Node* node;
    Error* error;

    Parser(std::list<Token> tokens) : tokens(tokens)
    {
        tokenIndex = -1;
        this->advance();
    }

    ~Parser()
    {
        delete node;
        delete error;
    }

    void advance()
    {
        this->tokenIndex++;
        if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens.size())
        {
            std::list<Token>::iterator listIter = tokens.begin();
            std::advance(listIter, this->tokenIndex);
            this->currentToken = *listIter;
        }
    }

    void parse()
    {
        this->expr();
    }

    //////////////////////////////

    void atom() {
        Token token = this->currentToken;

        if (token.type_ == integerT or token.type_ == floatT)
        {
            this->advance();
            node = new Node(nullptr, token, std::list<Node*>(), nullptr, NumberNode); // قم بإنشاء صنف عقدة جديد ومرر فيه الرمز الذي تم حفظه في متغير رمز ومرر نوع العقدة المنشأءة واسندها الى متغير عقدة

        }
        else if (token.type_ == stringT) {
            this->advance();
            node = new Node(nullptr, token, std::list<Node*>(), nullptr, StringNode);

        }
        else if (token.type_ == nameT)
        {
            this->advance();
            node = new Node(nullptr, token, std::list<Node*>(), nullptr, VarAccessNode);

        }
        else if (token.type_ == keywordT and token.value_ == L"صح")
        {
            this->advance();
            node = new Node(nullptr, token, std::list<Node*>(), nullptr, StatementCondationNode);

        }
        else if (token.type_ == keywordT and token.value_ == L"خطأ")
        {
            this->advance();
            node = new Node(nullptr, token, std::list<Node*>(), nullptr, StatementCondationNode);

        }
        else if (token.type_ == keywordT and token.value_ == L"عدم")
        {
            this->advance();
            node = new Node(nullptr, token, std::list<Node*>(), nullptr, StatementCondationNode);

        }
        else if (token.type_ == lSquareT)
        {
            this->advance();
            this->list_expr();
        }
    }

    void list_expr() 
    {
        Token token = this->currentToken;
        std::list<Node*> nodeElement;

        if (this->currentToken.type_ == rSquareT)
        {
            this->advance();
        }
        else
        {
            this->expr(); // تقوم بتنفيذ التعبير وضبط نتيجة العملية في متغير node
            nodeElement.push_back(node);

            while (this->currentToken.type_ == commaT) {
                this->advance();
                this->expr();
                nodeElement.push_back(node);

            }

            if (this->currentToken.type_ != rSquareT)
            {
                // error
            }
            this->advance();
        }

        node = new Node(nullptr, Token(), nodeElement, nullptr, ListNode);

    }

    void factor() {
        Token token = this->currentToken;
        Node* factor;

        if (token.type_ == plusT or token.type_ == minusT) {
            this->advance();
            this->factor();
            factor = node;
            node = new Node(nullptr, token, std::list<Node*>(), factor, UnaryOpNode);

        }

        this->power();
    }

    void power()
    {
        bin_op_repeat(&Parser::atom, powerT, L" ", &Parser::factor);
    }

    void term() {
        bin_op_repeat(&Parser::factor, multiplyT, divideT, &Parser::factor);
    }

    void expr() {
        Node* expr;

        if (this->currentToken.type_ == nameT)
        {
            Token varName = this->currentToken;
            this->advance();
            if (this->currentToken.type_ == equalT)
            {
                this->advance();
                this->expr(); // نفذ المعادلة وضع القيم في node
                expr = node;
                node = new Node(nullptr, varName, std::list<Node*>(), expr, VarAccessNode);
                return;
            }
        }

        bin_op_repeat(&Parser::term, plusT, minusT, &Parser::term);
    }

    void bin_op_repeat(void(Parser::* funcL)(), std::wstring fop, std::wstring sop, void(Parser::* funcR)()) {
        Token opToken;
        Node* left;
        Node* right;

        (this->*funcL)();
        left = node;

        while (this->currentToken.type_ == fop or this->currentToken.type_ == sop) {
            opToken = this->currentToken;
            this->advance();
            (this->*funcR)();

            right = node;

            left = new Node(left, opToken, std::list<Node*>(), right, BinOpNode);
        }
        node = left;
    }


    // النتائج في الوقت الفعلي
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void print_node(Node* root, int space = 0, int t = 0) {
        int count = 5;

        if (root == NULL)
            return;
        space += count;

        print_node(root->right, space, 1);

        for (int i = count; i < space; i++) {
            std::wcout << L" ";
        }
        if (t == 1) {
            std::wcout << L"/ " << root->token.type_ << std::endl;
        }
        else if (t == 2) {
            std::wcout << L"\\ " << root->token.type_ << std::endl;
        }
        else {
            std::wcout << root->token.type_ << std::endl;
        }
        print_node(root->left, space, 2);
    }

};

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    std::wstring input_;

    while (true) {
        std::wcout << L"alif -> ";
        std::getline(std::wcin, input_);


        if (input_ == L"خروج") {
            exit(0);
        }

        // المعرب اللغوي
        /////////////////////////////////////////////////////////////////

        clock_t start = clock(); // بداية حساب الوقت


        std::wstring fileName = L"الملف_الرئيسي";
        Lexer lexer(fileName, input_);
        lexer.make_token();
        //lexer.print();



        // المحلل اللغوي
        /////////////////////////////////////////////////////////////////

        Parser parser = Parser(lexer.tokens);
        parser.parse();
        Node* AST = parser.node;
        
        //parser.print_node(AST);
        //for (int i = 0; i < AST->list_.size(); i++) {
        //    std::list<Node*> ::iterator listIter = AST->list_.begin();
        //    std::advance(listIter, i);
        //    Node* a = *listIter;
        //    parser.print_node(a);
        //}

        std::wcout << float(clock() - start) / CLOCKS_PER_SEC << std::endl; // طباعة نتائج الوقت

    }
}
