#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Parser.h"
#include "AlifCore_State.h"
#include "AlifCore_SymTable.h"



#define LOCATION(x) SRC_LOCATION_FROM_AST(x) // 79


#define IS_ASYNC_DEF(_st) ((_st)->cur->type == BlockType_::Function_Block and (_st)->cur->coroutine)


static SymTableEntry* ste_new(AlifSymTable* _st, AlifObject* _name, BlockType_ _block,
	void* _key, AlifSourceLocation _loc) { // 88
	SymTableEntry* ste_ = nullptr;
	AlifObject* k_ = nullptr;

	k_ = alifLong_fromVoidPtr(_key);
	if (k_ == nullptr)
		goto fail;
	ste_ = ALIFOBJECT_NEW(SymTableEntry, &_alifSTEntryType_);
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




static void ste_dealloc(SymTableEntry* _ste) { // 163
	_ste->table = nullptr;
	ALIF_XDECREF(_ste->id);
	ALIF_XDECREF(_ste->name);
	ALIF_XDECREF(_ste->symbols);
	ALIF_XDECREF(_ste->varNames);
	ALIF_XDECREF(_ste->children);
	ALIF_XDECREF(_ste->directives);
	ALIF_XDECREF(_ste->annotationBlock);
	ALIF_XDECREF(_ste->mangledNames);
	alifMem_objFree(_ste);
}




AlifTypeObject _alifSTEntryType_ = { // 192
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مدخل_جدول_الاسماء",
	.basicSize = sizeof(SymTableEntry),
	.dealloc = (Destructor)ste_dealloc,

	.getAttro = alifObject_genericGetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT,
};


static AlifIntT symtable_analyze(AlifSymTable*); // 233
static AlifIntT symtable_enterBlock(AlifSymTable*, AlifObject*, BlockType_, void*, AlifSourceLocation); // 234
static AlifIntT symtable_exitBlock(AlifSymTable*); // 236
static AlifIntT symtable_visitStmt(AlifSymTable* , StmtTy ); // 237
static AlifIntT symtable_visitExpr(AlifSymTable* , ExprTy ); // 238
static AlifIntT symtable_visitTypeParam(AlifSymTable*, TypeParamTy); // 239
static AlifIntT symtable_visitListComp(AlifSymTable*, ExprTy); // 241
static AlifIntT symtable_visitArguments(AlifSymTable*, ArgumentsTy); // 244
static AlifIntT symtable_visitAlias(AlifSymTable*, AliasTy); // 246
static AlifIntT symtable_visitKeyword(AlifSymTable*, KeywordTy); // 248
static AlifIntT symtable_visitAnnotations(AlifSymTable*, StmtTy, ArgumentsTy, ExprTy, SymTableEntry*); // 253
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
		alifSymtable_free(st_);
		return nullptr;
	}
	AlifIntT recursionDepth = ALIFCPP_RECURSION_LIMIT - tstate->cppRecursionRemaining;
	startingRecursionDepth = recursionDepth;
	st_->recursionDepth = startingRecursionDepth;
	st_->recursionLimit = ALIFCPP_RECURSION_LIMIT;

	AlifSourceLocation loc0 = { 0, 0, 0, 0 };
	if (!symtable_enterBlock(st_, &ALIF_ID(Top), BlockType_::Module_Block, (void*)_mod, loc0)) {
		alifSymtable_free(st_);
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
	if (!symtable_exitBlock(st_)) {
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


SymTableEntry* _alifSymtable_lookup(AlifSymTable* _st, void* _key) { // 493
	AlifObject* k{}, * v{};

	k = alifLong_fromVoidPtr(_key);
	if (k == nullptr) return nullptr;
	if (alifDict_getItemRef(_st->blocks, k, &v) == 0) {
		//alifErr_setString(_alifExcKeyError_,
		//	"unknown symbol table entry");
	}
	ALIF_DECREF(k);

	return (SymTableEntry*)v;
}


long _alifST_getSymbol(SymTableEntry* _ste, AlifObject* _name) { // 527
	AlifObject* v_{};
	if (alifDict_getItemRef(_ste->symbols, _name, &v_) < 0) {
		return -1;
	}
	if (!v_) {
		return 0;
	}
	long symbol = alifLong_asLong(v_);
	ALIF_DECREF(v_);
	if (symbol < 0) {
		//if (!alifErr_occurred()) {
			//alifErr_setString(_alifExcSystemError_, "invalid symbol");
		//}
		return -1;
	}
	return symbol;
}

AlifIntT alifST_getScope(SymTableEntry* _ste, AlifObject* _name) { // 548
	long symbol = _alifST_getSymbol(_ste, _name);
	if (symbol < 0) {
		return -1;
	}
	return SYMBOL_TO_SCOPE(symbol);
}

AlifIntT alifST_isFunctionLike(SymTableEntry* _ste) { // 558
	return _ste->type == BlockType_::Function_Block
		or _ste->type == BlockType_::Annotation_Block
		or _ste->type == BlockType_::Type_Variable_Block
		or _ste->type == BlockType_::Type_Alias_Block
		or _ste->type == BlockType_::Type_Parameters_Block;
}

static AlifIntT error_atDirective(SymTableEntry* _ste, AlifObject* _name) { // 568
	AlifSizeT i{};
	AlifObject* data{};
	for (i = 0; i < ALIFLIST_GET_SIZE(_ste->directives); i++) {
		data = ALIFLIST_GET_ITEM(_ste->directives, i);
		if (alifUStr_compare(ALIFTUPLE_GET_ITEM(data, 0), _name) == 0) {
			//alifErr_rangedSyntaxLocationObject(_ste->table->filename,
				//alifLong_asLong(ALIFTUPLE_GET_ITEM(data, 1)),
				//alifLong_asLong(ALIFTUPLE_GET_ITEM(data, 2)) + 1,
				//alifLong_asLong(ALIFTUPLE_GET_ITEM(data, 3)),
				//alifLong_asLong(ALIFTUPLE_GET_ITEM(data, 4)) + 1);

			return 0;
		}
	}
	//alifErr_setString(_alifExcRuntimeError_,
		//"BUG: internal directive bookkeeping broken");
	return 0;
}

// 638
#define SET_SCOPE(_dict, _name, _i) \
    do { \
        AlifObject *o_ = alifLong_fromLong(_i); \
        if (!o_) \
            return 0; \
        if (alifDict_setItem((_dict), (_name), o_) < 0) { \
            ALIF_DECREF(o_); \
            return 0; \
        } \
        ALIF_DECREF(o_); \
    } while(0)

static AlifIntT analyze_name(SymTableEntry* _ste, AlifObject* _scopes, AlifObject* _name, long _flags,
	AlifObject* _bound, AlifObject* _local, AlifObject* _free,
	AlifObject* _global, AlifObject* _typeParams, SymTableEntry* _classEntry) { // 658
	AlifIntT contains{};
	if (_flags & DEF_GLOBAL) {
		if (_flags & DEF_NONLOCAL) {
			//alifErr_format(_alifExcSyntaxError_,
				//"name '%U' is nonlocal and global",
				//_name);
			return error_atDirective(_ste, _name);
		}
		SET_SCOPE(_scopes, _name, GLOBAL_EXPLICIT);
		if (alifSet_add(_global, _name) < 0)
			return 0;
		if (_bound and (alifSet_discard(_bound, _name) < 0))
			return 0;
		return 1;
	}
	if (_flags & DEF_NONLOCAL) {
		if (!_bound) {
			//alifErr_format(_alifExcSyntaxError_,
				//"nonlocal declaration not allowed at module level");
			return error_atDirective(_ste, _name);
		}
		contains = alifSet_contains(_bound, _name);
		if (contains < 0) {
			return 0;
		}
		if (!contains) {
			//alifErr_format(_alifExcSyntaxError_,
				//"no binding for nonlocal '%U' found",
				//name);

			return error_atDirective(_ste, _name);
		}
		contains = alifSet_contains(_typeParams, _name);
		if (contains < 0) {
			return 0;
		}
		if (contains) {
			//alifErr_format(_alifExcSyntaxError_,
				//"nonlocal binding not allowed for type parameter '%U'",
				//name);
			return error_atDirective(_ste, _name);
		}
		SET_SCOPE(_scopes, _name, FREE);
		return alifSet_add(_free, _name) >= 0;
	}
	if (_flags & DEF_BOUND) {
		SET_SCOPE(_scopes, _name, LOCAL);
		if (alifSet_add(_local, _name) < 0)
			return 0;
		if (alifSet_discard(_global, _name) < 0)
			return 0;
		if (_flags & DEF_TYPE_PARAM) {
			if (alifSet_add(_typeParams, _name) < 0)
				return 0;
		}
		else {
			if (alifSet_discard(_typeParams, _name) < 0)
				return 0;
		}
		return 1;
	}
	if (_classEntry != nullptr) {
		long class_flags = _alifST_getSymbol(_classEntry, _name);
		if (class_flags < 0) {
			return 0;
		}
		if (class_flags & DEF_GLOBAL) {
			SET_SCOPE(_scopes, _name, GLOBAL_EXPLICIT);
			return 1;
		}
		else if ((class_flags & DEF_BOUND) and !(class_flags & DEF_NONLOCAL)) {
			SET_SCOPE(_scopes, _name, GLOBAL_IMPLICIT);
			return 1;
		}
	}
	if (_bound) {
		contains = alifSet_contains(_bound, _name);
		if (contains < 0) {
			return 0;
		}
		if (contains) {
			SET_SCOPE(_scopes, _name, FREE);
			return alifSet_add(_free, _name) >= 0;
		}
	}
	if (_global) {
		contains = alifSet_contains(_global, _name);
		if (contains < 0) {
			return 0;
		}
		if (contains) {
			SET_SCOPE(_scopes, _name, GLOBAL_IMPLICIT);
			return 1;
		}
	}
	SET_SCOPE(_scopes, _name, GLOBAL_IMPLICIT);
	return 1;
}

static AlifIntT isFree_inAnyChild(SymTableEntry* _entry, AlifObject* _key) { // 777
	for (AlifSizeT i = 0; i < ALIFLIST_GET_SIZE(_entry->children); i++) {
		SymTableEntry* childSte = (SymTableEntry*)ALIFLIST_GET_ITEM(
			_entry->children, i);
		long scope = alifST_getScope(childSte, _key);
		if (scope < 0) {
			return -1;
		}
		if (scope == FREE) {
			return 1;
		}
	}
	return 0;
}

static AlifIntT inline_comprehension(SymTableEntry* _ste, SymTableEntry* _comp,
	AlifObject* _scopes, AlifObject* _compFree,
	AlifObject* _inlinedCells) { // 794
	AlifObject* k_{}, * v_{};
	AlifSizeT pos_ = 0;
	AlifIntT removeDunderClass = 0;

	while (alifDict_next(_comp->symbols, &pos_, &k_, &v_)) {
		long compFlags = alifLong_asLong(v_);
		if (compFlags == -1 and alifErr_occurred()) {
			return 0;
		}
		if (compFlags & DEF_PARAM) {
			continue;
		}
		AlifIntT scope = SYMBOL_TO_SCOPE(compFlags);
		AlifIntT onlyFlags = compFlags & ((1 << SCOPE_OFFSET) - 1);
		if (scope == CELL or onlyFlags & DEF_COMP_CELL) {
			if (alifSet_add(_inlinedCells, k_) < 0) {
				return 0;
			}
		}
		AlifObject* existing = alifDict_getItemWithError(_ste->symbols, k_);
		if (existing == nullptr and alifErr_occurred()) {
			return 0;
		}
		if (scope == FREE and _ste->type == BlockType_::Class_Block and
			alifUStr_equalToASCIIString(k_, "__class__")) {
			scope = GLOBAL_IMPLICIT;
			if (alifSet_discard(_compFree, k_) < 0) {
				return 0;
			}
			removeDunderClass = 1;
		}
		if (!existing) {
			AlifObject* vFlags = alifLong_fromLong(onlyFlags);
			if (vFlags == nullptr) {
				return 0;
			}
			AlifIntT ok_ = alifDict_setItem(_ste->symbols, k_, vFlags);
			ALIF_DECREF(vFlags);
			if (ok_ < 0) {
				return 0;
			}
			SET_SCOPE(_scopes, k_, scope);
		}
		else {
			long flags = alifLong_asLong(existing);
			if (flags == -1 /*and alifErr_occurred()*/) {
				return 0;
			}
			if ((flags & DEF_BOUND) and _ste->type != BlockType_::Class_Block) {
				AlifIntT ok_ = isFree_inAnyChild(_comp, k_);
				if (ok_ < 0) {
					return 0;
				}
				if (!ok_) {
					if (alifSet_discard(_compFree, k_) < 0) {
						return 0;
					}
				}
			}
		}
	}
	if (removeDunderClass and alifDict_delItemString(_comp->symbols, "__class__") < 0) {
		return 0;
	}
	return 1;
}

static AlifIntT analyze_cells(AlifObject* _scopes, AlifObject* _free, AlifObject* _inlinedCells) { // 885
	AlifObject* name{}, * v_{}, * vCell{};
	AlifIntT success = 0;
	AlifSizeT pos_ = 0;

	vCell = alifLong_fromLong(CELL);
	if (!vCell)
		return 0;
	while (alifDict_next(_scopes, &pos_, &name, &v_)) {
		long scope = alifLong_asLong(v_);
		if (scope == -1 /*and alifErr_occurred()*/) {
			goto error;
		}
		if (scope != LOCAL)
			continue;
		AlifIntT contains = alifSet_contains(_free, name);
		if (contains < 0) {
			goto error;
		}
		if (!contains) {
			contains = alifSet_contains(_inlinedCells, name);
			if (contains < 0) {
				goto error;
			}
			if (!contains) {
				continue;
			}
		}
		if (alifDict_setItem(_scopes, name, vCell) < 0)
			goto error;
		if (alifSet_discard(_free, name) < 0)
			goto error;
	}
	success = 1;
error:
	ALIF_DECREF(vCell);
	return success;
}

static AlifIntT drop_classFree(SymTableEntry* _ste, AlifObject* _free) { // 930
	AlifIntT res_{};
	res_ = alifSet_discard(_free, &ALIF_ID(__class__));
	if (res_ < 0)
		return 0;
	if (res_)
		_ste->needsClassClosure = 1;
	res_ = alifSet_discard(_free, &ALIF_ID(__classDict__));
	if (res_ < 0)
		return 0;
	if (res_)
		_ste->needsClassDict = 1;
	return 1;
}

static AlifIntT update_symbols(AlifObject* _symbols, AlifObject* _scopes,
	AlifObject* _bound, AlifObject* _free,
	AlifObject* _inlinedCells, AlifIntT _classFlag) { // 951
	AlifObject* name = nullptr, * itr_ = nullptr;
	AlifObject* v_ = nullptr, * vScope = nullptr, * vNew = nullptr, * vFree = nullptr;
	AlifSizeT pos_ = 0;

	while (alifDict_next(_symbols, &pos_, &name, &v_)) {
		long flags = alifLong_asLong(v_);
		if (flags == -1 /*and alifErr_occurred()*/) {
			return 0;
		}
		AlifIntT contains = alifSet_contains(_inlinedCells, name);
		if (contains < 0) {
			return 0;
		}
		if (contains) {
			flags |= DEF_COMP_CELL;
		}
		if (alifDict_getItemRef(_scopes, name, &vScope) < 0) {
			return 0;
		}
		if (!vScope) {
			//alifErr_setObject(_alifExcKeyError_, name);
			return 0;
		}
		long scope = alifLong_asLong(vScope);
		ALIF_DECREF(vScope);
		if (scope == -1 /*and alifErr_occurred()*/) {
			return 0;
		}
		flags |= (scope << SCOPE_OFFSET);
		vNew = alifLong_fromLong(flags);
		if (!vNew)
			return 0;
		if (alifDict_setItem(_symbols, name, vNew) < 0) {
			ALIF_DECREF(vNew);
			return 0;
		}
		ALIF_DECREF(vNew);
	}

	vFree = alifLong_fromLong(FREE << SCOPE_OFFSET);
	if (!vFree)
		return 0;

	itr_ = alifObject_getIter(_free);
	if (itr_ == nullptr) {
		ALIF_DECREF(vFree);
		return 0;
	}

	while ((name = alifIter_next(itr_))) {
		v_ = alifDict_getItemWithError(_symbols, name);

		if (v_) {

			if (_classFlag) {
				long flags = alifLong_asLong(v_);
				if (flags == -1 /*and alifErr_occurred()*/) {
					goto error;
				}
				flags |= DEF_FREE_CLASS;
				vNew = alifLong_fromLong(flags);
				if (!vNew) {
					goto error;
				}
				if (alifDict_setItem(_symbols, name, vNew) < 0) {
					ALIF_DECREF(vNew);
					goto error;
				}
				ALIF_DECREF(vNew);
			}
			ALIF_DECREF(name);
			continue;
		}
		//else if (alifErr_occurred()) {
			//goto error;
		//}
		if (_bound) {
			AlifIntT contains = alifSet_contains(_bound, name);
			if (contains < 0) {
				goto error;
			}
			if (!contains) {
				ALIF_DECREF(name);
				continue;       /* it's a global */
			}
		}
		if (alifDict_setItem(_symbols, name, vFree) < 0) {
			goto error;
		}
		ALIF_DECREF(name);
	}

	//if (alifErr_occurred()) {
		//goto error;
	//}

	ALIF_DECREF(itr_);
	ALIF_DECREF(vFree);
	return 1;
error:
	ALIF_XDECREF(vFree);
	ALIF_XDECREF(itr_);
	ALIF_XDECREF(name);
	return 0;
}


static AlifIntT analyze_childBlock(SymTableEntry* , AlifObject* , AlifObject* ,
	AlifObject* , AlifObject* , SymTableEntry* , AlifObject** ); // 1092


static AlifIntT analyze_block(SymTableEntry* _ste, AlifObject* _bound, AlifObject* _free,
	AlifObject* _global, AlifObject* _typeParams,
	SymTableEntry* _classEntry) { // 1097
	AlifObject* name{}, * v_{}, * local = nullptr, * scopes = nullptr, * newBound = nullptr;
	AlifObject* newGlobal = nullptr, * newFree = nullptr, * inlinedCells = nullptr;
	AlifObject* temp{};
	AlifIntT success = 0;
	AlifSizeT i_{}, pos_ = 0;

	local = alifSet_new(nullptr);  
	if (!local)
		goto error;
	scopes = alifDict_new();
	if (!scopes)
		goto error;

	newGlobal = alifSet_new(nullptr);
	if (!newGlobal)
		goto error;
	newFree = alifSet_new(nullptr);
	if (!newFree)
		goto error;
	newBound = alifSet_new(nullptr);
	if (!newBound)
		goto error;
	inlinedCells = alifSet_new(nullptr);
	if (!inlinedCells)
		goto error;

	if (_ste->type == BlockType_::Class_Block) {
		temp = alifNumber_inPlaceOr(newGlobal, _global);
		if (!temp)
			goto error;
		ALIF_DECREF(temp);
		if (_bound) {
			temp = alifNumber_inPlaceOr(newBound, _bound);
			if (!temp)
				goto error;
			ALIF_DECREF(temp);
		}
	}

	while (alifDict_next(_ste->symbols, &pos_, &name, &v_)) {
		long flags = alifLong_asLong(v_);
		if (flags == -1 /*and alifErr_occurred()*/) {
			goto error;
		}
		if (!analyze_name(_ste, scopes, name, flags,
			_bound, local, _free, _global, _typeParams, _classEntry))
			goto error;
	}

	if (_ste->type != BlockType_::Class_Block) {
		if (alifST_isFunctionLike(_ste)) {
			temp = alifNumber_inPlaceOr(newBound, local);
			if (!temp)
				goto error;
			ALIF_DECREF(temp);
		}
		if (_bound) {
			temp = alifNumber_inPlaceOr(newBound, _bound);
			if (!temp)
				goto error;
			ALIF_DECREF(temp);
		}
		temp = alifNumber_inPlaceOr(newGlobal, _global);
		if (!temp)
			goto error;
		ALIF_DECREF(temp);
	}
	else {
		if (alifSet_add(newBound, &ALIF_ID(__class__)) < 0)
			goto error;
		if (alifSet_add(newBound, &ALIF_ID(__classDict__)) < 0)
			goto error;
	}

	for (i_ = 0; i_ < ALIFLIST_GET_SIZE(_ste->children); ++i_) {
		AlifObject* childFree = nullptr;
		AlifObject* c_ = ALIFLIST_GET_ITEM(_ste->children, i_);
		SymTableEntry* entry = (SymTableEntry*)c_;

		SymTableEntry* newClassEntry = nullptr;
		if (entry->canSeeClassScope) {
			if (_ste->type == BlockType_::Class_Block) {
				newClassEntry = _ste;
			}
			else if (_classEntry) {
				newClassEntry = _classEntry;
			}
		}

		AlifIntT inlineComp =
			entry->comprehension and
			!entry->generator and
			!_ste->canSeeClassScope;

		if (!analyze_childBlock(entry, newBound, newFree, newGlobal,
			_typeParams, newClassEntry, &childFree))
		{
			goto error;
		}
		if (inlineComp) {
			if (!inline_comprehension(_ste, entry, scopes, childFree, inlinedCells)) {
				ALIF_DECREF(childFree);
				goto error;
			}
			entry->compInlined = 1;
		}
		temp = alifNumber_inPlaceOr(newFree, childFree);
		ALIF_DECREF(childFree);
		if (!temp)
			goto error;
		ALIF_DECREF(temp);
	}

	for (i_ = ALIFLIST_GET_SIZE(_ste->children) - 1; i_ >= 0; --i_) {
		AlifObject* c_ = ALIFLIST_GET_ITEM(_ste->children, i_);
		SymTableEntry* entry = (SymTableEntry*)c_;
		if (entry->compInlined and
			alifList_setSlice(_ste->children, i_, i_ + 1,
				entry->children) < 0)
		{
			goto error;
		}
	}

	if (alifST_isFunctionLike(_ste) and !analyze_cells(scopes, newFree, inlinedCells))
		goto error;
	else if (_ste->type == BlockType_::Class_Block and !drop_classFree(_ste, newFree))
		goto error;
	if (!update_symbols(_ste->symbols, scopes, _bound, newFree, inlinedCells,
		(_ste->type == BlockType_::Class_Block) or _ste->canSeeClassScope))
		goto error;

	temp = alifNumber_inPlaceOr(_free, newFree);
	if (!temp)
		goto error;
	ALIF_DECREF(temp);
	success = 1;
error:
	ALIF_XDECREF(scopes);
	ALIF_XDECREF(local);
	ALIF_XDECREF(newBound);
	ALIF_XDECREF(newGlobal);
	ALIF_XDECREF(newFree);
	ALIF_XDECREF(inlinedCells);
	return success;
}

static AlifIntT analyze_childBlock(SymTableEntry* _entry, AlifObject* _bound, AlifObject* _free,
	AlifObject* _global, AlifObject* _typeParams,
	SymTableEntry* _classEntry, AlifObject** _childFree) { // 1292

	AlifObject* tempBound = nullptr, * tempGlobal = nullptr, * tempFree = nullptr;
	AlifObject* tempTypeParams = nullptr;

	tempBound = alifSet_new(_bound);
	if (!tempBound)
		goto error;
	tempFree = alifSet_new(_free);
	if (!tempFree)
		goto error;
	tempGlobal = alifSet_new(_global);
	if (!tempGlobal)
		goto error;
	tempTypeParams = alifSet_new(_typeParams);
	if (!tempTypeParams)
		goto error;

	if (!analyze_block(_entry, tempBound, tempFree, tempGlobal,
		tempTypeParams, _classEntry))
		goto error;
	*_childFree = tempFree;
	ALIF_DECREF(tempBound);
	ALIF_DECREF(tempGlobal);
	ALIF_DECREF(tempTypeParams);
	return 1;
error:
	ALIF_XDECREF(tempBound);
	ALIF_XDECREF(tempFree);
	ALIF_XDECREF(tempGlobal);
	ALIF_XDECREF(tempTypeParams);
	return 0;
}

static AlifIntT symtable_analyze(AlifSymTable* _st) { // 1333
	AlifObject* free{}, * global{}, * typeParams{};
	AlifIntT r_{};

	free = alifSet_new(nullptr);
	if (!free)
		return 0;
	global = alifSet_new(nullptr);
	if (!global) {
		ALIF_DECREF(free);
		return 0;
	}
	typeParams = alifSet_new(nullptr);
	if (!typeParams) {
		ALIF_DECREF(free);
		ALIF_DECREF(global);
		return 0;
	}
	r_ = analyze_block(_st->top, nullptr, free, global, typeParams, nullptr);
	ALIF_DECREF(free);
	ALIF_DECREF(global);
	ALIF_DECREF(typeParams);
	return r_;
}

static AlifIntT symtable_exitBlock(AlifSymTable* _st) { // 1365
	AlifSizeT size{};

	_st->cur = nullptr;
	size = ALIFLIST_GET_SIZE(_st->stack);
	if (size) {
		if (alifList_setSlice(_st->stack, size - 1, size, nullptr) < 0)
			return 0;
		if (--size)
			_st->cur = (SymTableEntry*)ALIFLIST_GET_ITEM(_st->stack, size - 1);
	}
	return 1;
}

static AlifIntT symtable_enterExistingBlock(AlifSymTable* _st, SymTableEntry* _ste) { // 1381
	if (alifList_append(_st->stack, (AlifObject*)_ste) < 0) {
		return 0;
	}
	SymTableEntry* prev = _st->cur;

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
	SymTableEntry* _ste = ste_new(_st, _name, _block, _ast, _loc);
	if (_ste == nullptr) return 0;

	AlifIntT result = symtable_enterExistingBlock(_st, _ste);
	ALIF_DECREF(_ste);
	if (_block == BlockType_::Annotation_Block
		or _block == BlockType_::Type_Variable_Block
		or _block == BlockType_::Type_Alias_Block) {
		if (!symtable_addDef(_st, &ALIF_STR(Format), DEF_PARAM, _loc)) {
			return 0;
		}
		if (!symtable_addDef(_st, &ALIF_STR(Format), USE, _loc)) {
			return 0;
		}
	}
	return result;
}

static AlifIntT symtable_addDefHelper(AlifSymTable* _st,
	AlifObject* _name, AlifIntT _flag, SymTableEntry* _ste,
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
		if (val_ == -1 /*and alifErr_occurred()*/) {
			goto error;
		}
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

static AlifIntT check_keywords(AlifSymTable* _st, ASDLKeywordSeq* _keywords) { // 1572
	for (AlifSizeT i = 0; i < ASDL_SEQ_LEN(_keywords); i++) {
		KeywordTy key = ((KeywordTy)ASDL_SEQ_GET(_keywords, i));
		if (key->arg and !check_name(_st, key->arg, LOCATION(key), ExprContext_::Store)) {
			return 0;
		}
	}
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

static AlifIntT symtable_enterTypeParamBlock(AlifSymTable* _st, Identifier _name,
	void* _ast, AlifIntT _hasDefaults, AlifIntT _hasKwDefaults,
	StmtK_ _kind, AlifSourceLocation _loc) { // 1623
	BlockType_ current_type = _st->cur->type;
	if (!symtable_enterBlock(_st, _name, BlockType_::Type_Parameters_Block, _ast, _loc)) {
		return 0;
	}
	if (current_type == BlockType_::Class_Block) {
		_st->cur->canSeeClassScope = 1;
		if (!symtable_addDef(_st, &ALIF_ID(__classDict__), USE, _loc)) {
			return 0;
		}
	}
	if (_kind == StmtK_::ClassDefK) {
		if (!symtable_addDef(_st, &ALIF_STR(TypeParams), DEF_LOCAL, _loc)) {
			return 0;
		}
		if (!symtable_addDef(_st, &ALIF_STR(TypeParams), USE, _loc)) {
			return 0;
		}
		if (!symtable_addDef(_st, &ALIF_STR(GenericBase), DEF_LOCAL, _loc)) {
			return 0;
		}
		if (!symtable_addDef(_st, &ALIF_STR(GenericBase), USE, _loc)) {
			return 0;
		}
	}
	if (_hasDefaults) {
		if (!symtable_addDef(_st, &ALIF_STR(Defaults), DEF_PARAM, _loc)) {
			return 0;
		}
	}
	if (_hasKwDefaults) {
		if (!symtable_addDef(_st, &ALIF_STR(KWDefaults), DEF_PARAM, _loc)) {
			return 0;
		}
	}
	return 1;
}

// 1686
#define VISIT(_st, _type, _v) \
    do { \
        if (!symtable_visit ## _type((_st), (_v))) { \
            return 0; \
        } \
    } while(0)

// 1693
#define VISIT_SEQ(_st, _type, _seq) \
    do { \
        AlifSizeT i_{}; \
        ASDL ## _type ## Seq *seq = (_seq); /* avoid variable capture */ \
        for (i_ = 0; i_ < ASDL_SEQ_LEN(seq); i_++) { \
            _type ## Ty elt = (_type ## Ty)ASDL_SEQ_GET(seq, i_); \
            if (!symtable_visit ## _type((_st), elt)) \
                return 0;                 \
        } \
    } while(0)

#define VISIT_SEQ_WITH_NULL(_st, _type, _seq) \
    do { \
        AlifIntT i_ = 0; \
        ASDL ## _type ## Seq *seq = (_seq); /* avoid variable capture */ \
        for (i_ = 0; i_ < ASDL_SEQ_LEN(seq); i_++) { \
            _type ## Ty elt = (_type ## Ty)ASDL_SEQ_GET(seq, i_); \
            if (!elt) continue; /* can be NULL */ \
            if (!symtable_visit ## _type((_st), elt)) \
                return 0;             \
        } \
    } while(0)

#define VISIT_SEQ_TAIL(_st, _type, _seq, _start) \
    do { \
        AlifSizeT i{}; \
        ASDL ## _type ## Seq *seq = (_seq); /* avoid variable capture */ \
        for (i = (_start); i < ASDL_SEQ_LEN(seq); i++) { \
            _type ## Ty elt = (_type ## Ty)ASDL_SEQ_GET(seq, i); \
            if (!symtable_visit ## _type(_st, elt)) \
                return 0;                 \
        } \
    } while(0)

// 1727
#define ENTER_RECURSIVE(_st) \
    do { \
        if (++(_st)->recursionDepth > (_st)->recursionLimit) { \
			/*alifErr_setString(alifExcRecursionError,			\
			"maximum recursion depth exceeded during compilation");*/ \
            return 0; \
        } \
    } while(0)

// 1736
#define LEAVE_RECURSIVE(_st) \
    do { \
        --(_st)->recursionDepth; \
    } while(0)




static AlifIntT check_importFrom(AlifSymTable* st, StmtTy s) { // 1776
	AlifSourceLocation fut = st->future->location;
	if (s->V.importFrom.module and s->V.importFrom.level == 0 &&
		alifUStr_equalToASCIIString(s->V.importFrom.module, "__future__") and
		((s->lineNo > fut.lineNo) or
			((s->lineNo == fut.endLineNo) and (s->colOffset > fut.endColOffset))))
	{
		//alifErr_setString(_alifExcSyntaxError_,
		//	"from __future__ imports must occur "
		//	"at the beginning of the file");
		//SET_ERROR_LOCATION(st->fileName, LOCATION(s));
		return 0;
	}
	return 1;
}

static bool allows_topLevelAwait(AlifSymTable* st) { // 1795
	return (st->future->features & ALIFCF_ALLOW_TOP_LEVEL_AWAIT) and
		st->cur->type == BlockType_::Module_Block;
}

static AlifIntT symtable_visitStmt(AlifSymTable* _st, StmtTy _s) { // 1812
	ENTER_RECURSIVE(_st);
	switch (_s->type) {
	case StmtK_::FunctionDefK: {
		if (!symtable_addDef(_st, _s->V.functionDef.name, DEF_LOCAL, LOCATION(_s)))
			return 0;
		if (_s->V.functionDef.args->defaults)
			VISIT_SEQ(_st, Expr, _s->V.functionDef.args->defaults);
		if (_s->V.functionDef.args->kwDefaults)
			VISIT_SEQ_WITH_NULL(_st, Expr, _s->V.functionDef.args->kwDefaults);
		//if (_s->V.functionDef.decoratorList)
		//	VISIT_SEQ(_st, Expr, _s->V.functionDef.decoratorList);
		if (ASDL_SEQ_LEN(_s->V.functionDef.typeParams) > 0) {
			//if (!symtable_enterTypeParamBlock(
			//	_st, _s->V.functionDef.name,
			//	(void*)_s->V.functionDef.typeParams,
			//	_s->V.functionDef.args->defaults != nullptr,
			//	has_kwOnlyDefaults(_s->V.functionDef.args->kwOnlyArgs,
			//		_s->V.functionDef.args->kwDefaults),
			//	_s->type,
			//	LOCATION(_s))) {
			//	return 0;
			//}
			VISIT_SEQ(_st, TypeParam, _s->V.functionDef.typeParams);
		}
		SymTableEntry* newSte = ste_new(_st, _s->V.functionDef.name,
			BlockType_::Function_Block, (void*)_s,
			LOCATION(_s));
		if (!newSte) {
			return 0;
		}

		if (!symtable_visitAnnotations(_st, _s, _s->V.functionDef.args,
			_s->V.functionDef.returns, newSte)) {
			ALIF_DECREF(newSte);
			return 0;
		}
		if (!symtable_enterExistingBlock(_st, newSte)) {
			ALIF_DECREF(newSte);
			return 0;
		}
		ALIF_DECREF(newSte);
		VISIT(_st, Arguments, _s->V.functionDef.args);
		VISIT_SEQ(_st, Stmt, _s->V.functionDef.body);
		if (!symtable_exitBlock(_st))
			return 0;
		if (ASDL_SEQ_LEN(_s->V.functionDef.typeParams) > 0) {
			if (!symtable_exitBlock(_st))
				return 0;
		}
		break;
	}
	case StmtK_::ClassDefK: {
		AlifObject* tmp{};
		if (!symtable_addDef(_st, _s->V.classDef.name, DEF_LOCAL, LOCATION(_s)))
			return 0;
		//if (_s->V.classDef.decoratorList)
		//	VISIT_SEQ(_st, Expr, _s->V.classDef.decoratorList);
		tmp = _st->private_;
		if (ASDL_SEQ_LEN(_s->V.classDef.typeParams) > 0) {
			if (!symtable_enterTypeParamBlock(_st, _s->V.classDef.name,
				(void*)_s->V.classDef.typeParams,
				false, false, _s->type,
				LOCATION(_s))) {
				return 0;
			}
			_st->private_ = _s->V.classDef.name;
			_st->cur->mangledNames = alifSet_new(nullptr);
			if (!_st->cur->mangledNames) {
				return 0;
			}
			VISIT_SEQ(_st, TypeParam, _s->V.classDef.typeParams);
		}
		VISIT_SEQ(_st, Expr, _s->V.classDef.bases);
		if (!check_keywords(_st, _s->V.classDef.keywords)) {
			return 0;
		}
		VISIT_SEQ(_st, Keyword, _s->V.classDef.keywords);
		if (!symtable_enterBlock(_st, _s->V.classDef.name, BlockType_::Class_Block,
			(void*)_s, LOCATION(_s))) {
			return 0;
		}
		_st->private_ = _s->V.classDef.name;
		if (ASDL_SEQ_LEN(_s->V.classDef.typeParams) > 0) {
			if (!symtable_addDef(_st, &ALIF_ID(__typeParams__),
				DEF_LOCAL, LOCATION(_s))) {
				return 0;
			}
			if (!symtable_addDef(_st, &ALIF_STR(TypeParams),
				USE, LOCATION(_s))) {
				return 0;
			}
		}
		VISIT_SEQ(_st, Stmt, _s->V.classDef.body);
		if (!symtable_exitBlock(_st))
			return 0;
		if (ASDL_SEQ_LEN(_s->V.classDef.typeParams) > 0) {
			if (!symtable_exitBlock(_st))
				return 0;
		}
		_st->private_ = tmp;
		break;
	}
	case StmtK_::ReturnK:
		if (_s->V.return_.val) {
			VISIT(_st, Expr, _s->V.return_.val);
			_st->cur->returnsValue = 1;
		}
		break;
	case StmtK_::DeleteK:
		VISIT_SEQ(_st, Expr, _s->V.delete_.targets);
		break;
	case StmtK_::AssignK:
		VISIT_SEQ(_st, Expr, _s->V.assign.targets);
		VISIT(_st, Expr, _s->V.assign.val);
		break;
	case StmtK_::AugAssignK: {
		VISIT(_st, Expr, _s->V.augAssign.target);
		VISIT(_st, Expr, _s->V.augAssign.val);
		break;
	}
	case StmtK_::ForK:
		VISIT(_st, Expr, _s->V.for_.target);
		VISIT(_st, Expr, _s->V.for_.iter);
		VISIT_SEQ(_st, Stmt, _s->V.for_.body);
		//if (_s->V.for_.else_)
		//	VISIT_SEQ(_st, Stmt, _s->V.for_.else_);
		break;
	case StmtK_::WhileK:
		VISIT(_st, Expr, _s->V.while_.condition);
		VISIT_SEQ(_st, Stmt, _s->V.while_.body);
		//if (_s->V.while_.else_)
		//	VISIT_SEQ(_st, Stmt, _s->V.while_.else_);
		break;
	case StmtK_::IfK:
		VISIT(_st, Expr, _s->V.if_.condition);
		VISIT_SEQ(_st, Stmt, _s->V.if_.body);
		if (_s->V.if_.else_)
			VISIT_SEQ(_st, Stmt, _s->V.if_.else_);
		break;
	case StmtK_::ImportK:
		VISIT_SEQ(_st, Alias, _s->V.import.names);
		break;
	case StmtK_::ImportFromK:
		VISIT_SEQ(_st, Alias, _s->V.importFrom.names);
		if (!check_importFrom(_st, _s)) {
			return 0;
		}
		break;
	case StmtK_::ExprK:
		VISIT(_st, Expr, _s->V.expression.val);
		break;
	case StmtK_::PassK:
	case StmtK_::BreakK:
	case StmtK_::ContinueK:
		/* nothing to do here */
		break;
	}
	LEAVE_RECURSIVE(_st);
	return 1;
}

static AlifIntT symtable_visitExpr(AlifSymTable* _st, ExprTy _e) { // 2334
	
	ENTER_RECURSIVE(_st);
	switch (_e->type) {
	case ExprK_::BoolOpK:
		VISIT_SEQ(_st, Expr, _e->V.boolOp.vals);
		break;
	case ExprK_::BinOpK:
		VISIT(_st, Expr, _e->V.binOp.left);
		VISIT(_st, Expr, _e->V.binOp.right);
		break;
	case ExprK_::UnaryOpK:
		VISIT(_st, Expr, _e->V.unaryOp.operand);
		break;
	case ExprK_::IfExprK:
		VISIT(_st, Expr, _e->V.ifExpr.condition);
		VISIT(_st, Expr, _e->V.ifExpr.body);
		VISIT(_st, Expr, _e->V.ifExpr.else_);
		break;
	case ExprK_::DictK:
		VISIT_SEQ_WITH_NULL(_st, Expr, _e->V.dict.keys);
		VISIT_SEQ(_st, Expr, _e->V.dict.vals);
		break;
	case ExprK_::SetK:
		VISIT_SEQ(_st, Expr, _e->V.set.elts);
		break;
	case ExprK_::ListCompK:
		if (!symtable_visitListComp(_st, _e))
			return 0;
		break;
	case ExprK_::CompareK:
		VISIT(_st, Expr, _e->V.compare.left);
		VISIT_SEQ(_st, Expr, _e->V.compare.comparators);
		break;
	case ExprK_::CallK:
		VISIT(_st, Expr, _e->V.call.func);
		VISIT_SEQ(_st, Expr, _e->V.call.args);
		if (!check_keywords(_st, _e->V.call.keywords)) {
			return 0;
		}
		VISIT_SEQ_WITH_NULL(_st, Keyword, _e->V.call.keywords);
		break;
	case ExprK_::FormattedValK:
		VISIT(_st, Expr, _e->V.formattedValue.val);
		if (_e->V.formattedValue.formatSpec)
			VISIT(_st, Expr, _e->V.formattedValue.formatSpec);
		break;
	case ExprK_::JoinStrK:
		VISIT_SEQ(_st, Expr, _e->V.joinStr.vals);
		break;
	case ExprK_::ConstantK:
		/* Nothing to do here. */
		break;
	case ExprK_::AttributeK:
		if (!check_name(_st, _e->V.attribute.attr, LOCATION(_e), _e->V.attribute.ctx)) {
			return 0;
		}
		VISIT(_st, Expr, _e->V.attribute.val);
		break;
	case ExprK_::SubScriptK:
		VISIT(_st, Expr, _e->V.subScript.val);
		VISIT(_st, Expr, _e->V.subScript.slice);
		break;
	case ExprK_::SliceK:
		if (_e->V.slice.lower)
			VISIT(_st, Expr, _e->V.slice.lower);
		if (_e->V.slice.upper)
			VISIT(_st, Expr, _e->V.slice.upper);
		if (_e->V.slice.step)
			VISIT(_st, Expr, _e->V.slice.step);
		break;
	case ExprK_::NameK:
		if (!symtable_addDefCtx(_st, _e->V.name.name,
			_e->V.name.ctx == ExprContext_::Load ? USE : DEF_LOCAL,
			LOCATION(_e), _e->V.name.ctx)) {
			return 0;
		}
		if (_e->V.name.ctx == ExprContext_::Load and
			alifST_isFunctionLike(_st->cur) and
			alifUStr_equalToASCIIString(_e->V.name.name, "super")) {
			if (!symtable_addDef(_st, &ALIF_ID(__class__), USE, LOCATION(_e)))
				return 0;
		}
		break;
	case ExprK_::ListK:
		VISIT_SEQ(_st, Expr, _e->V.list.elts);
		break;
	case ExprK_::TupleK:
		VISIT_SEQ(_st, Expr, _e->V.tuple.elts);
		break;
	}
	LEAVE_RECURSIVE(_st);
	return 1;
}


static AlifIntT symtable_visitTypeParam(AlifSymTable* _st, TypeParamTy _tp) { // 2538
	ENTER_RECURSIVE(_st);
	switch (_tp->type) {
	case TypeParamK::TypeVarK: {
		if (!symtable_addDef(_st, _tp->V.typeVar.name, DEF_TYPE_PARAM | DEF_LOCAL, LOCATION(_tp)))
			return 0;

		const char* steScopeInfo = nullptr;
		const ExprTy bound = _tp->V.typeVar.bound;
		if (bound != nullptr) {
			steScopeInfo = bound->type == ExprK_::TupleK ? "a TypeVar constraint" : "a TypeVar bound";
		}

		//if (!symtable_visitTypeParamBoundOrDefault(_st, _tp->V.typeVar.bound, _tp->V.typeVar.name,
		//	(void*)_tp, ste_scope_info)) {
		//	return 0;
		//}

		//if (!symtable_visitTypeParamBoundOrDefault(_st, _tp->V.typeVar.defaultValue, _tp->V.typeVar.name,
		//	(void*)((uintptr_t)_tp + 1), "a TypeVar default")) {
		//	return 0;
		//}
		break;
	}
	case TypeParamK::TypeVarTupleK: {

		if (!symtable_addDef(_st, _tp->V.typeVarTuple.name, DEF_TYPE_PARAM | DEF_LOCAL, LOCATION(_tp))) {
			return 0;
		}

		//if (!symtable_visitTypeParamBoundOrDefault(_st, _tp->V.typeVarTuple.defaultValue, _tp->V.typeVarTuple.name,
		//	(void*)_tp, "a TypeVarTuple default")) {
		//	return 0;
		//}
		break;
	}
	//case TypeParamK::ParamSpecK:
	//	if (!symtable_addDef(_st, _tp->V.paramSpec.name, DEF_TYPE_PARAM | DEF_LOCAL, LOCATION(_tp))) {
	//		return 0;
	//	}

	//	if (!symtable_visitTypeParamBoundOrDefault(_st, _tp->V.paramSpec.defaultValue, _tp->V.paramSpec.name,
	//		(void*)_tp, "a ParamSpec default")) {
	//		return 0;
	//	}
	//	break;
	}
	LEAVE_RECURSIVE(_st);
	return 1;
}

static AlifIntT symtable_implicitArg(AlifSymTable* _st, AlifIntT _pos) { // 2650
	AlifObject* id = alifUStr_fromFormat(".%d", _pos);
	if (id == nullptr)
		return 0;
	if (!symtable_addDef(_st, id, DEF_PARAM, _st->cur->loc)) {
		ALIF_DECREF(id);
		return 0;
	}
	ALIF_DECREF(id);
	return 1;
}


static AlifIntT symtable_visitParams(AlifSymTable* _st, ASDLArgSeq* _args){ // 2664
	AlifSizeT i{};

	for (i = 0; i < ASDL_SEQ_LEN(_args); i++) {
		ArgTy arg = (ArgTy)ASDL_SEQ_GET(_args, i);
		if (!symtable_addDef(_st, arg->arg, DEF_PARAM, LOCATION(arg)))
			return 0;
	}

	return 1;
}


static AlifIntT symtable_visitAnnotations(AlifSymTable* st,
	StmtTy o, ArgumentsTy a, ExprTy returns,
	SymTableEntry* function_ste) { // 2726
	AlifIntT is_in_class = st->cur->canSeeClassScope;
	BlockType_ current_type = st->cur->type;
	if (!symtable_enterBlock(st, &ALIF_ID(__annotate__), BlockType_::Annotation_Block,
		(void*)a, LOCATION(o))) {
		return 0;
	}
	if (is_in_class or current_type == BlockType_::Class_Block) {
		st->cur->canSeeClassScope = 1;
		if (!symtable_addDef(st, &ALIF_ID(__classDict__), USE, LOCATION(o))) {
			return 0;
		}
	}
	//if (a->posOnlyArgs and !symtable_visitArgAnnotations(st, a->posOnlyArgs))
	//	return 0;
	//if (a->args and !symtable_visitArgAnnotations(st, a->args))
	//	return 0;
	//if (a->varArg and a->varArg->annotation) {
	//	st->cur->annotationsUsed = 1;
	//	VISIT(st, Expr, a->varArg->annotation);
	//}
	//if (a->kwArg and a->kwArg->annotation) {
	//	st->cur->annotationsUsed = 1;
	//	VISIT(st, Expr, a->kwArg->annotation);
	//}
	//if (a->kwOnlyArgs and !symtable_visitArgAnnotations(st, a->kwOnlyArgs))
	//	return 0;
	//if (returns) {
	//	st->cur->annotationsUsed = 1;
	//	VISIT(st, Expr, returns);
	//}
	if (!symtable_exitBlock(st)) {
		return 0;
	}
	return 1;
}


static AlifIntT symtable_visitArguments(AlifSymTable* _st, ArgumentsTy _a) { // 2766
	/* skip default arguments inside function block
	   XXX should ast be different?
	*/
	if (_a->posOnlyArgs and !symtable_visitParams(_st, _a->posOnlyArgs))
		return 0;
	if (_a->args and !symtable_visitParams(_st, _a->args))
		return 0;
	if (_a->kwOnlyArgs and !symtable_visitParams(_st, _a->kwOnlyArgs))
		return 0;
	if (_a->varArg) {
		if (!symtable_addDef(_st, _a->varArg->arg, DEF_PARAM, LOCATION(_a->varArg)))
			return 0;
		_st->cur->varArgs = 1;
	}
	if (_a->kwArg) {
		if (!symtable_addDef(_st, _a->kwArg->arg, DEF_PARAM, LOCATION(_a->kwArg)))
			return 0;
		_st->cur->varKeywords = 1;
	}
	return 1;
}



static AlifIntT symtable_visitAlias(AlifSymTable* _st, AliasTy _a) { // 2825
	AlifObject* storeName{};
	AlifObject* name = (_a->asName == nullptr) ? _a->name : _a->asName;
	AlifSizeT dot = alifUStr_findChar(name, '.', 0,
		ALIFUSTR_GET_LENGTH(name), 1);
	if (dot != -1) {
		storeName = alifUStr_subString(name, 0, dot);
		if (!storeName)
			return 0;
	}
	else {
		storeName = ALIF_NEWREF(name);
	}
	if (!alifUStr_equalToASCIIString(name, "*")) {
		AlifIntT r = symtable_addDef(_st, storeName, DEF_IMPORT, LOCATION(_a));
		ALIF_DECREF(storeName);
		return r;
	}
	else {
		if (_st->cur->type != BlockType_::Module_Block) {
			//alifErr_setString(_alifExcSyntaxError_, IMPORT_STAR_WARNING);
			//SET_ERROR_LOCATION(st->filename, LOCATION(a));
			ALIF_DECREF(storeName);
			return 0;
		}
		ALIF_DECREF(storeName);
		return 1;
	}
}


static AlifIntT symtable_visitComprehension(AlifSymTable* _st,
	ComprehensionTy _lc) { // 2862
	_st->cur->compIterTarget = 1;
	VISIT(_st, Expr, _lc->target);
	_st->cur->compIterTarget = 0;
	_st->cur->compIterExpr++;
	VISIT(_st, Expr, _lc->iter);
	_st->cur->compIterExpr--;
	VISIT_SEQ(_st, Expr, _lc->ifs);
	if (_lc->isAsync) {
		_st->cur->coroutine = 1;
	}
	return 1;
}


static AlifIntT symtable_visitKeyword(AlifSymTable* _st, KeywordTy _k) { // 2879
	VISIT(_st, Expr, _k->val);
	return 1;
}


static AlifIntT symtable_handleComprehension(AlifSymTable* st, ExprTy e,
	Identifier scope_name, ASDLComprehensionSeq* generators,
	ExprTy elt, ExprTy value) { // 2887
	AlifIntT is_generator = (e->type == ExprK_::GeneratorExprK);
	ComprehensionTy outermost = ((ComprehensionTy)
		ASDL_SEQ_GET(generators, 0));
	st->cur->compIterExpr++;
	VISIT(st, Expr, outermost->iter);
	st->cur->compIterExpr--;
	if (!scope_name or
		!symtable_enterBlock(st, scope_name, BlockType_::Function_Block, (void*)e, LOCATION(e))) {
		return 0;
	}
	switch (e->type) {
	case ExprK_::ListCompK:
		st->cur->comprehension = AlifComprehensionType::List_Comprehension;
		break;
	case ExprK_::SetCompK:
		st->cur->comprehension = AlifComprehensionType::Set_Comprehension;
		break;
	case ExprK_::DictCompK:
		st->cur->comprehension = AlifComprehensionType::Dict_Comprehension;
		break;
	default:
		st->cur->comprehension = AlifComprehensionType::Generator_Expression;
		break;
	}
	if (outermost->isAsync) {
		st->cur->coroutine = 1;
	}

	if (!symtable_implicitArg(st, 0)) {
		symtable_exitBlock(st);
		return 0;
	}
	st->cur->compIterTarget = 1;
	VISIT(st, Expr, outermost->target);
	st->cur->compIterTarget = 0;
	VISIT_SEQ(st, Expr, outermost->ifs);
	VISIT_SEQ_TAIL(st, Comprehension, generators, 1);
	if (value)
		VISIT(st, Expr, value);
	VISIT(st, Expr, elt);
	st->cur->generator = is_generator;
	AlifIntT is_async = st->cur->coroutine and !is_generator;
	if (!symtable_exitBlock(st)) {
		return 0;
	}
	if (is_async and
		!IS_ASYNC_DEF(st) and
		st->cur->comprehension == AlifComprehensionType::No_Comprehension and
		!allows_topLevelAwait(st))
	{
		//alifErr_setString(_alifExcSyntaxError_, "asynchronous comprehension outside of "
		//	"an asynchronous function");
		//SET_ERROR_LOCATION(st->fileName, LOCATION(e));
		return 0;
	}
	if (is_async) {
		st->cur->coroutine = 1;
	}
	return 1;
}


static AlifIntT symtable_visitListComp(AlifSymTable* _st, ExprTy _e) { // 2966
	return symtable_handleComprehension(_st, _e, &ALIF_ID(ListComp),
		_e->V.listComp.generators,
		_e->V.listComp.elt, nullptr);
}


AlifObject* alif_maybeMangle(AlifObject* _privateObj,
	SymTableEntry* _ste, AlifObject* _name) { // 3070
	if (_ste->mangledNames != nullptr) {
		AlifIntT result = alifSet_contains(_ste->mangledNames, _name);
		if (result < 0) {
			return nullptr;
		}
		if (result == 0) {
			return ALIF_NEWREF(_name);
		}
	}
	return alif_mangle(_privateObj, _name);
}

AlifObject* alif_mangle(AlifObject* _privateObj, AlifObject* _ident) { // 3090
	if (_privateObj == nullptr or !ALIFUSTR_CHECK(_privateObj) or
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
	ALIFUSTR_WRITE(ALIFUSTR_KIND(result), ALIFUSTR_DATA(result), 0, '_');
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
