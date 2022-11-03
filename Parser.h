#pragma once

// نتائج المحلل اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum NodeType {
    numberNode,
    stringNode,
    unaryOpNode,
    binOpNode,
    varAccessNode,
    varAssignNode,
    condationNode,
    listNode,
    nameCallNode,
    //NameCallArgsNode,
    //BuildInFunctionNode, // تجربة فقط
    //InverseNode,
    //LogicNode,
    //ExpressionsNode,
    //MultiStatementNode,

};

class Node {
public:
    //std::shared_ptr<Node> left = nullptr;
    //std::shared_ptr<Node> right = nullptr;
    //std::shared_ptr<Token> token = nullptr;
    //std::shared_ptr<std::vector<Token>> list_ = nullptr;
    //NodeType type;

    Node(){}
    //Node(NodeType nodeType, std::shared_ptr<Token> token = nullptr, std::shared_ptr<Node> left = nullptr, std::shared_ptr<Node> right = nullptr, std::shared_ptr<std::vector<Token>> list_ = nullptr) {
    //    this->left = left;
    //    this->right = right;
    //    this->token = token;
    //    this->list_ = list_;
    //    this->type = nodeType;
    //}

};

class NumberNode : public Node {
public:
    std::shared_ptr<Token> token = nullptr;
    NodeType type;

    NumberNode(NodeType nodeType, std::shared_ptr<Token> token)
    {
        this->type = nodeType;
        this->token = token;
    }
};


class VarAccessNode : public Node {
public:
    std::shared_ptr<Token> token = nullptr;
    NodeType type;

    VarAccessNode(NodeType nodeType, std::shared_ptr<Token> token)
    {
        this->type = nodeType;
        this->token = token;
    }

};

class CondetionNode : public Node {
public:
    std::shared_ptr<Token> token = nullptr;
    NodeType type;

    CondetionNode(NodeType nodeType, std::shared_ptr<Token> token)
    {
        this->type = nodeType;
        this->token = token;
    }

};

class StringNode : public Node {
public:
    std::shared_ptr<Token> token = nullptr;
    NodeType type;

    StringNode(NodeType nodeType, std::shared_ptr<Token> token)
    {
        this->type = nodeType;
        this->token = token;
    }

};

class NameCallNode : public Node {
public:
    std::shared_ptr<Token> token = nullptr;
    std::shared_ptr<Node> left = nullptr;
    NodeType type;

    NameCallNode(NodeType nodeType, std::shared_ptr<Token> token, std::shared_ptr<Node> left)
    {
        this->type = nodeType;
        this->left = left;
        this->token = token;
    }

};

class BinOpNode : public Node {
public:
    std::shared_ptr<Token> token = nullptr;
    std::shared_ptr<Node> left = nullptr;
    std::shared_ptr<Node> right = nullptr;
    NodeType type;

    BinOpNode(NodeType nodeType, std::shared_ptr<Token> token, std::shared_ptr<Node> left, std::shared_ptr<Node> right)
    {
        this->type = nodeType;
        this->left = left;
        this->left = right;
        this->token = token;
    }
};

class UnaryOpNode : public Node {
public:
    std::shared_ptr<Token> token = nullptr;
    std::shared_ptr<Node> left = nullptr;
    NodeType type;

    UnaryOpNode(NodeType nodeType, std::shared_ptr<Token> token, std::shared_ptr<Node> left)
    {
        this->type = nodeType;
        this->left = left;
        this->token = token;
    }
};

// المحلل اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Parser {
public:
    std::vector<Token> tokens;
    int tokenIndex = -1;
    std::shared_ptr<Token> currentToken;
    Node node;
    std::shared_ptr<Error> error;

    Parser(std::vector<Token> tokens) : tokens(tokens)
    {
        this->advance();
    }

    void advance()
    {
        this->tokenIndex++;
        if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens.size())
        {
            std::vector<Token>::iterator listIter = tokens.begin();
            std::advance(listIter, this->tokenIndex);
            this->currentToken = std::make_shared<Token>(* listIter);
        }
    }

    //void reverse(std::uint8_t count = 1) {
    //    this->tokenIndex -= count;
    //    if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens.size()) {
    //        std::vector<Token>::iterator listIter = tokens.begin();
    //        std::advance(listIter, this->tokenIndex);
    //        this->currentToken = *listIter;
    //    }
    //}

    void parse()
    {
        this->sum();
        //this->visit(node);
    }

    //////////////////////////////

    void atom() {
        std::shared_ptr<Token> token = this->currentToken;

        if (token->type == nameT)
        {
            if (token->value == L"صح" or token->value == L"خطا" or token->value == L"عدم")
            {
                this->advance();
                node = CondetionNode(condationNode, token);
            }
            else {
                this->advance();
                node = VarAccessNode(varAccessNode, token);

            }

        }
        else if (token->type == integerT or token->type == floatT)
        {
            this->advance();
            node = NumberNode(numberNode, token);

        }
        else if (token->type == stringT) {
            this->advance();
            node = StringNode(stringNode, token);

        }
        else if (token->type == lSquareT)
        {
            this->advance();
            //this->list_expr();
        }
    }

    //void list_expr() 
    //{
    //    Token token = this->currentToken;
    //    std::vector<Node*>* nodeElement = new std::vector<Node*>;

    //    if (this->currentToken.type_ == rSquareT)
    //    {
    //        this->advance();
    //    }
    //    else
    //    {
    //        this->expression(); // تقوم بتنفيذ التعبير وضبط نتيجة العملية في متغير node
    //        (*nodeElement).push_back(node);

    //        while (this->currentToken.type_ == commaT) {
    //            this->advance();
    //            this->expression();
    //            (*nodeElement).push_back(node);

    //        }

    //        if (this->currentToken.type_ != rSquareT)
    //        {
    //            error = new SyntaxError(this->currentToken.positionStart, this->currentToken.positionStart, L"لم يتم إغلاق قوس المصفوفة");
    //        }
    //        this->advance();
    //    }

    //    node = new Node(nullptr, token, nullptr, ListNode, nodeElement);

    //}

    void primary() {
        std::shared_ptr<Token> name = this->currentToken;

        this->atom();
        if (this->currentToken->type == dotT)
        {
            this->advance();
            this->primary();
            node = NameCallNode(nameCallNode, name, std::make_shared<Node>(node));
        }
        else if (this->currentToken->type == lParenthesisT)
        {
            this->advance();
            if (this->currentToken->type != rParenthesisT)
            {
                //this->parameters();
                if (this->currentToken->type == rParenthesisT)
                {
                    this->advance();
                }
                else
                {
                    // error
                }
            }
            else if (this->currentToken->type == rParenthesisT)
            {
                this->advance();
                node = NameCallNode(nameCallNode, name, std::make_shared<Node>(node));

            }
            else
            {
                // error
            }
        }
        else if (this->currentToken->type == lSquareT)
        {
            this->advance();
            if (this->currentToken->type != rSquareT)
            {
                //this->slices();
                if (this->currentToken->type == rSquareT)
                {
                    this->advance();
                }
                else
                {
                    // error
                }
            }
            else if (this->currentToken->type == rSquareT)
            {
                this->advance();
                node = NameCallNode(nameCallNode, name, std::make_shared<Node>(node));

            }
            else
            {
                // error
            }
        }
    }

    void power()
    {
        binary_operation(&Parser::primary, powerT, L"", &Parser::factor);
    }

    void factor() {
        std::shared_ptr<Token> token = this->currentToken;

        if (token->type == plusT or token->type == minusT) {
            this->advance();
            this->factor();
            node = UnaryOpNode(unaryOpNode, token, std::make_shared<Node>(node));
        }
        this->power();
    }

    void term() {
        binary_operation(&Parser::factor, multiplyT, divideT, &Parser::term);
    }

    void sum() {
        binary_operation(&Parser::term, plusT, minusT, &Parser::sum);
    }

    //void inversion() {
    //    Token token = this->currentToken;
    //    Node* expr;

    //    if (token.type_ == keywordT and token.value_ == L"ليس")
    //    {
    //        this->advance();
    //        this->sum();
    //        expr = node;
    //        node = new Node(nullptr, token, expr, InverseNode);

    //    }
    //    else {
    //        this->sum();

    //    }
    //}

    //void conjuction() {
    //    Token opToken;
    //    Node* left;
    //    Node* right;

    //    this->inversion();
    //    left = node;

    //    while (this->currentToken.type_ == keywordT and this->currentToken.value_ == L"و") {
    //        opToken = this->currentToken;
    //        opToken.type_ = L"And";
    //        this->advance();
    //        this->conjuction();

    //        right = node;

    //        left = new Node(left, opToken, right, LogicNode);
    //    }
    //    node = left;
    //}

    //void disjuction() {
    //    Token opToken;
    //    Node* left;
    //    Node* right;

    //    this->conjuction();
    //    left = node;

    //    while (this->currentToken.type_ == keywordT and this->currentToken.value_ == L"او") {
    //        opToken = this->currentToken;
    //        opToken.type_ = L"Or";
    //        this->advance();
    //        this->disjuction();

    //        right = node;

    //        left = new Node(left, opToken, right, LogicNode);
    //    }
    //    node = left;
    //}

    //void expression() {
    //    Token token = this->currentToken;
    //    Node* right;
    //    Node* left;

    //    this->disjuction();
    //    left = node;

    //    if (this->currentToken.type_ == keywordT and this->currentToken.value_ == L"اذا")
    //    {
    //        token = this->currentToken;
    //        this->advance();
    //        this->disjuction();
    //        right = node;
    //        node = new Node(left, token, right, BinOpNode); 
    //        if (this->currentToken.type_ == keywordT and this->currentToken.value_ == L"والا")
    //        {
    //            this->expression();
    //            right = node;
    //            node = new Node(left, this->currentToken, right, BinOpNode);
    //        }
    //    }
    //    else
    //    {
    //        node = left;

    //    }
    //}

    //void expressions() {
    //    Token opToken;
    //    Node* left;
    //    Node* right;

    //    this->expression();
    //    left = node;

    //    while (this->currentToken.type_ == commaT) {
    //        opToken = this->currentToken;
    //        this->advance();
    //        this->expression();

    //        right = node;

    //        left = new Node(left, opToken, right, ExpressionsNode);
    //    }
    //    node = left;
    //}

    //void class_defination() {
    //    expressions();
    //}

    //void function_defination() {
    //    class_defination();
    //}

    //void return_statement() {
    //    function_defination();
    //}

    //void while_statement() {
    //    return_statement();
    //}

    //void if_statement() {
    //    while_statement();
    //}

    //void import_from() {
    //    if_statement();
    //}

    //void import_name() {
    //    import_from();
    //}

    //void import_statement() {
    //    import_name();
    //}

    //void delete_statement() {
    //    import_statement();
    //}

    //void augassign() {
    //    delete_statement();
    //}

    //void assignment() {
    //    Token varName = this->currentToken;
    //    Node* expr;

    //    this->augassign();

    //    if (varName.type_ == nameT)
    //    {
    //        if (this->currentToken.type_ == equalT)
    //        {
    //            this->advance();
    //            this->assignment(); // نفذ المعادلة وضع القيم في node
    //            expr = node;
    //            node = new Node(nullptr, varName, expr, VarAssignNode);
    //        }
    //        //else {
    //        //    this->reverse();
    //        //}
    //    }
    //}

    //void compound_statement() {}

    //void simple_statement()
    //{
    //    assignment();
    //}

    //void statement() {
    //    simple_statement();
    //}

    //void statements() { // تجربة القراءة من ملف متعدد الاسطر
    //    Token token ;
    //    Node* right;
    //    Node* left;

    //    statement();

    //    token = this->currentToken;

    //    while (this->currentToken.type_ == newlineT)
    //    {
    //        this->advance();
    //        if (this->currentToken.type_ != newlineT)
    //        {
    //            right = node;
    //            if (this->currentToken.type_ != endOfFileT)
    //            {
    //                this->statements();
    //                left = node;
    //                node = new Node(left, token, right, MultiStatementNode);

    //            }

    //        }
    //    }
    //}



    void binary_operation(void(Parser::* funcL)(), std::wstring fop, std::wstring sop, void(Parser::* funcR)()) {
        std::shared_ptr<Token> opToken;
        Node left;
        Node right;

        (this->*funcL)();
        left = node;

        while (this->currentToken->type == fop or this->currentToken->type == sop) {
            opToken = this->currentToken;
            this->advance();
            (this->*funcR)();

            right = node;

            left = BinOpNode(binOpNode, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;
    }

    Node visit(Node node)
    {
        
    }

    //// طباعة نتائج المحلل اللغوي
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //void print_node(Node* root, int space = 0, int t = 0) {

    //    if (error) {
    //        std::wcout << error->print_() << std::endl;
    //    }
    //    else
    //    {
    //        int count = 7;

    //        if (root == NULL)
    //            return;
    //        space += count;

    //        print_node(root->right, space, 1);

    //        for (int i = count; i < space; i++) {
    //            std::wcout << L" ";
    //        }

    //        if (t == 1) {
    //            std::wcout << L"/ " << root->token.type_ << L": " << root->token.value_ << std::endl;
    //        }
    //        else if (t == 2) {
    //            std::wcout << L"\\ " << root->token.type_ << L": " << root->token.value_ << std::endl;
    //        }
    //        else {
    //            std::wcout << root->token.type_ << std::endl;
    //        }
    //        print_node(root->left, space, 2);
    //    }
    //}

};
