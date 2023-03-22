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
	std::wcout << res.V.NumberObj.numberValue << std::endl;
	//std::wcout << res->V.StringObj.strValue << std::endl;
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
void minus_num() {}
void mul_num() {}
void div_num() {}
void rem_num() {}
void pow_num() {}

void add_str() // هذه الطريقة اسرع من استخدام wcsncpy_s و wcsncat_s
{
	// stackLevel++;
	//AlifObject* left = (memory_ + stackLevel);
	// stackLevel++;
	//AlifObject* right = (memory_ + stackLevel);


	//const uint16_t rightSize = wcslen(right->V.StringObj.strValue);
	//const uint16_t leftSize = wcslen(left->V.StringObj.strValue);
	//wchar_t* res = new wchar_t[rightSize + leftSize + 1];

	//for (uint16_t i = 0; i < rightSize; i++)
	//{
	//	res[i] = right->V.StringObj.strValue[i];
	//}
	//for (uint16_t i = 0; i < leftSize; i++)
	//{
	//	res[i + rightSize] = left->V.StringObj.strValue[i];
	//}	

	//res[rightSize + leftSize] = L'\0';
	//
	//right->V.StringObj.strValue = res;
	//*(memory_ + stackLevel) = *right;
	//stackLevel--;
}

