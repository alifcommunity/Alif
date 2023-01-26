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

        struct Boolean_ {

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

        struct : Boolean_ {

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
            std::vector<AlifObj>* objList;

            void add_element(AlifObj _obj) {
                objList->push_back(_obj);
            }

        }List;

        struct
        {
            BuildInFuncType buildInFunc;
        }BuildInFunc;

    }A;
};


struct Scope
{
    std::map<NUM, AlifObj> symbols;
    std::map<NUM, Scope*> scopes;
    Scope* parent;
};

class SymbolTable {

    Scope* currentScope;
    Scope* globalScope;

public:
    SymbolTable() {
        globalScope = new Scope();
        globalScope->parent = nullptr;
        currentScope = globalScope;
    }

    void add_symbol(NUM type, AlifObj value) {
        currentScope->symbols[type] = value;
    }

    void add_value(NUM _key, NUM _value) {
        currentScope->symbols[_key].A.Number.value_ = _value;
    }

    void enter_scope(NUM type) {
        if (currentScope->scopes.count(type) == 0) {
            Scope* newScope = new Scope();
            newScope->parent = currentScope;
            currentScope->scopes[type] = newScope;
        }
        currentScope = currentScope->scopes[type];
    }

    void exit_scope() {
        currentScope = currentScope->parent;
    }

    AlifObj get_data(NUM type) {
        Scope* scope = currentScope;
        int level = 1;
        while (scope != nullptr)
        {
            if (scope->symbols.count(type) != 0)
            {
                return scope->symbols[type];
            }
            if (level == 0) {
                break;
            }
            scope = scope->parent;
            level--;
        }
        prnt(L"المتغير المستدعى غير معرف");
        exit(-1);
    }

};

// العقدة
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ExprNode
{
    VisitType type_;

    union UExprNode
    {
        struct {
            AlifObj value_;
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
            std::vector<AlifObj>* name_;
            ExprNode* value_;
        }NameAssign;

        struct {
            AlifObj name_;
            TokenType operator_;
            ExprNode* value_;
        }AugNameAssign;

        struct {
            AlifObj name_;
        }NameAccess;

        struct {
            ExprNode* func;
            ExprNode* name;
            std::vector<ExprNode*>* args;
        }Call;

        struct {
            ExprNode* expr_;
            ExprNode* condetion_;
            ExprNode* elseExpr;
        }Expr;

    }U;

    //Position posStart;
    //Position posEnd;
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
            std::vector<StmtsNode*>* elseIf;
            StmtsNode* else_;
        }If;

        struct {
            AlifObj itrName;
            std::vector<ExprNode*>* args_;
            StmtsNode* block_;
            StmtsNode* else_;
        }For;

        struct {
            ExprNode* condetion_;
            StmtsNode* block_;
            StmtsNode* else_;
        }While;

        struct
        {
            AlifObj name;
            StmtsNode* body;
            ExprNode* base;
        }ClassDef;

        struct
        {
            AlifObj name;
            std::vector<ExprNode*>* params;
            StmtsNode* body;
        }FunctionDef;

        struct
        {
            ExprNode* returnExpr;
        }Return;

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

    /// <flags>

    bool lastParam = false;
    bool returnFlag = false;

    /// </flags>

    unsigned int level = 5500;
    ExprNode* exprNode = (ExprNode*)malloc(level * sizeof(struct ExprNode));
    StmtsNode* stmtsNode = (StmtsNode*)malloc(level * sizeof(struct StmtsNode));

    SymbolTable symTable;

    Parser(std::vector<Token>* tokens, STR _fileName, STR _input) : tokens(tokens), fileName(_fileName), input_(_input)
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

    void parse_file()
    {
        StmtsNode* stmtsRes = nullptr;
        AlifObj intrRes;

        do {
            stmtsRes = this->statement();
            intrRes = this->visit_stmts(stmtsRes);
            //this->level = 5500;

        } while (currentToken.type_ != TTendOfFile);

    }

    void parse_terminal()
    {
        ExprNode* stmtsRes = this->expression();
        AlifObj intrRes = this->visit_expr(stmtsRes);

        if (intrRes.type_ == TTstring) { prnt(intrRes.A.String.value_); }
        else if (intrRes.type_ == TTnumber) { prnt(intrRes.A.Number.value_); }
        else if (intrRes.type_ == TTkeyword) { if (intrRes.A.Boolean.value_ == 1) { prnt(L"صح"); } else { prnt(L"خطا"); } }
        else if (intrRes.type_ == TTlist) { STR lst = L"["; for (AlifObj obj : *intrRes.A.List.objList) { lst.append(std::to_wstring((int)obj.A.Number.value_)); lst.append(L", "); } lst.replace(lst.length() - 2, lst.length(), L"]"); prnt(lst); }
    }

    //////////////////////////////

    std::vector<ExprNode*>* arguments() {

        std::vector<ExprNode*>* args = new std::vector<ExprNode*>;

        if (this->currentToken.type_ == TTname) {

            if (Next_Is(TTequal)) {

                std::vector<AlifObj>* names_ = new std::vector<AlifObj>;

                AlifObj name_;

                name_.type_ = TTname;
                name_.A.Name.name_ = this->currentToken.val.numVal;

                names_->push_back(name_);

                this->advance();
                this->advance();

                ExprNode* value = this->expression();

                level--;
                (exprNode + level)->U.NameAssign.name_ = names_;
                (exprNode + level)->U.NameAssign.value_ = value;
                (exprNode + level)->type_ = VAssign;

                args->push_back((exprNode + level));

            }
            else if (this->currentToken.type_ != TTrParenthesis or this->currentToken.type_ != TTcomma) {
                args->push_back(this->expression());
            }
        }
        else if (this->currentToken.type_ != TTrParenthesis or this->currentToken.type_ != TTcomma) {
            args->push_back(this->expression());
        }

        while (this->currentToken.type_ == TTcomma)
        {

            this->advance();

            if (this->currentToken.type_ == TTname) {

                if (Next_Is(TTequal)) {

                    std::vector<AlifObj>* names_ = new std::vector<AlifObj>;

                    AlifObj name_{};

                    name_.type_ = TTname;
                    name_.A.Name.name_ = this->currentToken.val.numVal;

                    names_->push_back(name_);

                    this->advance();
                    this->advance();

                    ExprNode* value = this->expression();

                    level--;
                    (exprNode + level)->U.NameAssign.name_ = names_;
                    (exprNode + level)->U.NameAssign.value_ = value;
                    (exprNode + level)->type_ = VAssign;

                    args->push_back((exprNode + level));

                }
                else if (this->currentToken.type_ != TTrParenthesis or this->currentToken.type_ != TTcomma) {
                    args->push_back(this->expression());
                }
            }
            else if (this->currentToken.type_ != TTrParenthesis or this->currentToken.type_ != TTcomma) {
                args->push_back(this->expression());
            }

        }
        return args;

    }


    ExprNode* atom() {

        Token token = this->currentToken;
        level--;

        if (token.type_ == TTname)
        {

            this->advance();
            (exprNode + level)->U.NameAccess.name_.type_ = TTname;
            (exprNode + level)->U.NameAccess.name_.A.Name.name_ = token.val.numVal;
            (exprNode + level)->type_ = VAccess;
            return (exprNode + level);
        }
        else if (token.type_ == TTbuildInFunc)
        {

            this->advance();
            (exprNode + level)->U.NameAccess.name_.type_ = TTbuildInFunc;
            (exprNode + level)->U.NameAccess.name_.A.BuildInFunc.buildInFunc = token.val.buildInFunc;
            (exprNode + level)->type_ = VAccess;
            return (exprNode + level);
        }
        else if (token.type_ == TTkeyword) {
            if (token.val.keywordType == True)
            {
                this->advance();
                (exprNode + level)->U.Object.value_.type_ = TTkeyword;
                (exprNode + level)->U.Object.value_.A.Boolean.Kkind_ = True;
                (exprNode + level)->U.Object.value_.A.Boolean.value_ = 1;
                (exprNode + level)->type_ = VObject;

                return (exprNode + level);
            }
            else if (token.val.keywordType == False)
            {
                this->advance();
                (exprNode + level)->U.Object.value_.type_ = TTkeyword;
                (exprNode + level)->U.Object.value_.A.Boolean.Kkind_ = False;
                (exprNode + level)->U.Object.value_.A.Boolean.value_ = 0;
                (exprNode + level)->type_ = VObject;

                return (exprNode + level);
            }
            else if (token.val.keywordType == None)
            {
                this->advance();
                (exprNode + level)->U.Object.value_.type_ = TTnone;
                (exprNode + level)->U.Object.value_.A.None.kind_ = None;
                (exprNode + level)->type_ = VObject;

                return (exprNode + level);
            }
        }
        else if (token.type_ == TTinteger)
        {
            this->advance();
            (exprNode + level)->U.Object.value_.type_ = TTnumber;
            (exprNode + level)->U.Object.value_.A.Number.Tkind_ = token.type_;
            (exprNode + level)->U.Object.value_.A.Number.value_ = token.val.numVal;
            (exprNode + level)->type_ = VObject;
            return (exprNode + level);
        }
        else if (token.type_ == TTfloat)
        {
            this->advance();
            (exprNode + level)->U.Object.value_.type_ = TTnumber;
            (exprNode + level)->U.Object.value_.A.Number.Tkind_ = token.type_;
            (exprNode + level)->U.Object.value_.A.Number.value_ = token.val.numVal;
            (exprNode + level)->type_ = VObject;
            return (exprNode + level);
        }
        else if (token.type_ == TTstring)
        {
            this->advance();
            (exprNode + level)->U.Object.value_.type_ = token.type_;
            (exprNode + level)->U.Object.value_.A.String.value_ = token.val.strVal;
            (exprNode + level)->type_ = VObject;
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
                this->advance();
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

    ExprNode* list_expr()
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

        (exprNode + level)->U.Object.value_.type_ = TTlist;
        (exprNode + level)->U.Object.value_.A.List.list_ = nodeElement;
        (exprNode + level)->type_ = VList;

        return (exprNode + level);

    }

    ExprNode* primary() {

        ExprNode* atom = this->atom();
        if (this->currentToken.type_ == TTdot) {

            this->advance();
            ExprNode* primary = this->primary();

            level--;
            (exprNode + level)->type_ = VCall;
            (exprNode + level)->U.Call.func = atom;
            (exprNode + level)->U.Call.name = primary;
            return (exprNode + level);
        }
        else if (this->currentToken.type_ == TTlParenthesis) {

            ExprNode* primary = atom;

            this->advance();

            if (this->currentToken.type_ == TTrParenthesis) {

                this->advance();

                level--;
                (exprNode + level)->type_ = VCall;
                (exprNode + level)->U.Call.name = primary;
                (exprNode + level)->U.Call.args = nullptr;
                return (exprNode + level);

            }

            std::vector<ExprNode*>* args = this->arguments();

            this->advance();

            level--;
            (exprNode + level)->type_ = VCall;
            (exprNode + level)->U.Call.name = primary;
            (exprNode + level)->U.Call.args = args;
            return (exprNode + level);

        }
        else {
            return atom;
        }

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
        }

        return left;
    }

    ExprNode* factor() {

        while (this->currentToken.type_ == TTplus or this->currentToken.type_ == TTminus) {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->power();
            level--;

            (exprNode + level)->U.UnaryOp.right_ = right;
            (exprNode + level)->U.UnaryOp.operator_ = opToken.type_;
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
            ExprNode* right = this->factor();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->type_ = VBinOp;

            left = (exprNode + level);
        }

        return left;
    }

    ExprNode* sum() {
        ExprNode* left = this->term();

        while (this->currentToken.type_ == TTplus or this->currentToken.type_ == TTminus) {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->term();
            level--;

            (exprNode + level)->U.BinaryOp.right_ = right;
            (exprNode + level)->U.BinaryOp.operator_ = opToken.type_;
            (exprNode + level)->U.BinaryOp.left_ = left;
            (exprNode + level)->type_ = VBinOp;

            left = (exprNode + level);
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
        }

        return left;
    }

    ExprNode* inversion() {

        while (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == Not)
        {
            Token opToken = this->currentToken;

            this->advance();
            ExprNode* right = this->comparesion();
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

            exprs_->push_back(expr_);
            do
            {
                this->advance();
                exprs_->push_back(this->expression());

            } while (this->currentToken.type_ == TTcomma);

            level--;

            (exprNode + level)->U.Object.value_.type_ = TTlist;
            (exprNode + level)->U.Object.value_.A.List.list_ = exprs_;
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
                std::vector<AlifObj>* names_ = new std::vector<AlifObj>;

                while (Next_Is(TTequal))
                {
                    AlifObj name_{};

                    name_.type_ = TTname;
                    name_.A.Name.name_ = this->currentToken.val.numVal;

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

                AlifObj name_{};
                name_.type_ = TTname;
                name_.A.Name.name_ = this->currentToken.val.numVal;

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

    StmtsNode* return_statement() {

        ExprNode* expr_;

        if (this->currentToken.type_ != TTnewline)
        {
            expr_ = this->expression();
        }
        else
        {
            expr_ = nullptr;
        }
        this->advance();

        level--;
        (stmtsNode + level)->U.Return.returnExpr = expr_;
        (stmtsNode + level)->type_ = VReturn;

        return (stmtsNode + level);
    }

    std::vector<ExprNode*>* parameters() {

        std::vector<ExprNode*>* params = new std::vector<ExprNode*>;

        if (this->currentToken.type_ == TTname) {

            if (Next_Is(TTequal)) {
                lastParam = True;

                std::vector<AlifObj>* names_ = new std::vector<AlifObj>;
                AlifObj name_{};

                name_.type_ = TTname;
                name_.A.Name.name_ = this->currentToken.val.numVal;

                names_->push_back(name_);

                this->advance();
                this->advance();

                ExprNode* value = this->expression();

                level--;
                (exprNode + level)->U.NameAssign.name_ = names_;
                (exprNode + level)->U.NameAssign.value_ = value;
                (exprNode + level)->type_ = VAssign;

                params->push_back((exprNode + level));

            }
            if (lastParam) {
                prnt(L"not allow assign expression to parameter");
                exit(-1);
            }
            params->push_back(this->atom());
        }

        while (this->currentToken.type_ == TTcomma)
        {

            this->advance();

            if (this->currentToken.type_ == TTname) {

                if (Next_Is(TTequal)) {

                    lastParam = True;

                    std::vector<AlifObj>* names_ = new std::vector<AlifObj>;

                    AlifObj name_{};

                    name_.type_ = TTname;
                    name_.A.Name.name_ = this->currentToken.val.numVal;

                    names_->push_back(name_);

                    this->advance();
                    this->advance();

                    ExprNode* value = this->expression();

                    level--;
                    (exprNode + level)->U.NameAssign.name_ = names_;
                    (exprNode + level)->U.NameAssign.value_ = value;
                    (exprNode + level)->type_ = VAssign;

                    params->push_back((exprNode + level));

                }
                if (lastParam) {
                    prnt(L"not allow assign expression to parameter");
                    exit(-1);
                }
                params->push_back(this->atom());
            }

        }
        return params;

    }

    StmtsNode* function_def() {

        AlifObj name{};
        StmtsNode* body = nullptr;
        std::vector<ExprNode*>* params = nullptr;

        if (this->currentToken.type_ == TTname or this->currentToken.type_ == TTbuildInFunc) {

            name.type_ = this->currentToken.type_;

            if (this->currentToken.type_ == TTname)
            {
                name.A.Name.name_ = this->currentToken.val.numVal;
            }
            else {
                name.A.BuildInFunc.buildInFunc = this->currentToken.val.buildInFunc;
            }

            this->advance();

            if (this->currentToken.type_ == TTlParenthesis) {

                this->advance();

                if (this->currentToken.type_ == TTrParenthesis) {
                    this->advance();
                }
                else {
                    params = this->parameters();
                    this->advance();
                }
            }

            if (this->currentToken.type_ == TTcolon) {

                this->advance();
                returnFlag = true;
                body = this->block_();
                returnFlag = false;

                level--;
                (stmtsNode + level)->type_ = VFunction;
                (stmtsNode + level)->U.FunctionDef.name = name;
                (stmtsNode + level)->U.FunctionDef.params = params;
                (stmtsNode + level)->U.FunctionDef.body = body;
                return (stmtsNode + level);
            }
        }

    }

    StmtsNode* class_def() {

        ExprNode* bases = nullptr;
        StmtsNode* body = nullptr;
        AlifObj name{};

        if (this->currentToken.type_ == TTname) {

            name.type_ = TTname;
            name.A.Name.name_ = this->currentToken.val.numVal;

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

    StmtsNode* while_statement() {

        ExprNode* condetion_ = this->expression();
        StmtsNode* block_ = nullptr;
        StmtsNode* else_ = nullptr;

        if (this->currentToken.type_ == TTcolon)
        {
            this->advance();
            block_ = this->block_();
        }
        if (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == Else)
        {
            else_ = this->else_();
        }

        level--;

        (stmtsNode + level)->type_ = VWhile;
        (stmtsNode + level)->U.While.condetion_ = condetion_;
        (stmtsNode + level)->U.While.block_ = block_;
        (stmtsNode + level)->U.While.else_ = else_;

        return (stmtsNode + level);

    }

    StmtsNode* for_statement()
    {
        if (this->currentToken.type_ == TTname)
        {
            AlifObj itrName{};
            std::vector<ExprNode*>* args_ = new std::vector<ExprNode*>;
            StmtsNode* block_ = nullptr;
            StmtsNode* else_ = nullptr;

            if (Next_Is(TTkeyword))
            {

                itrName.type_ = TTname;
                itrName.A.Name.name_ = this->currentToken.val.numVal;

                this->advance();

                if (this->currentToken.val.keywordType == In)
                {
                    this->advance();

                    if (this->currentToken.type_ == TTlParenthesis)
                    {
                        this->advance();

                        if (this->currentToken.type_ == TTrParenthesis)
                        {
                            prnt(L"for loop args is less than expexted");
                            exit(-1);
                        }

                        while (this->currentToken.type_ == TTinteger or this->currentToken.type_ == TTname) // يجب تعديل الخوارزمية لانه يمكن إحتواء تعبير داخل معاملات حالة لاجل
                        {
                            args_->push_back(this->atom());
                            if (!Next_Is(TTinteger) and !Next_Is(TTname))
                            {
                                break;
                            }
                            this->advance();

                        }

                        if (args_->size() > 3)
                        {
                            prnt(L"for loop args is more than expected");
                            exit(-1);
                        }

                        if (this->currentToken.type_ == TTrParenthesis)
                        {
                            this->advance();

                            if (this->currentToken.type_ == TTcolon)
                            {
                                this->advance();

                                block_ = this->block_();

                                if (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == Else)
                                {
                                    this->advance();

                                    else_ = this->else_();
                                }
                            }
                        }

                    }
                }
            }

            level--;

            (stmtsNode + level)->type_ = VFor;
            (stmtsNode + level)->U.For.itrName = itrName;
            (stmtsNode + level)->U.For.args_ = args_;
            (stmtsNode + level)->U.For.block_ = block_;
            (stmtsNode + level)->U.For.else_ = else_;

            return (stmtsNode + level);
        }
    }

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
        std::vector<StmtsNode*>* elseIf = new std::vector<StmtsNode*>;
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
            elseIf->push_back(this->else_if());
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

                if (this->currentToken.type_ == TTdedent or this->currentToken.type_ == TTendOfFile)
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
    ////}


    StmtsNode* compound_statement()
    {
        if (this->currentToken.val.keywordType == Function)
        {
            this->advance();
            return this->function_def();
        }
        else if (this->currentToken.val.keywordType == If)
        {
            this->advance();
            return this->if_statement();
        }
        else if (this->currentToken.val.keywordType == For)
        {
            this->advance();
            return this->for_statement();
        }
        else if (this->currentToken.val.keywordType == While)
        {
            this->advance();
            return this->while_statement();
        }
        else if (this->currentToken.val.keywordType == Class)
        {
            this->advance();
            return this->class_def();
        }
        if (this->currentToken.val.keywordType == Return)
        {
            if (returnFlag)
            {
                this->advance();
                return this->return_statement();
            }
            else
            {
                prnt(L"لا يمكن إستدعاء حالة ارجع من خارج دالة");
                exit(-1);
            }
        }
    }

    ExprNode* simple_statement()
    {
        return this->assignment();
    }

    StmtsNode* statement() {
        if (this->currentToken.type_ == TTkeyword)
        {
            if (this->currentToken.val.keywordType == Function or this->currentToken.val.keywordType == If or this->currentToken.val.keywordType == Class or this->currentToken.val.keywordType == For or this->currentToken.val.keywordType == While or this->currentToken.val.keywordType == Return)
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
            if (this->currentToken.type_ == TTindent)
            {
                prnt(L"خطأ في المسافات البادئة - لقد خرجت عن النطاق الحالي");
                exit(-1);
            }
            statements_->push_back(this->statement());

        }
        level--;

        (stmtsNode + level)->type_ = VStmts;
        (stmtsNode + level)->U.Stmts.stmts_ = statements_;
        return (stmtsNode + level);

    }

    //// المفسر اللغوي
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::map<NUM, AlifObj> namesTable;
    std::map<BuildInFuncType, AlifObj(Parser::*)(ExprNode*)> buildInFuncsTable{ {Print, &Parser::print} , {Input, &Parser::input} };
    std::map<NUM, StmtsNode*> functionsTable;

    AlifObj visit_stmts(StmtsNode* _node)
    {
        if (_node->type_ == VExpr)
        {
            return this->visit_expr(_node->U.Expr.expr_);
        }
        else if (_node->type_ == VFunction)
        {
            if (_node->U.FunctionDef.name.type_ != TTbuildInFunc) {
                functionsTable[_node->U.FunctionDef.name.A.Name.name_] = _node;
            }
            else {
                buildInFuncsTable.erase(_node->U.FunctionDef.name.A.BuildInFunc.buildInFunc);
                functionsTable[_node->U.FunctionDef.name.A.BuildInFunc.buildInFunc] = _node;
            }
        }
        else if (_node->type_ == VClass)
        {

        }
        else if (_node->type_ == VFor)
        {
            NUM itrName = _node->U.For.itrName.A.Name.name_;

            NUM startVal = 0;
            NUM endVal;
            NUM stepVal = 1;

            if (_node->U.For.args_->size() == 3)
            {
                startVal = this->visit_expr(_node->U.For.args_->at(0)).A.Number.value_;
                endVal = this->visit_expr(_node->U.For.args_->at(1)).A.Number.value_;
                stepVal = this->visit_expr(_node->U.For.args_->at(2)).A.Number.value_;
            }
            else if (_node->U.For.args_->size() == 2)
            {
                startVal = this->visit_expr(_node->U.For.args_->at(0)).A.Number.value_;
                endVal = this->visit_expr(_node->U.For.args_->at(1)).A.Number.value_;
            }
            else
            {
                endVal = this->visit_expr(_node->U.For.args_->at(0)).A.Number.value_;
            }

            //namesTable[itrName] = _node->U.For.args_->at(0);
            symTable.add_symbol(itrName, this->visit_expr(_node->U.For.args_->at(0)));

            
            AlifObj result{};
            for (NUM i = startVal; i < endVal; i += stepVal)
            {
                if (returnFlag)
                {
                    return result;
                }

                //namesTable[itrName].A.Number.value_ = i;
                symTable.add_value(itrName, i);
                result = this->visit_stmts(_node->U.For.block_);

            }
            if (_node->U.For.else_ != nullptr)
            {
                this->visit_stmts(_node->U.For.else_);
            }

        }
        else if (_node->type_ == VWhile)
        {
            AlifObj result{};
            while (this->visit_expr(_node->U.While.condetion_).A.Boolean.value_)
            {
                if (returnFlag) 
                {
                    return result;
                }
                result = this->visit_stmts(_node->U.While.block_);
            }
            if (_node->U.While.else_ != nullptr)
            {
                this->visit_stmts(_node->U.While.else_);
            }
            return _node->U.While.condetion_->U.Object.value_;
        }
        else if (_node->type_ == VIf)
        {

            if (this->visit_expr(_node->U.If.condetion_).A.Boolean.value_)
            {
                return this->visit_stmts(_node->U.If.block_);
            }
            else if (_node->U.If.elseIf != nullptr)
            {
                for (StmtsNode* elseIfs : *_node->U.If.elseIf)
                {
                    if (this->visit_expr(elseIfs->U.If.condetion_).A.Boolean.value_)
                    {
                        return this->visit_stmts(elseIfs->U.If.block_);
                    }
                }

            }
            if (_node->U.If.else_ != nullptr) {
                return this->visit_stmts(_node->U.If.else_);
            }

        }
        else if (_node->type_ == VStmts)
        {
            AlifObj result{};
            for (StmtsNode* stmt_ : *_node->U.Stmts.stmts_)
            {
                result = this->visit_stmts(stmt_);
                if (returnFlag) {
                    return result;
                }
            }

        }
        else if (_node->type_ == VReturn) {

            returnFlag = true;
            if (_node->U.Return.returnExpr != nullptr)
            {
                return visit_expr(_node->U.Return.returnExpr);
            }
            else {
                AlifObj nullObj{};
                nullObj.type_ == TTnone;
                nullObj.A.None.kind_ == None;
                return nullObj;
            }
        }
    }


    AlifObj visit_expr(ExprNode* _node)
    {

        if (_node->type_ == VObject)
        {
            return _node->U.Object.value_;
        }
        else if (_node->type_ == VList)
        {
            _node->U.Object.value_.A.List.objList = new std::vector<AlifObj>;
            for (ExprNode* obj : *_node->U.Object.value_.A.List.list_)
            {
                _node->U.Object.value_.A.List.objList->push_back(this->visit_expr(obj));
            };
            return _node->U.Object.value_;
        }
        else if (_node->type_ == VUnaryOp)
        {
            AlifObj right = this->visit_expr(_node->U.UnaryOp.right_);

            if (_node->U.UnaryOp.operator_ != TTkeyword)
            {
                if (_node->U.UnaryOp.operator_ == TTplus)
                {
                    return right;
                }
                else if (_node->U.UnaryOp.operator_ == TTminus)
                {
                    right.A.Number.value_ = -right.A.Number.value_;
                }
            }
            else
            {
                if (_node->U.UnaryOp.keyword_ == Not)
                {
                    right.A.Boolean.not_();
                }
            }
            return right;
        }
        else if (_node->type_ == VBinOp)
        {
            AlifObj right = this->visit_expr(_node->U.BinaryOp.right_);
            AlifObj left = this->visit_expr(_node->U.BinaryOp.left_);

            if (_node->U.BinaryOp.operator_ != TTkeyword)
            {
                if (_node->U.BinaryOp.operator_ == TTplus)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.add_(&right);
                    }
                    else if (left.type_ == TTstring)
                    {
                        left.A.String.add_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTminus)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.sub_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTmultiply)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.mul_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTdivide)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.div_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTremain)
                {
                    if (left.type_ == TTnumber and left.A.Number.Tkind_ == TTinteger)
                    {
                        left.A.Number.rem_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTpower)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.pow_(&right);
                    }
                }

                else if (_node->U.BinaryOp.operator_ == TTequalEqual)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.equalE_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTnotEqual)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.notE_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTgreaterThan)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.greaterT_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTlessThan)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.lessT_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTgreaterThanEqual)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.greaterTE_(&right);
                    }
                }
                else if (_node->U.BinaryOp.operator_ == TTlessThanEqual)
                {
                    if (left.type_ == TTnumber)
                    {
                        left.A.Number.lessTE_(&right);
                    }
                }
            }
            else
            {
                if (_node->U.BinaryOp.keyword_ == Or)
                {
                    left.A.Boolean.or_(&right);
                }
                else if (_node->U.BinaryOp.keyword_ == And)
                {
                    left.A.Boolean.and_(&right);
                }
            }

            return left;
        }
        else if (_node->type_ == VExpr)
        {
            AlifObj expr_ = this->visit_expr(_node->U.Expr.expr_);
            if (_node->U.Expr.condetion_ != nullptr)
            {
                AlifObj condetion_ = this->visit_expr(_node->U.Expr.condetion_);
                if (condetion_.A.Boolean.value_ != 0)
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
            if (_node->U.NameAssign.name_->size() < 2)
            {
                //namesTable[_node->U.NameAssign.name_->front().A.Name.name_] = this->visit_expr(_node->U.NameAssign.value_);
                symTable.add_symbol(_node->U.NameAssign.name_->front().A.Name.name_, this->visit_expr(_node->U.NameAssign.value_));
            }
            else
            {
                for (AlifObj i : *_node->U.NameAssign.name_)
                {
                    //namesTable[i.A.Name.name_] = this->visit_expr(_node->U.NameAssign.value_);
                    symTable.add_symbol(_node->U.NameAssign.name_->front().A.Name.name_, this->visit_expr(_node->U.NameAssign.value_));
                }

            }
        }
        else if (_node->type_ == VAccess)
        {
            //return namesTable[_node->U.NameAccess.name_.A.Name.name_];
            return symTable.get_data(_node->U.NameAccess.name_.A.Name.name_);
        }
        else if (_node->type_ == VCall) {

            // متبقي فقط اضافة وسيطات بقيم مسبقة والتحقق من وجودها في الدالة ام لا

            if (_node->U.Call.name->U.NameAccess.name_.type_ == TTbuildInFunc)
            {
                if (buildInFuncsTable.count(_node->U.Call.name->U.NameAccess.name_.A.BuildInFunc.buildInFunc))
                {
                    return (this->*buildInFuncsTable[_node->U.Call.name->U.NameAccess.name_.A.BuildInFunc.buildInFunc])(_node);
                }
                else {
                    StmtsNode* func = functionsTable[_node->U.Call.name->U.NameAccess.name_.A.Name.name_];

                    symTable.enter_scope(_node->U.Call.name->U.NameAccess.name_.A.Name.name_);

                    if (func->U.FunctionDef.params != nullptr)
                    {

                        int lenArg = _node->U.Call.args->size();

                        int i = 0;
                        for (ExprNode* param : *func->U.FunctionDef.params)
                        {
                            if (param->type_ == VAccess) {
                                //namesTable[param->U.NameAccess.name_.A.Name.name_] = this->visit_expr(_node->U.Call.args->at(lenArg));
                                //namesTable[param->U.NameAccess.name_->A.Name.name_] = this->visit_expr(_node->U.Call.args->at(i));
                                symTable.add_symbol(param->U.NameAccess.name_.A.Name.name_, this->visit_expr(_node->U.Call.args->at(i)));
                            }
                            else
                            {
                                if (_node->U.Call.args->at(lenArg) != nullptr) {

                                    //namesTable[param->U.NameAssign.name_->at(0)->A.Name.name_] = this->visit_expr(_node->U.Call.args->at((lenArg - 1)));
                                    symTable.add_symbol(param->U.NameAssign.name_->at(0).A.Name.name_, this->visit_expr(_node->U.Call.args->at((lenArg - 1))));
                                }
                                else {

                                    //namesTable[param->U.NameAssign.name_->at(0)->A.Name.name_] = this->visit_expr(param->U.NameAssign.value_);
                                    symTable.add_symbol(param->U.NameAssign.name_->at(0).A.Name.name_, this->visit_expr(param->U.NameAssign.value_));
                                }
                            }
                            i++;
                        }
                    }
                    AlifObj res = visit_stmts(func->U.FunctionDef.body);
                    symTable.exit_scope();
                    return res;
                }
            }
            else {
                StmtsNode* func = functionsTable[_node->U.Call.name->U.NameAccess.name_.A.Name.name_];

                symTable.enter_scope(_node->U.Call.name->U.NameAccess.name_.A.Name.name_);

                if (func->U.FunctionDef.params != nullptr)
                {

                    int lenArg = _node->U.Call.args->size();

                    int i = 0;
                    for (ExprNode* param : *func->U.FunctionDef.params)
                    {
                        if (param->type_ == VAccess) {

                            //namesTable[param->U.NameAccess.name_->A.Name.name_] = this->visit_expr(_node->U.Call.args->at(i));
                            symTable.add_symbol(param->U.NameAccess.name_.A.Name.name_, this->visit_expr(_node->U.Call.args->at(i)));

                        }
                        else if (lenArg == (i + 1)) {

                            //namesTable[param->U.NameAssign.name_->at(0)->A.Name.name_] = this->visit_expr(_node->U.Call.args->at((lenArg - 1)));
                            symTable.add_symbol(param->U.NameAssign.name_->at(0).A.Name.name_, this->visit_expr(_node->U.Call.args->at((lenArg - 1))));

                        }
                        else {

                            //namesTable[param->U.NameAssign.name_->at(0)->A.Name.name_] = this->visit_expr(param->U.NameAssign.value_);
                            symTable.add_symbol(param->U.NameAssign.name_->at(0).A.Name.name_, this->visit_expr(param->U.NameAssign.value_));

                        }
                        i++;
                    }
                }
                AlifObj res = visit_stmts(func->U.FunctionDef.body);
                returnFlag = false;
                symTable.exit_scope();
                return res;
            }

        }
        else if (_node->type_ == VAugAssign)
        {
            AlifObj value = this->visit_expr(_node->U.AugNameAssign.value_);
            AlifObj name = symTable.get_data(_node->U.AugNameAssign.name_.A.Name.name_);
            //AlifObj name = namesTable[_node->U.AugNameAssign.name_.A.Name.name_];

            if (_node->U.AugNameAssign.operator_ == TTplusEqual)
            {
                if (name.type_ == TTnumber)
                {
                    name.A.Number.add_(&value);
                }
                else if (name.type_ == TTstring)
                {
                    name.A.String.add_(&value);
                }
            }
            else if (_node->U.AugNameAssign.operator_ == TTminusEqual)
            {
                if (name.type_ == TTnumber)
                {
                    name.A.Number.sub_(&value);
                }
            }
            else if (_node->U.AugNameAssign.operator_ == TTmultiplyEqual)
            {
                if (name.type_ == TTnumber)
                {
                    name.A.Number.mul_(&value);
                }
            }
            else if (_node->U.AugNameAssign.operator_ == TTdivideEqual)
            {
                if (name.type_ == TTnumber)
                {
                    name.A.Number.div_(&value);
                }
            }
            else if (_node->U.AugNameAssign.operator_ == TTremainEqual)
            {
                if (name.type_ == TTnumber)
                {
                    name.A.Number.rem_(&value);
                }
            }
            symTable.add_symbol(_node->U.AugNameAssign.name_.A.Name.name_, name);
            //namesTable[_node->U.AugNameAssign.name_.A.Name.name_] = name;
            return name;
        }

    }

    AlifObj str{};
    AlifObj input(ExprNode* node) {
        str.type_ = TTstring;
        str.A.String.value_ = new std::wstring();
        std::wcin >> *str.A.String.value_;
        return str;
    }

    AlifObj print(ExprNode* node) {
        AlifObj val;
        for (ExprNode* arg : *node->U.Call.args) {
            val = this->visit_expr(arg);
            if (val.type_ == TTstring) { prnt(*val.A.String.value_); }
            else if (val.type_ == TTnumber) { prnt(val.A.Number.value_); }
            else if (val.type_ == TTkeyword) { if (val.A.Boolean.value_ == 1) { prnt(L"صح"); } else { prnt(L"خطا"); } }
            else if (val.type_ == TTlist) { STR lst = L"["; for (AlifObj obj : *val.A.List.objList) { lst.append(std::to_wstring((int)obj.A.Number.value_)); lst.append(L", "); } lst.replace(lst.length() - 2, lst.length(), L"]"); prnt(lst); }
        }
        return val;
    }
};