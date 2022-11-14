#pragma once

// المحلل اللغوي
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
    FunctionDefine,
    ForLoop,
    WhileLoop,
    LogicNode,
    ExpressionNode,
    MultiStatementNode,
    IfCondetion,

};

class Node {
public:
    std::shared_ptr<Node> left{};
    std::shared_ptr<Node> right{};
    std::shared_ptr<Token> token{};
    std::shared_ptr<std::vector<Token>> list_{};
    NodeType type{};

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
    std::vector<Node> list;
    std::shared_ptr<Error> error;

    uint16_t currentBlockCount = 0;
    uint16_t currentTabCount = 0;

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

    void reverse(std::uint8_t count = 1) {
        this->tokenIndex -= count;
        if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens.size()) {
            std::vector<Token>::iterator listIter = tokens.begin();
            std::advance(listIter, this->tokenIndex);
            this->currentToken = std::make_shared<Token>(*listIter);
        }
    }

    void parse()
    {
        this->statements();
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
        Node params;

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
                //this->advance();
                this->parameters();
                params = node;
                if (this->currentToken->type == rParenthesisT)
                {
                    this->advance();
                    node = Node(NameCallNode, name, std::make_shared<Node>(node), std::make_shared<Node>(params));
                }
                else
                {
                    // error
                }
            }
            else if (this->currentToken->type == rParenthesisT)
            {
                this->advance();
                node = Node(NameCallNode, name, std::make_shared<Node>(node), std::make_shared<Node>(params));

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

        this->disjuction();
        Node left = node;

        if (this->currentToken->value == L"اذا")
        {
            std::shared_ptr<Token> token = this->currentToken;

            this->advance();
            this->disjuction();
            Node condetion = Node(MultiStatementNode, token, nullptr, std::make_shared<Node>(node));
            
            if (this->currentToken->value == L"والا")
            {
                token = this->currentToken;

                this->advance();
                this->expression();
                Node right = Node(MultiStatementNode, token, std::make_shared<Node>(condetion), std::make_shared<Node>(node));

                node = Node(ExpressionNode, token, std::make_shared<Node>(right), std::make_shared<Node>(left));
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

    void parameters()
    {
        this->expression();
    }

    void class_defination() {
        expressions();
    }

    void function_defination() {

        if (this->currentToken->value == L"دالة")
        {
            this->advance();
            if (this->currentToken->type == nameT)
            {
                std::shared_ptr<Token> name = this->currentToken;

                this->advance();
                if (this->currentToken->type == lParenthesisT)
                {
                    this->advance();
                    if (this->currentToken->type != rParenthesisT)
                    {
                        this->advance();
                        //this->parameter();
                    }
                    else if (this->currentToken->type == rParenthesisT)
                    {
                        this->advance();
                    }

                    if (this->currentToken->type == colonT)
                    {
                        this->advance();
                        this->func_body(name);
                    }
                }
            }
        }
    }

    void func_body(std::shared_ptr<Token> name)
    {
        if (this->currentToken->type == newlineT) {


            // move list content to other store temporary to start store new body content
            std::vector<Node> tempList = this->list;
            this->list.clear();

            this->advance();

            this->indentent();
            
            this->statements();

            node = Node(FunctionDefine, name, std::make_shared<Node>(node)); // node = body node
            if (currentBlockCount != 0)
            {
                this->list = tempList;
            }
        }
        //else {
        //    this->simple_statement();
        //}
    }

    void indentent ()
    {
        currentBlockCount++;
        while (this->currentToken->type == tabT) {
            this->advance();
        }
        currentTabCount++; // ملاحظة: يجب التقدم بعدد المسافات وليس تقدم مرة واحدة فقط
    }

    void deindentent ()
    {
        currentBlockCount--;
        while (this->currentToken->type == tabT)
        {
            this->advance();
        }
        currentTabCount--;
    }

    //void return_statement() {
    //}

    void while_statement() {
        Node expr;

        this->advance();

        this->expression();
        expr = node;

        if (this->currentToken->type == colonT)
        {
            this->advance();
            this->while_body();
            node = Node(WhileLoop, this->currentToken, std::make_shared<Node>(expr), std::make_shared<Node>(node));
        }
    }

    void while_body()
    {
        if (this->currentToken->type == newlineT)
        {
            // move list content to other store temporary to start store new body content
            std::vector<Node> tempList = this->list;
            this->list.clear();

            this->advance();

            this->indentent();

            this->statements();

            if (currentBlockCount != 0)
            {
                this->list = tempList;
            }
        }
    }

    void for_statement() 
    {
        Node expr;
        std::shared_ptr<Token> name;

        this->advance();
        if (this->currentToken->type == nameT)
        {
            name = this->currentToken;
            this->advance();
            if (this->currentToken->value == L"في")
            {
                this->advance();
                if (this->currentToken->type == lParenthesisT)
                {
                    this->advance();
                    this->expression();
                    expr = node;
                }
                if (this->currentToken->type == rParenthesisT)
                {
                    this->advance();
                }
                if (this->currentToken->type == colonT)
                {
                    this->advance();
                    this->for_body(expr, name);
                }
            }
        }
    }

    void for_body(Node expr, std::shared_ptr<Token> name)
    {
        if (this->currentToken->type == newlineT) {


            // move list content to other store temporary to start store new body content
            std::vector<Node> tempList = this->list;
            this->list.clear();

            this->advance();

            this->indentent();

            this->statements();

            node = Node(ForLoop, name, std::make_shared<Node>(expr), std::make_shared<Node>(node)); // node = body node

            if (currentBlockCount != 0)
            {
                this->list = tempList;
            }
        }
        //else {
        //    this->simple_statement();
        //}
    }

    void if_statement() 
    {
        Node expr;

        this->advance();

        this->expression();
        expr = node;

        if (this->currentToken->type == colonT)
        {
            this->advance();
            this->if_body();
            node = Node(IfCondetion, this->currentToken, std::make_shared<Node>(expr), std::make_shared<Node>(node));
        }
    }

    void if_body()
    {
        if (this->currentToken->type == newlineT)
        {
            // move list content to other store temporary to start store new body content
            std::vector<Node> tempList = this->list;
            this->list.clear();

            this->advance();

            this->indentent();

            this->statements();

            if (currentBlockCount != 0)
            {
                this->list = tempList;
            }
        }
    }

    //void import_from() {
    //}

    //void import_name() {
    //}

    //void import_statement() {
    //}

    //void delete_statement() {
    //    import_statement();
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

    void compound_statement() 
    {
        if (this->currentToken->value == L"دالة")
        {
            this->function_defination();
        }
        else if (this->currentToken->value == L"لاجل")
        {
            this->for_statement();
        }
        else if (this->currentToken->value == L"بينما")
        {
            this->while_statement();
        }
        else if (this->currentToken->value == L"اذا")
        {
            this->if_statement();
        }
    }

    void simple_statement()
    {
        this->assignment();
    }

    void statement() {
        if (this->currentToken->value == L"دالة" or this->currentToken->value == L"اذا" or this->currentToken->value == L"صنف" or this->currentToken->value == L"لاجل" or this->currentToken->value == L"بينما")
        {
            this->compound_statement();
        }
        else
        {
            this->simple_statement();
        }
    }

    void statements() {
        uint16_t tabCount = 0;

        this->statement();

        if (currentBlockCount != 0)
        {
            this->list.push_back(node);
        }

        this->advance();
        
        while (this->currentToken->type == tabT)
        {
            this->advance();
            tabCount++;
        }

        if (currentTabCount != tabCount)
        {
            this->deindentent();
            // for i in list : node = Node(MultiStatementNode, Token(), std::make_shared<Node>(i));
            std::vector<Node>::iterator listIter;
            for (listIter = this->list.begin(); listIter != this->list.end(); ++listIter)
            {
                node = Node(MultiStatementNode, std::make_shared<Token>(Token()), std::make_shared<Node>(node), std::make_shared<Node>(*listIter));
            }
            this->reverse(tabCount + 1);
            return;

        }

        if (currentBlockCount == 0)
        {
            this->visit(node);
        }
        

        if (this->currentToken->type != endOfFileT and error == nullptr)
        {
            this->statements();
        }
        else if (error)
        {
            // error;
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

    // المفسر اللغوي
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Node result;
    std::map<std::wstring, Node> namesTable;
    std::map<std::wstring, void(Parser::*)(Node)> buildinFunction{{L"اطبع", &Parser::print}};


    void visit(Node node)
    {
    
        if (node.type == NumberNode)
        {
            result = node;
        }
        else if (node.type == StringNode)
        {
            result = node;
        }
        else if (node.type == BinOpNode)
        {
            this->binary_op_interprete(node);
        }
        else if (node.type == UnaryOpNode)
        {
            this->unary_op_interprete(node);
        }
        else if (node.type == CompareNode)
        {
            this->compare_op_interprete(node);
        }
        else if (node.type == LogicNode)
        {
            this->logic_op_interprete(node);
        }
        else if (node.type == ExpressionNode)
        {
            this->expreesion_interprete(node);
        }
        else if (node.type == VarAssignNode)
        {
            this->var_assign_interpreter(node);
        }
        else if (node.type == RetVarAssignNode)
        {
            this->return_var_assign(node);
        }
        else if (node.type == VarAccessNode)
        {
            this->var_access_interperte(node);
        }
        else if (node.type == FunctionDefine)
        {
            this->function_define_interprete(node);
        }
        else if (node.type == MultiStatementNode)
        {
            this->multi_statement_interprete(node);
        }
        else if (node.type == NameCallNode)
        {
            this->name_call_interprete(node);
        }
        else if (node.type == ForLoop)
        {
            this->for_interprete(node);
        }
        else if (node.type == WhileLoop)
        {
            this->while_interprete(node);
        }
        else if (node.type == IfCondetion)
        {
            this->if_interprete(node);
        }
    }

    void binary_op_interprete(Node node)
    {
        this->visit(*node.left);
        Node left = result;
        this->visit(*node.right);
        Node right = result;
        Node temp = Node(NumberNode, std::make_shared<Token>(Token()));

        if (node.token->type == plusT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) + std::stof(right.token->value));
        }
        else if (node.token->type == minusT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) - std::stof(right.token->value));
        }
        else if (node.token->type == multiplyT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) * std::stof(right.token->value));
        }
        else if (node.token->type == divideT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) / std::stof(right.token->value));
        }
        else if (node.token->type == powerT)
        {
            temp.token->value = std::to_wstring(pow(std::stof(left.token->value), std::stof(right.token->value)));
        }
        result = temp;
    }

    void unary_op_interprete(Node node)
    {
        this->visit(*node.left);

        if (node.token->type == plusT)
        {
            result.token->value = std::to_wstring(std::stof(result.token->value));
        }
        else if (node.token->type == minusT)
        {
            result.token->value = std::to_wstring(-1 * std::stof(result.token->value));
        }
        else
        {
            if (result.token->value == L"0")
            {
                result.token->value = L"1";
            }
            else
            {
                result.token->value = L"0";
            }
        }

    }

    void compare_op_interprete(Node node)
    {
        this->visit(*node.left);
        Node left = result;
        this->visit(*node.right);
        Node right = result;
        Node temp = Node(NumberNode, std::make_shared<Token>(Token()));

        if (node.token->type == equalEqualT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) == std::stof(right.token->value));
        }
        else if (node.token->type == notEqualT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) != std::stof(right.token->value));
        }
        else if (node.token->type == lessThanT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) < std::stof(right.token->value));
        }
        else if (node.token->type == greaterThanT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) > std::stof(right.token->value));
        }
        else if (node.token->type == lessThanEqualT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) <= std::stof(right.token->value));
        }
        else if (node.token->type == greaterThanEqualT)
        {
            temp.token->value = std::to_wstring(std::stof(left.token->value) >= std::stof(right.token->value));
        }
        result = temp;
    }

    void logic_op_interprete(Node node)
    {
        this->visit(*node.left);
        Node left = result;
        this->visit(*node.right);
        Node right = result;
        Node temp = Node(NumberNode, std::make_shared<Token>(Token()));

        if (node.token->value == L"و")
        {
            if (left.token->value != L"0" and right.token->value != L"0")
            {
                temp.token->value = L"1";
            }
            else
            {
                temp.token->value = L"0";
            }
        }
        else if (node.token->value == L"او")
        {
            if (left.token->value != L"0" or right.token->value != L"0")
            {
                temp.token->value = L"1";
            }
            else
            {
                temp.token->value = L"0";
            }
        }
        result = temp;
    }

    void expreesion_interprete(Node node)
    {

        this->visit(*node.left->right);
        Node condetion = result;

        if (condetion.token->value == L"1")
        {
            this->visit(*node.left);
        }
        else
        {
            this->visit(*node.right);
        }
    }

    void var_assign_interpreter(Node node)
    {
        this->visit(*node.left);
        Node temp = result;
        namesTable[node.token->value] = temp;
    }

    void var_access_interperte(Node node)
    {
        result = namesTable[node.token->value];
    }

    void return_var_assign(Node node)
    {
        this->visit(*node.left);
        Node left = result;
        this->visit(*node.right);
        Node right = result;

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
        result = right;

    }

    void function_define_interprete(Node node)
    {
        namesTable[node.token->value] = *node.left;
    }

    void multi_statement_interprete(Node node)
    {
        if (node.left->type == MultiStatementNode)
        {
            this->visit(*node.left);
        }
        this->visit(*node.right);
    }

    void name_call_interprete(Node node)
    {
        if (buildinFunction[node.token->value])
        {
            (this->*(buildinFunction[node.token->value]))(*node.right);

        }
        else
        {
            this->visit(namesTable[node.token->value]);

        }
    }

    void for_interprete(Node node)
    {
        this->visit(*node.left);
        int value = stoi(result.token->value);

        for (int i = 0; i < value; i++)
        {
            namesTable[node.token->value] = Node(NumberNode, std::make_shared<Token>(Token(Position(), Position(), integerT, std::to_wstring(i))));
            this->visit(*node.right);

        }
    }

    void while_interprete(Node node)
    {
        this->visit(*node.left);

        while (result.token->value != L"0")
        {
            this->visit(*node.right);
            this->visit(*node.left);
        }
    }
    
    void if_interprete(Node node)
    {
        this->visit(*node.left);

        if (result.token->value != L"0")
        {
            this->visit(*node.right);
        }
    }




    void print(Node node)
    {
        this->visit(node);
        std::wcout << result.token->value << std::endl;
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
