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
		instructions_.push_back(SendObj);
		data_.push_back(& _node->U.Object.value_);
		return &_node->U.Object.value_;
	}
	else if (_node->type_ == VTBinOp)
	{
		ObjectType left = this->expr_visit(_node->U.BinaryOp.left_)->objType;
		ObjectType right = this->expr_visit(_node->U.BinaryOp.right_)->objType;

		if (_node->U.BinaryOp.operator_ == TTPlus)
		{
			if (left == OTNumber or right == OTNumber)
			{
				instructions_.push_back(SumNumbers);
			}
		}
	}
}

AlifObject* Compiler::stmts_visit(StmtsNode* _node)
{
	return nullptr;
}