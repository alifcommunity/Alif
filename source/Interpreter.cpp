#include "AddrFuncs.h"
#include "Interpreter.h"


Interpreter::Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data) {
	data_ = _data;
	instructions_ = _instructions;
}

void none_() {}
void get_data() {}

void set_data()
{
	*(memory_ + stackLevel) = data_->front();
	stackLevel--;
	data_->erase(data_->begin());
}

void num_add()
{
	stackLevel++;
	AlifObject* left = *(memory_ + stackLevel);
	stackLevel++;
	AlifObject* right = *(memory_ + stackLevel);

	AlifObject* result = new AlifObject();
	result->objType = OTNumber;
	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? result->V.NumberObj.numberType = TTFloat : result->V.NumberObj.numberType = TTInteger;
	result->V.NumberObj.numberValue = right->V.NumberObj.numberValue + left->V.NumberObj.numberValue;
	*(memory_ + stackLevel) = result;
	stackLevel--;
}

void num_minus() {}

void str_add()
{
	stackLevel++;
	AlifObject* left = *(memory_ + stackLevel);
	stackLevel++;
	AlifObject* right = *(memory_ + stackLevel);

	uint16_t rightSize = wcslen(right->V.StringObj.strValue);
	uint16_t leftSize = wcslen(left->V.StringObj.strValue);
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
	*(memory_ + stackLevel) = right;
	stackLevel--;
}



void Interpreter::run_code()
{
	for (InstructionsType command_ : *instructions_)
	{
		instr_funcs[command_]();
	}

	stackLevel++;
	AlifObject* res = *(memory_ + stackLevel);
	std::wcout << res->V.NumberObj.numberValue << std::endl;
	//std::wcout << res->V.StringObj.strValue << std::endl;
}
