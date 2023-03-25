#include "Compiler.h"

Compiler::Compiler(std::vector<ExprNode*>* _statements) :
	statements_(_statements) {}

void Compiler::compile_file() 
{
	for (ExprNode* node_ : *statements_)
	{
		this->expr_visit(node_);
	}

}

AlifObject* Compiler::expr_visit(ExprNode* _node)
{
	if (_node->type_ == VTObject)
	{
		instructions_.push_back(SET_DATA);
		data_.push_back(&_node->U.Object.value_);
		return &_node->U.Object.value_;
	}
	else if (_node->type_ == VTBinOp)
	{
		AlifObject* left = this->expr_visit(_node->U.BinaryOp.left_);
		AlifObject* right = this->expr_visit(_node->U.BinaryOp.right_);

		if (_node->U.BinaryOp.operator_ == TTPlus)
		{
			if (left->objType == OTNumber)
			{
				if (right->objType == OTNumber)
				{
					instructions_.push_back(ADD_NUM);
				}
			}
			else if (left->objType == OTString)
			{
				if (right->objType == OTString)
				{
					instructions_.push_back(ADD_STR);
				}
			}
			else
			{
				// error
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
	else if (_node->type_ == VTUnaryOp)
	{
		AlifObject* right = this->expr_visit(_node->U.UnaryOp.right_);

		if (!wcscmp(_node->U.UnaryOp.keyword_, L"ليس"))
		{
			instructions_.push_back(NOT_LOGIC);
		}
	}
	else if (_node->type_ == VTExpr)
    {
        this->expr_visit(_node->U.Expr.expr_);
		this->expr_visit(_node->U.Expr.elseExpr);
        this->expr_visit(_node->U.Expr.condetion_);

		instructions_.push_back(EXPR_OP);
    }
}

AlifObject* Compiler::stmts_visit(StmtsNode* _node)
{
	return nullptr;
}