#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Parser.h"
#include "AlifCore_State.h"
#include "AlifCore_SymTable.h"






static AlifSTEntryObject* ste_new(AlifSymTable* _st, AlifObject* _name, BlockType_ _block,
	void* _key, AlifSourceLocation _loc) { // 88
	AlifSTEntryObject* ste_ = nullptr;
	AlifObject* k_ = nullptr;

	k_ = alifLong_fromVoidPtr(_key);
	if (k_ == nullptr)
		goto fail;
	ste_ = ALIFOBJECT_NEW(AlifSTEntryObject, &_alifSTEntryType_);
	if (ste_ == nullptr) {
		ALIF_DECREF(k_);
		goto fail;
	}
	ste_->table = _st;
	ste_->id = k_; 

	ste_->name = ALIF_NEWREF(_name);

	ste_->symbols = nullptr;
	ste_->varNames = nullptr;
	ste_->children = nullptr;

	ste_->directives = nullptr;
	ste_->mangledNames = nullptr;

	ste_->type = _block;
	ste_->scopeInfo = nullptr;

	ste_->nested = 0;
	ste_->varArgs = 0;
	ste_->varKeywords = 0;
	ste_->annotationsUsed = 0;
	ste_->loc = _loc;

	if (_st->cur != nullptr and
		(_st->cur->nested or
			alifST_isFunctionLike(_st->cur)))
		ste_->nested = 1;
	ste_->generator = 0;
	ste_->coroutine = 0;
	ste_->comprehension = AlifComprehensionType::No_Comprehension;
	ste_->returnsValue = 0;
	ste_->needsClassClosure = 0;
	ste_->compInlined = 0;
	ste_->compIterTarget = 0;
	ste_->canSeeClassScope = 0;
	ste_->compIterExpr = 0;
	ste_->needsClassDict = 0;
	ste_->annotationBlock = nullptr;

	ste_->symbols = alifDict_new();
	ste_->varNames = alifList_new(0);
	ste_->children = alifList_new(0);
	if (ste_->symbols == nullptr
		or ste_->varNames == nullptr
		or ste_->children == nullptr)
		goto fail;

	if (alifDict_setItem(_st->blocks, ste_->id, (AlifObject*)ste_) < 0)
		goto fail;

	return ste_;
fail:
	ALIF_XDECREF(ste_);
	return nullptr;
}

AlifTypeObject _alifSTEntryType_ = { // 192
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مدخل_جدول_الاسماء",
	.basicSize = sizeof(AlifSTEntryObject),
	.itemSize = 0,

	.getAttro = alifObject_genericGetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT,
};



static AlifIntT symtable_enterBlock(AlifSymTable*, AlifObject*, BlockType_, void*, AlifSourceLocation); // 234
static AlifIntT symtable_addDef(AlifSymTable*, AlifObject*, AlifIntT, AlifSourceLocation); // 261


static AlifSymTable* symtable_new() { // 367
	AlifSymTable* st_{};

	st_ = (AlifSymTable*)alifMem_dataAlloc(sizeof(AlifSymTable));
	if (st_ == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}

	st_->fileName = nullptr;
	st_->blocks = nullptr;

	if ((st_->stack = alifList_new(0)) == nullptr)
		goto fail;
	if ((st_->blocks = alifDict_new()) == nullptr)
		goto fail;
	st_->cur = nullptr;
	st_->private_ = nullptr;
	return st_;
fail:
	alifSymtable_free(st_);
	return nullptr;
}

AlifSymTable* alifSymtable_build(ModuleTy _mod, AlifObject* _filename,
	AlifFutureFeatures* _future) { // 394
	AlifSymTable* st_ = symtable_new();
	ASDLStmtSeq* seq_{};
	AlifSizeT i_{};
	AlifThread* tstate{};
	AlifIntT startingRecursionDepth{};

	if (st_ == nullptr)
		return nullptr;
	if (_filename == nullptr) {
		alifSymtable_free(st_);
		return nullptr;
	}
	st_->fileName = ALIF_NEWREF(_filename);
	st_->future = _future;

	tstate = _alifThread_get();
	if (!tstate) {
		//alifSymtable_free(st_);
		return nullptr;
	}
	AlifIntT recursionDepth = ALIFCPP_RECURSION_LIMIT - tstate->cppRecursionRemaining;
	startingRecursionDepth = recursionDepth;
	st_->recursionDepth = startingRecursionDepth;
	st_->recursionLimit = ALIFCPP_RECURSION_LIMIT;

	AlifSourceLocation loc0 = { 0, 0, 0, 0 };
	if (!symtable_enterBlock(st_, &ALIF_ID(top), BlockType_::Module_Block, (void*)_mod, loc0)) {
		//alifSymtable_free(st_);
		return nullptr;
	}

	st_->top = st_->cur;
	switch (_mod->type) {
	case ModK_::ModuleK:
		seq_ = _mod->V.module.body;
		for (i_ = 0; i_ < ASDL_SEQ_LEN(seq_); i_++)
			if (!symtable_visitStmt(st_, (StmtTy)ASDL_SEQ_GET(seq_, i_)))
				goto error;
		break;
	case ExpressionK:
		if (!symtable_visitExpr(st_, _mod->V.expression.body))
			goto error;
		break;
	case InteractiveK:
		seq_ = _mod->V.interactive.body;
		for (i_ = 0; i_ < ASDL_SEQ_LEN(seq_); i_++)
			if (!symtable_visitStmt(st_, (StmtTy)ASDL_SEQ_GET(seq_, i_)))
				goto error;
		break;
	case ModK_::FunctionK:
		//alifErr_setString(_alifExcRuntimeError_,
			//"this compiler does not handle FunctionTypes");
		goto error;
	}
	if (!symtable_exit_block(st_)) {
		alifSymtable_free(st_);
		return nullptr;
	}
	/* Check that the recursion depth counting balanced correctly */
	if (st_->recursionDepth != startingRecursionDepth) {
		//alifErr_format(_alifExcsystemError_,
			//"symtable analysis recursion depth mismatch (before=%d, after=%d)",
			//startingRecursionDepth, st_->recursionDepth);
		//alifSymtable_free(st_);
		return nullptr;
	}
	if (symtable_analyze(st_)) {
		return st_;
	}
	alifSymtable_free(st_);
	return nullptr;

error:
	(void)symtable_exitBlock(st_);
	alifSymtable_free(st_);
	return nullptr;
}


void alifSymtable_free(AlifSymTable* _st) { // 485
	ALIF_XDECREF(_st->fileName);
	ALIF_XDECREF(_st->blocks);
	ALIF_XDECREF(_st->stack);
	alifMem_dataFree((void*)_st);
}

AlifIntT alifST_isFunctionLike(AlifSTEntryObject* _ste) { // 558
	return _ste->type == BlockType_::Function_Block
		or _ste->type == BlockType_::Annotation_Block
		or _ste->type == BlockType_::Type_Variable_Block
		or _ste->type == BlockType_::Type_Alias_Block
		or _ste->type == BlockType_::Type_Parameters_Block;
}

static AlifIntT symtableEnter_existingBlock(AlifSymTable* _st, AlifSTEntryObject* _ste) { // 1381
	if (alifList_append(_st->stack, (AlifObject*)_ste) < 0) {
		return 0;
	}
	AlifSTEntryObject* prev = _st->cur;

	if (prev) {
		_ste->compIterExpr = prev->compIterExpr;
	}
	
	if (prev and prev->mangledNames != nullptr and _ste->type != BlockType_::Class_Block) {
		_ste->mangledNames = ALIF_NEWREF(prev->mangledNames);
	}

	_st->cur = _ste;

	if (_st->future->features & CO_FUTURE_ANNOTATIONS and _ste->type == BlockType_::Annotation_Block) {
		return 1;
	}

	if (_ste->type == BlockType_::Module_Block)
		_st->global = _st->cur->symbols;

	if (prev) {
		if (alifList_append(prev->children, (AlifObject*)_ste) < 0) {
			return 0;
		}
	}
	return 1;
}

static AlifIntT symtable_enterBlock(AlifSymTable* _st, AlifObject* _name, BlockType_ _block,
	void* _ast, AlifSourceLocation _loc) { // 1421
	AlifSTEntryObject* _ste = ste_new(_st, _name, _block, _ast, _loc);
	if (_ste == nullptr) return 0;

	AlifIntT result = symtableEnter_existingBlock(_st, _ste);
	ALIF_DECREF(_ste);
	if (_block == BlockType_::Annotation_Block
		or _block == BlockType_::Type_Variable_Block
		or _block == BlockType_::Type_Alias_Block) {
		if (!symtable_addDef(_st, &ALIF_STR(format), DEF_PARAM, _loc)) {
			return 0;
		}
		if (!symtable_addDef(_st, &ALIF_STR(format), USE, _loc)) {
			return 0;
		}
	}
	return result;
}

static AlifIntT symtable_addDefHelper(AlifSymTable* _st, AlifObject* _name, AlifIntT _flag, AlifSTEntryObject* _ste,
	AlifSourceLocation _loc) { // 1463
	AlifObject* o_{};
	AlifObject* dict{};
	long val_{};
	AlifObject* mangled = alif_maybeMangle(_st->private_, _st->cur, _name);

	if (!mangled)
		return 0;
	dict = _ste->symbols;
	if ((o_ = alifDict_getItemWithError(dict, mangled))) {
		val_ = alifLong_asLong(o_);
		//if (val_ == -1 and alifErr_occurred()) {
			//goto error;
		//}
		if ((_flag & DEF_PARAM) and (val_ & DEF_PARAM)) {
			/* Is it better to use 'mangled' or '_name' here? */
			//alifErr_format(_alifExcSyntaxError_, DUPLICATE_ARGUMENT, _name);
			//SET_ERROR_LOCATION(_st->fileName, _loc);
			goto error;
		}
		if ((_flag & DEF_TYPE_PARAM) and (val_ & DEF_TYPE_PARAM)) {
			//alifErr_format(_alifExcSyntaxError_, DUPLICATE_TYPE_PARAM, _name);
			//SET_ERROR_LOCATION(_st->fileName, _loc);
			goto error;
		}
		val_ |= _flag;
	}
	//else if (alifErr_occurred()) {
		//goto error;
	//}
	else {
		val_ = _flag;
	}
	if (_ste->compIterTarget) {
		if (val_ & (DEF_GLOBAL | DEF_NONLOCAL)) {
			//alifErr_format(_alifExcSyntaxError_,
				//NAMED_EXPR_COMP_INNER_LOOP_CONFLICT, _name);
			//SET_ERROR_LOCATION(_st->fileName, _loc);
			goto error;
		}
		val_ |= DEF_COMP_ITER;
	}
	o_ = alifLong_fromLong(val_);
	if (o_ == nullptr)
		goto error;
	if (alifDict_setItem(dict, mangled, o_) < 0) {
		ALIF_DECREF(o_);
		goto error;
	}
	ALIF_DECREF(o_);

	if (_flag & DEF_PARAM) {
		if (alifList_append(_ste->varNames, mangled) < 0)
			goto error;
	}
	else if (_flag & DEF_GLOBAL) {
		/* XXX need to update DEF_GLOBAL for other flags too;
		   perhaps only DEF_FREE_GLOBAL */
		val_ = 0;
		if ((o_ = alifDict_getItemWithError(_st->global, mangled))) {
			val_ = alifLong_asLong(o_);
			//if (val_ == -1 and alifErr_occurred()) {
				//goto error;
			//}
		}
		//else if (alifErr_occurred()) {
			//goto error;
		//}
		val_ |= _flag;
		o_ = alifLong_fromLong(val_);
		if (o_ == nullptr)
			goto error;
		if (alifDict_setItem(_st->global, mangled, o_) < 0) {
			ALIF_DECREF(o_);
			goto error;
		}
		ALIF_DECREF(o_);
	}
	ALIF_DECREF(mangled);
	return 1;

error:
	ALIF_DECREF(mangled);
	return 0;
}

static AlifIntT check_name(AlifSymTable* _st, AlifObject* _name, AlifSourceLocation _loc,
	ExprContext_ _ctx) { // 1556
	//if (_ctx == Store and alifUStr_equalToASCIIString(name, "__debug__")) {
		//alifErr_setString(_alifExcSyntaxError_, "cannot assign to __debug__");
		//SET_ERROR_LOCATION(_st->fileName, _loc);
		//return 0;
	//}
	//if (_ctx == Del and alifUStr_equalToASCIIString(name, "__debug__")) {
		//alifErr_setString(_alifExcSyntaxError_, "cannot delete __debug__");
		//SET_ERROR_LOCATION(_st->fileName, _loc);
		//return 0;
	//}
	return 1;
}

static AlifIntT symtable_addDefCtx(AlifSymTable* _st, AlifObject* _name, AlifIntT _flag,
	AlifSourceLocation _loc, ExprContext_ _ctx) { // 1600
	AlifIntT writeMask = DEF_PARAM | DEF_LOCAL | DEF_IMPORT;
	if ((_flag & writeMask) and !check_name(_st, _name, _loc, _ctx)) {
		return 0;
	}
	if ((_flag & DEF_TYPE_PARAM) and _st->cur->mangledNames != nullptr) {
		if (alifSet_add(_st->cur->mangledNames, _name) < 0) {
			return 0;
		}
	}
	return symtable_addDefHelper(_st, _name, _flag, _st->cur, _loc);
}

static AlifIntT symtable_addDef(AlifSymTable* _st, AlifObject* _name, AlifIntT _flag,
	AlifSourceLocation _loc) { // 1616
	return symtable_addDefCtx(_st, _name, _flag, _loc,
		_flag == USE ? ExprContext_::Load : ExprContext_::Store);
}

AlifObject* alif_maybeMangle(AlifObject* _privateObj, AlifSTEntryObject* ste, AlifObject* name) { // 3070
	if (ste->mangledNames != nullptr) {
		AlifIntT result = alifSet_contains(ste->mangledNames, name);
		if (result < 0) {
			return nullptr;
		}
		if (result == 0) {
			return ALIF_NEWREF(name);
		}
	}
	return alif_mangle(_privateObj, name);
}

AlifObject* alif_mangle(AlifObject* _privateObj, AlifObject* _ident) { // 3090
	if (_privateObj == nullptr or !ALIFUSTR_CHECK(_privateObj) ||
		ALIFUSTR_READ_CHAR(_ident, 0) != '_' or
		ALIFUSTR_READ_CHAR(_ident, 1) != '_') {
		return ALIF_NEWREF(_ident);
	}
	AlifUSizeT nLen = ALIFUSTR_GET_LENGTH(_ident);
	AlifUSizeT pLen = ALIFUSTR_GET_LENGTH(_privateObj);
	if ((ALIFUSTR_READ_CHAR(_ident, nLen - 1) == '_' and
		ALIFUSTR_READ_CHAR(_ident, nLen - 2) == '_') or
		alifUStr_findChar(_ident, '.', 0, nLen, 1) != -1) {
		return ALIF_NEWREF(_ident); /* Don't mangle __whatever__ */
	}
	AlifUSizeT iPriv = 0;
	while (ALIFUSTR_READ_CHAR(_privateObj, iPriv) == '_') {
		iPriv++;
	}
	if (iPriv == pLen) {
		return ALIF_NEWREF(_ident); /* Don't mangle if class is just underscores */
	}
	pLen -= iPriv;

	if (pLen + nLen >= ALIF_SIZET_MAX - 1) {
		//alifErr_setString(_alifExcOverflowError_,
			//"private identifier too large to be mangled");
		return nullptr;
	}

	AlifUCS4 maxChar = ALIFUSTR_MAX_CHAR_VALUE(_ident);
	if (ALIFUSTR_MAX_CHAR_VALUE(_privateObj) > maxChar) {
		maxChar = ALIFUSTR_MAX_CHAR_VALUE(_privateObj);
	}

	AlifObject* result = alifUStr_new(1 + nLen + pLen, maxChar);
	if (!result) {
		return nullptr;
	}
	alifUStr_write(ALIFUSTR_KIND(result), ALIFUSTR_DATA(result), 0, '_');
	if (alifUStr_copyCharacters(result, 1, _privateObj, iPriv, pLen) < 0) {
		ALIF_DECREF(result);
		return nullptr;
	}
	if (alifUStr_copyCharacters(result, pLen + 1, _ident, 0, nLen) < 0) {
		ALIF_DECREF(result);
		return nullptr;
	}
	return result;
}
