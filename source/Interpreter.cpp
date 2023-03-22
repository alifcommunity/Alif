#include "AddrFuncs.h"
#include "Interpreter.h"

std::vector<InstructionsType>* instructions_;
std::vector<AlifObject*>* data_;
std::stack<AlifObject> stackMemory;

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

	
	AlifObject res = stackMemory.top();
	stackMemory.pop();
	//std::wcout << res.V.NumberObj.numberValue << std::endl;
	std::wcout << res.V.StringObj.strValue << std::endl;
}


void none_() {}

void get_data() {}
void set_data()
{
	stackMemory.push(*data_->front());
	data_->erase(data_->begin());
}

void add_num()
{
	AlifObject left = stackMemory.top();
	stackMemory.pop();
	AlifObject right = stackMemory.top();
	stackMemory.pop();

	left.V.NumberObj.numberType == TTFloat or right.V.NumberObj.numberType == TTFloat ? right.V.NumberObj.numberType = TTFloat : right.V.NumberObj.numberType = TTInteger;
	right.V.NumberObj.numberValue = right.V.NumberObj.numberValue + left.V.NumberObj.numberValue;
	stackMemory.push(right);
}
void minus_num() 
{
	AlifObject left = stackMemory.top();
	stackMemory.pop();
	AlifObject right = stackMemory.top();
	stackMemory.pop();

	left.V.NumberObj.numberType == TTFloat or right.V.NumberObj.numberType == TTFloat ? right.V.NumberObj.numberType = TTFloat : right.V.NumberObj.numberType = TTInteger;
	right.V.NumberObj.numberValue = right.V.NumberObj.numberValue - left.V.NumberObj.numberValue;
	stackMemory.push(right);
}
void mul_num() 
{
	AlifObject left = stackMemory.top();
	stackMemory.pop();
	AlifObject right = stackMemory.top();
	stackMemory.pop();

	left.V.NumberObj.numberType == TTFloat or right.V.NumberObj.numberType == TTFloat ? right.V.NumberObj.numberType = TTFloat : right.V.NumberObj.numberType = TTInteger;
	right.V.NumberObj.numberValue = right.V.NumberObj.numberValue * left.V.NumberObj.numberValue;
	stackMemory.push(right);
}
void div_num() 
{
	AlifObject left = stackMemory.top();
	stackMemory.pop();
	AlifObject right = stackMemory.top();
	stackMemory.pop();

	left.V.NumberObj.numberType == TTFloat;
	right.V.NumberObj.numberValue = right.V.NumberObj.numberValue / left.V.NumberObj.numberValue;
	stackMemory.push(right);
}
void rem_num() 
{
	AlifObject left = stackMemory.top();
	stackMemory.pop();
	AlifObject right = stackMemory.top();
	stackMemory.pop();

	left.V.NumberObj.numberType == TTFloat or right.V.NumberObj.numberType == TTFloat ? right.V.NumberObj.numberType = TTFloat : right.V.NumberObj.numberType = TTInteger;
	right.V.NumberObj.numberValue = (int)right.V.NumberObj.numberValue % (int)left.V.NumberObj.numberValue;
	stackMemory.push(right);
}
void pow_num() 
{
	AlifObject left = stackMemory.top();
	stackMemory.pop();
	AlifObject right = stackMemory.top();
	stackMemory.pop();

	left.V.NumberObj.numberType == TTFloat or right.V.NumberObj.numberType == TTFloat ? right.V.NumberObj.numberType = TTFloat : right.V.NumberObj.numberType = TTInteger;
	right.V.NumberObj.numberValue = pow(right.V.NumberObj.numberValue  ,left.V.NumberObj.numberValue);
	stackMemory.push(right);
}

void add_str() // هذه الطريقة اسرع من استخدام wcsncpy_s و wcsncat_s
{
	AlifObject left = stackMemory.top();
	stackMemory.pop();
	AlifObject right = stackMemory.top();
	stackMemory.pop();


	const uint16_t rightSize = wcslen(right.V.StringObj.strValue);
	const uint16_t leftSize = wcslen(left.V.StringObj.strValue);
	wchar_t* res = new wchar_t[rightSize + leftSize + 1];

	for (uint16_t i = 0; i < rightSize; i++)
	{
		res[i] = right.V.StringObj.strValue[i];
	}
	for (uint16_t i = 0; i < leftSize; i++)
	{
		res[i + rightSize] = left.V.StringObj.strValue[i];
	}	

	res[rightSize + leftSize] = L'\0';
	
	right.V.StringObj.strValue = res;
	stackMemory.push(right);

}

void mul_str() {
	
	AlifObject left = stackMemory.top();
	stackMemory.pop();
	AlifObject right = stackMemory.top();
	stackMemory.pop();

	if (left.objType == OTNumber) {
		const uint16_t rightSize = wcslen(right.V.StringObj.strValue);
		const uint16_t leftSize = left.V.NumberObj.numberValue;
		wchar_t* res = new wchar_t[rightSize * leftSize + 1];

		for (uint16_t i = 0; i < leftSize; i++)
		{
			for (uint16_t l = 0; l < rightSize; l++)
			{
				res[i + l] = right.V.StringObj.strValue[l];
			}
		}

		res[rightSize * leftSize] = L'\0';

		right.V.StringObj.strValue = res;
		stackMemory.push(right);
	}
	else {
		const uint16_t rightSize = right.V.NumberObj.numberValue;
		const uint16_t leftSize = wcslen(left.V.StringObj.strValue);
		wchar_t* res = new wchar_t[rightSize * leftSize + 1];

		for (uint16_t i = 0; i < rightSize; i++)
		{
			for (uint16_t l = 0; l < leftSize; l++)
			{
				res[i + l] = left.V.StringObj.strValue[l];
			}
		}

		res[rightSize * leftSize] = L'\0';

		right.V.StringObj.strValue = res;
		stackMemory.push(right);
	}

}