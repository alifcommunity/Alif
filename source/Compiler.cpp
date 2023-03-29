#include "Compiler.h"


Compiler::Compiler(std::vector<ExprNode*>* _statements) :
	statements_(_statements) {}

void Compiler::compile_file() 
{
	for (ExprNode* node_ : *statements_)
	{
		VISIT_(exprs, node_); // تم شرح طريقة الاستدعاء في ملف compiler.h
	}
}


AlifObject* Compiler::visit_object(ExprNode* _node)
{
	instructions_.push_back(SET_DATA);
	data_.push_back(_node->U.Object.value_);
	return _node->U.Object.value_;
}


void Compiler::visit_unaryOp(ExprNode* _node)
{
	AlifObject* right = VISIT_(exprs, _node->U.UnaryOp.right_);

	if (!wcscmp(_node->U.UnaryOp.keyword_, L"ليس"))
	{
		instructions_.push_back(NOT_LOGIC);
	}
}


AlifObject* Compiler::visit_binOp(ExprNode* _node)
{
	AlifObject* right = VISIT_(exprs, _node->U.BinaryOp.right_);
	AlifObject* left = VISIT_(exprs, _node->U.BinaryOp.left_);

	if (_node->U.BinaryOp.operator_ == TTPlus)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(ADD_NUM);
			}
			else
			{
				// TypeError: int and other
			}
		}
		else if (left->objType == OTString)
		{
			if (right->objType == OTString)
			{
				instructions_.push_back(ADD_STR);
			}
			else
			{
				// TypeError: str and other
			}
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTMinus)
	{
		instructions_.push_back(MINUS_NUM);
	}
	else if (_node->U.BinaryOp.operator_ == TTPower)
	{
		instructions_.push_back(POW_NUM);
	}
	else if (_node->U.BinaryOp.operator_ == TTDivide)
	{
		instructions_.push_back(DIV_NUM);
	}
	else if (_node->U.BinaryOp.operator_ == TTMultiply)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(MUL_NUM);
			}
			else if (right->objType == OTString)
			{
				instructions_.push_back(MUL_STR);
			}
			else {
				// error
			}
		}
		else if (left->objType == OTString)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(MUL_STR);
			}
		}
		else
		{
			// error
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTRemain)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(REM_NUM);
			}
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTEqualEqual)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(EQEQ_NUM);
			}
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTNotEqual)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(NOTEQ_NUM);
			}
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTGreaterThan)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(GRTHAN_NUM);
			}
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTLessThan)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(LSTHAN_NUM);
			}
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTGreaterThanEqual)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(GRTHANEQ_NUM);
			}
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTLessThanEqual)
	{
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
			{
				instructions_.push_back(LSTHANEQ_NUM);
			}
		}
	}
	else if (!wcscmp(_node->U.UnaryOp.keyword_, L"و"))
	{
		instructions_.push_back(AND_LOGIC);
	}
	else if (!wcscmp(_node->U.UnaryOp.keyword_, L"او"))
	{
		instructions_.push_back(OR_LOGIC);
	}

	return left;
}


void Compiler::visit_assign(ExprNode* _node)
{
	for (AlifObject* i : *_node->U.NameAssign.name_)
	{
		VISIT_(exprs,_node->U.NameAssign.value_);

		data_.push_back(i);
		instructions_.push_back(SET_DATA);

		instructions_.push_back(STORE_NAME);
	}
}


void Compiler::visit_access(ExprNode* _node)
{
	data_.push_back(_node->U.NameAccess.name_);
	instructions_.push_back(SET_DATA);

	instructions_.push_back(GET_DATA);
}


void Compiler::visit_expr(ExprNode* _node)
{
	VISIT_(exprs, _node->U.Expr.expr_);
	VISIT_(exprs, _node->U.Expr.elseExpr);
	VISIT_(exprs, _node->U.Expr.condetion_);

	instructions_.push_back(EXPR_OP);
}


AlifObject* Compiler::visit_exprs(ExprNode* _node)
{
	if (_node->type_ == VTObject)
	{
		return VISIT_(object, _node);
	}
	else if (_node->type_ == VTUnaryOp)
	{
		VISIT_(unaryOp, _node);
	}
	else if (_node->type_ == VTBinOp)
	{
		return VISIT_(binOp,_node);
	}
	else if (_node->type_ == VTAssign)
    {
		VISIT_(assign,_node);
    }
	else if (_node->type_ == VTAccess)
	{
		VISIT_(access, _node);
	}
	else if (_node->type_ == VTExpr)
    {
		VISIT_(expr, _node);
    }
}

AlifObject* Compiler::visit_stmts(StmtsNode* _node)
{
	return nullptr;
}