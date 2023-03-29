#pragma once

#include "Node.h"
#include "Types.h"
#include "SymbolTable.h"

#define VISIT_(func,node) (visit_ ## func(node)) // -> visit_func(arg) <<-->> VISIT_(func, node)
												 //			^                        ^
												 //			|                        |
												 //			|                        |
												 //    طريقة الاستدعاء           شكل الاستدعاء


static SymbolTable symTable; // تم تعريفه ك متغير عام لمنع حذف المتغيرات عند استخدام الطرفية بعد الانتقال الى سطر جديد


class Compiler {
public:
	std::vector<ExprNode*>* statements_{};
	std::vector<InstructionsType> instructions_{};
	std::vector<AlifObject*> data_{}; // لماذا لا تكون البيانات هي ذاكرة المكدس ويتم إرسال عنوانها بدل نقل البيانات مرة اخرى؟

	Compiler(std::vector<ExprNode*>* _statements);

	void compile_file();

	AlifObject* visit_object(ExprNode*);
	AlifObject* visit_unaryOp(ExprNode*);
	AlifObject* visit_binOp(ExprNode*);
	void visit_assign(ExprNode*);
	void visit_access(ExprNode*);
	void visit_expr(ExprNode*);

	AlifObject* visit_exprs(ExprNode* _node);
	AlifObject* visit_stmts(StmtsNode* _node);
};

/*
	
	يتم تخزين الاوامر في مصفوفة الاوامر

	في كل نطاق يتم التخلص من البيانات الموجودة في المكدس لانها إما اسندت او طبعت وبالتالي انتها العمل منها
	ويقصد بالنطاق الحالة الجديدة
*/

//std::map<NUM, AlifObject> namesTable;
//std::map<BuildInFuncType, AlifObject(Parser::*)(ExprNode*)> buildInFuncsTable{ {Print, &Parser::print} , {Input, &Parser::input} };
//std::map<NUM, StmtsNode*> functionsTable;
//
//AlifObject visit_stmts(StmtsNode* _node)
//{
//    if (_node->type_ == VExpr)
//    {
//        return this->visit_expr(_node->U.Expr.expr_);
//    }
//    else if (_node->type_ == VFunction)
//    {
//        if (_node->U.FunctionDef.name.type_ != TTbuildInFunc) {
//            functionsTable[_node->U.FunctionDef.name.A.Name.name_] = _node;
//        }
//        else {
//            buildInFuncsTable.erase(_node->U.FunctionDef.name.A.BuildInFunc.buildInFunc);
//            functionsTable[_node->U.FunctionDef.name.A.BuildInFunc.buildInFunc] = _node;
//        }
//    }
//    else if (_node->type_ == VClass)
//    {
//
//    }
//    else if (_node->type_ == VFor)
//    {
//        NUM itrName = _node->U.For.itrName.A.Name.name_;
//
//        NUM startVal = 0;
//        NUM endVal;
//        NUM stepVal = 1;
//
//        if (_node->U.For.args_->size() == 3)
//        {
//            startVal = this->visit_expr(_node->U.For.args_->at(0)).A.Number.value_;
//            endVal = this->visit_expr(_node->U.For.args_->at(1)).A.Number.value_;
//            stepVal = this->visit_expr(_node->U.For.args_->at(2)).A.Number.value_;
//        }
//        else if (_node->U.For.args_->size() == 2)
//        {
//            startVal = this->visit_expr(_node->U.For.args_->at(0)).A.Number.value_;
//            endVal = this->visit_expr(_node->U.For.args_->at(1)).A.Number.value_;
//        }
//        else
//        {
//            endVal = this->visit_expr(_node->U.For.args_->at(0)).A.Number.value_;
//        }
//
//        //namesTable[itrName] = _node->U.For.args_->at(0);
//        symTable.add_symbol(itrName, this->visit_expr(_node->U.For.args_->at(0)));
//
//
//        AlifObject result{};
//        for (NUM i = startVal; i < endVal; i += stepVal)
//        {
//            if (returnFlag)
//            {
//                return result;
//            }
//
//            //namesTable[itrName].A.Number.value_ = i;
//            symTable.add_value(itrName, i);
//            result = this->visit_stmts(_node->U.For.block_);
//
//        }
//        if (_node->U.For.else_ != nullptr)
//        {
//            this->visit_stmts(_node->U.For.else_);
//        }
//
//    }
//    else if (_node->type_ == VWhile)
//    {
//        AlifObject result{};
//        while (this->visit_expr(_node->U.While.condetion_).A.Boolean.value_)
//        {
//            if (returnFlag)
//            {
//                return result;
//            }
//            result = this->visit_stmts(_node->U.While.block_);
//        }
//        if (_node->U.While.else_ != nullptr)
//        {
//            this->visit_stmts(_node->U.While.else_);
//        }
//        return _node->U.While.condetion_->U.Object.value_;
//    }
//    else if (_node->type_ == VIf)
//    {
//
//        if (this->visit_expr(_node->U.If.condetion_).A.Boolean.value_)
//        {
//            return this->visit_stmts(_node->U.If.block_);
//        }
//        else if (_node->U.If.elseIf != nullptr)
//        {
//            for (StmtsNode* elseIfs : *_node->U.If.elseIf)
//            {
//                if (this->visit_expr(elseIfs->U.If.condetion_).A.Boolean.value_)
//                {
//                    return this->visit_stmts(elseIfs->U.If.block_);
//                }
//            }
//
//        }
//        if (_node->U.If.else_ != nullptr) {
//            return this->visit_stmts(_node->U.If.else_);
//        }
//
//    }
//    else if (_node->type_ == VStmts)
//    {
//        AlifObject result{};
//        for (StmtsNode* stmt_ : *_node->U.Stmts.stmts_)
//        {
//            result = this->visit_stmts(stmt_);
//            if (returnFlag) {
//                return result;
//            }
//        }
//
//    }
//    else if (_node->type_ == VReturn) {
//
//        returnFlag = true;
//        if (_node->U.Return.returnExpr != nullptr)
//        {
//            return visit_expr(_node->U.Return.returnExpr);
//        }
//        else {
//            AlifObject nullObj{};
//            nullObj.type_ = TTnone;
//            nullObj.A.None.kind_ = None;
//            return nullObj;
//        }
//    }
//}




//AlifObject visit_expr(ExprNode* _node)
//{
//
//    if (_node->type_ == VObject)
//    {
//        return _node->U.Object.value_;
//    }
//    else if (_node->type_ == VList)
//    {
//        _node->U.Object.value_.A.List.objList = new std::vector<AlifObject>;
//        for (ExprNode* obj : *_node->U.Object.value_.A.List.list_)
//        {
//            _node->U.Object.value_.A.List.objList->push_back(this->visit_expr(obj));
//        }
//        return _node->U.Object.value_;
//    }
//    else if (_node->type_ == VUnaryOp)
//    {
//        AlifObject right = this->visit_expr(_node->U.UnaryOp.right_);
//
//        if (_node->U.UnaryOp.operator_ != TTkeyword)
//        {
//            if (_node->U.UnaryOp.operator_ == TTplus)
//            {
//                return right;
//            }
//            else if (_node->U.UnaryOp.operator_ == TTminus)
//            {
//                right.A.Number.value_ = -right.A.Number.value_;
//            }
//        }
//        else
//        {
//            if (_node->U.UnaryOp.keyword_ == Not)
//            {
//                right.A.Boolean.not_();
//                right.type_ = TTnumber;
//            }
//        }
//        return right;
//    }
//    else if (_node->type_ == VBinOp)
//    {
//        AlifObject right = this->visit_expr(_node->U.BinaryOp.right_);
//        AlifObject left = this->visit_expr(_node->U.BinaryOp.left_);
//
//        if (_node->U.BinaryOp.operator_ != TTkeyword)
//        {
//            if (_node->U.BinaryOp.operator_ == TTplus)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.add_(&right);
//                }
//                else if (left.type_ == TTstring)
//                {
//                    left.A.String.add_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTminus)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.sub_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTmultiply)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.mul_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTdivide)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.div_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTremain)
//            {
//                if (left.type_ == TTnumber and left.A.Number.Tkind_ == TTinteger)
//                {
//                    left.A.Number.rem_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTpower)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.pow_(&right);
//                }
//            }
//
//            else if (_node->U.BinaryOp.operator_ == TTequalEqual)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.equalE_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTnotEqual)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.notE_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTgreaterThan)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.greaterT_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTlessThan)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.lessT_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTgreaterThanEqual)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.greaterTE_(&right);
//                }
//            }
//            else if (_node->U.BinaryOp.operator_ == TTlessThanEqual)
//            {
//                if (left.type_ == TTnumber)
//                {
//                    left.A.Number.lessTE_(&right);
//                }
//            }
//        }
//        else
//        {
//            if (_node->U.BinaryOp.keyword_ == Or)
//            {
//                left.A.Boolean.or_(&right);
//                left.type_ = TTnumber;
//            }
//            else if (_node->U.BinaryOp.keyword_ == And)
//            {
//                left.A.Boolean.and_(&right);
//                left.type_ = TTnumber;
//            }
//        }
//
//        return left;
//    }
//    else if (_node->type_ == VExpr)
//    {
//        AlifObject expr_ = this->visit_expr(_node->U.Expr.expr_);
//        if (_node->U.Expr.condetion_ != nullptr)
//        {
//            AlifObject condetion_ = this->visit_expr(_node->U.Expr.condetion_);
//            if (condetion_.A.Boolean.value_ != 0)
//            {
//                return expr_;
//            }
//            else
//            {
//                return this->visit_expr(_node->U.Expr.elseExpr);
//            }
//        }
//        return expr_;
//    }
//    else if (_node->type_ == VAssign)
//    {
//        if (_node->U.NameAssign.name_->size() < 2)
//        {
//            //namesTable[_node->U.NameAssign.name_->front().A.Name.name_] = this->visit_expr(_node->U.NameAssign.value_);
//            symTable.add_symbol(_node->U.NameAssign.name_->front().A.Name.name_, this->visit_expr(_node->U.NameAssign.value_));
//        }
//        else
//        {
//            for (AlifObject i : *_node->U.NameAssign.name_)
//            {
//                //namesTable[i.A.Name.name_] = this->visit_expr(_node->U.NameAssign.value_);
//                symTable.add_symbol(_node->U.NameAssign.name_->front().A.Name.name_, this->visit_expr(_node->U.NameAssign.value_));
//            }
//
//        }
//    }
//    else if (_node->type_ == VAccess)
//    {
//        //return namesTable[_node->U.NameAccess.name_.A.Name.name_];
//        return symTable.get_data(_node->U.NameAccess.name_.A.Name.name_);
//    }
//    else if (_node->type_ == VCall) {
//
//        if (_node->U.Call.name->U.NameAccess.name_.type_ == TTbuildInFunc)
//        {
//            if (buildInFuncsTable.count(_node->U.Call.name->U.NameAccess.name_.A.BuildInFunc.buildInFunc))
//            {
//                return (this->*buildInFuncsTable[_node->U.Call.name->U.NameAccess.name_.A.BuildInFunc.buildInFunc])(_node);
//            }
//            else {
//                StmtsNode* func = functionsTable[_node->U.Call.name->U.NameAccess.name_.A.Name.name_];
//
//                symTable.enter_scope(_node->U.Call.name->U.NameAccess.name_.A.Name.name_);
//
//                if (func->U.FunctionDef.params != nullptr)
//                {
//                    int argLength = 0;
//                    if (_node->U.Call.args)
//                    {
//                        argLength = _node->U.Call.args->size();
//
//                    }
//
//                    int i = 0;
//                    for (ExprNode* param : *func->U.FunctionDef.params)
//                    {
//                        if (param->type_ == VAccess) {
//                            if (argLength > i) // التحقق ما إذا كان عدد الوسيطات الممررة يكفي لعدد المعاملات في الدالة ام لا
//                            {
//                                symTable.add_symbol(param->U.NameAccess.name_.A.Name.name_, this->visit_expr(_node->U.Call.args->at(i)));
//                            }
//                            else {
//                                prnt(L"لم يتم تمرير عدد كاف من القيم في الدالة");
//                                exit(-1);
//                            }
//                            //namesTable[param->U.NameAccess.name_->A.Name.name_] = this->visit_expr(_node->U.Call.args->at(i));
//
//                        }
//                        else if (argLength == (i + 1)) {
//
//                            //namesTable[param->U.NameAssign.name_->at(0)->A.Name.name_] = this->visit_expr(_node->U.Call.args->at((argLength - 1)));
//                            symTable.add_symbol(param->U.NameAssign.paramName.A.Name.name_, this->visit_expr(_node->U.Call.args->at((argLength - 1))));
//
//                        }
//                        else {
//
//                            //namesTable[param->U.NameAssign.name_->at(0)->A.Name.name_] = this->visit_expr(param->U.NameAssign.value_);
//                            symTable.add_symbol(param->U.NameAssign.paramName.A.Name.name_, this->visit_expr(param->U.NameAssign.value_));
//
//                        }
//                        i++;
//                    }
//                }
//                AlifObject res = visit_stmts(func->U.FunctionDef.body);
//                symTable.exit_scope();
//                return res;
//            }
//        }
//        else {
//            StmtsNode* func = functionsTable[_node->U.Call.name->U.NameAccess.name_.A.Name.name_];
//
//            symTable.enter_scope(_node->U.Call.name->U.NameAccess.name_.A.Name.name_);
//
//            if (func->U.FunctionDef.params != nullptr)
//            {
//                int argLength = 0;
//                if (_node->U.Call.args)
//                {
//                    argLength = _node->U.Call.args->size();
//
//                }
//
//                int i = 0;
//                for (ExprNode* param : *func->U.FunctionDef.params)
//                {
//                    if (param->type_ == VAccess) {
//                        if (argLength > i) // التحقق ما إذا كان عدد الوسيطات الممررة يكفي لعدد المعاملات في الدالة ام لا
//                        {
//                            symTable.add_symbol(param->U.NameAccess.name_.A.Name.name_, this->visit_expr(_node->U.Call.args->at(i)));
//                        }
//                        else {
//                            prnt(L"لم يتم تمرير عدد كاف من القيم في الدالة");
//                            exit(-1);
//                        }
//                        //namesTable[param->U.NameAccess.name_->A.Name.name_] = this->visit_expr(_node->U.Call.args->at(i));
//
//                    }
//                    else if (argLength == (i + 1)) {
//
//                        //namesTable[param->U.NameAssign.name_->at(0)->A.Name.name_] = this->visit_expr(_node->U.Call.args->at((argLength - 1)));
//                        symTable.add_symbol(param->U.NameAssign.paramName.A.Name.name_, this->visit_expr(_node->U.Call.args->at((argLength - 1))));
//
//                    }
//                    else {
//
//                        //namesTable[param->U.NameAssign.name_->at(0)->A.Name.name_] = this->visit_expr(param->U.NameAssign.value_);
//                        symTable.add_symbol(param->U.NameAssign.paramName.A.Name.name_, this->visit_expr(param->U.NameAssign.value_));
//
//                    }
//                    i++;
//                }
//            }
//            AlifObject res = visit_stmts(func->U.FunctionDef.body);
//            returnFlag = false;
//            symTable.exit_scope();
//            return res;
//        }
//
//    }
//    else if (_node->type_ == VAugAssign)
//    {
//        AlifObject value = this->visit_expr(_node->U.AugNameAssign.value_);
//        AlifObject name = symTable.get_data(_node->U.AugNameAssign.name_.A.Name.name_);
//        //AlifObject name = namesTable[_node->U.AugNameAssign.name_.A.Name.name_];
//
//        if (_node->U.AugNameAssign.operator_ == TTplusEqual)
//        {
//            if (name.type_ == TTnumber)
//            {
//                name.A.Number.add_(&value);
//            }
//            else if (name.type_ == TTstring)
//            {
//                name.A.String.add_(&value);
//            }
//        }
//        else if (_node->U.AugNameAssign.operator_ == TTminusEqual)
//        {
//            if (name.type_ == TTnumber)
//            {
//                name.A.Number.sub_(&value);
//            }
//        }
//        else if (_node->U.AugNameAssign.operator_ == TTmultiplyEqual)
//        {
//            if (name.type_ == TTnumber)
//            {
//                name.A.Number.mul_(&value);
//            }
//        }
//        else if (_node->U.AugNameAssign.operator_ == TTdivideEqual)
//        {
//            if (name.type_ == TTnumber)
//            {
//                name.A.Number.div_(&value);
//            }
//        }
//        else if (_node->U.AugNameAssign.operator_ == TTremainEqual)
//        {
//            if (name.type_ == TTnumber)
//            {
//                name.A.Number.rem_(&value);
//            }
//        }
//        symTable.add_symbol(_node->U.AugNameAssign.name_.A.Name.name_, name);
//        //namesTable[_node->U.AugNameAssign.name_.A.Name.name_] = name;
//        return name;
//    }
//
//}
//
//AlifObject str{};
//AlifObject input(ExprNode* node) {
//    str.type_ = TTstring;
//    str.A.String.value_ = new std::wstring();
//    std::getline(std::wcin, *str.A.String.value_);
//    return str;
//}
//
//AlifObject print(ExprNode* node) {
//    AlifObject val;
//    for (ExprNode* arg : *node->U.Call.args) {
//        val = this->visit_expr(arg);
//        if (val.type_ == TTnumber) { prnt(val.A.Number.value_); }
//        else if (val.type_ == TTstring) { prnt(*val.A.String.value_); }
//        else if (val.type_ == TTnone) { prnt(L"عدم"); }
//        else if (val.type_ == TTkeyword) { if (val.A.Boolean.value_ == 1) { prnt(L"صح"); } else { prnt(L"خطا"); } }
//        else if (val.type_ == TTlist) {
//            this->list_print(val);
//            prnt(lst);
//        }
//    }
//    return val;
//}
