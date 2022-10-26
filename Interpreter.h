#pragma once

class VarTaple {
	std::map<std::wstring, Node*> vars;
	
public:
	void set_(std::wstring key, Node* value)
	{
		vars[key] = value;
	}

	Node* get_(std::wstring key)
	{
		return vars[key];
	}

};

class Interpreter {
public:
	VarTaple* varTaple = new VarTaple;
	Interpreter(VarTaple* varTaple) {
		this->varTaple = varTaple;
	}

	Node* visit(Node* node)
	{
		if (node->type == BinOpNode)
		{
			return this->bin_op_interprete(node);
		}
		else if (node->type == UnaryOpNode)
		{
			return this->unary_op_interprete(node);
		}
		else if (node->type == VarAssignNode)
		{
			this->var_assign(node);
		}
		else if (node->type == VarAccessNode)
		{
			return this->var_accsses(node);
		}
		else if (node->type == BuildInFunctionNode)
		{
			if (node->token.value_ == L"اطبع")
			{
				this->print_interprete(node);
			}
		}
		else if (node->type == InverseNode)
		{
			return this->inverse_(node);
		}
	}

	Node* bin_op_interprete(Node* node)
	{
		Node* left = this->visit(node->left);
		Node* right = this->visit(node->right);
		Node* result = new Node(left, left->token, nullptr, left->type);

		if (node->token.type_ == plusT)
		{
			result->token.value_ = std::to_wstring(stoi(left->token.value_) + stoi(right->token.value_));
		}
		else if (node->token.type_ == minusT)
		{
			result->token.value_ = std::to_wstring(stoi(left->token.value_) - stoi(right->token.value_));
		}
		else if (node->token.type_ == multiplyT)
		{
			result->token.value_ = std::to_wstring(stoi(left->token.value_) * stoi(right->token.value_));
		}
		else if (node->token.type_ == divideT)
		{
			result->token.value_ = std::to_wstring(stoi(left->token.value_) / stoi(right->token.value_));
		}
		return result;
	}

	Node* unary_op_interprete(Node* node)
	{
		Node* right = this->visit(node->right);

		if (node->token.type_ == plusT)
		{
			right->token.value_ = std::to_wstring(stoi(right->token.value_));
		}
		else if (node->token.type_ == minusT)
		{
			right->token.value_ = std::to_wstring(-stoi(right->token.value_));
		}
		return right;
	}

	Node* var_assign(Node* node)
	{
		Node* right = this->visit(node->right);
		(*this->varTaple).set_(node->token.value_, right);
		return (*this->varTaple).get_(node->token.value_);
	}

	Node* var_accsses(Node* node)
	{
		return (*this->varTaple).get_(node->token.value_);
	}

	Node* inverse_(Node* node)
	{
		node->right->token.value_ = L"خطأ";
		return node->right;
	}

	void print_interprete(Node* node)
	{
		Node* result = this->visit(node->right);
		std::wcout << result->token.value_ << std::endl;
	}

	void Interpreter_print(Node* AST)
	{
		Node* res = visit(AST);
		//std::wcout << res->token.value_ << std::endl;
	}
};

