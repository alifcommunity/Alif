#include "AddrFuncs.h"
#include "Interpreter.h"

AlifArray<Container*>* containers_;
AlifArray<AlifObject*>* dataArr;
AlifArray<InstructionsType>* instrArr;
AlifStack<AlifObject*>* stackMemory;
AlifMemory* alifMemory;
size_t dataIndex = 0;
size_t instructionsIndex = 0;

Interpreter::Interpreter(AlifArray<Container*>* _containers, AlifMemory* _alifMemory) {
	containers_ = _containers;
	alifMemory = _alifMemory;
}

void Interpreter::run_code()
{
	uint32_t endContainers = containers_->size();
	for (unsigned int i = 0; i < endContainers; i++)
	{
		dataArr = containers_->get(i)->data_;
		instrArr = containers_->get(i)->instructions_;
		stackMemory = new AlifStack<AlifObject*>;

		uint32_t endContainer = instrArr->size();
		for (instructionsIndex = 0; instructionsIndex < endContainer; instructionsIndex++)
		{
			instr_funcs[instrArr->get(instructionsIndex)]();
		}

		delete stackMemory;
		dataIndex = 0;
	}
}


void none_() {}

void get_data() 
{
	AlifObject* name_ = stackMemory->pop();

	stackMemory->push(symTable->get_data(*name_->V.NameObj.name_));

	AlifObject* res = stackMemory->get();
	std::wcout << res->V.NumberObj.numberValue << std::endl;
}
void set_data()
{
	stackMemory->push(dataArr->get(dataIndex));
	dataIndex++;
}

void plus_num() 
{
	
}
void minus_num() 
{
	AlifObject* number_ = stackMemory->pop();

	number_->V.NumberObj.numberValue = - number_->V.NumberObj.numberValue;
}

void add_num()
{
	AlifObject* right = stackMemory->pop();
	AlifObject* left = stackMemory->pop();
	// يجب حذف المتغير left
	// لانه لم يتم حذفه بعد وقد تم العمل عليه ولم يعد له حاجة

	AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? res->V.NumberObj.numberType = TTFloat : res->V.NumberObj.numberType = TTInteger;
	res->objType = OTNumber;
	res->V.NumberObj.numberValue = right->V.NumberObj.numberValue + left->V.NumberObj.numberValue;
	
	stackMemory->push(res);
	alifMemory->deallocate(res);
}
void sub_num() 
{
	AlifObject* right = stackMemory->pop();
	AlifObject* left = stackMemory->pop();

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue -= left->V.NumberObj.numberValue;
	stackMemory->push(right);
}
void mul_num() 
{
	AlifObject* right = stackMemory->pop();
	AlifObject* left = stackMemory->pop();


	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue *= left->V.NumberObj.numberValue;
	stackMemory->push(right);
}
void div_num() 
{
	AlifObject* right = stackMemory->pop();
	AlifObject* left = stackMemory->pop();

	left->V.NumberObj.numberType == TTFloat;
	right->V.NumberObj.numberValue /= left->V.NumberObj.numberValue;
	stackMemory->push(right);
}
void rem_num() 
{
	AlifObject* right = stackMemory->pop();
	AlifObject* left = stackMemory->pop();

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue = (int)right->V.NumberObj.numberValue % (int)left->V.NumberObj.numberValue;
	stackMemory->push(right);
}
void pow_num() 
{
	AlifObject* right = stackMemory->pop();
	AlifObject* left = stackMemory->pop();

	left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat ? right->V.NumberObj.numberType = TTFloat : right->V.NumberObj.numberType = TTInteger;
	right->V.NumberObj.numberValue = pow(right->V.NumberObj.numberValue  ,left->V.NumberObj.numberValue);
	stackMemory->push(right);
}
void augAdd_num()
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* value_ = stackMemory->pop();

	symTable->get_data(*name_->V.NameObj.name_)->V.NumberObj.numberValue += value_->V.NumberObj.numberValue;
	//symTable->get_data(*name_->V.NameObj.name_)->V.NumberObj.numberValue += 1;

}
void augSub_num()
{
	
}
void augMul_num()
{
	
}
void augDiv_num()
{
	
}
void augRem_num()
{
	
}
void augPow_num()
{
	
}


void equal_equal()
{
	AlifObject* right = stackMemory->pop();
	AlifObject* left = stackMemory->pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.BoolObj.numberValue == left->V.BoolObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.BoolObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.BoolObj.numberValue = 0;
		}
		stackMemory->push(res);
	}
	else
	{
		// error
	}

}
void not_equal() 
{
	AlifObject* right = stackMemory->pop();
	AlifObject* left = stackMemory->pop();

	if (right->objType == left->objType)
	{
		AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.BoolObj.numberValue != left->V.BoolObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.BoolObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.BoolObj.numberValue = 0;
		}
		stackMemory->push(res);
	}
	else
	{
		// error
	}
}
void gr_than_num()
{
	AlifObject* right = stackMemory->pop();

	AlifObject* left = stackMemory->pop();


	if (right->objType == left->objType)
	{
		AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.BoolObj.numberValue > left->V.BoolObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.BoolObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.BoolObj.numberValue = 0;
		}
		stackMemory->push(res);
	}
	else
	{
		// error
	}
}
void gr_than_eq_num()
{
	AlifObject* right = stackMemory->pop();

	AlifObject* left = stackMemory->pop();


	if (right->objType == left->objType)
	{
		AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.BoolObj.numberValue >= left->V.BoolObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.BoolObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.BoolObj.numberValue = 0;
		}
		stackMemory->push(res);
	}
	else
	{
		// error
	}
}
void ls_than_num()
{
	AlifObject* right = stackMemory->pop();

	AlifObject* left = stackMemory->pop();


	if (right->objType == left->objType)
	{
		AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.BoolObj.numberValue < left->V.BoolObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.BoolObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.BoolObj.numberValue = 0;
		}
		stackMemory->push(res);
	}
	else
	{
		// error
	}
}
void ls_than_eq_num()
{
	AlifObject* right = stackMemory->pop();

	AlifObject* left = stackMemory->pop();


	if (right->objType == left->objType)
	{
		AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.BoolObj.numberValue <= left->V.BoolObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.BoolObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.BoolObj.numberValue = 0;
		}
		stackMemory->push(res);
	}
	else
	{
		// error
	}
}
void not_logic() 
{
	AlifObject* right = stackMemory->pop();


	AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

	res->objType = OTBoolean;
	res->tokLine = right->tokLine;

	if (right->V.BoolObj.numberValue == 0)
	{
		res->V.BoolObj.boolType = L"صح";
		res->V.BoolObj.numberValue = 1;
	}
	else
	{
		res->V.BoolObj.boolType = L"خطا";
		res->V.BoolObj.numberValue = 0;
	}
	stackMemory->push(res);

}
void and_logic()
{
	AlifObject* right = stackMemory->pop();

	AlifObject* left = stackMemory->pop();


	if (right->objType == left->objType)
	{
		AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.BoolObj.numberValue and left->V.BoolObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.BoolObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.BoolObj.numberValue = 0;
		}
		stackMemory->push(res);
	}
	else
	{
		// error
	}
}
void or_logic() 
{
	AlifObject* right = stackMemory->pop();

	AlifObject* left = stackMemory->pop();


	if (right->objType == left->objType)
	{
		AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

		res->objType = OTBoolean;
		res->tokLine = right->tokLine;

		if (right->V.BoolObj.numberValue or left->V.BoolObj.numberValue)
		{
			res->V.BoolObj.boolType = L"صح";
			res->V.BoolObj.numberValue = 1;
		}
		else
		{
			res->V.BoolObj.boolType = L"خطا";
			res->V.BoolObj.numberValue = 0;
		}
		stackMemory->push(res);
	}
	else
	{
		// error
	}
}

void add_str() // هذه الطريقة اسرع من استخدام wcsncpy_s و wcsncat_s
{
	AlifObject* right = stackMemory->pop();

	AlifObject* left = stackMemory->pop();



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
	stackMemory->push(right);

}
void mul_str() {
	
	AlifObject* right = stackMemory->pop();

	AlifObject* left = stackMemory->pop();


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
		stackMemory->push(right);
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
		stackMemory->push(right);
	}

}


void expr_op() 
{
	AlifObject* compRes = stackMemory->pop();

	if (compRes->V.NumberObj.numberValue != 0)
	{
		stackMemory->pop();
	}
}


void store_name()
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* value_ = stackMemory->pop();

	symTable->add_symbol(*name_->V.NameObj.name_, value_);
}

void list_make()
{
	AlifObject* list_ = stackMemory->pop();


	AlifObject* elementCount = stackMemory->pop();


	AlifObject* element_{};

	for (int i = 0; i < elementCount->V.NumberObj.numberValue; i++)
	{
		element_ = stackMemory->pop();
	
		list_->V.ListObj.objList->push_back(element_);
	}
}


void jump_to()
{
	AlifObject* dataAddress = stackMemory->pop();
	AlifObject* address_ = stackMemory->pop();

	instructionsIndex = address_->V.NumberObj.numberValue;
	dataIndex = dataAddress->V.NumberObj.numberValue;
}
void jump_if()
{
	AlifObject* address_ = stackMemory->pop();
	AlifObject* dataAddress = stackMemory->pop();
	AlifObject* condetion_ = stackMemory->pop();

	if (!condetion_->V.BoolObj.numberValue)
	{
		instructionsIndex = address_->V.NumberObj.numberValue;
		dataIndex = dataAddress->V.NumberObj.numberValue;
	}
}

size_t startFor = 0, endFor = 1, stepFor = 1;
void jump_for()
{
	AlifObject* jumpAddress = stackMemory->pop();
	AlifObject* dataAddress = stackMemory->pop();
	AlifObject* iterName = stackMemory->pop();

	startFor += stepFor;
	if (startFor < endFor)
	{
		symTable->get_data(*iterName->V.NameObj.name_)->V.NumberObj.numberValue = startFor;

		instructionsIndex = jumpAddress->V.NumberObj.numberValue;
		dataIndex = dataAddress->V.NumberObj.numberValue;

		stackMemory->reset();
	}
}
void for_iter()
{
	AlifObject* stepForObj = stackMemory->pop();
	AlifObject* endForObj = stackMemory->pop();
	AlifObject* startForObj = stackMemory->pop();

	startFor = startForObj->V.NumberObj.numberValue;
	endFor = endForObj->V.NumberObj.numberValue;
	stepFor = stepForObj->V.NumberObj.numberValue;
}

void get_scope()
{
	AlifObject* name_ = stackMemory->pop();

	symTable->enter_scope(*name_->V.NameObj.name_);

	stackMemory->push(symTable->get_data(*name_->V.NameObj.name_));
}

void call_name()
{
	AlifObject* container_ = stackMemory->pop();
	int instrIndexBackup;
	int dataIndexBackup;

	instrIndexBackup = instructionsIndex;
	dataIndexBackup = dataIndex;

	// هذا القسم يقوم بإسناد الوسيطات الممررة الى المعاملات في حال وجودها
	AlifArray<AlifObject*> args_;

	AlifObject* argSize = stackMemory->pop();
	if (argSize->V.NumberObj.numberValue)
	{
		for (int i = 0; i < argSize->V.NumberObj.numberValue; i++)
		{
			args_.push_back(stackMemory->pop());
		}
	}

	size_t paramSize = container_->V.ContainerObj.container_->data_->get(0)->V.NumberObj.numberValue;
	if (paramSize)
	{
		if (argSize->V.NumberObj.numberValue > paramSize)
		{
			PRINT_(L"عدد المعاملات الممرر اكبر من المطلوب");
			exit(-1);
		}

		AlifObject* params_ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		params_ = container_->V.ContainerObj.container_->data_->get(1);


		dataArr = params_->V.ContainerObj.container_->data_;
		instrArr = params_->V.ContainerObj.container_->instructions_;
		instructionsIndex = 0;
		dataIndex = 0;


		uint32_t endContainer = instrArr->size();
		int arg = argSize->V.NumberObj.numberValue - 1;
		for (; instructionsIndex < endContainer;)
		{
			instr_funcs[instrArr->get(instructionsIndex)]();
			instructionsIndex++;
			if (instrArr->get(instructionsIndex) == GET_DATA)
			{
				symTable->add_symbol(*stackMemory->pop()->V.NameObj.name_, args_.get(arg));
				instructionsIndex++;
				arg--;
			}
			else if (instrArr->get(instructionsIndex) == STORE_NAME)
			{
				AlifObject* nameBackup = stackMemory->get();
				instr_funcs[instrArr->get(instructionsIndex)]();
				instructionsIndex++;

				if (arg > -1)
				{
					symTable->add_symbol(*nameBackup->V.NameObj.name_, args_.get(arg));
					arg--;
				}
			}
		}
	}



	// هذا القسم يقوم بتنفيذ الدالة المستدعاة

	//instrIndexBackup = instructionsIndex;
	//dataIndexBackup = dataIndex;

	dataArr = container_->V.ContainerObj.container_->data_;
	instrArr = container_->V.ContainerObj.container_->instructions_;
	instructionsIndex = 0;
	dataIndex = 2;

	uint32_t endContainer = instrArr->size();
	for (instructionsIndex; instructionsIndex < endContainer; instructionsIndex++)
	{
		instr_funcs[instrArr->get(instructionsIndex)]();
	}


	instructionsIndex = instrIndexBackup;
	dataIndex = dataIndexBackup;

	symTable->exit_scope();
}
