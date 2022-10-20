#pragma once

class Interpreter {
public:
	Node* AST;
	int res;

	//Interpreter()
	//{
	//	delete AST;
	//}

	Node* visit(Node* node)
	{
		if (node->type == BinOpNode)
		{
			return this->bin_op_interprete(node);
		}
	}

	Node* bin_op_interprete(Node* node)
	{
		Node* left = this->visit(node->left);
		Node* right = this->visit(node->right);

		if (node->token.type_ == plusT)
		{
			res = stoi(left->token.value_) + stoi(right->token.value_);
			left->token.value_ = std::to_wstring(res);
		}
		return left;
	}

	void Interpreter_print(Node* AST)
	{
		Node* res = visit(AST);
		std::wcout << res->token.value_ << std::endl;
	}
};

