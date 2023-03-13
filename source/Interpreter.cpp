#include "AddrFuncs.h"
#include "Interpreter.h"


Interpreter::Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data) {
	data_ = _data;
	instructions_ = _instructions;
}

void none_() {}
void bring_name() {}

void send_name()
{
	*(memory_ + stackLevel) = data_->front();
	stackLevel--;
	data_->erase(data_->begin());
}

void add_op()
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

void minus_op() {}

void Interpreter::run_code()
{
	for (InstructionsType command_ : *instructions_)
	{
		instr_funcs[command_]();
	}

	//stackLevel++;
	//AlifObject* res = *(memory_ + stackLevel);
	//std::wcout << res->V.NumberObj.numberValue << std::endl;
}
