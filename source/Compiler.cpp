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
			//if (left and left->objType == OTNumber)
			if (left->objType == OTNumber)
			{
				if (right->objType == OTNumber)
				{
					instructions_.push_back(ADD_NUM);
				}
			}
			//else if (left and left->objType == OTString)
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
			}
			else if (left->objType == OTString)
			{
				if (right->objType == OTString)
				{
					instructions_.push_back(MUL_STR);
				}
			}
			else
			{
				// error
			}
		}

		return left;

	}
}

AlifObject* Compiler::stmts_visit(StmtsNode* _node)
{
	return nullptr;
}