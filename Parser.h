#pragma once

class Parser;

// العقدة
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Node {
public:
    std::shared_ptr<Node> left{};
    std::shared_ptr<Node> right{};
    Token token{};
    std::vector<Token> list_{};
    void(Parser::* func)(Node);

    Node(){}
    Node(void(Parser::* func)(Node node), Token token = Token(), std::shared_ptr<Node> left = nullptr, std::shared_ptr<Node> right = nullptr, std::vector<Token> list_ = std::vector<Token>()) {
        this->left = left;
        this->right = right;
        this->token = token;
        this->list_ = list_;
        this->func = func;

    }

};

// المحلل اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Parser {
public:
    std::vector<Token> tokens;
    int tokenIndex = -1;
    Token currentToken;
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
            this->currentToken = *listIter;
        }
    }

    void reverse(std::uint8_t count = 1) {
        this->tokenIndex -= count;
        if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens.size()) {
            std::vector<Token>::iterator listIter = tokens.begin();
            std::advance(listIter, this->tokenIndex);
            this->currentToken = *listIter;
        }
    }

    void parse()
    {
        this->statements();
    }

    //////////////////////////////

    void atom() {
        Token token = this->currentToken;

        if (token.type == nameT)
        {
            if (token.value == L"صح" or token.value == L"خطا" or token.value == L"عدم")
            {
                this->advance();
                node = Node(&Parser::compare_op_interprete, token);
            }
            else 
            {
                this->advance();
                node = Node(&Parser::var_access_interperte, token);

            }

        }
        else if (token.type == integerT or token.type == floatT)
        {
            this->advance();
            node = Node(&Parser::str_num_interpreter, token);

        }
        else if (token.type == stringT) {
            this->advance();
            node = Node(&Parser::str_num_interpreter, token);

        }
        else if (token.type == lSquareT)
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
        Token name = this->currentToken;
        Node params;

        this->atom();
        if (this->currentToken.type == dotT)
        {
            this->advance();
            this->primary();
            node = Node(&Parser::name_call_interpreter, name, std::make_shared<Node>(node));
        }
        else if (this->currentToken.type == lParenthesisT)
        {
            this->advance();
            if (this->currentToken.type != rParenthesisT)
            {
                //this->advance();
                this->parameters();
                params = node;
                if (this->currentToken.type == rParenthesisT)
                {
                    this->advance();
                    node = Node(&Parser::name_call_interpreter, name, std::make_shared<Node>(node), std::make_shared<Node>(params));
                }
                else
                {
                    // error
                }
            }
            else if (this->currentToken.type == rParenthesisT)
            {
                this->advance();
                node = Node(&Parser::name_call_interpreter, name, std::make_shared<Node>(node), std::make_shared<Node>(params));

            }
            else
            {
                // error
            }
        }
        else if (this->currentToken.type == lSquareT)
        {
            this->advance();
            if (this->currentToken.type != rSquareT)
            {
                //this->slices();
                if (this->currentToken.type == rSquareT)
                {
                    this->advance();
                }
                else
                {
                    // error
                }
            }
            else if (this->currentToken.type == rSquareT)
            {
                this->advance();
                node = Node(&Parser::name_call_interpreter, name, std::make_shared<Node>(node));

            }
            else
            {
                // error
            }
        }
    }

    void power()
    {
        binary_operation(&Parser::primary, powerT, None, &Parser::factor);
    }

    void factor() {

        if (this->currentToken.type == plusT or this->currentToken.type == minusT) {
            Token token = this->currentToken;
            this->advance();
            this->factor();
            node = Node(&Parser::unary_op_interprete, token, std::make_shared<Node>(node));
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

        while (this->currentToken.type == equalEqualT or this->currentToken.type == notEqualT or this->currentToken.type == lessThanT or this->currentToken.type == greaterThanT or this->currentToken.type == lessThanEqualT or this->currentToken.type == greaterThanEqualT) {
            Token opToken = this->currentToken;

            this->advance();
            this->sum();

            Node right = node;

            left = Node(&Parser::compare_op_interprete, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;

    }

    void inversion() {

        if (this->currentToken.value == L"ليس")
        {
            Token token = this->currentToken;
            this->advance();
            this->inversion();
            node = Node(&Parser::unary_op_interprete, token, std::make_shared<Node>(node));

        }
        else {
            this->comparesion();

        }
    }

    void conjuction() {
        Node left;

        this->inversion();
        left = node;

        while (this->currentToken.value == L"و") {
            Token opToken = this->currentToken;

            this->advance();
            this->inversion();

            Node right = node;

            left = Node(&Parser::logic_op_interprete, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;
    }

    void disjuction() {
        Node left;

        this->conjuction();
        left = node;

        while (this->currentToken.value == L"او") {
            Token opToken = this->currentToken;

            this->advance();
            this->conjuction();

            Node right = node;

            left = Node(&Parser::logic_op_interprete, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;
    }

    void expression() {

        this->disjuction();
        Node left = node;

        if (this->currentToken.value == L"اذا")
        {
            Token token = this->currentToken;

            this->advance();
            this->disjuction();
            Node condetion = Node(&Parser::multi_statement_interprete, token, nullptr, std::make_shared<Node>(node));
            
            if (this->currentToken.value == L"والا")
            {
                token = this->currentToken;

                this->advance();
                this->expression();
                Node right = Node(&Parser::multi_statement_interprete, token, std::make_shared<Node>(condetion), std::make_shared<Node>(node));

                node = Node(&Parser::expreesion_interprete, token, std::make_shared<Node>(right), std::make_shared<Node>(left));
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

    void return_statement() {

        this->advance();
        this->expression();
        node = Node(&Parser::return_interprete, this->currentToken, std::make_shared<Node>(node));
    }

    void parameters()
    {
        this->expression();
    }

    void class_defination() {
        expressions();
    }

    void function_defination() {

        if (this->currentToken.value == L"دالة")
        {
            this->advance();
            if (this->currentToken.type == nameT)
            {
                Token name = this->currentToken;

                this->advance();
                if (this->currentToken.type == lParenthesisT)
                {
                    this->advance();
                    if (this->currentToken.type != rParenthesisT)
                    {
                        this->advance();
                        //this->parameter();
                    }
                    else if (this->currentToken.type == rParenthesisT)
                    {
                        this->advance();
                    }

                    if (this->currentToken.type == colonT)
                    {
                        this->advance();
                        this->func_body(name);
                    }
                }
            }
        }
    }

    void func_body(Token name)
    {
        if (this->currentToken.type == newlineT) {


            // move list content to other store temporary to start store new body content
            std::vector<Node> tempList = this->list;
            this->list.clear();

            this->advance();

            this->indentent();
            
            this->statements();

            node = Node(&Parser::function_define_interprete, name, std::make_shared<Node>(node)); // node = body node
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
        while (this->currentToken.type == tabT) {
            this->advance();
        }
        currentTabCount++; // ملاحظة: يجب التقدم بعدد المسافات وليس تقدم مرة واحدة فقط
    }

    void deindentent ()
    {
        currentBlockCount--;
        while (this->currentToken.type == tabT)
        {
            this->advance();
        }
        currentTabCount--;
    }


    void while_statement() {
        Node expr;

        this->advance();

        this->expression();
        expr = node;

        if (this->currentToken.type == colonT)
        {
            this->advance();
            this->while_body();
            node = Node(&Parser::while_interprete, this->currentToken, std::make_shared<Node>(expr), std::make_shared<Node>(node));
        }
    }

    void while_body()
    {
        if (this->currentToken.type == newlineT)
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
        Token name;

        this->advance();
        if (this->currentToken.type == nameT)
        {
            name = this->currentToken;
            this->advance();
            if (this->currentToken.value == L"في")
            {
                this->advance();
                if (this->currentToken.type == lParenthesisT)
                {
                    this->advance();
                    this->expression();
                    expr = node;
                }
                if (this->currentToken.type == rParenthesisT)
                {
                    this->advance();
                }
                if (this->currentToken.type == colonT)
                {
                    this->advance();
                    this->for_body(expr, name);
                }
            }
        }
    }

    void for_body(Node expr, Token name)
    {
        if (this->currentToken.type == newlineT) {


            // move list content to other store temporary to start store new body content
            std::vector<Node> tempList = this->list;
            this->list.clear();

            this->advance();

            this->indentent();

            this->statements();

            node = Node(&Parser::for_interprete, name, std::make_shared<Node>(expr), std::make_shared<Node>(node)); // node = body node

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
        std::vector<Node> tempList;

        this->advance();
        this->expression();
        expr = node;

        if (this->currentToken.type == colonT)
        {
            this->advance();
            this->if_body();
            node = Node(&Parser::if_interprete, this->currentToken, std::make_shared<Node>(expr), std::make_shared<Node>(node));
            tempList.push_back(node);
        }

        this->advance();
        while (this->currentToken.value == L"واذا")
        {
            this->advance();
            this->expression();
            expr = node;

            if (this->currentToken.type == colonT)
            {
                this->advance();
                this->if_body();
                node = Node(&Parser::if_interprete, this->currentToken, std::make_shared<Node>(expr), std::make_shared<Node>(node));
                tempList.push_back(node);
            }
            this->advance();
        }

        std::vector<Node>::iterator listIter;
        for (listIter = tempList.begin(); listIter != tempList.end(); ++listIter)
        {
            node = Node(&Parser::multi_statement_interprete, Token(), std::make_shared<Node>(node), std::make_shared<Node>(*listIter));
        }

        this->reverse();
    }

    void if_body()
    {
        if (this->currentToken.type == newlineT)
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

        if (this->currentToken.type == nameT)
        {
            Token varName = this->currentToken;

            this->expression();
            Node varAccess = node;

            if (this->currentToken.type == equalT)
            {
                this->advance();
                this->assignment();
                node = Node(&Parser::var_assign_interpreter, varName, std::make_shared<Node>(node));
            }
            else if (this->currentToken.type == plusEqualT or this->currentToken.type == minusEqualT or this->currentToken.type == multiplyEqualT or this->currentToken.type == divideEqualT)
            {
                Token operation = this->currentToken;
                this->advance();
                this->assignment();
                node = Node(&Parser::return_var_assign, operation, std::make_shared<Node>(node), std::make_shared<Node>(varAccess));
            }
        }
        else
        {
            this->expression();
        }

    }

    void compound_statement() 
    {
        if (this->currentToken.value == L"دالة")
        {
            this->function_defination();
        }
        else if (this->currentToken.value == L"لاجل")
        {
            this->for_statement();
        }
        else if (this->currentToken.value == L"بينما")
        {
            this->while_statement();
        }
        else if (this->currentToken.value == L"اذا")
        {
            this->if_statement();
        }
    }

    void simple_statement()
    {
        if (this->currentToken.value == L"ارجع")
        {
            this->return_statement();
        }
        else if (this->currentToken.type == nameT)
        {
            this->assignment();
        }
    }

    void statement() {
        if (this->currentToken.value == L"دالة" or this->currentToken.value == L"اذا" or this->currentToken.value == L"صنف" or this->currentToken.value == L"لاجل" or this->currentToken.value == L"بينما")
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

        while (this->currentToken.type == tabT) // لتجاهل المسافة تاب بعد السطر
        {
            this->advance();
        }

        this->advance();
        
        while (this->currentToken.type == tabT)
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
                node = Node(&Parser::multi_statement_interprete, Token(), std::make_shared<Node>(node), std::make_shared<Node>(*listIter));
            }
            this->reverse(tabCount + 1);
            return;

        }

        if (currentBlockCount == 0)
        {
            (this->*(node.func))(node); // visit (node.left->func) and pass (node.left) as parameter node
        }
        

        if (this->currentToken.type != endOfFileT and error == nullptr)
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

        while (this->currentToken.type == fop or this->currentToken.type == sop) {
            Token opToken = this->currentToken;

            this->advance();
            (this->*funcR)();

            Node right = node;

            left = Node(&Parser::binary_op_interprete, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
        }
        node = left;
    }

    // المفسر اللغوي
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Node result;
    bool return_ = false;
    std::map<std::wstring, Node> namesTable;
    std::map<std::wstring, void(Parser::*)(Node)> buildinFunction{{L"اطبع", &Parser::print}};

    inline void str_num_interpreter(Node node)
    {
        result = node;
    }

    void binary_op_interprete(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        Node left = result;
        (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
        Node right = result;
        Node temp = Node();

        if (node.token.type == plusT)
        {
            temp.token.value = std::to_wstring(std::stof(left.token.value) + std::stof(right.token.value));
        }
        else if (node.token.type == minusT)
        {
            temp.token.value = std::to_wstring(std::stof(left.token.value) - std::stof(right.token.value));
        }
        else if (node.token.type == multiplyT)
        {
            temp.token.value = std::to_wstring(std::stof(left.token.value) * std::stof(right.token.value));
        }
        else if (node.token.type == divideT)
        {
            temp.token.value = std::to_wstring(std::stof(left.token.value) / std::stof(right.token.value));
        }
        else if (node.token.type == powerT)
        {
            temp.token.value = std::to_wstring(pow(std::stof(left.token.value), std::stof(right.token.value)));
        }
        result = temp;
    }

    void unary_op_interprete(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node

        if (node.token.type == plusT)
        {
            result.token.value = std::to_wstring(std::stof(result.token.value));
        }
        else if (node.token.type == minusT)
        {
            result.token.value = std::to_wstring(-1 * std::stof(result.token.value));
        }
        else
        {
            if (result.token.value == L"0")
            {
                result.token.value = L"1";
            }
            else
            {
                result.token.value = L"0";
            }
        }

    }

    void compare_op_interprete(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        Node left = result;
        (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
        Node right = result;
        Node temp = Node(nullptr, Token());

        if (node.token.type == equalEqualT)
        {
            temp.token.value = std::to_wstring(left.token.value == right.token.value);
        }
        else if (node.token.type == notEqualT)
        {
            temp.token.value = std::to_wstring(left.token.value != right.token.value);
        }
        else if (node.token.type == lessThanT)
        {
            temp.token.value = std::to_wstring(std::stof(left.token.value) < std::stof(right.token.value));
        }
        else if (node.token.type == greaterThanT)
        {
            temp.token.value = std::to_wstring(std::stof(left.token.value) > std::stof(right.token.value));
        }
        else if (node.token.type == lessThanEqualT)
        {
            temp.token.value = std::to_wstring(std::stof(left.token.value) <= std::stof(right.token.value));
        }
        else if (node.token.type == greaterThanEqualT)
        {
            temp.token.value = std::to_wstring(std::stof(left.token.value) >= std::stof(right.token.value));
        }
        result = temp;
    }

    void logic_op_interprete(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        Node left = result;
        (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
        Node right = result;
        Node temp = Node(nullptr, Token());

        if (node.token.value == L"و")
        {
            if (left.token.value != L"0" and right.token.value != L"0")
            {
                temp.token.value = L"1";
            }
            else
            {
                temp.token.value = L"0";
            }
        }
        else if (node.token.value == L"او")
        {
            if (left.token.value != L"0" or right.token.value != L"0")
            {
                temp.token.value = L"1";
            }
            else
            {
                temp.token.value = L"0";
            }
        }
        result = temp;
    }

    void expreesion_interprete(Node node)
    {

        (this->*(node.left->right->func))(*node.left->right); // visit (node.left->func) and pass (node.left) as parameter node
        Node condetion = result;

        if (condetion.token.value == L"1")
        {
            (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        }
        else
        {
            (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
        }
    }

    void var_assign_interpreter(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        Node temp = result;
        namesTable[node.token.value] = temp;
    }

    void var_access_interperte(Node node)
    {
        result = namesTable[node.token.value];
    }

    void return_var_assign(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        Node right = result;
        (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
        Node left = result;

        if (node.token.type == plusEqualT)
        {
            if (right.token.type == integerT and left.token.type == integerT) 
            {
                right.token.value = std::to_wstring(std::stoi(left.token.value) + std::stoi(right.token.value));
            }
            else if (right.token.type == floatT or left.token.type == floatT)
            {
                right.token.value = std::to_wstring(std::stof(left.token.value) + std::stof(right.token.value));
            }
            else
            {
                std::wcout << "return value error" << std::endl;
            }
        }
        else if (node.token.type == minusEqualT)
        {
            if (right.token.type == integerT and left.token.type == integerT)
            {
                right.token.value = std::to_wstring(std::stoi(left.token.value) - std::stoi(right.token.value));
            }
            else if (right.token.type == floatT or left.token.type == floatT)
            {
                right.token.value = std::to_wstring(std::stof(left.token.value) - std::stof(right.token.value));
            }
            else
            {
                std::wcout << "return value error" << std::endl;
            }
        }
        else if (node.token.type == multiplyEqualT)
        {
            if (right.token.type == integerT and left.token.type == integerT)
            {
                right.token.value = std::to_wstring(std::stoi(left.token.value) * std::stoi(right.token.value));
            }
            else if (right.token.type == floatT or left.token.type == floatT)
            {
                right.token.value = std::to_wstring(std::stof(left.token.value) * std::stof(right.token.value));
            }
            else
            {
                std::wcout << "return value error" << std::endl;
            }
        }
        else if (node.token.type == divideEqualT)
        {
            if (right.token.type == integerT and left.token.type == integerT)
            {
                right.token.value = std::to_wstring(std::stoi(left.token.value) / std::stoi(right.token.value));
            }
            else if (right.token.type == floatT or left.token.type == floatT)
            {
                right.token.value = std::to_wstring(std::stof(left.token.value) / std::stof(right.token.value));
            }
            else
            {
                std::wcout << "return value error" << std::endl;
            }
        }
        else if (node.token.type == powerEqualT)
        {
            if (right.token.type == integerT and left.token.type == integerT)
            {
                right.token.value = std::to_wstring(pow(std::stoi(left.token.value), std::stoi(right.token.value)));
            }
            else if (right.token.type == floatT or left.token.type == floatT)
            {
                right.token.value = std::to_wstring(pow(std::stof(left.token.value), std::stof(right.token.value)));
            }
            else
            {
                std::wcout << "return value error" << std::endl;
            }
        }

        namesTable[node.right->token.value] = right;

    }

    void function_define_interprete(Node node)
    {
        namesTable[node.token.value] = *node.left;
    }

    inline  void multi_statement_interprete(Node node)
    {
        if (node.left->func == &Parser::multi_statement_interprete)
        {
            (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        }
        if (!return_)
        {
            (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
        }
    }

    void name_call_interpreter(Node node)
    {
        if (buildinFunction[node.token.value])
        {
            (this->*(buildinFunction[node.token.value]))(*node.right);

        }
        else
        {
            (this->*(namesTable[node.token.value].func))(namesTable[node.token.value]); // visit (node.left->func) and pass (node.left) as parameter node
            return_ = false;
        }
    }

    void return_interprete(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        return_ = true;
    }

    void for_interprete(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        int value = stoi(result.token.value);
        Node res = Node(nullptr, Token(Position(), Position(), integerT, std::to_wstring(0)));

        for (unsigned int i = 0; i < value; i++)
        {
            if (!return_)
            {
                res.token.value = std::to_wstring(i);
                namesTable[node.token.value] = res;
                (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node

            }
            else
            {
                break;
            }
        }
    }

    void while_interprete(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node

        while (result.token.value != L"0")
        {
            (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
            (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
        }
    }
    
    void if_interprete(Node node)
    {
        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node

        if (result.token.value != L"0")
        {
            (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
        }
    }

    // الدوال المدمجة
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void print(Node node)
    {
        (this->*(node.func))(node); // visit (node.left->func) and pass (node.left) as parameter node
        std::wcout << result.token.value << std::endl;
    }

};
