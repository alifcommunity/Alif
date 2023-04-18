#include "Compiler.h"


SymbolTable* symTable = new SymbolTable;


Compiler::Compiler(std::vector<StmtsNode*>* _statements, AlifMemory* _alifMemory) :
	statements_(_statements), alifMemory(_alifMemory) {}

bool a = true;
void Compiler::compile_file() 
{
	for (StmtsNode* node_ : *statements_)
	{
		dataContainer = (Container*)alifMemory->allocate(sizeof(Container));
		dataContainer->instructions_ = new AlifArray<InstructionsType>;
		dataContainer->data_ = new AlifArray<AlifObject*>;

		VISIT_(stmts, node_); // تم شرح طريقة الاستدعاء في ملف compiler.h

		if (a) // يقوم بالتحقق في حال كان تعريف دالة لا يجب ان يتم إضافة الحاوية الى مصفوفة الحاويات
		{
			containers_.push_back(dataContainer);
		}
		else
		{
			a = true;
		}
	}
}


AlifObject* Compiler::visit_object(ExprNode* _node)
{
	dataContainer->instructions_->push_back(SET_DATA);
	dataContainer->data_->push_back(_node->U.Object.value_);
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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
		if (left->objType == OTNumber)
		{
			if (right->objType == OTNumber)
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


void Compiler::visit_assign(ExprNode* _node)
{
	for (AlifObject* i : *_node->U.NameAssign.name_)
	{
		VISIT_(exprs,_node->U.NameAssign.value_);

		dataContainer->data_->push_back(i);
		dataContainer->instructions_->push_back(SET_DATA); // خزن الاسم في المكدس

		dataContainer->instructions_->push_back(STORE_NAME);
	}
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

	dataContainer->instructions_->push_back(GET_DATA);

	return _node->U.NameAccess.name_;
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

bool b = true;
AlifObject* jTAdress = new AlifObject(); // Address of instructions
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

	//AlifObject* jTAdress = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // Address of instructions
	jTAdress->objType = OTNumber;
	this->dataContainer->data_->push_back(jTAdress);
	this->dataContainer->instructions_->push_back(SET_DATA);

	//AlifObject* jTDataAddress = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // Address of data
	jTDataAddress->objType = OTNumber;
	this->dataContainer->data_->push_back(jTDataAddress);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(JUMP_TO);

	dataAddress->V.NumberObj.numberValue = this->dataContainer->data_->size();
	jumpAddress->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;


	if (b) // يتحقق هل المستدعى "اذا" ام "واذا" ففي حال الاولى يسمح بالمرور والثانية لا يسمح
	{
		b = false;

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
		endFor->V.NumberObj.numberValue = _node->U.For.args_->at(0)->U.Object.value_->V.NumberObj.numberValue;
		stepFor->V.NumberObj.numberValue = 1;
	}
	else if (_node->U.For.args_->size() == 2)
	{
		startFor->V.NumberObj.numberValue = _node->U.For.args_->at(0)->U.Object.value_->V.NumberObj.numberValue;
		endFor->V.NumberObj.numberValue = _node->U.For.args_->at(1)->U.Object.value_->V.NumberObj.numberValue;
		stepFor->V.NumberObj.numberValue = 1;
	}
	else 
	{
		startFor->V.NumberObj.numberValue = _node->U.For.args_->at(0)->U.Object.value_->V.NumberObj.numberValue;
		endFor->V.NumberObj.numberValue = _node->U.For.args_->at(1)->U.Object.value_->V.NumberObj.numberValue;
		stepFor->V.NumberObj.numberValue = _node->U.For.args_->at(2)->U.Object.value_->V.NumberObj.numberValue;
	}
	dataContainer->data_->push_back(startFor);
	dataContainer->instructions_->push_back(SET_DATA);

	dataContainer->data_->push_back(_node->U.For.itrName); // varName
	dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(STORE_NAME);


	dataContainer->data_->push_back(startFor); // startFor
	dataContainer->instructions_->push_back(SET_DATA);

	dataContainer->data_->push_back(endFor); // endFor
	this->dataContainer->instructions_->push_back(SET_DATA);

	dataContainer->data_->push_back(stepFor); // stepFor
	dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(FOR_ITER);


	AlifObject* jumpAddress = (AlifObject*)alifMemory->allocate(sizeof(AlifObject)); // address
	jumpAddress->objType = OTNumber;
	jumpAddress->V.NumberObj.numberValue = this->dataContainer->instructions_->size() - 1;


	VISIT_(stmts, _node->U.For.block_);


	dataContainer->data_->push_back(_node->U.For.itrName); // varName
	dataContainer->instructions_->push_back(SET_DATA);

	dataContainer->data_->push_back(jumpAddress);
	this->dataContainer->instructions_->push_back(SET_DATA); // set address after for jumb


	this->dataContainer->instructions_->push_back(JUMP_FOR);
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


	dataAddressJ->V.NumberObj.numberValue = this->dataContainer->data_->size() + 2;
	jumpAddress->V.NumberObj.numberValue = this->dataContainer->instructions_->size() + 2; // this size most be greater than size of instructions to insure break the loop

	this->dataContainer->data_->push_back(jumpTo);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->data_->push_back(dataAddressJT);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(JUMP_TO); // this command jump to begin of the "Container"
}


void Compiler::visit_function(StmtsNode* _node)
{
	a = false;

	dataContainer = new Container;
	dataContainer->instructions_ = new AlifArray<InstructionsType>;
	dataContainer->data_ = new AlifArray<AlifObject*>;

	VISIT_(stmts, _node->U.FunctionDef.body_);

	AlifObject* containerObject = (AlifObject*)alifMemory->allocate(sizeof(AlifObject));
	containerObject->V.ContainerObj.container_ = dataContainer;

	symTable->add_symbol(*_node->U.FunctionDef.name_->V.NameObj.name_, containerObject);

	//dataContainer->data_->clear();
	//dataContainer->instructions_->clear();
}


void Compiler::visit_call(ExprNode* _node)
{
	this->dataContainer->data_->push_back(_node->U.Call.name_->U.Object.value_);
	this->dataContainer->instructions_->push_back(SET_DATA);

	this->dataContainer->instructions_->push_back(GET_DATA);

	this->dataContainer->instructions_->push_back(CALL_NAME);
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
	else if (_node->type_ == VTStmts)
	{
		for (StmtsNode* stmt : *_node->U.Stmts.stmts_)
		{
			//if (!a and stmt->type_ != VTFunction)
			//{
				VISIT_(stmts, stmt);
			//}
		}
	}
}