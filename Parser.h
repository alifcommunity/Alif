#pragma once

// نتائج المحلل اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum NodeType {
    NumberNode,
    StringNode,
    UnaryOpNode,
    BinOpNode,
    CompareNode,
    VarAccessNode,
    VarAssignNode,
    RetVarAssignNode,
    CondationNode,
    ListNode,
    NameCallNode,
    //NameCallArgsNode,
    //BuildInFunctionNode,
    LogicNode,
    ExpressionNode,
    //MultiStatementNode,

};

class Node {
public:
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
    std::shared_ptr<Token> token;
    std::shared_ptr<std::vector<Token>> list_;
    NodeType type;

    Node(){}
    Node(NodeType nodeType, std::shared_ptr<Token> token = nullptr, std::shared_ptr<Node> left = nullptr, std::shared_ptr<Node> right = nullptr, std::shared_ptr<std::vector<Token>> list_ = nullptr) {
        this->left = left;
        this->right = right;
        this->token = token;
        this->list_ = list_;
        this->type = nodeType;
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
        this->statements();
        //node = this->visit(node);
        //std::wcout << node.token->value << std::endl;
    }

    //////////////////////////////

    void atom() {
        std::shared_ptr<Token> token = this->currentToken;

        if (token->type == nameT)
        {
            if (token->value == L"صح" or token->value == L"خطا" or token->value == L"عدم")
            {
                this->advance();
                node = Node(CondationNode, token);
            }
            else {
                this->advance();
                node = Node(VarAccessNode, token);

            }

        }
        else if (token->type == integerT or token->type == floatT)
        {
            this->advance();
            node = Node(NumberNode, token);

        }
        else if (token->type == stringT) {
            this->advance();
            node = Node(StringNode, token);

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
            node = Node(NameCallNode, name, std::make_shared<Node>(node));
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
                node = Node(NameCallNode, name, std::make_shared<Node>(node));

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
                node = Node(NameCallNode, name, std::make_shared<Node>(node));

            }
            else
            {
                // error
            }
        }
    }

    void power()
    {
        binary_operation(&Parser::primary, powerT, Undefined, &Parser::factor);
    }

    void factor() {

        if (this->currentToken->type == plusT or this->currentToken->type == minusT) {
            std::shared_ptr<Token> token = this->currentToken;
            this->advance();
            this->factor();
            node = Node(UnaryOpNode, token, std::make_shared<Node>(node));
        }
        else
        {
            this->power();
        }
    }

    void term() {
        binary_operation(&Parser::factor, multiplyT, divideT, &Parser::term);
    }

    void sum() {
        binary_operation(&Parser::term, plusT, minusT, &Parser::sum);
    }

    void comparesion() {
        Node left;

        this->sum();
        left = node;

        while (this->currentToken->type == equalEqualT or this->currentToken->type == notEqualT or this->currentToken->type == lessThanT or this->currentToken->type == greaterThanT or this->currentToken->type == lessThanEqualT or this->currentToken->type == greaterThanEqualT) {
            std::shared_ptr<Token> opToken = this->currentToken;

            this->advance();
            this->sum();

            Node right = node;

            left = Node(CompareNode, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;

    }

    void inversion() {

        if (this->currentToken->value == L"ليس")
        {
            std::shared_ptr<Token> token = this->currentToken;
            this->advance();
            this->inversion();
            node = Node(UnaryOpNode, token, std::make_shared<Node>(node));

        }
        else {
            this->comparesion();

        }
    }

    void conjuction() {
        Node left;

        this->inversion();
        left = node;

        while (this->currentToken->value == L"و") {
            std::shared_ptr<Token> opToken = this->currentToken;

            this->advance();
            this->inversion();

            Node right = node;

            left = Node(LogicNode, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;
    }

    void disjuction() {
        Node left;

        this->conjuction();
        left = node;

        while (this->currentToken->value == L"او") {
            std::shared_ptr<Token> opToken = this->currentToken;

            this->advance();
            this->conjuction();

            Node right = node;

            left = Node(LogicNode, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;
    }

    void expression() {
        Node left;

        this->disjuction();
        left = node;

        if (this->currentToken->value == L"اذا")
        {
            std::shared_ptr<Token> token = this->currentToken;

            this->advance();
            this->disjuction();
            Node right = node;

            Node condetionNode = Node(CompareNode, token, std::make_shared<Node>(left), std::make_shared<Node>(right));
            
            if (this->currentToken->value == L"والا")
            {
                token = this->currentToken;

                this->advance();
                this->expression();
                right = node;

                node = Node(ExpressionNode, token, std::make_shared<Node>(condetionNode), std::make_shared<Node>(right));
            }
            else
            {
                //error
            }
        }
        else
        {
            node = left;
        }

    }

    void expressions() {
        this->expression();
    }

    void class_defination() {
        expressions();
    }

    void function_defination() {
        class_defination();
    }

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

    void assignment() {

        if (this->currentToken->type == nameT)
        {
            std::shared_ptr<Token> varName = this->currentToken;

            this->expression();
            Node varAccess = node;

            if (this->currentToken->type == equalT)
            {
                this->advance();
                this->assignment();
                node = Node(VarAssignNode, varName, std::make_shared<Node>(node));
            }
            else if (this->currentToken->type == plusEqualT or this->currentToken->type == minusEqualT or this->currentToken->type == multiplyEqualT or this->currentToken->type == divideEqualT)
            {
                std::shared_ptr<Token> operation = this->currentToken;
                this->advance();
                this->assignment();
                node = Node(RetVarAssignNode, operation, std::make_shared<Node>(node), std::make_shared<Node>(varAccess));
            }
        }
        else
        {
            this->expression();
        }

    }

    void compound_statement() {}

    void simple_statement()
    {
        this->assignment();
        
        //if (StatementDone)
        //{
            //this->expression();
        //}
    }

    void statement() {
        if (this->currentToken->value == L"دالة" or this->currentToken->value == L"اذا" or this->currentToken->value == L"صنف" or this->currentToken->value == L"لاجل" or this->currentToken->value == L"بينما")
        {
            this->compound_statement();
        }
        else
        {
            simple_statement();
        }
    }

    void statements() {
        statement();
        while (this->currentToken->type != endOfFileT)
        {
            Node result = this->visit(node);
            std::wcout << result.token->value << std::endl;
            this->advance();
            statement();
        }
    }


    void binary_operation(void(Parser::* funcL)(), TokenType fop, TokenType sop, void(Parser::* funcR)()) {
        Node left;

        (this->*funcL)();
        left = node;

        while (this->currentToken->type == fop or this->currentToken->type == sop) {
            std::shared_ptr<Token> opToken = this->currentToken;

            this->advance();
            (this->*funcR)();

            Node right = node;

            left = Node(BinOpNode, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;
    }





    std::map<std::wstring, Node> namesTable;

    Node visit(Node node)
    {
    
        if (node.type == NumberNode)
        {
            return node;
        }
        else if (node.type == BinOpNode)
        {
            return this->binary_op_interprete(node);
        }
        else if (node.type == UnaryOpNode)
        {
            return this->unary_op_interprete(node);
        }
        else if (node.type == CompareNode)
        {
            return this->compare_op_interprete(node);
        }
        else if (node.type == LogicNode)
        {
            return this->logic_op_interprete(node);
        }
        else if (node.type == ExpressionNode)
        {
            return this->expreesion_interprete(node);
        }
        else if (node.type == VarAssignNode)
        {
            return this->var_assign_interpreter(node);
        }
        else if (node.type == RetVarAssignNode)
        {
            return this->return_var_assign(node);
        }
        else if (node.type == VarAccessNode)
        {
            return this->var_access_interperte(node);
        }
    }

    Node binary_op_interprete(Node node)
    {
        Node left = this->visit(*node.left);
        Node right = this->visit(*node.right);
        Node result = Node(left.type, std::make_shared<Token>(Token()));

        if (node.token->type == plusT)
        {
            result.token->value = std::to_wstring(std::stof(left.token->value) + std::stof(right.token->value));
        }
        else if (node.token->type == minusT)
        {
            result.token->value = std::to_wstring(std::stof(left.token->value) - std::stof(right.token->value));
        }
        else if (node.token->type == multiplyT)
        {
            result.token->value = std::to_wstring(std::stof(left.token->value) * std::stof(right.token->value));
        }
        else if (node.token->type == divideT)
        {
            result.token->value = std::to_wstring(std::stof(left.token->value) / std::stof(right.token->value));
        }
        else if (node.token->type == powerT)
        {
            result.token->value = std::to_wstring(pow(std::stof(left.token->value), std::stof(right.token->value)));
        }
        return result;
    }

    Node unary_op_interprete(Node node)
    {
        Node left = this->visit(*node.left);
        Node result = Node(left.type, std::make_shared<Token>(Token()));

        if (node.token->type == plusT)
        {
            result.token->value = std::to_wstring(+ std::stof(left.token->value));
        }
        else if (node.token->type == minusT)
        {
            result.token->value = std::to_wstring(- std::stof(left.token->value));
        }
        else
        {
            if (left.token->value == L"0")
            {
                result.token->value = L"1";
            }
            else
            {
                result.token->value = L"0";
            }
        }
        return result;

    }

    Node compare_op_interprete(Node node)
    {
        Node left = this->visit(*node.left);
        Node right = this->visit(*node.right);

        if (node.token->type == equalEqualT)
        {
            left.token->value = std::to_wstring(std::stof(left.token->value) == std::stof(right.token->value));
        }
        else if (node.token->type == notEqualT)
        {
            left.token->value = std::to_wstring(std::stof(left.token->value) != std::stof(right.token->value));
        }
        else if (node.token->type == lessThanT)
        {
            left.token->value = std::to_wstring(std::stof(left.token->value) < std::stof(right.token->value));
        }
        else if (node.token->type == greaterThanT)
        {
            left.token->value = std::to_wstring(std::stof(left.token->value) > std::stof(right.token->value));
        }
        else if (node.token->type == lessThanEqualT)
        {
            left.token->value = std::to_wstring(std::stof(left.token->value) <= std::stof(right.token->value));
        }
        else if (node.token->type == greaterThanEqualT)
        {
            left.token->value = std::to_wstring(std::stof(left.token->value) >= std::stof(right.token->value));
        }
        return left;
    }

    Node logic_op_interprete(Node node)
    {
        Node left = this->visit(*node.left);
        Node right = this->visit(*node.right);

        if (node.token->value == L"و")
        {
            if (left.token->value != L"0" and right.token->value != L"0")
            {
                left.token->value = L"1";
            }
            else
            {
                left.token->value = L"0";
            }
        }
        else if (node.token->value == L"او")
        {
            if (left.token->value != L"0" or right.token->value != L"0")
            {
                left.token->value = L"1";
            }
            else
            {
                left.token->value = L"0";
            }
        }
        return left;
    }

    Node expreesion_interprete(Node node)
    {

        Node condetion = this->visit(*node.left->right);
        if (condetion.token->value == L"1")
        {
            return this->visit(*node.left);
        }
        else
        {
            return this->visit(*node.right);
        }
    }

    Node var_assign_interpreter(Node node)
    {
        Node left = this->visit(*node.left);
        namesTable[node.token->value] = left;
        return namesTable[node.token->value];
    }

    Node var_access_interperte(Node node)
    {
        return namesTable[node.token->value];
    }

    Node return_var_assign(Node node)
    {
        Node left = this->visit(*node.left);
        Node right = this->visit(*node.right);

        if (node.token->type == plusEqualT)
        {
            right.token->value = std::to_wstring(std::stof(left.token->value) + std::stof(right.token->value));
        }
        else if (node.token->type == minusEqualT)
        {
            right.token->value = std::to_wstring(std::stof(left.token->value) - std::stof(right.token->value));
        }
        else if (node.token->type == multiplyEqualT)
        {
            right.token->value = std::to_wstring(std::stof(left.token->value) * std::stof(right.token->value));
        }
        else if (node.token->type == divideEqualT)
        {
            right.token->value = std::to_wstring(std::stof(left.token->value) / std::stof(right.token->value));
        }
        else if (node.token->type == powerEqualT)
        {
            right.token->value = std::to_wstring(pow(std::stof(left.token->value), std::stof(right.token->value)));
        }
        return right;

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
