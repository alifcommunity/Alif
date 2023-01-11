#pragma once

#define Next_Is(t) (this->tokens->at(this->tokenIndex + 1).type_ == t ? true : false )

class Parser;
struct ExprNode;

struct AlifObj
{
    TokenType type_;

    union UObj
    {
        struct {

            KeywordType kind_;

        }None;

        struct Boolean_{

            KeywordType Kkind_;
            NUM value_;

            void or_(AlifObj* _other)
            {
                this->value_ = this->value_ or _other->A.Boolean.value_;
            }

            void and_(AlifObj* _other)
            {
                this->value_ = this->value_ and _other->A.Boolean.value_;
            }

            void not_()
            {
                this->value_ = not this->value_;
            }

        }Boolean;

        struct : Boolean_{

            TokenType Tkind_;

            void add_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ + _other->A.Number.value_;
                }
                else {
                    prnt(L"int add_ error");
                }
            }

            void sub_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ - _other->A.Number.value_;
                }
                else {
                    prnt(L"int sub_ error");
                }
            }

            void mul_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ * _other->A.Number.value_;
                }
                else {
                    prnt(L"int mul_ error");
                }
            }

            void div_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    if (_other->A.Number.value_ != 0)
                    {
                        this->value_ = this->value_ / _other->A.Number.value_;
                    }
                    else
                    {
                        prnt(L"cant divide by zero error");
                    }
                }
                else {
                    prnt(L"int div_ error");
                }
            }

            void rem_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber and _other->A.Number.Tkind_ == TTinteger)
                {
                    this->value_ = (int)this->value_ % (int)_other->A.Number.value_;
                }
                else {
                    prnt(L"int rem_ error");
                }
            }

            void pow_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = std::pow(this->value_, _other->A.Number.value_);
                }
                else
                {
                    prnt(L"int pow_ error");
                }
            }

            void equalE_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ == _other->A.Number.value_;
                }
                else
                {
                    prnt(L"int equalE_ error");
                }
            }

            void notE_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ != _other->A.Number.value_;
                }
                else
                {
                    prnt(L"int notE_ error");
                }
            }

            void greaterT_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ > _other->A.Number.value_;
                }
                else
                {
                    prnt(L"int greaterT_ error");
                }
            }

            void lessT_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ < _other->A.Number.value_;
                }
                else
                {
                    prnt(L"int lessT_ error");
                }
            }

            void greaterTE_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ >= _other->A.Number.value_;
                }
                else
                {
                    prnt(L"int greaterTE_ error");
                }
            }

            void lessTE_(AlifObj* _other)
            {
                if (_other->type_ == TTnumber)
                {
                    this->value_ = this->value_ <= _other->A.Number.value_;
                }
                else
                {
                    prnt(L"int lessTE_ error");
                }
            }

        }Number;

        struct {
            STR* value_;

            void add_(AlifObj* _other)
            {
                if (_other->type_ == TTstring)
                {
                    *this->value_ = *this->value_ + *_other->A.String.value_;
                }
                else {
                    prnt(L"str add_ error");
                }
            }
        }String;

        struct {
            NUM name_;
            //Context ctx_;
        }Name;

        struct {
            ExprNode* node_;
        }ExprNodes;

        struct {
            std::vector<ExprNode*>* list_;
            std::vector<AlifObj*>* objList;

            std::vector<ExprNode*>* add_element(AlifObj* _obj) {
                objList->push_back(_obj);
            }

        }List;

    }A;
};


// العقدة
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ExprNode
{
    VisitType type_;

    union UExprNode
    {
        struct {
            AlifObj* value_;
        }Object;

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
            std::vector<AlifObj*>* name_;
            ExprNode* value_;
        }NameAssign;

        struct {
            AlifObj* name_;
            TokenType operator_;
            ExprNode* value_;
        }AugNameAssign;

        struct {
            AlifObj* name_;
        }NameAccess;

        struct {
            std::vector<AlifObj*>* names_;
        }Call;

        struct {
            ExprNode* expr_;
            ExprNode* condetion_ = nullptr;
            ExprNode* elseExpr;
        }Expr;

        struct {
            ExprNode* expr_;
        }Return;

    }U;

    Position posStart;
    Position posEnd;
};

struct StmtsNode {

    VisitType type_;

    union UStmtsNode
    {
        struct {
            ExprNode* expr_;
        }Expr;

        struct {
            ExprNode* condetion_;
            StmtsNode* block_;
            StmtsNode* elseIf;
            StmtsNode* else_;
        }If;

        struct
        {
            AlifObj* name;
            StmtsNode* body;
            ExprNode* base;
        }ClassDef;

        struct {
            std::vector<StmtsNode*>* stmts_;
        }Stmts;
    }U;
};

// المحلل اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Parser {
public:
    std::vector<Token>* tokens;
    int tokenIndex = -1;
    Token currentToken;
    STR fileName;
    STR input_;

    unsigned int level = 5500;
    ExprNode* exprNode = (ExprNode*)malloc(level * sizeof(struct ExprNode));
    StmtsNode* stmtsNode = (StmtsNode*)malloc(level * sizeof(struct StmtsNode));

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

    void reverse() {
        this->tokenIndex--;
        if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens->size()) 
        {
            std::vector<Token>::iterator listIter = tokens->begin();
            std::advance(listIter, this->tokenIndex);
            this->currentToken = *listIter;
        }
    }

    void parse()
    {
        StmtsNode* result = nullptr;
        AlifObj* res = nullptr;
        
        do {
            result = this->statement();
            res = this->visit_stmts(result);
            this->level = 5500;

            //STR lst = L"[";
            //for (AlifObj* obj : *namesTable[result->U.NameAccess.name_.A.Name.name_].A.List.objList)
            //{
            //    lst.append(std::to_wstring((int)obj->A.Number.value_));
            //    lst.append(L", ");
            //} // for print list only
            //lst.replace(lst.length() - 2, lst.length(), L"]");
            //prnt(lst);
            // 
            prnt(res->A.Number.value_);

        } while (currentToken.type_ != TTendOfFile);

    }

    //////////////////////////////

    ExprNode* atom() {

        Token token = this->currentToken;
        AlifObj* obj_ = new AlifObj;
        level--;

        if (token.type_ == TTname)
        {
            
            this->advance();
            obj_->A.Name.name_ = token.val.numVal;
            (exprNode + level)->U.NameAccess.name_ = obj_;
            (exprNode + level)->type_ = VAccess;
            return (exprNode + level);
        }
        else if (token.type_ == TTkeyword) {
            if (token.val.keywordType == True)
            {
                this->advance();
                obj_->A.Boolean.Kkind_ = True;
                obj_->A.Boolean.value_ = 1;
                (exprNode + level)->U.Object.value_ = obj_;
                (exprNode + level)->type_ = VObject;

                return (exprNode + level);
            }
            else if (token.val.keywordType == False)
            {
                this->advance();
                obj_->A.Boolean.Kkind_ = False;
                obj_->A.Boolean.value_ = 0;
                (exprNode + level)->U.Object.value_ = obj_;
                (exprNode + level)->type_ = VObject;

                return (exprNode + level);
            }
            else if (token.val.keywordType == None)
            {
                this->advance();
                obj_->type_ = TTnone;
                obj_->A.None.kind_ = None;
                (exprNode + level)->U.Object.value_ = obj_;
                (exprNode + level)->type_ = VObject;

                return (exprNode + level);
            }
        }
        else if (token.type_ == TTinteger)
        {
            this->advance();
            obj_->type_ = TTnumber;
            obj_->A.Number.Tkind_ = token.type_;
            obj_->A.Number.value_ = token.val.numVal;
            (exprNode + level)->U.Object.value_ = obj_;
            (exprNode + level)->type_ = VObject;
            return (exprNode + level);
        }
        else if (token.type_ == TTfloat)
        {
            this->advance();
            obj_->type_ = TTnumber;
            obj_->A.Number.Tkind_ = token.type_;
            obj_->A.Number.value_ = token.val.numVal;
            (exprNode + level)->U.Object.value_ = obj_;
            (exprNode + level)->type_ = VObject;
            return (exprNode + level);
        }
        else if (token.type_ == TTstring)
        {
            this->advance();
            obj_->type_ = token.type_;
            obj_->A.String.value_ = token.val.strVal;
            (exprNode + level)->U.Object.value_ = obj_;
            (exprNode + level)->type_ = VObject;
            return (exprNode + level);
        }
        else if (token.type_ == TTlSquare)
        {
            return this->list_expr(obj_);
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
                exit(0);
            }
        }
        else
        {
            prnt("atom error");
        }
    }

    ExprNode* list_expr(AlifObj* _obj)
    {
        std::vector<ExprNode*>* nodeElement = new std::vector<ExprNode*>;

        if (this->currentToken.type_ == TTrSquare)
        {
            this->advance();
        }
        else
        {
            do {
                this->advance();
                nodeElement->push_back(this->expression());

            } while (this->currentToken.type_ == TTcomma);

            if (this->currentToken.type_ != TTrSquare)
            {
                prnt(SyntaxError(this->currentToken.positionStart, this->currentToken.positionEnd, L"لم يتم إغلاق قوس المصفوفة", fileName, input_).print_());
                exit(0);
            }
            this->advance();
        }

        level--;

        _obj->type_ = TTlist;
        _obj->A.List.list_ = nodeElement;
        (exprNode + level)->U.Object.value_ = _obj;
        (exprNode + level)->type_ = VList;

        return (exprNode + level);

    }

    ExprNode* primary() {

        //if (this->currentToken.type_ == TTname)
        //{
        //    std::vector<AlifObj*>* names_ = new std::vector<AlifObj*>; // not correct >> it will create repeatly so we should find other algorithm

        //    AlifObj* name_ = new AlifObj;
        //    name_->type_ = TTname;
        //    name_->A.Name.name_ = this->currentToken.val.numVal;

        //    names_->push_back(name_);

        //    this->advance();

        //    if (this->currentToken.type_ == TTdot)
        //    {
        //        this->advance();

        //        this->primary();

        //        level--;

        //        (exprNode + level)->U.Call.names_ = names_;
        //        (exprNode + level)->type_ = VCall;

        //        return (exprNode + level);
        //    }

        //}
        return this->atom();

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
            (exprNode + level)->type_ = VBinOp;

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
            (exprNode + level)->U.UnaryOp.right_ = this->factor();
            (exprNode + level)->type_ = VUnaryOp;

            return (exprNode + level);
        }

        return this->power();
    }

    ExprNode* term() {
        ExprNode* left = this->factor();

        while (this->currentToken.type_ == TTmultiply or this->currentToken.type_ == TTdivide or this->currentToken.type_ == TTremain) {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->term();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->type_ = VBinOp;

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
            (exprNode + level)->type_ = VBinOp;

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
            (exprNode + level)->type_ = VBinOp;

            left = (exprNode + level);
            return left;
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
            (exprNode + level)->type_ = VUnaryOp;

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
            (exprNode + level)->U.BinaryOp.keyword_ = opToken.val.keywordType;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->type_ = VBinOp;

            left = (exprNode + level);
            return left;
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
            (exprNode + level)->U.BinaryOp.keyword_ = opToken.val.keywordType;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->type_ = VBinOp;

            left = (exprNode + level);
            return left;
        }

        return left;
    }

    ExprNode* expression() {

        ExprNode* expr_ = this->disjuction();

        if (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == If)
        {
            this->advance();
            ExprNode* condetion = this->disjuction();
            
            if (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == Else)
            {
                this->advance();
                ExprNode* elseExpr = this->expression();
                level--;

                (exprNode + level)->U.Expr.expr_ = expr_;
                (exprNode + level)->U.Expr.condetion_ = condetion;
                (exprNode + level)->U.Expr.elseExpr = elseExpr;
                (exprNode + level)->type_ = VExpr;

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
            std::vector<ExprNode*>* exprs_ = new std::vector<ExprNode*>;
            AlifObj* obj_ = new AlifObj;

            exprs_->push_back(expr_);
            do
            {
                this->advance();
                exprs_->push_back(this->expression());

            } while (this->currentToken.type_ == TTcomma);

            level--;
        
            obj_->A.List.list_ = exprs_;
            obj_->type_ = TTlist;
            (exprNode + level)->U.Object.value_ = obj_; 
            (exprNode + level)->type_ = VList;

            return (exprNode + level);
        }
        return expr_;
    }

    ExprNode* assignment() {


        if (this->currentToken.type_ == TTname)
        {
            if (Next_Is(TTequal))
            {
                std::vector<AlifObj*>* names_ = new std::vector<AlifObj*>;

                while (Next_Is(TTequal))
                {
                    AlifObj* name_ = new AlifObj;

                    name_->type_ = TTname;
                    name_->A.Name.name_ = this->currentToken.val.numVal;

                    names_->push_back(name_);

                    this->advance();
                    this->advance();

                }

                ExprNode* expr_ = this->expressions();
                level--;

                (exprNode + level)->U.NameAssign.name_ = names_;
                (exprNode + level)->U.NameAssign.value_ = expr_;
                (exprNode + level)->type_ = VAssign;

                return (exprNode + level);

            }
            else if (Next_Is(TTplusEqual) or Next_Is(TTminusEqual) or Next_Is(TTmultiplyEqual) or Next_Is(TTdivideEqual) or Next_Is(TTpowerEqual) or Next_Is(TTremainEqual))
            {
                // يجب إختصار نوع التحقق الى TTaugAssign
                // بحيث يتم تخزين النوع في العملية بشكل مباشر دون التحقق منها
                // if token.type == TTaugassign then operator = opToken.type 

                AlifObj* name_ = new AlifObj;
                name_->type_ = TTname;
                name_->A.Name.name_ = this->currentToken.val.numVal;

                this->advance();

                Token opToken = this->currentToken;


                this->advance();
                ExprNode* expr_ = this->expression();
                level--;

                (exprNode + level)->U.AugNameAssign.name_ = name_;
                (exprNode + level)->U.AugNameAssign.operator_ = opToken.type_;
                (exprNode + level)->U.AugNameAssign.value_ = expr_;
                (exprNode + level)->type_ = VAugAssign;

                return (exprNode + level);

            }
        }

        return this->expressions();
    }

    ExprNode* return_statement() {

        this->advance();
        ExprNode* expr_ = this->expression();
        level--;

        (exprNode + level)->U.Return.expr_ = expr_;
        (exprNode + level)->type_ = VReturn;

        return (exprNode + level);
    }

    //void parameters()
    //{
    //    this->expression();
    //}

    //void class_defination() {
    //    expressions();
    //}


    StmtsNode* class_def() {

        ExprNode* bases = nullptr;
        StmtsNode* body = nullptr;
        AlifObj* name = new AlifObj;

        if (this->currentToken.type_ == TTname) {

            name->type_ = TTname;
            name->A.Name.name_ = this->currentToken.val.numVal;

            this->advance();

            if (this->currentToken.type_ == TTlParenthesis) {

                this->advance();
                    
                bases = this->expressions();
                    
                if (this->currentToken.type_ == TTrParenthesis)
                {
                    this->advance();
                }
            }
            if (this->currentToken.type_ == TTcolon) {

                this->advance();
                    
                body = this->block_();
                    
                level--;
                (stmtsNode + level)->type_ = VClass;
                (stmtsNode + level)->U.ClassDef.name = name;
                (stmtsNode + level)->U.ClassDef.base = bases;
                (stmtsNode + level)->U.ClassDef.body = body;
                return (stmtsNode + level);
            }
        }
    }

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


    StmtsNode* else_if() 
    {
        StmtsNode* block_{};
        ExprNode* condetion_ = this->expression();

        if (this->currentToken.type_ == TTcolon)
        {
            this->advance();
            block_ = this->block_();
        }

        level--;
        (stmtsNode + level)->type_ = VElseIf;
        (stmtsNode + level)->U.If.condetion_ = condetion_;
        (stmtsNode + level)->U.If.block_ = block_;
        return (stmtsNode + level);
    }

    StmtsNode* else_() {
        
        if (this->currentToken.type_ == TTcolon)
        {
            this->advance();
            return this->block_();
        }
        else
        {
            prnt(L"else of if statement error");
            exit(-1);
        }
    }

    StmtsNode* if_statement() 
    {

        StmtsNode* block_{};
        StmtsNode* elseIf{};
        StmtsNode* else_{};
        ExprNode* condetion_ = this->expression();

        if (this->currentToken.type_ == TTcolon)
        {
            this->advance();
            block_ = this->block_();
        }
        while (this->currentToken.val.keywordType == Elseif)
        {
            this->advance();
            elseIf = this->else_if();
        }
        if (this->currentToken.val.keywordType == Else)
        {
            this->advance();
            else_ = this->else_();
        }

        level--;
        (stmtsNode + level)->type_ = VIf;
        (stmtsNode + level)->U.If.condetion_ = condetion_;
        (stmtsNode + level)->U.If.block_ = block_;
        (stmtsNode + level)->U.If.elseIf = elseIf;
        (stmtsNode + level)->U.If.else_ = else_;
        return (stmtsNode + level);
        
    }

    StmtsNode* block_()
    {
        if (this->currentToken.type_ == TTnewline)
        {
            this->advance();

            if (this->currentToken.type_ == TTindent)
            {
                this->advance();

                StmtsNode* stmts_ = this->statements();

                if (this->currentToken.type_ == TTdedent)
                {
                    this->advance();
                    return stmts_;
                }
                else if (this->currentToken.type_ == TTindent)
                {
                    prnt(L"indent error in if body");
                }
            }

        }
    }

    ////void import_from() {
    ////}

    ////void import_name() {
    ////}

    ////void import_statement() {
    ////}

    ////void delete_statement() {
    ////    import_statement();
    ////}


    StmtsNode* compound_statement() 
    {
        if (this->currentToken.val.keywordType == Function)
        {
            // return this->function_defination();
        }
        else if (this->currentToken.val.keywordType == If)
        {
            this->advance();
            return this->if_statement();
        }
        else if (this->currentToken.val.keywordType == For)
        {
            // return this->for_statement();
        }
        else if (this->currentToken.val.keywordType == While)
        {
            // return this->while_statement();
        }
        else if (this->currentToken.val.keywordType == Class)
        {
            this->advance();
            return this->class_def();
        }
    }

    ExprNode* simple_statement()
    {
        return this->assignment();
    }

    StmtsNode* statement() {
        if (this->currentToken.type_ == TTkeyword)
        {
            if (this->currentToken.val.keywordType == Function or this->currentToken.val.keywordType == If or this->currentToken.val.keywordType == Class or this->currentToken.val.keywordType == For or this->currentToken.val.keywordType == While)
            {
                return this->compound_statement();
            }
        }
        else
        {
            ExprNode* exprNode = this->simple_statement();
            this->advance();

            level--;
            (stmtsNode + level)->type_ = VExpr;
            (stmtsNode + level)->U.Expr.expr_ = exprNode;
            return (stmtsNode + level);
        }
    }

    StmtsNode* statements() {

        std::vector<StmtsNode*>* statements_ = new std::vector<StmtsNode*>;

        while (this->currentToken.type_ != TTdedent and this->currentToken.type_ != TTendOfFile)
        {
            statements_->push_back(this->statement());

        }
        level--;

        (stmtsNode + level)->type_ = VStmts;
        (stmtsNode + level)->U.Stmts.stmts_ = statements_;
        return (stmtsNode + level);

        //if (currentBlockCount != 0)
        //{
        //    this->list.push_back(node);
        //}

        //while (this->currentToken.type == tabT) // لتجاهل المسافة تاب بعد السطر
        //{
        //    this->advance();
        //}

        //this->advance();
        //
        //while (this->currentToken.type == tabT)
        //{
        //    this->advance();
        //    tabCount++;
        //}

        //if (currentTabCount != tabCount)
        //{
        //    this->deindentent();
        //    // for i in list : node = Node(MultiStatementNode, Token(), std::make_shared<Node>(i));
        //    std::vector<Node>::iterator listIter;
        //    for (listIter = this->list.begin(); listIter != this->list.end(); ++listIter)
        //    {
        //        node = Node(&Parser::multi_statement_interprete, Token(), std::make_shared<Node>(node), std::make_shared<Node>(*listIter));
        //    }
        //    this->reverse(tabCount + 1);
        //    return;

        //}

        //if (currentBlockCount == 0)
        //{
        //    (this->*(node.func))(node); // visit_expr (node.left->func) and pass (node.left) as parameter node
        //}
        //

        //if (this->currentToken.type != endOfFileT and error == nullptr)
        //{
        //    this->statements();
        //}
        //else if (error)
        //{
        //    // error;
        //}
    }

    //// المفسر اللغوي
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //bool return_ = false;
    //
    //void function_define_interprete(Node node)
    //{
    //    namesTable[node.token.value] = *node.left;
    //}
    //
    //
    //void name_call_interpreter(Node node)
    //{
    //    if (buildinFunction[node.token.value])
    //    {
    //        (this->*(buildinFunction[node.token.value]))(*node.right);
    //
    //    }
    //    else
    //    {
    //        (this->*(namesTable[node.token.value].func))(namesTable[node.token.value]); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //        return_ = false;
    //    }
    //}
    //
    //void for_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //    int value = stoi(result.token.value);
    //    Node res = Node(nullptr, Token(Position(), Position(), integerT, std::to_wstring(0)));
    //
    //    for (unsigned int i = 0; i < value; i++)
    //    {
    //        if (!return_)
    //        {
    //            res.token.value = std::to_wstring(i);
    //            namesTable[node.token.value] = res;
    //            (this->*(node.right->func))(*node.right); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //
    //        }
    //        else
    //        {
    //            break;
    //        }
    //    }
    //}
    //
    //void while_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //
    //    while (result.token.value != L"0")
    //    {
    //        (this->*(node.right->func))(*node.right); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //        (this->*(node.left->func))(*node.left); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //    }
    //}
    //
    //void if_interprete(Node node)
    //{
    //    (this->*(node.left->func))(*node.left); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //
    //    if (result.token.value != L"0")
    //    {
    //        (this->*(node.right->func))(*node.right); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //    }
    //}
    //
    //// الدوال المدمجة
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //void print(Node node)
    //{
    //    (this->*(node.func))(node); // visit_expr (node.left->func) and pass (node.left) as parameter node
    //    std::wcout << result.token.value << std::endl;
    //}

    std::map<NUM, AlifObj*> namesTable;
    
    AlifObj* visit_stmts(StmtsNode* _node)
    {
        if (_node->type_ == VExpr)
        {
            return this->visit_expr(_node->U.Expr.expr_);
        }
        else if (_node->type_ == VFunction)
        {

        }
        else if (_node->type_ == VFor)
        {

        }
        else if (_node->type_ == VWhile)
        {

        }
        else if (_node->type_ == VIf)
        {

        }
        else if (_node->type_ == VClass)
        {

        }
    }





    AlifObj* visit_expr(ExprNode* _node)
    {

        if (_node->type_ == VObject)
        {
            return _node->U.Object.value_;
        }
        else if (_node->type_ == VList)
        {
            _node->U.Object.value_->A.List.objList = new std::vector<AlifObj*>;
            for (ExprNode* obj : *_node->U.Object.value_->A.List.list_)
            {
                _node->U.Object.value_->A.List.objList->push_back(this->visit_expr(obj));
            };
            return _node->U.Object.value_;
        }
        else if (_node->type_ == VUnaryOp)
        {
            AlifObj* right = this->visit_expr(_node->U.UnaryOp.right_);

            if (_node->U.UnaryOp.operator_ != TTkeyword)
            {
                if (_node->U.UnaryOp.operator_ == TTplus)
                {
                    return right;
                }
                else if (_node->U.UnaryOp.operator_ == TTminus)
                {
                    right->A.Number.value_ = -right->A.Number.value_;
                }
            }
            else
            {
                if (_node->U.UnaryOp.keyword_ == Not)
                { 
                    right->A.Boolean.not_();
                }
            }
            return right;
        }
        else if (_node->type_ == VBinOp) 
        {
            AlifObj* right = this->visit_expr(_node->U.BinaryOp.right_);
            AlifObj* left = this->visit_expr(_node->U.BinaryOp.left_);
            AlifObj* res = new AlifObj(*left);

            if (_node->U.BinaryOp.operator_ != TTkeyword)
            {
                if (_node->U.BinaryOp.operator_ == TTplus)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.add_(right);
                    }
                    else if (left->type_ == TTstring)
                    {
                        res->A.String.add_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTminus)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.sub_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTmultiply)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.mul_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTdivide)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.div_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTremain)
                {
                    if (left->type_ == TTnumber and left->A.Number.Tkind_ == TTinteger)
                    {
                        res->A.Number.rem_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTpower)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.pow_(right);
                    }
                }

                else if (_node->U.BinaryOp.operator_ == TTequalEqual)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.equalE_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTnotEqual)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.notE_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTgreaterThan)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.greaterT_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTlessThan)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.lessT_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTgreaterThanEqual)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.greaterTE_(right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTlessThanEqual)
                {
                    if (left->type_ == TTnumber)
                    {
                        res->A.Number.lessTE_(right);
                    }
                }
            }
            else
            {
                if (_node->U.BinaryOp.keyword_ == Or)
                {
                    res->A.Boolean.or_(right);
                }
                else if (_node->U.BinaryOp.keyword_ == And)
                {
                    res->A.Boolean.and_(right);
                }
            }

            return res;
        }
        else if (_node->type_ == VExpr)
        {
            AlifObj* expr_ = this->visit_expr(_node->U.Expr.expr_);
            if (_node->U.Expr.condetion_ != nullptr)
            {
                AlifObj* condetion_ = this->visit_expr(_node->U.Expr.condetion_);
                if (condetion_->A.Boolean.value_ != 0)
                {
                    return expr_;
                }
                else
                {
                    return this->visit_expr(_node->U.Expr.elseExpr);
                }
            }
            return expr_;
        }
        else if (_node->type_ == VAssign)
        {
            for (AlifObj* i : *_node->U.NameAssign.name_)
            {
                namesTable[i->A.Name.name_] = this->visit_expr(_node->U.NameAssign.value_);
            }
        }
        else if (_node->type_ == VAccess)
        {
            return namesTable[_node->U.NameAccess.name_->A.Name.name_];
        }
        else if (_node->type_ == VAugAssign)
        {
            AlifObj* value = this->visit_expr(_node->U.AugNameAssign.value_);
            AlifObj* name = namesTable[_node->U.AugNameAssign.name_->A.Name.name_];

            if (_node->U.AugNameAssign.operator_ == TTplusEqual)
            {
                if (name->type_ == TTnumber)
                {
                    name->A.Number.add_(value);
                }
                else if (name->type_ == TTstring)
                {
                    name->A.String.add_(value);
                }
            }
            else if (_node->U.AugNameAssign.operator_ == TTminusEqual)
            {
                if (name->type_ == TTnumber)
                {
                    name->A.Number.sub_(value);
                }
            }
            else if (_node->U.AugNameAssign.operator_ == TTmultiplyEqual)
            {
                if (name->type_ == TTnumber)
                {
                    name->A.Number.mul_(value);
                }
            }
            else if (_node->U.AugNameAssign.operator_ == TTdivideEqual)
            {
                if (name->type_ == TTnumber)
                {
                    name->A.Number.div_(value);
                }
            }
            else if (_node->U.AugNameAssign.operator_ == TTremainEqual)
            {
                if (name->type_ == TTnumber)
                {
                    name->A.Number.rem_(value);
                }
            }
            return name;
        }
        else if (_node->type_ == VReturn)
        {
            return this->visit_expr(_node->U.Return.expr_);
        }
    }

};