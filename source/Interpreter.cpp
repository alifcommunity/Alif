#include "Interpreter.h"

Interpreter::Interpreter(std::vector<InstructionsType>* _instructions, std::vector<AlifObject*>* _data) :
	instructions_(_instructions), data_(_data) {}

void Interpreter::run_code()
{
	for (InstructionsType command_ : *instructions_)
	{
		switch (command_)
		{
		case SendObj:
			memory_.push_back(data_->front());
			data_->erase(data_->begin());
		case BringObj:
			break;
				
		case SumNumbers:
			AlifObject* left = memory_.front();
			memory_.erase(memory_.begin());
			AlifObject* right = memory_.front();
			memory_.erase(memory_.begin());

			AlifObject* result = new AlifObject();
			result->objType = OTNumber;
			left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? result->V.NumberObj.numberType = TTFloat : result->V.NumberObj.numberType = TTInteger;
			result->V.NumberObj.numberValue = left->V.NumberObj.numberValue + right->V.NumberObj.numberValue;
			memory_.push_back(result);
			break;
		//default:
		//	std::wcout << memory_.front()->V.NumberObj.numberValue << std::endl;
		//	break;
		}
	}
}