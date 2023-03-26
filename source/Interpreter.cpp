#include "AddrFuncs.h"
#include "Interpreter.h"

std::vector<InstructionsType>* instructions_;
std::vector<AlifObject*>* data_;
std::stack<AlifObject*> stackMemory;

Interpreter::Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data) {
	data_ = _data;
	instructions_ = _instructions;
}

void Interpreter::run_code()
{
	for (InstructionsType command_ : *instructions_)
	{
		instr_funcs[command_]();
	}

	
	//AlifObject* res = stackMemory.top();
	//stackMemory.pop();
	//std::wcout << res->V.NumberObj.numberValue << std::endl;
	//std::wcout << symTable.get_data(name_->V.NameObj.name_)->V.NumberObj.numberValue << std::endl;
	//std::wcout << res.V.BoolObj.boolType << std::endl;
	//std::wcout << res.V.StringObj.strValue << std::endl;
}


void none_() {}

void get_data() 
{
	AlifObject* name_ = stackMemory.top();
	stackMemory.pop();
	stackMemory.push(symTable.get_data(*name_->V.NameObj.name_));

	AlifObject* res = stackMemory.top();
	stackMemory.pop();
	std::wcout << res->V.NumberObj.numberValue << std::endl;
}
void set_data()
{
	stackMemory.push(data_->front());
	data_->erase(data_->begin());
}

void add_num()
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue = right->V.NumberObj.numberValue + left->V.NumberObj.numberValue;
	stackMemory.push(right);
}
void minus_num() 
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue = right->V.NumberObj.numberValue - left->V.NumberObj.numberValue;
	stackMemory.push(right);
}
void mul_num() 
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue = right->V.NumberObj.numberValue * left->V.NumberObj.numberValue;
	stackMemory.push(right);
}
void div_num() 
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	left->V.NumberObj.numberType == TTFloat;
	right->V.NumberObj.numberValue = right->V.NumberObj.numberValue / left->V.NumberObj.numberValue;
	stackMemory.push(right);
}
void rem_num() 
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue = (int)right->V.NumberObj.numberValue % (int)left->V.NumberObj.numberValue;
	stackMemory.push(right);
}
void pow_num() 
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue = pow(right->V.NumberObj.numberValue  ,left->V.NumberObj.numberValue);
	stackMemory.push(right);
}

void equal_equal()
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = new AlifObject();

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.NumberObj.numberValue == left->V.NumberObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.NumberObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.NumberObj.numberValue = 0;
		}
		stackMemory.push(res);
	}
	else
	{
		// error
	}

}
void not_equal() 
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = new AlifObject();

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.NumberObj.numberValue != left->V.NumberObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.NumberObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.NumberObj.numberValue = 0;
		}
		stackMemory.push(res);
	}
	else
	{
		// error
	}
}

void gr_than_num()
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = new AlifObject();

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.NumberObj.numberValue > left->V.NumberObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.NumberObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.NumberObj.numberValue = 0;
		}
		stackMemory.push(res);
	}
	else
	{
		// error
	}
}
void gr_than_eq_num()
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = new AlifObject();

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.NumberObj.numberValue >= left->V.NumberObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.NumberObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.NumberObj.numberValue = 0;
		}
		stackMemory.push(res);
	}
	else
	{
		// error
	}
}
void ls_than_num()
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = new AlifObject();

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.NumberObj.numberValue < left->V.NumberObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.NumberObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.NumberObj.numberValue = 0;
		}
		stackMemory.push(res);
	}
	else
	{
		// error
	}
}
void ls_than_eq_num()
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = new AlifObject();

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.NumberObj.numberValue <= left->V.NumberObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.NumberObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.NumberObj.numberValue = 0;
		}
		stackMemory.push(res);
	}
	else
	{
		// error
	}
}

void not_logic() 
{
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	AlifObject* res = new AlifObject();

	res->objType = OTBoolean;
	res->tokLine = right->tokLine;

	if (right->V.NumberObj.numberValue == 0)
	{
		res->V.BoolObj.boolType = L"صح";
		res->V.NumberObj.numberValue = 1;
	}
	else
	{
		res->V.BoolObj.boolType = L"خطا";
		res->V.NumberObj.numberValue = 0;
	}
	stackMemory.push(res);

}
void and_logic()
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = new AlifObject();

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.NumberObj.numberValue and left->V.NumberObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.NumberObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.NumberObj.numberValue = 0;
		}
		stackMemory.push(res);
	}
	else
	{
		// error
	}
}
void or_logic() 
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = new AlifObject();

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.NumberObj.numberValue or left->V.NumberObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.NumberObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.NumberObj.numberValue = 0;
		}
		stackMemory.push(res);
	}
	else
	{
		// error
	}
}

void add_str() // هذه الطريقة اسرع من استخدام wcsncpy_s و wcsncat_s
{
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();


	const uint16_t rightSize = wcslen(right->V.StringObj.strValue);
	const uint16_t leftSize = wcslen(left->V.StringObj.strValue);
	wchar_t* res = new wchar_t[rightSize + leftSize + 1];

	for (uint16_t i = 0; i < rightSize; i++)
	{
		res[i] = right->V.StringObj.strValue[i];
	}
	for (uint16_t i = 0; i < leftSize; i++)
	{
		res[i + rightSize] = left->V.StringObj.strValue[i];
	}	

	res[rightSize + leftSize] = L'\0';
	
	right->V.StringObj.strValue = res;
	stackMemory.push(right);

}

void mul_str() {
	
	AlifObject* left = stackMemory.top();
	stackMemory.pop();
	AlifObject* right = stackMemory.top();
	stackMemory.pop();

	if (left->objType == OTNumber) {
		const uint16_t rightSize = wcslen(right->V.StringObj.strValue);
		const uint16_t leftSize = left->V.NumberObj.numberValue;
		wchar_t* res = new wchar_t[rightSize * leftSize + 1];

		int currentIndex = 0;
		for (uint16_t i = 0; i < rightSize; i++)
		{
			for (uint16_t l = 0; l < leftSize; l++)
			{
				res[currentIndex++] = left->V.StringObj.strValue[l];
			}
		}

		res[rightSize * leftSize] = L'\0';

		right->V.StringObj.strValue = res;
		stackMemory.push(right);
	}
	else {
		const uint16_t rightSize = right->V.NumberObj.numberValue;
		const uint16_t leftSize = wcslen(left->V.StringObj.strValue);
		wchar_t* res = new wchar_t[rightSize * leftSize + 1];

		int currentIndex = 0;
		for (uint16_t i = 0; i < rightSize; i++)
		{
			for (uint16_t l = 0; l < leftSize; l++)
			{
				res[currentIndex++] = left->V.StringObj.strValue[l];
			}
		}

		res[rightSize * leftSize] = L'\0';

		right->V.StringObj.strValue = res;
		stackMemory.push(right);
	}

}


void expr_op() 
{
	AlifObject* compRes = stackMemory.top();
	stackMemory.pop();

	if (compRes->V.NumberObj.numberValue != 0)
	{
		stackMemory.pop();
	}
}


void store_name()
{
	AlifObject* name_ = stackMemory.top();
	stackMemory.pop();
	AlifObject* value_ = stackMemory.top();
	stackMemory.pop();

	symTable.add_symbol(*name_->V.NameObj.name_, value_);

}