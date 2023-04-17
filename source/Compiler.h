#pragma once

#include "Node.h"
#include "Types.h"
#include "SymbolTable.h"
#include "AlifMemory.h"
#include "Container.h"
#include "AlifArray.h"

#define VISIT_(func,node) (visit_ ## func(node)) // -> visit_func(arg) <<-->> VISIT_(func, node)
												 //			^                        ^
												 //			|                        |
												 //			|                        |
												 //    طريقة الاستدعاء           شكل الاستدعاء


static SymbolTable symTable; // تم تعريفه ك متغير عام لمنع حذف المتغيرات عند استخدام الطرفية بعد الانتقال الى سطر جديد


class Compiler {
public:
	std::vector<StmtsNode*>* statements_{};
	//std::vector<InstructionsType> instructions_{};
	//std::vector<AlifObject*> data_{}; // لماذا لا تكون البيانات هي ذاكرة المكدس ويتم إرسال عنوانها بدل نقل البيانات مرة اخرى؟
	Container* dataContainer{};
	AlifArray<Container*> containers_{};
	AlifMemory* alifMemory;

	Compiler(std::vector<StmtsNode*>* _statements, AlifMemory* _alifMemory);

	void compile_file();

	AlifObject* visit_object(ExprNode*);
	AlifObject* visit_unaryOp(ExprNode*);
	AlifObject* visit_binOp(ExprNode*);
	void visit_assign(ExprNode*);
	void visit_augAssign(ExprNode*);
	AlifObject* visit_access(ExprNode*);
	void visit_expr(ExprNode*);
	void visit_list(ExprNode*);

	void visit_for_(StmtsNode*);
	void visit_while_(StmtsNode*);
	void visit_if_(StmtsNode*);
	void visit_function(StmtsNode*);

	AlifObject* visit_exprs(ExprNode* _node);
	AlifObject* visit_stmts(StmtsNode* _node);
};


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

