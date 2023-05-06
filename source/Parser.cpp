#include "Parser.h"



Parser::Parser(std::vector<Token>* _tokens, wstr _fileName, wstr* _input, AlifMemory* _alifMemory) :
    tokens_(_tokens), fileName(_fileName), input_(_input), alifMemory(_alifMemory)
{
    // تنقية الرموز من رمز السطر الجديد غير الضروري
    size_t size_ = tokens_->size();
    for (size_t a = 0; a < size_;)
    {
        if (tokens_->at(a).type_ == TTNewline or tokens_->at(a).type_ == TTDedent)
        {
            a++;
            while (a < size_ and tokens_->at(a).type_ == TTNewline)
            {
                tokens_->erase(tokens_->begin() + a);
                size_--;
            }
        }
        else
        {
            a++;
        }
    }
    if (tokens_->begin()->type_ == TTNewline) { tokens_->erase(tokens_->begin()); }
    tokens_->shrink_to_fit();
    ///////////////////////
    
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
    do {
        this->statements_.push_back(this->statement());
    } while (currentToken.type_ != TTEndOfFile);
}

void Parser::parse_terminal()
{
    this->statements_.push_back(this->statement());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ExprNode*>* Parser::arguments() 
{
    std::vector<ExprNode*>* args = new std::vector<ExprNode*>;

    do {
        args->push_back(this->expression_statement());

        if (this->currentToken.type_ == TTComma) 
        {
            this->advance();
        }
    } while (this->currentToken.type_ != TTRrightParenthesis);

    return args;
}

ExprNode* Parser::atom_statement()
{
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
        return this->list_statement();
    }
    else if (this->currentToken.type_ == TTLeftParenthesis)
    {
        this->advance();
        ExprNode* priorExpr = this->expression_statement();

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

ExprNode* Parser::list_statement()
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
            nodeElement->push_back(this->expression_statement());

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

ExprNode* Parser::primary_statement() 
{
    ExprNode* atom = this->atom_statement();
    if (this->currentToken.type_ == TTDot) 
    {
        this->advance();
        ExprNode* primary = this->primary_statement();

        ExprNode* primary_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        primary_->type_ = VTCall;
        primary_->U.Call.name_ = atom;
        primary_->U.Call.func_ = primary;
        primary_->U.Call.args_ = nullptr;
        return primary_;
    }
    else if (this->currentToken.type_ == TTLeftParenthesis) 
    {
        //ExprNode* primary = atom;

        this->advance();

        if (this->currentToken.type_ == TTRrightParenthesis) 
        {
            this->advance();

            if (this->currentToken.type_ == TTDot)
            {
                ExprNode* primary = this->primary_statement();
            }
            
            ExprNode* callNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
            callNode->type_ = VTCall;
            callNode->U.Call.name_ = atom;
            callNode->U.Call.func_ = nullptr;
            callNode->U.Call.args_ = nullptr;
            return callNode;
        }

        std::vector<ExprNode*>* args = this->arguments();

        if (this->currentToken.type_ == TTRrightParenthesis)
        {
            this->advance();

            if (this->currentToken.type_ == TTDot)
            {
                ExprNode* primary = this->primary_statement();
            }

            ExprNode* callNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
            callNode->type_ = VTCall;
            callNode->U.Call.name_ = atom;
            callNode->U.Call.func_ = nullptr;
            callNode->U.Call.args_ = args;
            return callNode;
        }
    }
    else
    {
        return atom;
    }

}

ExprNode* Parser::power_statement()
{
    ExprNode* left_ = this->primary_statement();

    while (this->currentToken.type_ == TTPower) {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->factor_statement();

        ExprNode* power_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        power_->U.BinaryOp.right_ = right_;
        power_->U.BinaryOp.operator_ = opToken.type_;
        power_->U.BinaryOp.left_ = left_;
        power_->type_ = VTBinOp;

        left_ = power_;
    }

    return left_;
}

ExprNode* Parser::factor_statement()
{
    while (this->currentToken.type_ == TTPlus or this->currentToken.type_ == TTMinus) {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->power_statement();

        ExprNode* factor_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        factor_->U.UnaryOp.right_ = right_;
        factor_->U.UnaryOp.operator_ = opToken.type_;
        factor_->type_ = VTUnaryOp;

        return factor_;
    }

    return this->power_statement();
}

ExprNode* Parser::term_statement()
{
    ExprNode* left_ = this->factor_statement();

    while (this->currentToken.type_ == TTMultiply or this->currentToken.type_ == TTDivide or this->currentToken.type_ == TTRemain) {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->factor_statement();

        ExprNode* term_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        term_->U.BinaryOp.right_ = right_;
        term_->U.BinaryOp.operator_ = opToken.type_;
        term_->U.BinaryOp.left_ = left_;
        term_->type_ = VTBinOp;

        left_ = term_;
    }

    return left_;
}

ExprNode* Parser::sum_statement()
{
    ExprNode* left_ = this->term_statement();

    while (this->currentToken.type_ == TTPlus or this->currentToken.type_ == TTMinus)
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->term_statement();

        ExprNode* sum_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        sum_->U.BinaryOp.right_ = right_;
        sum_->U.BinaryOp.operator_ = opToken.type_;
        sum_->U.BinaryOp.left_ = left_;
        sum_->type_ = VTBinOp;

        left_ = sum_;
    }

    return left_;
}

ExprNode* Parser::comparesion_statement()
{
    ExprNode* left_ = this->sum_statement();

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
        ExprNode* right_ = this->sum_statement();

        ExprNode* comparesion_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        comparesion_->U.BinaryOp.right_ = right_;
        comparesion_->U.BinaryOp.operator_ = opToken.type_;
        comparesion_->U.BinaryOp.left_ = left_;
        comparesion_->type_ = VTBinOp;

        left_ = comparesion_;
    }

    return left_;
}

ExprNode* Parser::inversion_statement()
{

    while (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"ليس"))
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->comparesion_statement();

        ExprNode* inversion_ = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

        inversion_->U.UnaryOp.right_ = right_;
        inversion_->U.UnaryOp.operator_ = opToken.type_;
        inversion_->U.UnaryOp.keyword_ = opToken.value_;
        inversion_->type_ = VTUnaryOp;

        return inversion_;
    }

    return this->comparesion_statement();
}

ExprNode* Parser::conjuction_statement()
{

    ExprNode* left_ = this->inversion_statement();

    while (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"و"))
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->inversion_statement();

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

ExprNode* Parser::disjuction_statement()
{

    ExprNode* left_ = this->conjuction_statement();

    while (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"او"))
    {
        Token opToken = this->currentToken;

        this->advance();
        ExprNode* right_ = this->conjuction_statement();

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

ExprNode* Parser::expression_statement()
{

    ExprNode* expr_ = this->disjuction_statement();

    if (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"اذا"))
    {
        this->advance();
        ExprNode* condetion = this->disjuction_statement();

        if (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"والا"))
        {
            this->advance();
            ExprNode* elseExpr = this->expression_statement();

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

ExprNode* Parser::expressions_statement()
{

    ExprNode* expr_ = this->expression_statement();

    if (this->currentToken.type_ == TTComma)
    {
        std::vector<ExprNode*>* exprs_ = new std::vector<ExprNode*>;

        exprs_->push_back(expr_);
        do
        {
            this->advance();
            exprs_->push_back(this->expression_statement());

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


ExprNode* Parser::assignment_statement() // يجب إيجاد خوارزمية افضل بالإضافة الى استخدام ذاكرة ألف بدل من new
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

            ExprNode* expr_ = this->expressions_statement();

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
                    ExprNode* expr_ = this->expression_statement();

                    ExprNode* augAssignment = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

                    augAssignment->U.AugNameAssign.name_ = name_;
                    augAssignment->U.AugNameAssign.operator_ = opToken.type_;
                    augAssignment->U.AugNameAssign.value_ = expr_;
                    augAssignment->type_ = VTAugAssign;

                    return augAssignment;
             }
    }

    return this->expressions_statement();
}

ExprNode* Parser::import_statement()
{
    return nullptr;
}

StmtsNode* Parser::return_statement() {

    ExprNode* expr_;

    if (this->currentToken.type_ != TTNewline)
    {
        expr_ = this->expression_statement();
    }
    else
    {
        expr_ = nullptr;
    }
    this->advance();

    StmtsNode* returnNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));
    returnNode->U.Return.returnExpr = expr_;
    returnNode->type_ = VTReturn;

    return returnNode;
}


std::vector<ExprNode*>* Parser::parameters() 
{
    std::vector<ExprNode*>* params_ = new std::vector<ExprNode*>;
    lastParam = false;

    do 
    {
        if (this->currentToken.type_ != TTRrightParenthesis)
        {
            this->advance();
            if (Next_Is(TTEqual))
            {
                lastParam = true;

                AlifObject* name_ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

                name_->objType = OTName;
                name_->V.NameObj.name_ = this->currentToken.value_;

                this->advance();
                this->advance();

                ExprNode* expr_ = this->expression_statement();

                ExprNode* paramNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));

                paramNode->type_ = VTAssign;
                //paramNode->U.NameAssign.paramName = name_;
                paramNode->U.NameAssign.name_ = new std::vector<AlifObject*>;
                paramNode->U.NameAssign.name_->push_back(name_);
                paramNode->U.NameAssign.value_ = expr_;

                params_->push_back(paramNode);

            }
            else {
                if (!lastParam)
                {
                    params_->push_back(this->atom_statement());
                }
                else {
                    PRINT_(L"لا يمكن تمرير متغير بدون قيمة افتراضية بعد متغير ذو قيمة افتراضية");
                    exit(-1);
                }
            }
        }
        else
        {
            return params_;
        }
    } while (this->currentToken.type_ == TTComma);

    if (this->currentToken.type_ == TTRrightParenthesis)
    {
        return params_;
    }
    else {
        PRINT_(L"لم يتم إغلاق القوس");
        exit(-1);
    }

}

StmtsNode* Parser::function_statement() 
{
    AlifObject* name = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
    StmtsNode* body = nullptr;
    std::vector<ExprNode*>* params = nullptr;

    if (this->currentToken.type_ == TTName) 
    {
        name->objType = OTName;
        name->V.NameObj.name_ = this->currentToken.value_;

        //if (this->currentToken.type_ == TTName)
        //{
        //}
        //else if (this->currentToken.type_ == TTbuildInFunc) {
        //    name.V.BuildInFunc.buildInFunc = this->currentToken.val.buildInFunc;
        //}
        //else
        //{
        //    PRINT_(L"يتوقع وجود اسم للدالة");
        //}

        this->advance();

        if (this->currentToken.type_ == TTLeftParenthesis) 
        {
            if (Next_Is(TTRrightParenthesis))
            {
                this->advance();
                this->advance();
            }
            else {
                params = this->parameters();
                this->advance();
            }
        }
        else
        {
            // error
        }

        if (this->currentToken.type_ == TTColon) {

            this->advance();
            returnFlag = true;
            body = this->block_statement();
            returnFlag = false;

            StmtsNode* funcNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));
            funcNode->type_ = VTFunction;
            funcNode->U.FunctionDef.name_ = name;
            funcNode->U.FunctionDef.params_ = params;
            funcNode->U.FunctionDef.body_ = body;
            return funcNode;
        }
        else {
            PRINT_(L"لم يتم إنهاء دالة بنقطتين \:");
            exit(-1);
        }
    }

}

StmtsNode* Parser::class_statement()
{
    ExprNode* bases_ = nullptr;
    StmtsNode* body_ = nullptr;
    AlifObject* name_ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

    if (this->currentToken.type_ == TTName) {

        name_->objType = OTName;
        name_->V.NameObj.name_ = this->currentToken.value_;

        this->advance();

        if (this->currentToken.type_ == TTLeftParenthesis) {

            this->advance();

            bases_ = this->expressions_statement();

            if (this->currentToken.type_ == TTRrightParenthesis)
            {
                this->advance();
            }
        }
        if (this->currentToken.type_ == TTColon) {

            this->advance();

            body_ = this->block_statement();

            StmtsNode* classNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));
            classNode->type_ = VTClass;
            classNode->U.ClassDef.name_ = name_;
            classNode->U.ClassDef.base_ = bases_;
            classNode->U.ClassDef.body_ = body_;
            return classNode;
        }
    }
}

StmtsNode* Parser::while_statement() 
{
    ExprNode* condetion_ = this->expression_statement();
    StmtsNode* block_ = nullptr;
    //StmtsNode* else_ = nullptr;

    if (this->currentToken.type_ == TTColon)
    {
        this->advance();
        block_ = this->block_statement();
    }
    else {
        PRINT_(L"لم يتم إنهاء بينما بنقطتين \:");
        exit(-1);
    }
    //if (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"والا"))
    //{
    //    this->advance();
    //    else_ = this->else_();
    //}

    StmtsNode* whileNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));
    whileNode->type_ = VTWhile;
    whileNode->U.While.condetion_ = condetion_;
    whileNode->U.While.block_ = block_;
    //whileNode->U.While.else_ = else_;

    return whileNode;
}

StmtsNode* Parser::for_statement()
{
    if (this->currentToken.type_ == TTName)
    {
        AlifObject* itrName = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
        std::vector<ExprNode*>* args_ = new std::vector<ExprNode*>;
        StmtsNode* block_ = nullptr;
        //StmtsNode* else_ = nullptr;

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
                        args_->push_back(this->atom_statement());

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

                            block_ = this->block_statement();

                            //if (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"والا")) // يجب إكملب هذه الحالة في المترجم او إلغائها
                            //{
                            //    this->advance();

                            //    else_ = this->else_();
                            //}
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
        //forNode->U.For.else_ = else_;

        return forNode;
    }
    else {
        PRINT_(L"يتوقع وجود اسم لاسناد قيمة له");
        exit(-1);
    }
}

StmtsNode* Parser::else_if_statement()
{
    StmtsNode* block_{};
    ExprNode* condetion_ = this->expression_statement();

    if (this->currentToken.type_ == TTColon)
    {
        this->advance();
        block_ = this->block_statement();
    }
    else {
        PRINT_(L"لم يتم إنهاء واذا بنقطتين \:");
        exit(-1);
    }

    StmtsNode* elseIfNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));
    elseIfNode->type_ = VTIf;
    elseIfNode->U.If.condetion_ = condetion_;
    elseIfNode->U.If.block_ = block_;
    return elseIfNode;
}

StmtsNode* Parser::else_statement() {

    if (this->currentToken.type_ == TTColon)
    {
        this->advance();
        return this->block_statement();
    }
    else {
        PRINT_(L"لم يتم إنهاء والا بنقطتين \:");
        exit(-1);
    }
}

StmtsNode* Parser::if_statement()
{

    StmtsNode* block_{};
    std::vector<StmtsNode*>* elseIf = new std::vector<StmtsNode*>;
    StmtsNode* else_{};
    ExprNode* condetion_ = this->expression_statement();

    if (this->currentToken.type_ == TTColon)
    {
        this->advance();
        block_ = this->block_statement();
    }
    else {
        PRINT_(L"لم يتم إنهاء اذا بنقطتين \:");
        exit(-1);
    }
    while (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"واذا"))
    {
        this->advance();
        elseIf->push_back(this->else_if_statement());
    }
    if (this->currentToken.type_ == TTName and !wcscmp(this->currentToken.value_, L"والا"))
    {
        this->advance();
        else_ = this->else_statement();
    }

    StmtsNode* ifNode = (StmtsNode*)alifMemory->allocate(sizeof(StmtsNode));
    ifNode->type_ = VTIf;
    ifNode->U.If.condetion_ = condetion_;
    ifNode->U.If.block_ = block_;
    ifNode->U.If.elseIf = elseIf;
    ifNode->U.If.else_ = else_;
    return ifNode;

}

StmtsNode* Parser::block_statement()
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

    return nullptr;
}

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
    else if (!wcscmp(this->currentToken.value_, L"دالة"))
    {
        this->advance();
        return this->function_statement();
    }
    else if (!wcscmp(this->currentToken.value_, L"اذا"))
    {
        this->advance();
        return this->if_statement();
    }
    else if (!wcscmp(this->currentToken.value_, L"بينما"))
    {
        this->advance();
        return this->while_statement();
    }
    else if (!wcscmp(this->currentToken.value_, L"صنف"))
    {
        this->advance();
        return this->class_statement();
    }
    if (!wcscmp(this->currentToken.value_, L"ارجع"))
    {
        if (returnFlag)
        {
            this->advance();
            return this->return_statement();
        }
        else
        {
            PRINT_(L"لا يمكن إستدعاء ارجع من خارج دالة");
            exit(-1);
        }
    }
    return nullptr;
}

ExprNode* Parser::simple_statement()
{
    if (!wcscmp(this->currentToken.value_, L"توقف"))
    {
        ExprNode* stopNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        stopNode->type_ = VTStop;

        this->advance();

        return stopNode;
    }
    if (!wcscmp(this->currentToken.value_, L"استمر"))
    {
        ExprNode* continueNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        continueNode->type_ = VTContinue;

        this->advance();

        return continueNode;
    }
    if (!wcscmp(this->currentToken.value_, L"استورد"))
    {
        ExprNode* continueNode = (ExprNode*)alifMemory->allocate(sizeof(ExprNode));
        continueNode->type_ = VTImport;

        this->advance();

        return this->import_statement();
    }

    return this->assignment_statement();
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