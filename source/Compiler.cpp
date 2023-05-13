#include "Compiler.h"

/*
	يجب عمل أمر مماثل ل GET_DATA
	ولكن لا يقوم بعمل pop()
	من ذاكرة المكدس في المفسر لكي لا يتم إسناد القيمة او الكائن اكثر من مرة ليتم استخدامه
*/
Compiler::Compiler(std::vector<StmtsNode*>* _statements, AlifMemory* _alifMemory, AlifNamesTable* _namesTable) :
	statements_(_statements), alifMemory(_alifMemory), namesTable(_namesTable) {}


void Compiler::compile_file() 
{
	for (StmtsNode* node_ : *statements_)
	{
		dataContainer = (Container*)alifMemory->allocate(sizeof(Container));
		dataContainer->instructions_ = new AlifArray<InstructionsType>;
		dataContainer->data_ = new AlifArray<AlifObject*>;

		VISIT_(stmts, node_); // تم شرح طريقة الاستدعاء في ملف compiler.h

		containers_.push_back(dataContainer);
	}
}


AlifObject* Compiler::visit_object(ExprNode* _node)
{
	dataContainer->data_->push_back(_node->U.Object.value_);
	dataContainer->instructions_->push_back(SET_DATA);
	return _node->U.Object.value_;
}


AlifObject* Compiler::visit_unaryOp(ExprNode* _node)
{
	AlifObject* right = VISIT_(exprs, _node->U.UnaryOp.right_);

	if (_node->U.UnaryOp.operator_ == TTMinus)
	{
		dataContainer->instructions_->push_back(MINUS_NUM);
	}
	else if (_node->U.UnaryOp.operator_ == TTPlus)
	{
		dataContainer->instructions_->push_back(PLUS_NUM);
	}
	else if (!wcscmp(_node->U.UnaryOp.keyword_, L"ليس"))
	{
		dataContainer->instructions_->push_back(NOT_LOGIC);
	}

	return right;
}


AlifObject* Compiler::visit_binOp(ExprNode* _node)
{
	AlifObject* right = VISIT_(exprs, _node->U.BinaryOp.right_);
	AlifObject* left = VISIT_(exprs, _node->U.BinaryOp.left_);

	if (_node->U.BinaryOp.operator_ == TTPlus)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(ADD_NUM);
			}
			else
			{
				// TypeError: int and other
			}
		}
		else if (left->objType == OTString)
		{
			if (right->objType == OTString)
			{
				dataContainer->instructions_->push_back(ADD_STR);
			}
			else
			{
				// TypeError: str and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTMinus)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(MINUS_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTPower)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(POW_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTDivide)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(DIV_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTMultiply)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(MUL_NUM);
			}
			else if (right->objType == OTString)
			{
				dataContainer->instructions_->push_back(MUL_STR);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else if (left->objType == OTString)
		{
			if (right->objType == OTNumber)
			{
				dataContainer->instructions_->push_back(MUL_STR);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTRemain)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(REM_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTEqualEqual)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(EQEQ_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTNotEqual)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(NOTEQ_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTGreaterThan)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(GRTHAN_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTLessThan)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or left->objType == OTName)
			{
				dataContainer->instructions_->push_back(LSTHAN_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTGreaterThanEqual)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or right->objType == OTName)
			{
				dataContainer->instructions_->push_back(GRTHANEQ_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (_node->U.BinaryOp.operator_ == TTLessThanEqual)
	{
		if (left->objType == OTNumber or left->objType == OTName)
		{
			if (right->objType == OTNumber or left->objType == OTName)
			{
				dataContainer->instructions_->push_back(LSTHANEQ_NUM);
			}
			else
			{
				// TypeError: this and other
			}
		}
		else
		{
			// TypeError: this and other
		}
	}
	else if (!wcscmp(_node->U.UnaryOp.keyword_, L"و"))
	{
		dataContainer->instructions_->push_back(AND_LOGIC);
	}
	else if (!wcscmp(_node->U.UnaryOp.keyword_, L"او"))
	{
		dataContainer->instructions_->push_back(OR_LOGIC);
	}

	return left;
}

AlifStack<AlifObject*> assignName = AlifStack<AlifObject*>(256);
void Compiler::visit_assign(ExprNode* _node)
{
	isAssignFlag = true; // علم خاص بعملية الاستدعاء
	
	for (AlifObject* n : *_node->U.NameAssign.name_)
	{
		assignName.push(n);
		VISIT_(exprs,_node->U.NameAssign.value_);

		if (!isCallFlag)
		{
			dataContainer->data_->push_back(n);
			dataContainer->instructions_->push_back(SET_DATA); // خزن الاسم في المكدس
			dataContainer->instructions_->push_back(STORE_NAME);
		}
		else {
			isCallFlag = false;
		}
	}

	isAssignFlag = false;
}

void Compiler::visit_augAssign(ExprNode* _node)
{
	VISIT_(exprs, _node->U.AugNameAssign.value_);

	dataContainer->data_->push_back(_node->U.AugNameAssign.name_);
	dataContainer->instructions_->push_back(SET_DATA);

	switch (_node->U.AugNameAssign.operator_)
	{
	case TTPlusEqual:
		dataContainer->instructions_->push_back(AUGADD_NUM);
		break;
	case TTMinusEqual:
		dataContainer->instructions_->push_back(AUGSUB_NUM);
		break;
	case TTMultiplyEqual:
		dataContainer->instructions_->push_back(AUGMUL_NUM);
		break;
	case TTDivideEqual:
		dataContainer->instructions_->push_back(AUGDIV_NUM);
		break;
	case TTRemainEqual:
		dataContainer->instructions_->push_back(AUGREM_NUM);
		break;
	case TTPowerEqual:
		dataContainer->instructions_->push_back(AUGPOW_NUM);
		break;
	default:
		break;
	}
}

AlifObject* Compiler::visit_access(ExprNode* _node)
{
	dataContainer->data_->push_back(_node->U.NameAccess.name_);
	dataContainer->instructions_->push_back(SET_DATA);
	if (attrFlag)
	{ dataContainer->instructions_->push_back(ENTER_SCOPE); }
	else { dataContainer->instructions_->push_back(GET_DATA); }

	return _node->U.NameAccess.name_;
}
int exitScopeAttr = 0;
AlifObject* Compiler::visit_attr(ExprNode* _node)
{
	attrFlag = true;
	AlifObject* name = VISIT_(exprs, _node->U.Attr.name_);
	exitScopeAttr++;
	if (_node->U.Attr.next_->type_ == VTAccess or _node->U.Attr.next_->type_ == VTCall)
	{ attrFlag = false; }
	VISIT_(exprs, _node->U.Attr.next_);

	return name;
}

void Compiler::visit_expr(ExprNode* _node)
{
	VISIT_(exprs, _node->U.CondExpr.expr_);
	VISIT_(exprs, _node->U.CondExpr.elseExpr);
	VISIT_(exprs, _node->U.CondExpr.condetion_);

	dataContainer->instructions_->push_back(EXPR_OP);
}

void Compiler::visit_list(ExprNode* _node)
{
	// إسناد العناصر التي سيتم إعدادها
	for (ExprNode* element : *_node->U.Object.value_->V.ListObj.list_)
	{
		VISIT_(exprs, element);
	}


	// لتحديد نهاية المصفوفة في المفسر
	AlifObject* listEnd = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	listEnd->objType = OTNumber;
	listEnd->V.NumberObj.numberValue = _node->U.Object.value_->V.ListObj.list_->size();

	dataContainer->data_->push_back(listEnd);
	dataContainer->instructions_->push_back(SET_DATA);


	// تعريف المصفوفة التي ستحتوي على العناصر التي تم إعداداها
	AlifObject* listObj = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	std::vector<AlifObject*>* elements_ = new std::vector<AlifObject*>;

	listObj->objType = OTList;
	listObj->V.ListObj.objList = elements_;

	dataContainer->data_->push_back(listObj);
	dataContainer->instructions_->push_back(SET_DATA);



	dataContainer->instructions_->push_back(LIST_MAKE);
}

// نظام لتخزين العناوين لحالة "توقف" في حال كان هنالك تداخل في الحلقات
AlifObject* instrAddressForStop = new AlifObject[18];
AlifObject* dataAddressForStop = new AlifObject[18];
size_t stopLevel = 0;
void Compiler::visit_stop()
{
	AlifObject* instrAFS = (instrAddressForStop + stopLevel);
	AlifObject* dataAFS = (dataAddressForStop + stopLevel);

	dataContainer->data_->push_back(instrAFS);
	this->dataContainer->instructions_->push_back(SET_DATA);

	dataContainer->data_->push_back(dataAFS);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(JUMP_TO);
}

AlifObject* instrAddressForContinue = new AlifObject[18];
AlifObject* dataAddressForContinue = new AlifObject[18];
size_t continueLevel = 0;
void Compiler::visit_continue_()
{
	// يجب القفز الى عنوان بداية الحلقة أي إعادة تكرارها من جديد

	AlifObject* instrAFS = (instrAddressForContinue + continueLevel);
	AlifObject* dataAFS = (dataAddressForContinue + continueLevel);

	dataContainer->data_->push_back(instrAFS);
	this->dataContainer->instructions_->push_back(SET_DATA);

	dataContainer->data_->push_back(dataAFS);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(JUMP_TO);
}


AlifObject* jTAdress = new AlifObject; // Address of instructions
AlifObject* jTDataAddress = new AlifObject; // Address of data
void Compiler::visit_if_(StmtsNode* _node)
{
	VISIT_(exprs, _node->U.If.condetion_);

	AlifObject* dataAddress = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // Address of data
	dataAddress->objType = OTNumber;
	this->dataContainer->data_->push_back(dataAddress);
	this->dataContainer->instructions_->push_back(SET_DATA);

	AlifObject* jumpAddress = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // Address of instructions
	jumpAddress->objType = OTNumber;
	this->dataContainer->data_->push_back(jumpAddress);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(JUMP_IF);


	VISIT_(stmts, _node->U.If.block_);

	jTAdress->objType = OTNumber;
	this->dataContainer->data_->push_back(jTAdress);
	this->dataContainer->instructions_->push_back(SET_DATA);

	jTDataAddress->objType = OTNumber;
	this->dataContainer->data_->push_back(jTDataAddress);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(JUMP_TO);

	dataAddress->V.NumberObj.numberValue = this->dataContainer->data_->size();
	jumpAddress->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;


	if (elseIfFlag) // يتحقق هل المستدعى "اذا" ام "واذا" ففي حال الاولى يسمح بالمرور والثانية لا يسمح
	{
		elseIfFlag = false;

		if (_node->U.If.elseIf->size())
		{
			for (StmtsNode* node : *_node->U.If.elseIf)
			{
				VISIT_(stmts, node);
			}
		}

		if (_node->U.If.else_)
		{
			VISIT_(stmts, _node->U.If.else_);
		
			jTDataAddress->V.NumberObj.numberValue = this->dataContainer->data_->size();
			jTAdress->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;
		}
	}
}

void Compiler::visit_for_(StmtsNode* _node)
{
	AlifObject* startFor = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // startFor
	AlifObject* stepFor = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // stepFor
	AlifObject* endFor = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // endFor
	startFor->objType = OTNumber;
	stepFor->objType = OTNumber;
	endFor->objType = OTNumber;

	if (_node->U.For.args_->size() == 1)
	{
		startFor->V.NumberObj.numberValue = 0;

		dataContainer->data_->push_back(startFor);
		dataContainer->instructions_->push_back(SET_DATA);

		dataContainer->data_->push_back(_node->U.For.itrName); // varName
		dataContainer->instructions_->push_back(SET_DATA);

		this->dataContainer->instructions_->push_back(STORE_NAME);
	}
	else if (_node->U.For.args_->size() == 2)
	{
		startFor = VISIT_(exprs, _node->U.For.args_->at(0)); // startFor

		dataContainer->data_->push_back(_node->U.For.itrName); // varName
		dataContainer->instructions_->push_back(SET_DATA);

		this->dataContainer->instructions_->push_back(STORE_NAME);
	}
	else if (_node->U.For.args_->size() == 3)
	{
		startFor = VISIT_(exprs, _node->U.For.args_->at(0)); // startFor

		dataContainer->data_->push_back(_node->U.For.itrName); // varName
		dataContainer->instructions_->push_back(SET_DATA);

		this->dataContainer->instructions_->push_back(STORE_NAME);
	}


	AlifObject* jumpAddress = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // Instructions address
	jumpAddress->objType = OTNumber;
	jumpAddress->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;

	AlifObject* dataAddress = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // Data address
	dataAddress->objType = OTNumber;
	dataAddress->V.NumberObj.numberValue = this->dataContainer->data_->size();


	VISIT_(stmts, _node->U.For.block_);

	/// هذا القسم خاص ب حالة استمر فقط
	(instrAddressForContinue + continueLevel)->objType = OTNumber;
	(instrAddressForContinue + continueLevel)->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;

	(dataAddressForContinue + continueLevel)->objType = OTNumber;
	(dataAddressForContinue + continueLevel)->V.NumberObj.numberValue = this->dataContainer->data_->size();

	if (continueLevel < 18) continueLevel++;
	//////////////////////////////

	dataContainer->data_->push_back(_node->U.For.itrName); // varName
	dataContainer->instructions_->push_back(SET_DATA);

	dataContainer->data_->push_back(dataAddress);
	this->dataContainer->instructions_->push_back(SET_DATA); // set data address after for jumb

	dataContainer->data_->push_back(jumpAddress);
	this->dataContainer->instructions_->push_back(SET_DATA); // set instructions address after for jumb


	if (_node->U.For.args_->size() == 1)
	{
		stepFor->V.NumberObj.numberValue = 1;

		AlifObject* startForMain = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		startForMain->V.NumberObj.numberValue = 0;
		startForMain->objType = OTNumber;
		startForMain->V.NumberObj.numberType = TTInteger;
		dataContainer->data_->push_back(startForMain); // startForMain
		dataContainer->instructions_->push_back(SET_DATA);

		AlifObject* startForBackup = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		startForBackup->V.NumberObj.numberValue = 0;
		startForBackup->objType = OTNumber;
		startForBackup->V.NumberObj.numberType = TTInteger;
		dataContainer->data_->push_back(startForBackup); // startForBackup
		dataContainer->instructions_->push_back(SET_DATA);

		VISIT_(exprs, _node->U.For.args_->at(0)); // endFor

		dataContainer->data_->push_back(stepFor); // stepFor
		dataContainer->instructions_->push_back(SET_DATA);
	}
	else if (_node->U.For.args_->size() == 2)
	{
		AlifObject* startForMain = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		startForMain->V.NumberObj.numberValue = startFor->V.NumberObj.numberValue;
		startForMain->objType = OTNumber;
		startForMain->V.NumberObj.numberType = TTInteger;
		dataContainer->data_->push_back(startForMain); // startForMain
		dataContainer->instructions_->push_back(SET_DATA);

		AlifObject* startForBackup = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		startForBackup->V.NumberObj.numberValue = startFor->V.NumberObj.numberValue;
		startForBackup->objType = OTNumber;
		startForBackup->V.NumberObj.numberType = TTInteger;
		dataContainer->data_->push_back(startForBackup); // startForBackup
		dataContainer->instructions_->push_back(SET_DATA);


		VISIT_(exprs, _node->U.For.args_->at(1)); // endFor
		stepFor->V.NumberObj.numberValue = 1; // stepFor


		dataContainer->data_->push_back(stepFor); // stepFor
		dataContainer->instructions_->push_back(SET_DATA);
	}
	else if (_node->U.For.args_->size() == 3)
	{
		AlifObject* startForMain = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		startForMain->V.NumberObj.numberValue = startFor->V.NumberObj.numberValue;
		startForMain->objType = OTNumber;
		startForMain->V.NumberObj.numberType = TTInteger;
		dataContainer->data_->push_back(startForMain); // startForMain
		dataContainer->instructions_->push_back(SET_DATA);

		AlifObject* startForBackup = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		startForBackup->V.NumberObj.numberValue = startFor->V.NumberObj.numberValue;
		dataContainer->data_->push_back(startForBackup); // startForBackup
		dataContainer->instructions_->push_back(SET_DATA);


		VISIT_(exprs, _node->U.For.args_->at(1)); // endFor
		VISIT_(exprs, _node->U.For.args_->at(2)); // stepFor
	}
	else
	{
		//error
	}

	this->dataContainer->instructions_->push_back(JUMP_FOR);


	/// هذا القسم خاص ب حالة توقف فقط
	(instrAddressForStop + stopLevel)->objType = OTNumber;
	(instrAddressForStop + stopLevel)->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;

	(dataAddressForStop + stopLevel)->objType = OTNumber;
	(dataAddressForStop + stopLevel)->V.NumberObj.numberValue = this->dataContainer->data_->size();

	if (stopLevel < 18) stopLevel++;
	//////////////////////////////
}


void Compiler::visit_while_(StmtsNode* _node) 
{
	AlifObject* jumpTo = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	jumpTo->objType = OTNumber;
	this->dataContainer->instructions_->empty() ? jumpTo->V.NumberObj.numberValue = -1 : jumpTo->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;

	AlifObject* dataAddressJT = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	dataAddressJT->objType = OTNumber;
	dataAddressJT->V.NumberObj.numberValue = this->dataContainer->data_->size();


	VISIT_(exprs, _node->U.While.condetion_);


	AlifObject* dataAddressJ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	dataAddressJ->objType = OTNumber;
	dataContainer->data_->push_back(dataAddressJ); // dataAddress
	dataContainer->instructions_->push_back(SET_DATA);

	AlifObject* jumpAddress = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	jumpAddress->objType = OTNumber;
	this->dataContainer->data_->push_back(jumpAddress);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(JUMP_IF);


	VISIT_(stmts, _node->U.While.block_);


	/// هذا القسم خاص ب حالة استمر فقط
	(instrAddressForContinue + continueLevel)->objType = OTNumber;
	(instrAddressForContinue + continueLevel)->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;

	(dataAddressForContinue + continueLevel)->objType = OTNumber;
	(dataAddressForContinue + continueLevel)->V.NumberObj.numberValue = this->dataContainer->data_->size();

	if (continueLevel < 18) continueLevel++;
	//////////////////////////////

	this->dataContainer->data_->push_back(jumpTo);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->data_->push_back(dataAddressJT);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(JUMP_TO); // this command jump to begin of the "Container"
	
	dataAddressJ->V.NumberObj.numberValue = this->dataContainer->data_->size();  // تم إضافة اثنين لتخطي التعليمتين التاليات jumpTo و dataAddressJT
	jumpAddress->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;


	/// هذا القسم خاص ب حالة توقف فقط
	(instrAddressForStop + stopLevel)->objType = OTNumber;
	(instrAddressForStop + stopLevel)->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;

	(dataAddressForStop + stopLevel)->objType = OTNumber;
	(dataAddressForStop + stopLevel)->V.NumberObj.numberValue = this->dataContainer->data_->size();

	if (stopLevel < 18) stopLevel++;
	//////////////////////////////
}

AlifStack<Container*> funcDepth = AlifStack<Container*>(512);
void Compiler::visit_function(StmtsNode* _node)
{
	funcDepth.push(dataContainer); // نسخ احتياطي في حال تم تعريف دالة داخل دالة

	namesTable->enter_scope(_node->U.FunctionDef.name_->V.NameObj.name_);

	AlifObject* paramContainer = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	if (_node->U.FunctionDef.params_)
	{
		dataContainer = (Container*)alifMemory->allocate(sizeof(Container));
		dataContainer->instructions_ = new AlifArray<InstructionsType>;
		dataContainer->data_ = new AlifArray<AlifObject*>;

		for (ExprNode* node : *_node->U.FunctionDef.params_)
		{
			VISIT_(exprs, node);
		}

		paramContainer->V.ContainerObj.container_ = dataContainer;
		paramContainer->objType = OTContainer;
	}

	dataContainer = (Container*)alifMemory->allocate(sizeof(Container));
	dataContainer->instructions_ = new AlifArray<InstructionsType>;
	dataContainer->data_ = new AlifArray<AlifObject*>;

	if (_node->U.FunctionDef.params_)
	{
		AlifObject* paramSize = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		paramSize->V.NumberObj.numberValue = _node->U.FunctionDef.params_->size();

		dataContainer->data_->push_back(paramSize);
	}
	else
	{
		AlifObject* paramSize = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		paramSize->V.NumberObj.numberValue = 0;

		dataContainer->data_->push_back(paramSize);
	}

	dataContainer->data_->push_back(paramContainer);


	VISIT_(stmts, _node->U.FunctionDef.body_);


	namesTable->exit_scope();

	AlifObject* containerObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	containerObject->objType = OTContainer;
	containerObject->V.ContainerObj.container_ = dataContainer;
	namesTable->assign_name(_node->U.FunctionDef.name_->V.NameObj.name_, containerObject);

	dataContainer = funcDepth.pop(); // اعد تخزين الحالة السابقة لمتابعة العمل عليها
}

void Compiler::visit_class_(StmtsNode* _node)
{
	funcDepth.push(dataContainer); // نسخ احتياطي في حال تم تعريف صنف داخل صنف

	namesTable->enter_scope(_node->U.ClassDef.name_->V.NameObj.name_);

	AlifObject* paramContainer = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	//if (_node->U.FunctionDef.params_) // يجب استبداله بدالة التهيئة
	//{
	//	dataContainer = (Container*)alifMemory->allocate(sizeof(Container));
	//	dataContainer->instructions_ = new AlifArray<InstructionsType>;
	//	dataContainer->data_ = new AlifArray<AlifObject*>;

	//	for (ExprNode* node : *_node->U.FunctionDef.params_)
	//	{
	//		VISIT_(exprs, node);
	//	}

	//	paramContainer->V.ContainerObj.container_ = dataContainer;
	//	paramContainer->objType = OTContainer;
	//}

	dataContainer = (Container*)alifMemory->allocate(sizeof(Container));
	dataContainer->instructions_ = new AlifArray<InstructionsType>;
	dataContainer->data_ = new AlifArray<AlifObject*>;

	//if (_node->U.FunctionDef.params_)
	//{
	//	AlifObject* paramSize = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	//	paramSize->V.NumberObj.numberValue = _node->U.FunctionDef.params_->size();

	//	dataContainer->data_->push_back(paramSize);
	//}
	//else
	//{
	AlifObject* paramSize = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	paramSize->V.NumberObj.numberValue = 0;

	dataContainer->data_->push_back(paramSize);
	//}

	dataContainer->data_->push_back(paramContainer);


	VISIT_(stmts, _node->U.ClassDef.body_);


	namesTable->exit_scope();

	AlifObject* containerObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	containerObject->objType = OTContainer;
	containerObject->V.ContainerObj.container_ = dataContainer;
	namesTable->assign_name(_node->U.ClassDef.name_->V.NameObj.name_, containerObject);

	dataContainer = funcDepth.pop();
}


void Compiler::visit_call(ExprNode* _node)
{
	isCallFlag = true;
	if (_node->U.Call.args_)
	{
		for (ExprNode* node : *_node->U.Call.args_)
		{
			VISIT_(exprs, node);
		}

		AlifObject* size_ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		size_->V.NumberObj.numberValue = _node->U.Call.args_->size();
		dataContainer->data_->push_back(size_);
		dataContainer->instructions_->push_back(SET_DATA);
	}
	else
	{
		AlifObject* size_ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
		size_->V.NumberObj.numberValue = 0;
		dataContainer->data_->push_back(size_);
		dataContainer->instructions_->push_back(SET_DATA);
	}
	
	AlifObject* nameObject = _node->U.Call.name_->U.Object.value_;

	if (isAssignFlag and assignName.index_)
	{
		nameObject = assignName.pop();
	}

	this->dataContainer->data_->push_back(nameObject);
	this->dataContainer->instructions_->push_back(SET_DATA);
	this->dataContainer->instructions_->push_back(GET_DATA); // اجلب قيمة النطاق

	this->dataContainer->data_->push_back(nameObject);
	this->dataContainer->instructions_->push_back(SET_DATA);
	this->dataContainer->instructions_->push_back(ENTER_SCOPE);

	this->dataContainer->instructions_->push_back(CALL_NAME);

	if (_node->U.Call.func_)
	{
		VISIT_(exprs, _node->U.Call.func_);
	}

	//if (isAssignFlag)
	//{
	//	this->dataContainer->data_->push_back(nameObject);
	//	this->dataContainer->instructions_->push_back(SET_DATA);
	//	this->dataContainer->instructions_->push_back(GET_DATA);
	//}

	this->dataContainer->instructions_->push_back(EXIT_SCOPE);

	//if (isAssignFlag)
	//{
	for (int i = 0; i < exitScopeAttr; i++)
	{
		this->dataContainer->instructions_->push_back(EXIT_SCOPE);
	}
	exitScopeAttr = 0;
	//}
}

AlifObject* Compiler::visit_return_(ExprNode* _node)
{
	AlifObject* a = VISIT_(exprs, _node);

	dataContainer->instructions_->push_back(RETURN_EXPR);

	return a;
}













AlifObject* Compiler::visit_exprs(ExprNode* _node)
{
	if (_node->type_ == VTObject)
	{
		return VISIT_(object, _node);
	}
	else if (_node->type_ == VTUnaryOp)
	{
		VISIT_(unaryOp, _node);
	}
	else if (_node->type_ == VTBinOp)
	{
		return VISIT_(binOp,_node);
	}
	else if (_node->type_ == VTAssign)
    {
		VISIT_(assign,_node);
    }
	else if (_node->type_ == VTAugAssign)
	{
		VISIT_(augAssign, _node);
	}
	else if (_node->type_ == VTAccess)
	{
		return VISIT_(access, _node);
	}
	else if (_node->type_ == VTAttr)
	{
		VISIT_(attr, _node);
	}
	else if (_node->type_ == VTCondExpr)
    {
		VISIT_(expr, _node);
    }
	else if (_node->type_ == VTList)
	{
		VISIT_(list, _node);
	}
	else if (_node->type_ == VTCall)
	{
		VISIT_(call, _node);
	}
	else if (_node->type_ == VTStop)
	{
		//if (inLoop)
		//{
			// التحقق اذا كان داخل حلقة ام لا
		//}
		VISIT_(stop);
	}
	else if (_node->type_ == VTContinue)
	{
		//if (inLoop)
		//{
			// التحقق اذا كان داخل حلقة ام لا
		//}
		VISIT_(continue_);
	}
}

AlifObject* Compiler::visit_stmts(StmtsNode* _node)
{
	if (_node->type_ == VTExpr)
	{
		return VISIT_(exprs, _node->U.Expr.expr_);
	}
	else if (_node->type_ == VTFor)
	{
		VISIT_(for_, _node);
	}
	else if (_node->type_ == VTWhile)
	{
		VISIT_(while_, _node);
	}
	else if (_node->type_ == VTIf)
	{
		VISIT_(if_, _node);
	}
	else if (_node->type_ == VTFunction)
	{
		VISIT_(function, _node);
	}
	else if (_node->type_ == VTClass)
	{
		VISIT_(class_, _node);
	}
	else if (_node->type_ == VTStmts)
	{
		for (StmtsNode* stmt : *_node->U.Stmts.stmts_)
		{
			VISIT_(stmts, stmt);
		}
	}
	else if (_node->type_ == VTReturn) 
	{
		if (_node->U.Return.returnExpr != nullptr)
		{
			AlifObject* a = VISIT_(return_, _node->U.Return.returnExpr);
			return a;
		}
		else {
			AlifObject* nullObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
			nullObject->objType = OTNone;
			nullObject->V.NoneObj.noneValue = L"عدم";

			dataContainer->data_->push_back(nullObject);
			dataContainer->instructions_->push_back(SET_DATA);

			dataContainer->instructions_->push_back(RETURN_EXPR);
			
			return nullObject;
		}
	}
}










// تجهيز الدوال الضمنية

void Compiler::visit_print()
{
	wchar_t* name = new wchar_t[5];
	int i = 0;
	for (wchar_t a : L"اطبع") // تم استخدام هذه الطريقة لانه سيتم عمل حذف لاحقا delete[]
	{
		name[i] = a;
		i++;
	}

	wcstr* pName = is_repeated(name);
	namesTable->create_scope(pName);
	namesTable->enter_scope(pName);

	AlifObject* paramContainer = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));

	Container* paramCont = (Container*)alifMemory->allocate(sizeof(Container));
	paramCont->instructions_ = new AlifArray<InstructionsType>;
	paramCont->data_ = new AlifArray<AlifObject*>;


	AlifObject* name_ = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	name_->V.NameObj.name_ = (wcstr*)0x01; // يجب مراجعة النظام
	paramCont->data_->push_back(name_);
	paramCont->instructions_->push_back(SET_DATA);
	paramCont->instructions_->push_back(GET_DATA);

	paramContainer->V.ContainerObj.container_ = paramCont;
	paramContainer->objType = OTContainer;


	Container* dataCont = (Container*)alifMemory->allocate(sizeof(Container));
	dataCont->instructions_ = new AlifArray<InstructionsType>;
	dataCont->data_ = new AlifArray<AlifObject*>;

	AlifObject* paramSize = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	paramSize->V.NumberObj.numberValue = 1;

	dataCont->data_->push_back(paramSize);
	dataCont->data_->push_back(paramContainer);


	dataCont->instructions_->push_back(PRINT_FUNC);


	AlifObject* containerObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	containerObject->objType = OTContainer;
	containerObject->V.ContainerObj.container_ = dataCont;

	namesTable->exit_scope();
	namesTable->create_name(pName, containerObject);
}