#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Parser.h"
#include "AlifCore_State.h"
#include "AlifCore_SymTable.h"






static AlifSTEntryObject* ste_new(AlifSymTable* _st, AlifObject* _name, BlockType_ _block,
	void* _key, AlifSourceLocation _loc) { // 88
	AlifSTEntryObject* ste_ = nullptr;
	AlifObject* k = nullptr;

	k = alifLong_fromVoidPtr(_key);
	if (k == nullptr)
		goto fail;
	ste_ = ALIFOBJECT_NEW(AlifSTEntryObject, &_alifSTEntryType_);
	if (ste_ == nullptr) {
		ALIF_DECREF(k);
		goto fail;
	}
	ste_->table = _st;
	ste_->id = k; 

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
	if (_block == BlockType_::Annotation_Block or _block == BlockType_::Type_Variable_Block or _block == BlockType_::Type_Alias_Block) {
		if (!symtable_addDef(_st, &ALIF_STR(format), DEF_PARAM, _loc)) {
			return 0;
		}
		if (!symtable_addDef(_st, &ALIF_STR(format), USE, _loc)) {
			return 0;
		}
	}
	return result;
}

