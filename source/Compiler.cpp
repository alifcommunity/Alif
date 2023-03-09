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

void Compiler::expr_visit(ExprNode* _node)
{
	if (_node->type_ == VTObject)
	{
		Instructions* sendData = new Instructions();
		sendData->instruction_ = SendObj;
		sendData->data_ = &_node->U.Object.value_;
		instructions_.push_back(sendData);
	}
	else if (_node->type_ == VTBinOp)
	{

	}
}

void Compiler::stmts_visit(StmtsNode* _node)
{

}