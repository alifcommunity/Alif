#pragma once

class Parser;
struct ExprNode;


struct List {
    std::vector<ExprNode*> list_;

    std::vector<ExprNode*> get_element() {

    }

};

// العقدة
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ExprNode
{
    ExprNode*(Parser::* func)(ExprNode*);

    union UExprNode
    {
        struct {
            NUM intVal;
        }Number;

        struct {
            STR* stringVal;
        }String;

        struct {
            ExprNode* left_;
            TokenType operator_;
            KeywordType keyword_;
            ExprNode* right_;
        }BinaryOp;

        struct {
            ExprNode* right_;
            TokenType operator_;
            KeywordType keyword_;
        }UnaryOp;

        struct {
            std::vector<NUM>* name_;
            ExprNode* value_;
        }VarAssign;

        struct {
            NUM name_;
            TokenType operator_;
            ExprNode* value_;
        }AugVarAssign;

        struct {
            NUM name_;
        }VarAccess;

        struct {
            ExprNode* node_;
            ExprNode* name_;
        }Call;

        struct {
            ExprNode* expr_;
            ExprNode* condetion_;
            ExprNode* elseExpr;
        }Expr;

        struct {
            std::vector<ExprNode*>* exprs_;
        }Exprs;

        struct {
            List* list_;
        }List;

    }U;

    Position posStart;
    Position posEnd;
}; // 84 byte

struct StmtsNode {

    StmtsNode*(Parser::* func)(StmtsNode*);

    union UStmtsNode
    {
        struct {
            ExprNode* expr_;
        }Expr;
    }U;
}; // 33 byte

// المحلل اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Parser {
public:
    std::vector<Token>* tokens;
    int tokenIndex = -1;
    Token currentToken;
    STR fileName;
    STR input_;

    unsigned int level = 6000;
    ExprNode* exprNode = (ExprNode*)malloc(level * 80);
    StmtsNode* stmtsNode = (StmtsNode*)malloc(level * 33);
    //std::vector<StmtsNode> list;

    //uint16_t currentBlockCount = 0;
    //uint16_t currentTabCount = 0;

    Parser(std::vector<Token>* tokens, STR _fileName, STR _input) : tokens(tokens) , fileName(_fileName), input_(_input)
    {
        this->advance();
    }

    void advance()
    {
        this->tokenIndex++;
        if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens->size())
        {
            std::vector<Token>::iterator listIter = tokens->begin();
            std::advance(listIter, this->tokenIndex);
            this->currentToken = *listIter;
        }
    }

    void reverse(unsigned int count = 1) {
        this->tokenIndex -= count;
        if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens->size()) {
            std::vector<Token>::iterator listIter = tokens->begin();
            std::advance(listIter, this->tokenIndex);
            this->currentToken = *listIter;
        }
    }

    void parse()
    {
        ExprNode* result = this->assignment();
        (this->*result->func)(result);
        this->level = 6000;
        this->advance();
        if (currentToken.type_ != TTendOfFile)
        {
            this->parse();
        }
        prnt(this->res->U.Number.intVal);
    }

    //////////////////////////////

    ExprNode* atom() {
        Token token = this->currentToken;
        level--;

        if (token.type_ == TTname)
        {
            this->advance();
            (exprNode + level)->U.VarAccess.name_ = token.val.numVal;
            (exprNode + level)->func = &Parser::nameAccess_intr;
            return (exprNode + level);
        }
        else if (token.type_ == TTkeyword) {
            if (token.val.keywordType == True or token.val.keywordType == False)
            {
                this->advance();
                // some work here
            }
            else if (token.val.keywordType == None)
            {
                this->advance();
                // some work here
            }
        }
        else if (token.type_ == TTinteger or token.type_ == TTfloat)
        {
            this->advance();
            (exprNode + level)->U.Number.intVal = token.val.numVal;
            (exprNode + level)->func = &Parser::number_intr;
            return (exprNode + level);
        }
        else if (token.type_ == TTstring)
        {
            this->advance();
            (exprNode + level)->U.String.stringVal = token.val.strVal;
            (exprNode + level)->func = &Parser::string_intr;
            return (exprNode + level);
        }
        else if (token.type_ == TTlSquare)
        {
            return this->list_expr();
        }
        else if (this->currentToken.type_ == TTlParenthesis)
        {
            this->advance();
            ExprNode* priorExpr = this->expression();

            if (this->currentToken.type_ == TTrParenthesis)
            {
                return priorExpr;
            }
            else
            {
                prnt(L"priorExpr Error");
            }
        }
        else
        {
            prnt("atom error");
        }
    }

    ExprNode* list_expr() 
    {
        Token token = this->currentToken;
        List* list_ = new List();
        std::vector<ExprNode*> nodeElement;

        if (this->currentToken.type_ == TTrSquare)
        {
            this->advance();
        }
        else
        {
            //nodeElement->push_back(this->expressions());

            if (this->currentToken.type_ != TTrSquare)
            {
                prnt(SyntaxError(this->currentToken.positionStart, this->currentToken.positionEnd, L"لم يتم إغلاق قوس المصفوفة", fileName, input_).print_());
                exit(0);
            }
        }

        list_->list_ = nodeElement;
        (exprNode + level)->U.List.list_ = list_;
        //(exprNode + level)->func = &Parser::list_intr;
        return (exprNode + level);

    }

    ExprNode* primary() {

        if (this->currentToken.type_ == TTdot)
        {
            this->advance();

            level--;

            (exprNode + level)->U.Call.name_ = this->atom();
            (exprNode + level)->U.Call.node_ = this->primary();
            //(exprNode + level)->func = &Parser::call_intr;

            return (exprNode + level);
        }
        //else if (this->currentToken.type == lParenthesisT)
        //{
        //    this->advance();
        //    if (this->currentToken.type != rParenthesisT)
        //    {
        //        //this->advance();
        //        this->parameters();
        //        params = node;
        //        if (this->currentToken.type == rParenthesisT)
        //        {
        //            this->advance();
        //            node = Node(&Parser::name_call_interpreter, name, std::make_shared<Node>(node), std::make_shared<Node>(params));
        //        }
        //        else
        //        {
        //            // error
        //        }
        //    }
        //    else if (this->currentToken.type == rParenthesisT)
        //    {
        //        this->advance();
        //        node = Node(&Parser::name_call_interpreter, name, std::make_shared<Node>(node), std::make_shared<Node>(params));
        //
        //    }
        //    else
        //    {
        //        // error
        //    }
        //}
        //else if (this->currentToken.type == lSquareT)
        //{
        //    this->advance();
        //    if (this->currentToken.type != rSquareT)
        //    {
        //        //this->slices();
        //        if (this->currentToken.type == rSquareT)
        //        {
        //            this->advance();
        //        }
        //        else
        //        {
        //            // error
        //        }
        //    }
        //    else if (this->currentToken.type == rSquareT)
        //    {
        //        this->advance();
        //        node = Node(&Parser::name_call_interpreter, name, std::make_shared<Node>(node));
        //
        //    }
        //    else
        //    {
        //        // error
        //    }
        //}

        return this->atom();
        
    }

    ExprNode* power()
    {
        ExprNode* left = this->primary();

        while (this->currentToken.type_ == TTpower) {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->factor();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->func = &Parser::binOp_intr;
            left = (exprNode + level);
            return left;
        }

        return left;
    }

    ExprNode* factor() {

        while (this->currentToken.type_ == TTplus or this->currentToken.type_ == TTminus) {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->factor();
            level--;

            (exprNode + level)->U.UnaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.UnaryOp.right_ = right;
            (exprNode + level)->func = &Parser::unaryOp_intr;

            return (exprNode + level);
        }

        return this->power();
    }

    ExprNode* term() {
        ExprNode* left = this->factor();

        while (this->currentToken.type_ == TTmultiply or this->currentToken.type_ == TTdivide) {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->term();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->func = &Parser::binOp_intr;
            left = (exprNode + level);
            return left;
        }

        return left;
    }

    ExprNode* sum() {
        ExprNode* left = this->term();

        while (this->currentToken.type_ == TTplus or this->currentToken.type_ == TTminus) {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->sum();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->func = &Parser::binOp_intr;
            left = (exprNode + level);
            return left;
        }

        return left;
    }

    ExprNode* comparesion() {
        ExprNode* left = this->sum();
    
        while (this->currentToken.type_ == TTequalEqual or this->currentToken.type_ == TTnotEqual or this->currentToken.type_ == TTlessThan or this->currentToken.type_ == TTgreaterThan or this->currentToken.type_ == TTlessThanEqual or this->currentToken.type_ == TTgreaterThanEqual) {
            Token opToken = this->currentToken;
    
            this->advance();
            ExprNode* right = this->sum();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->func = &Parser::binOp_intr;
            left = (exprNode + level);
        }
        
        return left;
    }

    ExprNode* inversion() {
    
        if (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == Not)
        {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->inversion();
            level--;

            (exprNode + level)->U.UnaryOp.right_ = right;
            (exprNode + level)->U.UnaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.UnaryOp.keyword_ = opToken.val.keywordType;
            (exprNode + level)->func = &Parser::unaryOp_intr;

            return (exprNode + level);    
        }

        return this->comparesion();
    }

    ExprNode* conjuction() {

        ExprNode* left = this->inversion();
    
        while (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == And) {
            Token opToken = this->currentToken;
    
            this->advance();
            ExprNode* right = this->inversion();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.keyword_ = opToken.val.keywordType;          (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->func = &Parser::binOp_intr;

            left = (exprNode + level);
        }

        return left;
    }

    ExprNode* disjuction() {

        ExprNode* left = this->conjuction();

        while (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == Or) {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->conjuction();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.keyword_ = opToken.val.keywordType;          (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->func = &Parser::binOp_intr;

            left = (exprNode + level);
        }

        return left;
    }

    ExprNode* expression() {

        ExprNode* expr_ = this->disjuction();

        if (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == If)
        {
            this->advance();
            ExprNode* condetion = this->disjuction();
            
            if (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == Elseif)
            {
                this->advance();
                ExprNode* elseExpr = this->expression();
                level--;

                (exprNode + level)->U.Expr.expr_ = expr_;
                (exprNode + level)->U.Expr.condetion_ = condetion;
                (exprNode + level)->U.Expr.elseExpr = elseExpr;
                //(exprNode + level)->func = &Parser::expr_intr;
                return (exprNode + level);

            }
            else
            {
                prnt(L"Expression error");
            }
        }

        return expr_;

    }

    ExprNode* expressions() {

        ExprNode* expr_ = this->expression();

        if (this->currentToken.type_ == TTcomma)
        {
            List* exprs_ = new List();

            exprs_->list_.push_back(expr_);
            do
            {
                this->advance();
                exprs_->list_.push_back(this->expression());

            } while (this->currentToken.type_ == TTcomma);

            level--;
        
            (exprNode + level)->U.List.list_ = exprs_; 
            //(exprNode + level)->func = &Parser::list_intr;
            return (exprNode + level);
        }
        return expr_;
    }

    ExprNode* assignment() {

        if (this->currentToken.type_ == TTname)
        {
            std::vector<NUM>* names_ = new std::vector<NUM>;
            Token AugVarName = this->currentToken;
            this->advance();

            if (this->currentToken.type_ == TTequal)
            {
                names_->push_back(AugVarName.val.numVal);
                this->advance();

                while (this->currentToken.type_ == TTname)
                {
                    AugVarName = this->currentToken;
                    this->advance();
                    if (this->currentToken.type_ == TTequal)
                    {
                        names_->push_back(AugVarName.val.numVal);
                        this->advance();

                    }
                    else
                    {
                        this->reverse();
                        break;
                    }

                }

                ExprNode* expr_ = this->expressions();
                level--;

                (exprNode + level)->U.VarAssign.name_ = names_;
                (exprNode + level)->U.VarAssign.value_ = expr_;
                //(exprNode + level)->func = &Parser::varAssign_intr;

                return (exprNode + level);
            }
            else if (this->currentToken.type_ == TTplusEqual or this->currentToken.type_ == TTminusEqual or this->currentToken.type_ == TTmultiplyEqual or this->currentToken.type_ == TTdivideEqual or this->currentToken.type_ == TTpowerEqual or this->currentToken.type_ == TTremainEqual)
            {
                Token opToken = this->currentToken;

                this->advance();
                ExprNode* expr_ = this->expression();
                level--;

                (exprNode + level)->U.AugVarAssign.name_ = AugVarName.val.numVal;
                (exprNode + level)->U.AugVarAssign.operator_ = opToken.type_;
                (exprNode + level)->U.AugVarAssign.value_ = expr_;
                //(exprNode + level)->func = &Parser::augVarAssign_intr;

                return (exprNode + level);

            }
        }

        return this->expressions();
    
    }

    //void return_statement() {

    //    this->advance();
    //    this->expression();
    //    node = Node(&Parser::return_interprete, this->currentToken, std::make_shared<Node>(node));
    //}

    //void parameters()
    //{
    //    this->expression();
    //}

    //void class_defination() {
    //    expressions();
    //}

    //void function_defination() {

    //    if (this->currentToken.value == L"دالة")
    //    {
    //        this->advance();
    //        if (this->currentToken.type == nameT)
    //        {
    //            Token name = this->currentToken;

    //            this->advance();
    //            if (this->currentToken.type == lParenthesisT)
    //            {
    //                this->advance();
    //                if (this->currentToken.type != rParenthesisT)
    //                {
    //                    this->advance();
    //                    //this->parameter();
    //                }
    //                else if (this->currentToken.type == rParenthesisT)
    //                {
    //                    this->advance();
    //                }

    //                if (this->currentToken.type == colonT)
    //                {
    //                    this->advance();
    //                    this->func_body(name);
    //                }
    //            }
    //        }
    //    }
    //}

    //void func_body(Token name)
    //{
    //    if (this->currentToken.type == newlineT) {


    //        // move list content to other store temporary to start store new body content
    //        std::vector<Node> tempList = this->list;
    //        this->list.clear();

    //        this->advance();

    //        this->indentent();
    //        
    //        this->statements();

    //        node = Node(&Parser::function_define_interprete, name, std::make_shared<Node>(node)); // node = body node
    //        if (currentBlockCount != 0)
    //        {
    //            this->list = tempList;
    //        }
    //    }
    //    //else {
    //    //    this->simple_statement();
    //    //}
    //}

    //void indentent ()
    //{
    //    currentBlockCount++;
    //    while (this->currentToken.type == tabT) {
    //        this->advance();
    //    }
    //    currentTabCount++; // ملاحظة: يجب التقدم بعدد المسافات وليس تقدم مرة واحدة فقط
    //}

    //void deindentent ()
    //{
    //    currentBlockCount--;
    //    while (this->currentToken.type == tabT)
    //    {
    //        this->advance();
    //    }
    //    currentTabCount--;
    //}


    //void while_statement() {
    //    Node expr;

    //    this->advance();

    //    this->expression();
    //    expr = node;

    //    if (this->currentToken.type == colonT)
    //    {
    //        this->advance();
    //        this->while_body();
    //        node = Node(&Parser::while_interprete, this->currentToken, std::make_shared<Node>(expr), std::make_shared<Node>(node));
    //    }
    //}

    //void while_body()
    //{
    //    if (this->currentToken.type == newlineT)
    //    {
    //        // move list content to other store temporary to start store new body content
    //        std::vector<Node> tempList = this->list;
    //        this->list.clear();

    //        this->advance();

    //        this->indentent();

    //        this->statements();

    //        if (currentBlockCount != 0)
    //        {
    //            this->list = tempList;
    //        }
    //    }
    //}

    //void for_statement() 
    //{
    //    Node expr;
    //    Token name;

    //    this->advance();
    //    if (this->currentToken.type == nameT)
    //    {
    //        name = this->currentToken;
    //        this->advance();
    //        if (this->currentToken.value == L"في")
    //        {
    //            this->advance();
    //            if (this->currentToken.type == lParenthesisT)
    //            {
    //                this->advance();
    //                this->expression();
    //                expr = node;
    //            }
    //            if (this->currentToken.type == rParenthesisT)
    //            {
    //                this->advance();
    //            }
    //            if (this->currentToken.type == colonT)
    //            {
    //                this->advance();
    //                this->for_body(expr, name);
    //            }
    //        }
    //    }
    //}

    //void for_body(Node expr, Token name)
    //{
    //    if (this->currentToken.type == newlineT) {


    //        // move list content to other store temporary to start store new body content
    //        std::vector<Node> tempList = this->list;
    //        this->list.clear();

    //        this->advance();

    //        this->indentent();

    //        this->statements();

    //        node = Node(&Parser::for_interprete, name, std::make_shared<Node>(expr), std::make_shared<Node>(node)); // node = body node

    //        if (currentBlockCount != 0)
    //        {
    //            this->list = tempList;
    //        }
    //    }
    //    //else {
    //    //    this->simple_statement();
    //    //}
    //}

    //void if_statement() 
    //{
    //    Node expr;
    //    std::vector<Node> tempList;

    //    this->advance();
    //    this->expression();
    //    expr = node;

    //    if (this->currentToken.type == colonT)
    //    {
    //        this->advance();
    //        this->if_body();
    //        node = Node(&Parser::if_interprete, this->currentToken, std::make_shared<Node>(expr), std::make_shared<Node>(node));
    //        tempList.push_back(node);
    //    }

    //    this->advance();
    //    while (this->currentToken.value == L"واذا")
    //    {
    //        this->advance();
    //        this->expression();
    //        expr = node;

    //        if (this->currentToken.type == colonT)
    //        {
    //            this->advance();
    //            this->if_body();
    //            node = Node(&Parser::if_interprete, this->currentToken, std::make_shared<Node>(expr), std::make_shared<Node>(node));
    //            tempList.push_back(node);
    //        }
    //        this->advance();
    //    }

    //    std::vector<Node>::iterator listIter;
    //    for (listIter = tempList.begin(); listIter != tempList.end(); ++listIter)
    //    {
    //        node = Node(&Parser::multi_statement_interprete, Token(), std::make_shared<Node>(node), std::make_shared<Node>(*listIter));
    //    }

    //    this->reverse();
    //}

    //void if_body()
    //{
    //    if (this->currentToken.type == newlineT)
    //    {
    //        // move list content to other store temporary to start store new body content
    //        std::vector<Node> tempList = this->list;
    //        this->list.clear();

    //        this->advance();

    //        this->indentent();

    //        this->statements();

    //        if (currentBlockCount != 0)
    //        {
    //            this->list = tempList;
    //        }
    //    }
    //}

    ////void import_from() {
    ////}

    ////void import_name() {
    ////}

    ////void import_statement() {
    ////}

    ////void delete_statement() {
    ////    import_statement();
    ////}


    //void compound_statement() 
    //{
    //    if (this->currentToken.value == L"دالة")
    //    {
    //        this->function_defination();
    //    }
    //    else if (this->currentToken.value == L"لاجل")
    //    {
    //        this->for_statement();
    //    }
    //    else if (this->currentToken.value == L"بينما")
    //    {
    //        this->while_statement();
    //    }
    //    else if (this->currentToken.value == L"اذا")
    //    {
    //        this->if_statement();
    //    }
    //}

    //void simple_statement()
    //{
    //    if (this->currentToken.value == L"ارجع")
    //    {
    //        this->return_statement();
    //    }
    //    else if (this->currentToken.type == nameT)
    //    {
    //        this->assignment();
    //    }
    //}

    //void statement() {
    //    if (this->currentToken.value == L"دالة" or this->currentToken.value == L"اذا" or this->currentToken.value == L"صنف" or this->currentToken.value == L"لاجل" or this->currentToken.value == L"بينما")
    //    {
    //        this->compound_statement();
    //    }
    //    else
    //    {
    //        this->simple_statement();
    //    }
    //}

    //void statements() {
    //    uint16_t tabCount = 0;

    //    this->statement();

    //    if (currentBlockCount != 0)
    //    {
    //        this->list.push_back(node);
    //    }

    //    while (this->currentToken.type == tabT) // لتجاهل المسافة تاب بعد السطر
    //    {
    //        this->advance();
    //    }

    //    this->advance();
    //    
    //    while (this->currentToken.type == tabT)
    //    {
    //        this->advance();
    //        tabCount++;
    //    }

    //    if (currentTabCount != tabCount)
    //    {
    //        this->deindentent();
    //        // for i in list : node = Node(MultiStatementNode, Token(), std::make_shared<Node>(i));
    //        std::vector<Node>::iterator listIter;
    //        for (listIter = this->list.begin(); listIter != this->list.end(); ++listIter)
    //        {
    //            node = Node(&Parser::multi_statement_interprete, Token(), std::make_shared<Node>(node), std::make_shared<Node>(*listIter));
    //        }
    //        this->reverse(tabCount + 1);
    //        return;

    //    }

    //    if (currentBlockCount == 0)
    //    {
    //        (this->*(node.func))(node); // visit (node.left->func) and pass (node.left) as parameter node
    //    }
    //    

    //    if (this->currentToken.type != endOfFileT and error == nullptr)
    //    {
    //        this->statements();
    //    }
    //    else if (error)
    //    {
    //        // error;
    //    }
    //}


    //void binary_operation(void(Parser::* funcL)(), TokenType fop, TokenType sop, void(Parser::* funcR)()) {
    //    Node left;

    //    (this->*funcL)();
    //    left = node;

    //    while (this->currentToken.type == fop or this->currentToken.type == sop) {
    //        Token opToken = this->currentToken;

    //        this->advance();
    //        (this->*funcR)();

    //        Node right = node;

    //        left = Node(&Parser::binary_op_interprete, opToken, std::make_shared<Node>(left), std::make_shared<Node>(right));
    //    }
    //    node = left;
    //}

    //// المفسر اللغوي
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //Node result;
    //bool return_ = false;
    //std::map<std::wstring, Node> namesTable;
    //std::map<std::wstring, void(Parser::*)(Node)> buildinFunction{{L"اطبع", &Parser::print}};


    //inline void str_num_interpreter(Node node)
    //{
    //    result = node;
    //}

    //void binary_op_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node left = result;
    //    (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node right = result;
    //    Node temp = Node();

    //    if (node.token.type == plusT)
    //    {
    //        temp.token.value = std::to_wstring(std::stof(left.token.value) + std::stof(right.token.value));
    //    }
    //    else if (node.token.type == minusT)
    //    {
    //        temp.token.value = std::to_wstring(std::stof(left.token.value) - std::stof(right.token.value));
    //    }
    //    else if (node.token.type == multiplyT)
    //    {
    //        temp.token.value = std::to_wstring(std::stof(left.token.value) * std::stof(right.token.value));
    //    }
    //    else if (node.token.type == divideT)
    //    {
    //        temp.token.value = std::to_wstring(std::stof(left.token.value) / std::stof(right.token.value));
    //    }
    //    else if (node.token.type == powerT)
    //    {
    //        temp.token.value = std::to_wstring(pow(std::stof(left.token.value), std::stof(right.token.value)));
    //    }
    //    result = temp;
    //}

    //void unary_op_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node

    //    if (node.token.type == plusT)
    //    {
    //        result.token.value = std::to_wstring(std::stof(result.token.value));
    //    }
    //    else if (node.token.type == minusT)
    //    {
    //        result.token.value = std::to_wstring(-1 * std::stof(result.token.value));
    //    }
    //    else
    //    {
    //        if (result.token.value == L"0")
    //        {
    //            result.token.value = L"1";
    //        }
    //        else
    //        {
    //            result.token.value = L"0";
    //        }
    //    }

    //}

    //void compare_op_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node left = result;
    //    (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node right = result;
    //    Node temp = Node(nullptr, Token());

    //    if (node.token.type == equalEqualT)
    //    {
    //        temp.token.value = std::to_wstring(left.token.value == right.token.value);
    //    }
    //    else if (node.token.type == notEqualT)
    //    {
    //        temp.token.value = std::to_wstring(left.token.value != right.token.value);
    //    }
    //    else if (node.token.type == lessThanT)
    //    {
    //        temp.token.value = std::to_wstring(std::stof(left.token.value) < std::stof(right.token.value));
    //    }
    //    else if (node.token.type == greaterThanT)
    //    {
    //        temp.token.value = std::to_wstring(std::stof(left.token.value) > std::stof(right.token.value));
    //    }
    //    else if (node.token.type == lessThanEqualT)
    //    {
    //        temp.token.value = std::to_wstring(std::stof(left.token.value) <= std::stof(right.token.value));
    //    }
    //    else if (node.token.type == greaterThanEqualT)
    //    {
    //        temp.token.value = std::to_wstring(std::stof(left.token.value) >= std::stof(right.token.value));
    //    }
    //    result = temp;
    //}

    //void logic_op_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node left = result;
    //    (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node right = result;
    //    Node temp = Node(nullptr, Token());

    //    if (node.token.value == L"و")
    //    {
    //        if (left.token.value != L"0" and right.token.value != L"0")
    //        {
    //            temp.token.value = L"1";
    //        }
    //        else
    //        {
    //            temp.token.value = L"0";
    //        }
    //    }
    //    else if (node.token.value == L"او")
    //    {
    //        if (left.token.value != L"0" or right.token.value != L"0")
    //        {
    //            temp.token.value = L"1";
    //        }
    //        else
    //        {
    //            temp.token.value = L"0";
    //        }
    //    }
    //    result = temp;
    //}

    //void expreesion_interprete(Node node)
    //{

    //    (this->*(node.left->right->func))(*node.left->right); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node condetion = result;

    //    if (condetion.token.value == L"1")
    //    {
    //        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    }
    //    else
    //    {
    //        (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
    //    }
    //}

    //void var_assign_interpreter(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node temp = result;
    //    namesTable[node.token.value] = temp;
    //}

    //void var_access_interperte(Node node)
    //{
    //    result = namesTable[node.token.value];
    //}

    //void return_var_assign(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node right = result;
    //    (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
    //    Node left = result;

    //    if (node.token.type == plusEqualT)
    //    {
    //        if (right.token.type == integerT and left.token.type == integerT) 
    //        {
    //            right.token.value = std::to_wstring(std::stoi(left.token.value) + std::stoi(right.token.value));
    //        }
    //        else if (right.token.type == floatT or left.token.type == floatT)
    //        {
    //            right.token.value = std::to_wstring(std::stof(left.token.value) + std::stof(right.token.value));
    //        }
    //        else
    //        {
    //            std::wcout << "return value error" << std::endl;
    //        }
    //    }
    //    else if (node.token.type == minusEqualT)
    //    {
    //        if (right.token.type == integerT and left.token.type == integerT)
    //        {
    //            right.token.value = std::to_wstring(std::stoi(left.token.value) - std::stoi(right.token.value));
    //        }
    //        else if (right.token.type == floatT or left.token.type == floatT)
    //        {
    //            right.token.value = std::to_wstring(std::stof(left.token.value) - std::stof(right.token.value));
    //        }
    //        else
    //        {
    //            std::wcout << "return value error" << std::endl;
    //        }
    //    }
    //    else if (node.token.type == multiplyEqualT)
    //    {
    //        if (right.token.type == integerT and left.token.type == integerT)
    //        {
    //            right.token.value = std::to_wstring(std::stoi(left.token.value) * std::stoi(right.token.value));
    //        }
    //        else if (right.token.type == floatT or left.token.type == floatT)
    //        {
    //            right.token.value = std::to_wstring(std::stof(left.token.value) * std::stof(right.token.value));
    //        }
    //        else
    //        {
    //            std::wcout << "return value error" << std::endl;
    //        }
    //    }
    //    else if (node.token.type == divideEqualT)
    //    {
    //        if (right.token.type == integerT and left.token.type == integerT)
    //        {
    //            right.token.value = std::to_wstring(std::stoi(left.token.value) / std::stoi(right.token.value));
    //        }
    //        else if (right.token.type == floatT or left.token.type == floatT)
    //        {
    //            right.token.value = std::to_wstring(std::stof(left.token.value) / std::stof(right.token.value));
    //        }
    //        else
    //        {
    //            std::wcout << "return value error" << std::endl;
    //        }
    //    }
    //    else if (node.token.type == powerEqualT)
    //    {
    //        if (right.token.type == integerT and left.token.type == integerT)
    //        {
    //            right.token.value = std::to_wstring(pow(std::stoi(left.token.value), std::stoi(right.token.value)));
    //        }
    //        else if (right.token.type == floatT or left.token.type == floatT)
    //        {
    //            right.token.value = std::to_wstring(pow(std::stof(left.token.value), std::stof(right.token.value)));
    //        }
    //        else
    //        {
    //            std::wcout << "return value error" << std::endl;
    //        }
    //    }

    //    namesTable[node.right->token.value] = right;

    //}

    //void function_define_interprete(Node node)
    //{
    //    namesTable[node.token.value] = *node.left;
    //}

    //inline  void multi_statement_interprete(Node node)
    //{
    //    if (node.left->func == &Parser::multi_statement_interprete)
    //    {
    //        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    }
    //    if (!return_)
    //    {
    //        (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
    //    }
    //}

    //void name_call_interpreter(Node node)
    //{
    //    if (buildinFunction[node.token.value])
    //    {
    //        (this->*(buildinFunction[node.token.value]))(*node.right);

    //    }
    //    else
    //    {
    //        (this->*(namesTable[node.token.value].func))(namesTable[node.token.value]); // visit (node.left->func) and pass (node.left) as parameter node
    //        return_ = false;
    //    }
    //}

    //void return_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    return_ = true;
    //}

    //void for_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    int value = stoi(result.token.value);
    //    Node res = Node(nullptr, Token(Position(), Position(), integerT, std::to_wstring(0)));

    //    for (unsigned int i = 0; i < value; i++)
    //    {
    //        if (!return_)
    //        {
    //            res.token.value = std::to_wstring(i);
    //            namesTable[node.token.value] = res;
    //            (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node

    //        }
    //        else
    //        {
    //            break;
    //        }
    //    }
    //}

    //void while_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node

    //    while (result.token.value != L"0")
    //    {
    //        (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
    //        (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node
    //    }
    //}
    //
    //void if_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit (node.left->func) and pass (node.left) as parameter node

    //    if (result.token.value != L"0")
    //    {
    //        (this->*(node.right->func))(*node.right); // visit (node.left->func) and pass (node.left) as parameter node
    //    }
    //}

    //// الدوال المدمجة
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //void print(Node node)
    //{
    //    (this->*(node.func))(node); // visit (node.left->func) and pass (node.left) as parameter node
    //    std::wcout << result.token.value << std::endl;
    //}

    //void visit(Node* _node)
    //{
    //    if (_node->nodeType == numberIntr)
    //    {
    //        _node;
    //    }
    //    else if (_node->nodeType == stringIntr)
    //    {
    //        return _node;
    //    }
    //    else if (_node->nodeType == binOpIntr)
    //    {
    //        return binOp_intr(_node);
    //    }
    //    else if (_node->nodeType == unaryOpIntr)
    //    {
    //        return unaryOp_intr(_node);
    //    }
    //}
;
    ExprNode* res = new ExprNode();
    
    ExprNode* nameAccess_intr(ExprNode* _node)
    {
        return _node;
    }

    ExprNode* number_intr(ExprNode* _node)
    {
        return _node;
    }

    ExprNode* string_intr(ExprNode* _node)
    {
        return _node;
    }

    ExprNode* binOp_intr(ExprNode* _node) {
        ExprNode* left = (this->*(_node->U.BinaryOp.left_->func))(_node->U.BinaryOp.left_);
        ExprNode* right = (this->*(_node->U.BinaryOp.right_->func))(_node->U.BinaryOp.right_);

        if (_node->U.BinaryOp.operator_ == TTplus)
        {
            res->U.Number.intVal = left->U.Number.intVal + right->U.Number.intVal;
        }

        return res;
    }

    ExprNode* unaryOp_intr(ExprNode* _node) {
        ExprNode* right = (this->*(_node->U.UnaryOp.right_->func))(_node->U.UnaryOp.right_);

        if (_node->U.UnaryOp.operator_ == TTplus)
        {
            res->U.Number.intVal = right->U.Number.intVal;
        }
        else if (_node->U.UnaryOp.operator_ == TTminus)
        {
            res->U.Number.intVal = -right->U.Number.intVal;
        }
        return res;
    }
};
