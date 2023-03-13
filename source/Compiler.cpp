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
		instructions_.push_back(SEND_MEM);
		data_.push_back(& _node->U.Object.value_);
		return nullptr;
	}
	else if (_node->type_ == VTBinOp)
	{
		this->expr_visit(_node->U.BinaryOp.left_)->objType;
		this->expr_visit(_node->U.BinaryOp.right_)->objType;

		if (_node->U.BinaryOp.operator_ == TTPlus)
		{
			instructions_.push_back(ADD_OP);
		}
		else if (_node->U.BinaryOp.operator_ == TTMinus)
		{
			instructions_.push_back(MINUS_OP);
		}
	}
}

AlifObject* Compiler::stmts_visit(StmtsNode* _node)
{
	return nullptr;
}