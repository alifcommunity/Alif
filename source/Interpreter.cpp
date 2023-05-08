#include "Interpreter.h"

AlifArray<Container*>* containers_;
AlifArray<AlifObject*>* dataArr;
AlifArray<InstructionsType>* instrArr;
AlifStack<AlifObject*>* stackMemory;
AlifMemory* alifMemory;
AlifNamesTable* namesTable;
size_t dataIndex = 0;
size_t instructionsIndex = 0;

Interpreter::Interpreter(AlifArray<Container*>* _containers, AlifMemory* _alifMemory, AlifNamesTable* _namesTable) {
	containers_ = _containers;
	alifMemory = _alifMemory;
	namesTable = _namesTable;
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


void none_() {} // لا يقوم بعمل شيء

void get_data() 
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* data_ = namesTable->get_object(name_->V.NameObj.name_);

	stackMemory->push(data_);
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

	AlifObject* res = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

	if (left->V.NumberObj.numberType == TTFloat or right->V.NumberObj.numberType == TTFloat)
	{
		// error
	}

	res->objType = OTNumber;
	res->V.NumberObj.numberValue = (int)right->V.NumberObj.numberValue % (int)left->V.NumberObj.numberValue;

	stackMemory->push(res);
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

	AlifObject* nameValue = namesTable->get_object(name_->V.NameObj.name_);

	// نسخ الرقم الثابت لكي لا يتم تعديله
	AlifObject* nameCopy = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	nameCopy->objType = nameValue->objType;
	nameCopy->V = nameValue->V; 

	nameCopy->V.NumberObj.numberValue += value_->V.NumberObj.numberValue;

	namesTable->assign_name(name_->V.NameObj.name_, nameCopy);
}
void augSub_num()
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* value_ = stackMemory->pop();

	AlifObject* nameValue = namesTable->get_object(name_->V.NameObj.name_);

	// نسخ الرقم الثابت لكي لا يتم تعديله
	AlifObject* nameCopy = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	nameCopy->objType = nameValue->objType;
	nameCopy->V = nameValue->V;

	nameCopy->V.NumberObj.numberValue -= value_->V.NumberObj.numberValue;

	namesTable->assign_name(name_->V.NameObj.name_, nameCopy);
}
void augMul_num()
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* value_ = stackMemory->pop();

	AlifObject* nameValue = namesTable->get_object(name_->V.NameObj.name_);

	// نسخ الرقم الثابت لكي لا يتم تعديله
	AlifObject* nameCopy = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	nameCopy->objType = nameValue->objType;
	nameCopy->V = nameValue->V;

	nameCopy->V.NumberObj.numberValue *= value_->V.NumberObj.numberValue;

	namesTable->assign_name(name_->V.NameObj.name_, nameCopy);
}
void augDiv_num()
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* value_ = stackMemory->pop();

	AlifObject* nameValue = namesTable->get_object(name_->V.NameObj.name_);

	// نسخ الرقم الثابت لكي لا يتم تعديله
	AlifObject* nameCopy = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	nameCopy->objType = nameValue->objType;
	nameCopy->V = nameValue->V;

	nameCopy->V.NumberObj.numberValue /= value_->V.NumberObj.numberValue;

	namesTable->assign_name(name_->V.NameObj.name_, nameCopy);
}
void augRem_num()
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* value_ = stackMemory->pop();

	AlifObject* nameValue = namesTable->get_object(name_->V.NameObj.name_);

	// نسخ الرقم الثابت لكي لا يتم تعديله
	AlifObject* nameCopy = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	nameCopy->objType = nameValue->objType;
	nameCopy->V = nameValue->V;

	nameCopy->V.NumberObj.numberValue = (size_t)(nameCopy->V.NumberObj.numberValue) % (size_t)(value_->V.NumberObj.numberValue);

	namesTable->assign_name(name_->V.NameObj.name_, nameCopy);
}
void augPow_num()
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* value_ = stackMemory->pop();

	AlifObject* nameValue = namesTable->get_object(name_->V.NameObj.name_);

	// نسخ الرقم الثابت لكي لا يتم تعديله
	AlifObject* nameCopy = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	nameCopy->objType = nameValue->objType;
	nameCopy->V = nameValue->V;

	nameCopy->V.NumberObj.numberValue = pow(nameCopy->V.NumberObj.numberValue, value_->V.NumberObj.numberValue);

	namesTable->assign_name(name_->V.NameObj.name_, nameCopy);
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

	AlifObject* valCopy = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	valCopy->objType = value_->objType;
	valCopy->V = value_->V;

	namesTable->assign_name(name_->V.NameObj.name_, valCopy);
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
	AlifObject* instrAddress = stackMemory->pop();

	dataIndex = dataAddress->V.NumberObj.numberValue;
	instructionsIndex = instrAddress->V.NumberObj.numberValue;
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

void jump_for()
{
	AlifObject* stepForObj = stackMemory->pop();
	AlifObject* endForObj = stackMemory->pop();
	AlifObject* startForObj = stackMemory->pop();
	AlifObject* startForBackupObj = stackMemory->pop();

	AlifObject* jumpAddress = stackMemory->pop();
	AlifObject* dataAddress = stackMemory->pop();
	AlifObject* iterName = stackMemory->pop();

	startForObj->V.NumberObj.numberValue += stepForObj->V.NumberObj.numberValue;
	if (startForObj->V.NumberObj.numberValue < endForObj->V.NumberObj.numberValue)
	{
		AlifObject* name_ = namesTable->get_object(iterName->V.NameObj.name_);
		name_->V.NumberObj.numberValue = startForObj->V.NumberObj.numberValue;

		instructionsIndex = jumpAddress->V.NumberObj.numberValue;
		dataIndex = dataAddress->V.NumberObj.numberValue;

		stackMemory->reset();													 
	}
	else // هذه الحالة تقوم بإعادة ضبط قيمة البداية الى القيمة الافتراضية بعد الانتهاء من تنفيذ الحلقة
	{
		AlifObject* name_ = namesTable->get_object(iterName->V.NameObj.name_);
		name_->V.NumberObj.numberValue = startForBackupObj->V.NumberObj.numberValue;

		startForObj->V.NumberObj.numberValue = startForBackupObj->V.NumberObj.numberValue;
	}
}

void create_scope()
{
	AlifObject* name_ = stackMemory->pop();

	namesTable->create_scope(name_->V.NameObj.name_);
}
void copy_scope()
{
	AlifObject* b = stackMemory->pop();
	stackMemory->swap();
	AlifObject* a = stackMemory->pop();

	namesTable->copy_scope(a->V.NameObj.name_, b->V.NameObj.name_);
}
void enter_scope()
{
	AlifObject* name_ = stackMemory->pop();

	if (!namesTable->enter_scope(name_->V.NameObj.name_)) 
	{
		PRINT_(L"لم يتمكن من الدخول الى النطاق"); exit(-1);
	}
}
void get_scope()
{
	AlifObject* name_ = stackMemory->pop();
	AlifObject* data_{};

	if (namesTable->enter_scope(name_->V.NameObj.name_))
	{
		data_ = namesTable->get_object(name_->V.NameObj.name_);
		namesTable->exit_scope();
	}

	stackMemory->push(data_);
}
void exit_scope()
{
	namesTable->exit_scope();
}
//void restore_scope()
//{
//	namesTable->restore_scope();
//}

bool returnFlag = false;
void call_name()
{
	AlifObject* container_ = stackMemory->pop();

	// عمل نسخة احتياطية للبرنامج الاصلي
	AlifArray<AlifObject*>* dataArrBackup = dataArr;
	AlifArray<InstructionsType>* instrArrBackup = instrArr;
	int instrIndexBackup = instructionsIndex;
	int dataIndexBackup = dataIndex;


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
				wcstr* name = stackMemory->pop()->V.NameObj.name_;
				if (!namesTable->assign_name(name, args_.get(arg)))
				{
					namesTable->create_name(name, args_.get(arg));
				}
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
					wcstr* name = nameBackup->V.NameObj.name_;
					if (!namesTable->assign_name(name, args_.get(arg)))
					{
						namesTable->create_name(name, args_.get(arg));
					}
					arg--;
				}
			}
		}
	}


	// هذا القسم يقوم بتنفيذ الدالة المستدعاة

	dataArr = container_->V.ContainerObj.container_->data_;
	instrArr = container_->V.ContainerObj.container_->instructions_;
	instructionsIndex = 0;
	dataIndex = 2; // تم تخطي متغير حاوية المعاملات ومتغير عدد المعاملات

	uint32_t endContainer = instrArr->size();
	for (instructionsIndex; instructionsIndex < endContainer; instructionsIndex++)
	{
		if (returnFlag)
		{
			break;
		}
		instr_funcs[instrArr->get(instructionsIndex)]();
	}


	dataArr = dataArrBackup;
	instrArr = instrArrBackup;
	instructionsIndex = instrIndexBackup;
	dataIndex = dataIndexBackup;

	//namesTable->restore_scope(); // يجب المراجعة وتصحيح النظام
	returnFlag = false;
}

void return_expr()
{
	returnFlag = true;
}










// الدوال الضمنية

void print_func() // سيتم اعتماد خوارزمية البحث الثنائي الى حين إيجاد حل لاستدعاء الطباعة بتعقيد زمني يساوي 1
{
	AlifObject* object = namesTable->get_object((wcstr*)0x01); // يجب مراجعة النظام

    if (object->objType == OTNumber) { PRINT_(object->V.NumberObj.numberValue); }
    else if (object->objType == OTString) { PRINT_(object->V.StringObj.strValue); }
    else if (object->objType == OTNone) { PRINT_(L"عدم"); }
    else if (object->objType == OTBoolean) { if (object->V.BoolObj.numberValue == 1) { PRINT_(L"صح"); } else { PRINT_(L"خطا"); } }
    else if (object->objType == OTList) {
        //this->list_print(object);
        //prnt(lst);
    }
 
 
 
	//print_types[0]();
}

//// دوال الطباعة
//void num_print()
//{
//	AlifObject* object = namesTable->get_object(*L"ع");
//
//	std::wcout << object->V.NumberObj.numberValue << std::endl;
//}