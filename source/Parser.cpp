#include "Parser.h"



Parser::Parser(std::vector<Token>* tokens_, wstr _fileName, wstr* _input, MemoryBlock* _alifMemory) : 
    tokens_(tokens_), fileName(_fileName), input_(_input), alifMemory(_alifMemory)
{
    this->advance();
}

void Parser::advance()
{
    this->tokenIndex++;
    if (this->tokenIndex >= 0 and this->tokenIndex < this->tokens_->size())
    {
        std::vector<Token>::iterator listIter = tokens_->begin();
        std::advance(listIter, this->tokenIndex);
        this->currentToken = *listIter;
    }
}

void Parser::parse_file()
{
    //ExprNode* stmtsRes = nullptr; // convert it to StmtsNode after finish test

    do {
        this->statements_.push_back(this->statement()); // convert it to StmtsNode after finish test

    } while (currentToken.type_ != TTEndOfFile);

}

void Parser::parse_terminal()
{
    this->statements_.push_back(this->statement()); // convert it to StmtsNode after finish test
}

void Parser::list_print(AlifObject _obj) {
    lst_.append(L"[");
    //if (_obj.objType == OTList) {
    //    for (AlifObject obj : *_obj.V.ListObj.objList) {
    //        if (obj.objType == OTNumber)
    //        {
    //            lst_.append(std::to_wstring(obj.V.NumberObj.numberValue));

    //        }
    //        else if (obj.objType == OTString)
    //        {
    //            lst_.append(*obj.V.StringObj.strValue);

    //        }
    //        else if (obj.objType == OTKeyword)
    //        {
    //            lst_.append(std::to_wstring(obj.V.BoolObj.boolValue));

    //        }
    //        else if (obj.objType == OTList) {
    //            this->list_print(obj);
    //        }
    //        lst_.append(L", ");
    //    }
    //    lst_.replace(lst_.length() - 2, lst_.length(), L"]");
    //}
}

    //////////////////////////////

std::vector<ExprNode*>* Parser::arguments() {

    std::vector<ExprNode*>* args = new std::vector<ExprNode*>;

    do {

        args->push_back(this->expression());

        if (this->currentToken.type_ == TTComma) {
            this->advance();
        }

    } while (this->currentToken.type_ != TTRrightParenthesis);

    return args;

}

ExprNode* Parser::atom() {

    Token token = this->currentToken;

    if (token.type_ == TTName)
    {
        if (!is_keyword(token.value_))
        {
            this->advance();

            AlifObject* nameObj = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
            nameObj->objType = OTName;
            nameObj->V.NameObj.name_ = token.value_;

            ExprNode* nameNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
            nameNode->U.NameAccess.name_ = nameObj;
            nameNode->type_ = VTAccess;

            return nameNode;
        }
        if (!wcscmp(token.value_, L"صح"))
        {
            this->advance();

            AlifObject* keyObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
            keyObject->objType = OTBoolean;
            keyObject->V.BoolObj.boolType = L"صح";
            keyObject->V.BoolObj.numberValue = 1;

            ExprNode* keyNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
            keyNode->U.Object.value_ = keyObject;
            keyNode->type_ = VTObject;

            return keyNode;
        }
        else if (!wcscmp(token.value_, L"خطا"))
        {
            this->advance();

            AlifObject* keyObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
            keyObject->objType = OTBoolean;
            keyObject->V.BoolObj.boolType = L"خطا";
            keyObject->V.BoolObj.numberValue = 0;

            ExprNode* keyNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
            keyNode->U.Object.value_ = keyObject;
            keyNode->type_ = VTObject;

            return keyNode;
        }
        else if (!wcscmp(token.value_, L"عدم"))
        {
            this->advance();

            AlifObject* keyObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
            keyObject->objType = OTNone;

            ExprNode* keyNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
            keyNode->U.Object.value_ = keyObject;
            keyNode->type_ = VTObject;

            return keyNode;
        }
        else
        {
            PRINT_(SyntaxError(token.posStart, token.posEnd, token.posIndex, token.tokLine, L"كلمة مفتاحية في غير سياقها", fileName, input_).print_());
            exit(-1);
        }
    }
    else if (token.type_ == TTInteger)
    {
        this->advance();
        wchar_t* pEnd{};

        AlifObject* intObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
        intObject->objType = OTNumber;
        intObject->V.NumberObj.numberType = token.type_;
        intObject->V.NumberObj.numberValue = wcstol(token.value_, &pEnd, 10);
        intObject->posStart = token.posStart;
        intObject->posEnd = token.posEnd;
        intObject->tokLine = token.tokLine;
        intObject->posIndex = token.posIndex;

        ExprNode* intNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        intNode->U.Object.value_ = intObject;
        intNode->type_ = VTObject;

        //alifMemory->deallocate(&token.value_); // لانه تم تحويل القيمة ولم يعد للقيمة النصية استخدام

        return intNode;
    }
    else if (token.type_ == TTFloat)
    {
        this->advance();
        wchar_t* pEnd{};

        AlifObject* floatObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
        floatObject->objType = OTNumber;
        floatObject->V.NumberObj.numberType = token.type_;
        floatObject->V.NumberObj.numberValue = wcstold(token.value_, &pEnd);
        floatObject->posStart = token.posStart;
        floatObject->posEnd = token.posEnd;
        floatObject->tokLine = token.tokLine;
        floatObject->posIndex = token.posIndex;

        ExprNode* floatNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        floatNode->U.Object.value_ = floatObject;
        floatNode->type_ = VTObject;

        //alifMemory->deallocate(&token.value_);

        return floatNode;

    }
    else if (token.type_ == TTString)
    {
        this->advance();

        AlifObject* stringObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

        stringObject->objType = OTString;
        stringObject->V.StringObj.strValue = token.value_;
        stringObject->posStart = token.posStart;
        stringObject->posEnd = token.posEnd;
        stringObject->tokLine = token.tokLine;
        stringObject->posIndex = token.posIndex;

        ExprNode* stringNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        stringNode->U.Object.value_ = stringObject;
        stringNode->type_ = VTObject;

        return stringNode;
    }
    else if (token.type_ == TTLeftSquare)
    {
        return this->list_expr();
    }
    else if (this->currentToken.type_ == TTLeftParenthesis)
    {
        this->advance();
        ExprNode* priorExpr = this->expression();

        if (this->currentToken.type_ == TTRrightParenthesis)
        {
            this->advance();
            return priorExpr;
        }
        else
        {
            token = this->currentToken;
            PRINT_(SyntaxError(token.posStart, token.posEnd, token.posIndex - 1, token.tokLine - 1, L"لم يتم إغلاق القوس", fileName, input_).print_());
            exit(-1);
        }
    }
    else
    {
        PRINT_(SyntaxError(token.posStart, token.posEnd, token.posIndex - 1, token.tokLine - 1, L"حالة غير مكتملة", fileName, input_).print_());
        exit(-1);
    }
}

ExprNode* Parser::list_expr()
{
    std::vector<ExprNode*>* nodeElement = new std::vector<ExprNode*>;

    if (this->currentToken.type_ == TTRightSquare)
    {
        this->advance();
    }
    else
    {
        do {
            this->advance();
            nodeElement->push_back(this->expression());

        } while (this->currentToken.type_ == TTComma);

        if (this->currentToken.type_ != TTRightSquare)
        {
            PRINT_(SyntaxError(this->currentToken.posStart, this->currentToken.posEnd, this->currentToken.posIndex - 1, this->currentToken.tokLine - 1, L"لم يتم إغلاق قوس المصفوفة", fileName, input_).print_());
            exit(-1);
        }
        this->advance();
    }

    AlifObject* listObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

    listObject->objType = OTList;
    listObject->V.ListObj.list_ = nodeElement;

    ExprNode* listNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
    listNode->U.Object.value_ = listObject;
    listNode->type_ = VTList;

    return listNode;

}

ExprNode* Parser::primary() {

    ExprNode* atom = this->atom();
    if (this->currentToken.type_ == TTDot) 
    {
        this->advance();
        ExprNode* primary = this->primary();

        ExprNode* primary_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        primary_->type_ = VTCall;
        primary_->U.Call.func_ = atom;
        primary_->U.Call.name_ = primary;
        return primary_;
    }
    else if (this->currentToken.type_ == TTLeftParenthesis) {

        ExprNode* primary = atom;

        this->advance();

        if (this->currentToken.type_ == TTRrightParenthesis) {

            this->advance();

            //exprLevel--;
            //(exprNode + exprLevel)->type_ = VTCall;
            //(exprNode + exprLevel)->U.Call.name_ = primary;
            //(exprNode + exprLevel)->U.Call.args_ = nullptr;
            //return (exprNode + exprLevel);

        }

        std::vector<ExprNode*>* args = this->arguments();

        this->advance();

        //exprLevel--;
        //(exprNode + exprLevel)->type_ = VTCall;
        //(exprNode + exprLevel)->U.Call.name_ = primary;
        //(exprNode + exprLevel)->U.Call.args_ = args;
        //return (exprNode + exprLevel);

    }
    else {
        return atom;
    }

}

ExprNode* Parser::power()
{
    ExprNode* left_ = this->primary();

    while (this->currentToken.type_ == TTPower) {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->factor();

        ExprNode* power_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        power_->U.BinaryOp.right_ = right_;
        power_->U.BinaryOp.operator_ = opToken.type_;
        power_->U.BinaryOp.left_ = left_;
        power_->type_ = VTBinOp;

        left_ = power_;
    }

    return left_;
}

ExprNode* Parser::factor() 
{
    while (this->currentToken.type_ == TTPlus or this->currentToken.type_ == TTMinus) {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->power();

        ExprNode* factor_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        factor_->U.UnaryOp.right_ = right_;
        factor_->U.UnaryOp.operator_ = opToken.type_;
        factor_->type_ = VTUnaryOp;

        return factor_;
    }

    return this->power();
}

ExprNode* Parser::term() 
{
    ExprNode* left_ = this->factor();

    while (this->currentToken.type_ == TTMultiply or this->currentToken.type_ == TTDivide or this->currentToken.type_ == TTRemain) {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->factor();

        ExprNode* term_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        term_->U.BinaryOp.right_ = right_;
        term_->U.BinaryOp.operator_ = opToken.type_;
        term_->U.BinaryOp.left_ = left_;
        term_->type_ = VTBinOp;

        left_ = term_;
    }

    return left_;
}

ExprNode* Parser::sum() 
{
    ExprNode* left_ = this->term();

    while (this->currentToken.type_ == TTPlus or this->currentToken.type_ == TTMinus)
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->term();

        ExprNode* sum_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        sum_->U.BinaryOp.right_ = right_;
        sum_->U.BinaryOp.operator_ = opToken.type_;
        sum_->U.BinaryOp.left_ = left_;
        sum_->type_ = VTBinOp;

        left_ = sum_;
    }

    return left_;
}

ExprNode* Parser::comparesion() 
{
    ExprNode* left_ = this->sum();

    while (
        this->currentToken.type_ == TTEqualEqual or
        this->currentToken.type_ == TTNotEqual or
        this->currentToken.type_ == TTLessThan or
        this->currentToken.type_ == TTGreaterThan or
        this->currentToken.type_ == TTLessThanEqual or
        this->currentToken.type_ == TTGreaterThanEqual
         ) 
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->sum();

        ExprNode* comparesion_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        comparesion_->U.BinaryOp.right_ = right_;
        comparesion_->U.BinaryOp.operator_ = opToken.type_;
        comparesion_->U.BinaryOp.left_ = left_;
        comparesion_->type_ = VTBinOp;

        left_ = comparesion_;
    }

    return left_;
}

ExprNode* Parser::inversion() 
{

    while (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"ليس"))
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->comparesion();

        ExprNode* inversion_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        inversion_->U.UnaryOp.right_ = right_;
        inversion_->U.UnaryOp.operator_ = opToken.type_;
        inversion_->U.UnaryOp.keyword_ = opToken.value_;
        inversion_->type_ = VTUnaryOp;

        return inversion_;
    }

    return this->comparesion();
}

ExprNode* Parser::conjuction()
{

    ExprNode* left_ = this->inversion();

    while (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"و"))
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->inversion();

        ExprNode* conjuction_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        conjuction_->U.BinaryOp.right_ = right_;
        conjuction_->U.BinaryOp.operator_ = opToken.type_;
        conjuction_->U.BinaryOp.keyword_ = opToken.value_;
        conjuction_->U.BinaryOp.left_ = left_;
        conjuction_->type_ = VTBinOp;

        left_ = conjuction_;
    }

    return left_;
}

ExprNode* Parser::disjuction() 
{

    ExprNode* left_ = this->conjuction();

    while (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"او"))
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->conjuction();

        ExprNode* disjuction_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        disjuction_->U.BinaryOp.right_ = right_;
        disjuction_->U.BinaryOp.operator_ = opToken.type_;
        disjuction_->U.BinaryOp.keyword_ = opToken.value_;
        disjuction_->U.BinaryOp.left_ = left_;
        disjuction_->type_ = VTBinOp;

        left_ = disjuction_;
    }

    return left_;
}

ExprNode* Parser::expression() 
{

    ExprNode* expr_ = this->disjuction();

    if (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"اذا"))
    {
        this->advance();
        ExprNode* condetion = this->disjuction();

        if (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"والا"))
        {
            this->advance();
            ExprNode* elseExpr = this->expression();

            ExprNode* expression_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

            expression_->U.CondExpr.expr_ = expr_;
            expression_->U.CondExpr.condetion_ = condetion;
            expression_->U.CondExpr.elseExpr = elseExpr;
            expression_->type_ = VTCondExpr;

            return expression_;
        }
        else
        {
            PRINT_(L"خطأ في حالة تعبير - لم يتم إضافة \"والا\" للحالة");
            exit(-1);
        }
    }

    return expr_;

}

ExprNode* Parser::expressions() 
{

    ExprNode* expr_ = this->expression();

    if (this->currentToken.type_ == TTComma)
    {
        std::vector<ExprNode*>* exprs_ = new std::vector<ExprNode*>;

        exprs_->push_back(expr_);
        do
        {
            this->advance();
            exprs_->push_back(this->expression());

        } while (this->currentToken.type_ == TTComma);

        AlifObject* exprObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

        exprObject->objType = OTList;
        exprObject->V.ListObj.list_ = exprs_;

        ExprNode* listNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        listNode->U.Object.value_ = exprObject;
        listNode->type_ = VTList;

        return listNode;
    }
    return expr_;
}


ExprNode* Parser::assignment() // يجب إيجاد خوارزمية افضل بالإضافة الى استخدام ذاكرة ألف بدل من new
{
    if (this->currentToken.type_ == TTName)
    {
        if (Next_Is(TTEqual))
        {
            std::vector<AlifObject*>* names_ = new std::vector<AlifObject*>;

            while (Next_Is(TTEqual))
            {
                AlifObject* name_ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

                name_->objType = OTName;
                name_->V.NameObj.name_ = this->currentToken.value_;

                names_->push_back(name_);

                this->advance();
                this->advance();

            }

            ExprNode* expr_ = this->expressions();

            ExprNode* assignment_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

            assignment_->U.NameAssign.name_ = names_;
            assignment_->U.NameAssign.value_ = expr_;
            assignment_->type_ = VTAssign;

            return assignment_;

        }
        else if (Next_Is(TTPlusEqual) or 
                 Next_Is(TTMinusEqual) or 
                 Next_Is(TTMultiplyEqual) or 
                 Next_Is(TTDivideEqual) or 
                 Next_Is(TTPowerEqual) or 
                 Next_Is(TTRemainEqual)) 
                    {
                    AlifObject* name_ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
                    name_->objType = OTName;
                    name_->V.NameObj.name_ = this->currentToken.value_;

                    this->advance();

                    Token opToken = this->currentToken;


                    this->advance();
                    ExprNode* expr_ = this->expression();

                    ExprNode* augAssignment = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

                    augAssignment->U.AugNameAssign.name_ = name_;
                    augAssignment->U.AugNameAssign.operator_ = opToken.type_;
                    augAssignment->U.AugNameAssign.value_ = expr_;
                    augAssignment->type_ = VTAugAssign;

                    return augAssignment;
             }
    }

    return this->expressions();
}

//StmtsNode* Parser::return_statement() {
//
//    ExprNode* expr_;
//
//    if (this->currentToken.type_ != TTNewline)
//    {
//        expr_ = this->expression();
//    }
//    else
//    {
//        expr_ = nullptr;
//    }
//    this->advance();
//
//    exprLevel--;
//    (stmtsNode + exprLevel)->U.Return.returnExpr = expr_;
//    (stmtsNode + exprLevel)->type_ = VTReturn;
//
//    return (stmtsNode + exprLevel);
//}

//    std::vector<ExprNode*>* parameters() {
//
//
//        std::vector<ExprNode*>* params_ = new std::vector<ExprNode*>;
//        lastParam = false;
//
//        do {
//
//            this->advance();
//
//            if (this->currentToken.type_ != TTRrightParenthesis)
//            {
//                if (Next_Is(TTequal))
//                {
//                    lastParam = true;
//
//                    AlifObject name_{};
//
//                    name_.type_ = TTname;
//                    name_.V.Name.name_ = this->currentToken.val.numVal;
//
//                    this->advance();
//                    this->advance();
//
//                    ExprNode* expr_ = this->expression();
//
//                    level--;
//
//                    (exprNode + level)->type_ = VAssign;
//                    (exprNode + level)->U.NameAssign.paramName = name_;
//                    (exprNode + level)->U.NameAssign.value_ = expr_;
//
//                    params_->push_back((exprNode + level));
//
//                }
//                else {
//                    if (!lastParam)
//                    {
//                        params_->push_back(this->atom());
//                    }
//                    else {
//                        PRINT_(L"لا يمكن تمرير متغير بدون قيمة افتراضية بعد متغير ذو قيمة افتراضية");
//                        exit(-1);
//                    }
//                }
//
//            }
//            else
//            {
//                return params_;
//            }
//
//        } while (this->currentToken.type_ == TTComma);
//
//        if (this->currentToken.type_ == TTRrightParenthesis)
//        {
//            return params_;
//        }
//        else {
//            PRINT_(L"لم يتم إغلاق القوس");
//            exit(-1);
//        }
//
//    }
//
//    StmtsNode* function_def() {
//
//        AlifObject name{};
//        StmtsNode* body = nullptr;
//        std::vector<ExprNode*>* params = nullptr;
//
//        if (this->currentToken.type_ == TTname or this->currentToken.type_ == TTbuildInFunc) {
//
//            name.type_ = this->currentToken.type_;
//
//            if (this->currentToken.type_ == TTname)
//            {
//                name.V.Name.name_ = this->currentToken.val.numVal;
//            }
//            else if (this->currentToken.type_ == TTbuildInFunc) {
//                name.V.BuildInFunc.buildInFunc = this->currentToken.val.buildInFunc;
//            }
//            else
//            {
//                PRINT_(L"يتوقع وجود اسم للدالة");
//            }
//
//            this->advance();
//
//            if (this->currentToken.type_ == TTLeftParenthesis and Next_Is(TTRrightParenthesis)) {
//
//                this->advance();
//                this->advance();
//            }
//            else {
//                params = this->parameters();
//                this->advance();
//            }
//
//            if (this->currentToken.type_ == TTcolon) {
//
//                this->advance();
//                returnFlag = true;
//                body = this->block_();
//                returnFlag = false;
//
//                level--;
//                (stmtsNode + level)->type_ = VFunction;
//                (stmtsNode + level)->U.FunctionDef.name = name;
//                (stmtsNode + level)->U.FunctionDef.params = params;
//                (stmtsNode + level)->U.FunctionDef.body = body;
//                return (stmtsNode + level);
//            }
//            else {
//                PRINT_(L"لم يتم إنهاء دالة بنقطتين \:");
//                exit(-1);
//            }
//        }
//
//    }
//
//    StmtsNode* class_def() {
//
//        ExprNode* bases = nullptr;
//        StmtsNode* body = nullptr;
//        AlifObject name{};
//
//        if (this->currentToken.type_ == TTname) {
//
//            name.type_ = TTname;
//            name.V.Name.name_ = this->currentToken.val.numVal;
//
//            this->advance();
//
//            if (this->currentToken.type_ == TTLeftParenthesis) {
//
//                this->advance();
//
//                bases = this->expressions();
//
//                if (this->currentToken.type_ == TTRrightParenthesis)
//                {
//                    this->advance();
//                }
//            }
//            if (this->currentToken.type_ == TTcolon) {
//
//                this->advance();
//
//                body = this->block_();
//
//                level--;
//                (stmtsNode + level)->type_ = VClass;
//                (stmtsNode + level)->U.ClassDef.name = name;
//                (stmtsNode + level)->U.ClassDef.base = bases;
//                (stmtsNode + level)->U.ClassDef.body = body;
//                return (stmtsNode + level);
//            }
//        }
//    }
//
//    StmtsNode* while_statement() {
//
//        ExprNode* condetion_ = this->expression();
//        StmtsNode* block_ = nullptr;
//        StmtsNode* else_ = nullptr;
//
//        if (this->currentToken.type_ == TTcolon)
//        {
//            this->advance();
//            block_ = this->block_();
//        }
//        else {
//            PRINT_(L"لم يتم إنهاء بينما بنقطتين \:");
//            exit(-1);
//        }
//        if (this->currentToken.type_ == TTKeyword and this->currentToken.val.keywordType == Else)
//        {
//            this->advance();
//            else_ = this->else_();
//        }
//
//        level--;
//
//        (stmtsNode + level)->type_ = VWhile;
//        (stmtsNode + level)->U.While.condetion_ = condetion_;
//        (stmtsNode + level)->U.While.block_ = block_;
//        (stmtsNode + level)->U.While.else_ = else_;
//
//        return (stmtsNode + level);
//
//    }

StmtsNode* Parser::for_statement()
{
    if (this->currentToken.type_ == TTName)
    {
        AlifObject* itrName = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
        std::vector<ExprNode*>* args_ = new std::vector<ExprNode*>;
        StmtsNode* block_ = nullptr;
        StmtsNode* else_ = nullptr;

        Token name_ = this->currentToken;
        this->advance();
        
        if (is_keyword(this->currentToken.value_))
        {
            itrName->objType = OTName;
            itrName->V.NameObj.name_ = name_.value_;

            //this->advance();
            if (!wcscmp(this->currentToken.value_, L"في"))
            {
                this->advance();

                if (this->currentToken.type_ == TTLeftParenthesis)
                {
                    this->advance();

                    if (this->currentToken.type_ == TTRrightParenthesis)
                    {
                        PRINT_(L"المعاملات المسندة اقل من المتوقع");
                        exit(-1);
                    }

                    while (this->currentToken.type_ == TTInteger or this->currentToken.type_ == TTName) // يجب تعديل الخوارزمية لانه يمكن إحتواء تعبير داخل معاملات حالة لاجل
                    {
                        args_->push_back(this->atom());

                        if (!Next_Is(TTInteger) and !Next_Is(TTName))
                        {
                            break;
                        }

                        this->advance();
                    }

                    if (args_->size() > 3)
                    {
                        PRINT_(L"المعاملات المسندة اكثر من المتوقع");
                        exit(-1);
                    }

                    if (this->currentToken.type_ == TTRrightParenthesis)
                    {
                        this->advance();

                        if (this->currentToken.type_ == TTColon)
                        {
                            this->advance();

                            block_ = this->block_();

                            //if (this->currentToken.type_ == TTKeyword and this->currentToken.val.keywordType == Else)
                            if (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"والا"))
                            {
                                this->advance();

                                else_ = this->else_();
                            }
                        }
                        else {
                            PRINT_(L"لم يتم إنهاء لاجل بنقطتين \:");
                            exit(-1);
                        }
                    }
                    else {
                        PRINT_(L"يتوقع وجود قوس ')'");
                        exit(-1);
                    }

                }
                else {
                    PRINT_(L"يتوقع وجود قوس '('");
                    exit(-1);
                }
            }
        }
        else {
            PRINT_(L"يتوقع وجود كلمة مفتاحية 'في'");
            exit(-1);
        }

        StmtsNode* forNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));
        forNode->type_ = VTFor;
        forNode->U.For.itrName = itrName;
        forNode->U.For.args_ = args_;
        forNode->U.For.block_ = block_;
        forNode->U.For.else_ = else_;

        return forNode;
    }
    else {
        PRINT_(L"يتوقع وجود اسم لاسناد قيمة له");
        exit(-1);
    }
}

//    StmtsNode* else_if()
//    {
//        StmtsNode* block_{};
//        ExprNode* condetion_ = this->expression();
//
//        if (this->currentToken.type_ == TTcolon)
//        {
//            this->advance();
//            block_ = this->block_();
//        }
//        else {
//            PRINT_(L"لم يتم إنهاء واذا بنقطتين \:");
//            exit(-1);
//        }
//
//        level--;
//        (stmtsNode + level)->type_ = VElseIf;
//        (stmtsNode + level)->U.If.condetion_ = condetion_;
//        (stmtsNode + level)->U.If.block_ = block_;
//        return (stmtsNode + level);
//    }

StmtsNode* Parser::else_() {

    if (this->currentToken.type_ == TTColon)
    {
        this->advance();
        return this->block_();
    }
    else {
        PRINT_(L"لم يتم إنهاء والا بنقطتين \:");
        exit(-1);
    }
}

//    StmtsNode* if_statement()
//    {
//
//        StmtsNode* block_{};
//        std::vector<StmtsNode*>* elseIf = new std::vector<StmtsNode*>;
//        StmtsNode* else_{};
//        ExprNode* condetion_ = this->expression();
//
//        if (this->currentToken.type_ == TTcolon)
//        {
//            this->advance();
//            block_ = this->block_();
//        }
//        else {
//            PRINT_(L"لم يتم إنهاء اذا بنقطتين \:");
//            exit(-1);
//        }
//        while (this->currentToken.val.keywordType == Elseif)
//        {
//            this->advance();
//            elseIf->push_back(this->else_if());
//        }
//        if (this->currentToken.val.keywordType == Else)
//        {
//            this->advance();
//            else_ = this->else_();
//        }
//
//        level--;
//        (stmtsNode + level)->type_ = VIf;
//        (stmtsNode + level)->U.If.condetion_ = condetion_;
//        (stmtsNode + level)->U.If.block_ = block_;
//        (stmtsNode + level)->U.If.elseIf = elseIf;
//        (stmtsNode + level)->U.If.else_ = else_;
//        return (stmtsNode + level);
//
//    }

StmtsNode* Parser::block_()
{
    if (this->currentToken.type_ == TTNewline)
    {
        this->advance();

        if (this->currentToken.type_ == TTIndent)
        {
            this->advance();

            StmtsNode* stmts_ = this->statements();

            if (this->currentToken.type_ == TTDedent or this->currentToken.type_ == TTEndOfFile)
            {
                this->advance();
                return stmts_;
            }
            else if (this->currentToken.type_ == TTIndent) {
                PRINT_(L"يتوقع وجود مسافة راجعة في نهاية الحالة المركبة");
                exit(-1);
            }
        }
        else {
            PRINT_(L"يتوقع وجود مسافة بادئة في بداية جسم الحالة المركبة");
            exit(-1);
        }

    }
}

//    ////void import_from() {
//    ////}
//
//    ////void import_name() {
//    ////}
//
//    ////void import_statement() {
//    ////}
//
//    ////void delete_statement() {
//    ////}


StmtsNode* Parser::compound_statement()
{
    if (!wcscmp(this->currentToken.value_, L"لاجل"))
    {
        this->advance();
        return this->for_statement();
    }
    //else if (!wcscmp(this->currentToken.value_, L"دالة"))
    //{
    //    this->advance();
    //    return this->function_def();
    //}
    //else if (!wcscmp(this->currentToken.value_, L"اذا"))
    //{
    //    this->advance();
    //    return this->if_statement();
    //}
    //else if (!wcscmp(this->currentToken.value_, L"بينما"))
    //{
    //    this->advance();
    //    return this->while_statement();
    //}
    //else if (!wcscmp(this->currentToken.value_, L"صنف"))
    //{
    //    this->advance();
    //    return this->class_def();
    //}
    //if (!wcscmp(this->currentToken.value_, L"ارجع"))
    //{
    //    if (returnFlag)
    //    {
    //        this->advance();
    //        return this->return_statement();
    //    }
    //    else
    //    {
    //        PRINT_(L"لا يمكن إستدعاء ارجع من خارج دالة");
    //        exit(-1);
    //    }
    //}
}

ExprNode* Parser::simple_statement()
{
    return this->assignment();
}

StmtsNode* Parser::statement() 
{

    if (!wcscmp(this->currentToken.value_, L"دالة") or
        !wcscmp(this->currentToken.value_, L"اذا") or
        !wcscmp(this->currentToken.value_, L"صنف") or
        !wcscmp(this->currentToken.value_, L"لاجل") or
        !wcscmp(this->currentToken.value_, L"بينما") or
        !wcscmp(this->currentToken.value_, L"ارجع"))
        {
            return this->compound_statement();
    }
    else
    {
        ExprNode* exprNode = this->simple_statement();

        if (this->currentToken.type_ != TTNewline) {
            PRINT_(L"لا يمكن وجود اكثر من حالة في نفس السطر");
            exit(-1);
        }
        this->advance();

        StmtsNode* stmtsNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));
        
        stmtsNode->type_ = VTExpr;
        stmtsNode->U.Expr.expr_ = exprNode;
        return stmtsNode;
    }
}

StmtsNode* Parser::statements() {

    std::vector<StmtsNode*>* statements_ = new std::vector<StmtsNode*>;

    while (this->currentToken.type_ != TTDedent and this->currentToken.type_ != TTEndOfFile)
    {
        if (this->currentToken.type_ == TTIndent)
        {
            PRINT_(L"خطأ في المسافات البادئة - لقد خرجت عن النطاق الحالي");
            exit(-1);
        }
        statements_->push_back(this->statement());

    }
    StmtsNode* stmtsNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));

    stmtsNode->type_ = VTStmts;
    stmtsNode->U.Stmts.stmts_ = statements_;
    return stmtsNode;

}





bool Parser::is_keyword(const wchar_t* _name)
{
    if (wcslen(_name) > 6) return false; // في حال كان الاسم اكبر من 6 احرف اذا هو ليس كلمة مفتاحية
    for (const wchar_t* keyword : keywordsList)
    {
        if (!wcscmp(_name, keyword))
        {
            return true;
        }
    }
    return false; // 
}