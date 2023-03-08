#include "Parser.h"

SymbolTable symTable; // تم تعريفه ك متغير عام لمنع حذف المتغيرات عند استخدام الطرفية بعد الانتقال الى سطر جديد

class Parser {
public:
    std::vector<Token>* tokens;
    int tokenIndex = -1;
    Token currentToken;
    STR fileName;
    STR input_;

    /// <اعلام>

    bool lastParam = false;
    bool returnFlag = false;

    /// </اعلام>

    uint16_t exprlevel = 4000;
    uint16_t stmtslevel = 1000;

    ExprNode* exprNode = (ExprNode*)malloc(exprlevel * sizeof(struct ExprNode));
    StmtsNode* stmtsNode = (StmtsNode*)malloc(stmtslevel * sizeof(struct StmtsNode));


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
        ExprNode* stmtsRes = this->assignment();
        AlifObj intrRes = this->visit_expr(stmtsRes);

        if (intrRes.type_ == TTnumber) { prnt(intrRes.A.Number.value_); }
        else if (intrRes.type_ == TTstring) { prnt(*intrRes.A.String.value_); }
        else if (intrRes.type_ == TTnone) { prnt(L"عدم"); }
        else if (intrRes.type_ == TTkeyword) { if (intrRes.A.Boolean.value_ == 1) { prnt(L"صح"); } else { prnt(L"خطا"); } }
        else if (intrRes.type_ == TTlist) {
            this->list_print(intrRes);
            prnt(lst);
        }
    }


    STR lst;
    void list_print(AlifObj _obj) {
        lst.append(L"[");
        if (_obj.type_ == TTlist) {
            for (AlifObj obj : *_obj.A.List.objList) {
                if (obj.type_ == TTnumber)
                {
                    lst.append(std::to_wstring(obj.A.Number.value_));

                }
                else if (obj.type_ == TTstring)
                {
                    lst.append(*obj.A.String.value_);

                }
                else if (obj.type_ == TTkeyword)
                {
                    lst.append(std::to_wstring(obj.A.Boolean.value_));

                }
                else if (obj.type_ == TTlist) {
                    this->list_print(obj);
                }
                lst.append(L", ");
            }
            lst.replace(lst.length() - 2, lst.length(), L"]");
        }
    }

    //////////////////////////////

    std::vector<ExprNode*>* arguments() {

        std::vector<ExprNode*>* args = new std::vector<ExprNode*>;

        do {

            args->push_back(this->expression());

            if (this->currentToken.type_ == TTcomma) {
                this->advance();
            }

        } while (this->currentToken.type_ != TTrParenthesis);

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
            else {
                prnt(L"يوجد كلمة مفتاحية في غير سياقها");
                exit(-1);
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
                prnt(L"لم يتم إغلاق قوس القوس");
                exit(-1);
            }
        }
        else
        {
            prnt(L"لم يتم العثور على حالة");
            exit(-1);
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
                prnt(L"خطأ في حالة تعبير - لم يتم إضافة \"والا\" للحالة");
                exit(-1);
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


        std::vector<ExprNode*>* params_ = new std::vector<ExprNode*>;
        lastParam = false;

        do {

            this->advance();

            if (this->currentToken.type_ != TTrParenthesis)
            {
                if (Next_Is(TTequal))
                {
                    lastParam = true;

                    AlifObj name_{};

                    name_.type_ = TTname;
                    name_.A.Name.name_ = this->currentToken.val.numVal;

                    this->advance();
                    this->advance();

                    ExprNode* expr_ = this->expression();

                    level--;

                    (exprNode + level)->type_ = VAssign;
                    (exprNode + level)->U.NameAssign.paramName = name_;
                    (exprNode + level)->U.NameAssign.value_ = expr_;

                    params_->push_back((exprNode + level));

                }
                else {
                    if (!lastParam)
                    {
                        params_->push_back(this->atom());
                    }
                    else {
                        prnt(L"لا يمكن تمرير متغير بدون قيمة افتراضية بعد متغير ذو قيمة افتراضية");
                        exit(-1);
                    }
                }

            }
            else
            {
                return params_;
            }

        } while (this->currentToken.type_ == TTcomma);

        if (this->currentToken.type_ == TTrParenthesis)
        {
            return params_;
        }
        else {
            prnt(L"لم يتم إغلاق القوس");
            exit(-1);
        }

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
            else if (this->currentToken.type_ == TTbuildInFunc) {
                name.A.BuildInFunc.buildInFunc = this->currentToken.val.buildInFunc;
            }
            else
            {
                prnt(L"يتوقع وجود اسم للدالة");
            }

            this->advance();

            if (this->currentToken.type_ == TTlParenthesis and Next_Is(TTrParenthesis)) {

                this->advance();
                this->advance();
            }
            else {
                params = this->parameters();
                this->advance();
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
            else {
                prnt(L"لم يتم إنهاء دالة بنقطتين \:");
                exit(-1);
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
        else {
            prnt(L"لم يتم إنهاء بينما بنقطتين \:");
            exit(-1);
        }
        if (this->currentToken.type_ == TTkeyword and this->currentToken.val.keywordType == Else)
        {
            this->advance();
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
                            prnt(L"المعاملات المسندة اقل من المتوقع");
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
                            prnt(L"المعاملات المسندة اكثر من المتوقع");
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
                            else {
                                prnt(L"لم يتم إنهاء لاجل بنقطتين \:");
                                exit(-1);
                            }
                        }
                        else {
                            prnt(L"يتوقع وجود قوس ')'");
                            exit(-1);
                        }

                    }
                    else {
                        prnt(L"يتوقع وجود قوس '('");
                        exit(-1);
                    }
                }
            }
            else {
                prnt(L"يتوقع وجود كلمة مفتاحية 'في'");
                exit(-1);
            }

            level--;

            (stmtsNode + level)->type_ = VFor;
            (stmtsNode + level)->U.For.itrName = itrName;
            (stmtsNode + level)->U.For.args_ = args_;
            (stmtsNode + level)->U.For.block_ = block_;
            (stmtsNode + level)->U.For.else_ = else_;

            return (stmtsNode + level);
        }
        else {
            prnt(L"يتوقع وجود اسم لاسناد قيمة له");
            exit(-1);
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
        else {
            prnt(L"لم يتم إنهاء واذا بنقطتين \:");
            exit(-1);
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
        else {
            prnt(L"لم يتم إنهاء والا بنقطتين \:");
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
        else {
            prnt(L"لم يتم إنهاء اذا بنقطتين \:");
            exit(-1);
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
                else if (this->currentToken.type_ == TTindent) {
                    prnt(L"يتوقع وجود مسافة راجعة في نهاية الحالة المركبة");
                    exit(-1);
                }
            }
            else {
                prnt(L"يتوقع وجود مسافة بادئة في بداية جسم الحالة المركبة");
                exit(-1);
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
                prnt(L"لا يمكن إستدعاء ارجع من خارج دالة");
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

            if (this->currentToken.type_ != TTnewline) {
                prnt(L"لا يمكن وجود اكثر من حالة في نفس السطر");
                exit(-1);
            }
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
};