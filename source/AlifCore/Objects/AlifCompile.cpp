#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Compile.h"
#include "AlifCore_FlowGraph.h"
#include "AlifCore_State.h"
#include "AlifCore_SetObject.h"


#include "Code.h"

















#undef SUCCESS
#undef ERROR
#define SUCCESS 0
#define ERROR -1

#define RETURN_IF_ERROR(_x)  \
    if ((_x) == -1) {        \
        return ERROR;       \
    }

typedef AlifSourceLocation Location;
typedef AlifJumpTargetLabel JumpTargetLabel;
typedef AlifInstructionSequence InstrSequence;
typedef AlifCFGBuilder CFGBuilder;
typedef AlifCompileFBlockInfo FBlockInfo;
typedef AlifCompileFBlockType FBlockType;




class CompilerUnit {
public:
	SymTableEntry* ste{};

	AlifIntT scopeType{};

	AlifObject* private_{};            /* for private name mangling */
	AlifObject* staticAttributes{};  /* for class: attributes accessed via self.X */
	AlifObject* deferredAnnotations{}; /* AnnAssign nodes deferred to the end of compilation */

	InstrSequence* instrSequence{}; /* codegen output */

	AlifIntT nfBlocks{};
	AlifIntT inInlinedComp{};

	FBlockInfo fBlock[MAXBLOCKS]{};

	AlifCompileCodeUnitMetadata metadata{};
};












class AlifCompiler {
public:
	AlifObject* filename{};
	AlifSymTable* st{};
	AlifFutureFeatures future{};
	AlifCompilerFlags flags{};

	AlifIntT optimize{};
	AlifIntT interactive{};
	AlifObject* constCache{};
	CompilerUnit* u_{};
	AlifObject* stack{};

	bool saveNestedSeqs{};
};



static AlifIntT compiler_setup(AlifCompiler* _c, ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _flags, AlifIntT _optimize, AlifASTMem* _astMem) { // 6445
	AlifCompilerFlags localFlags = ALIFCOMPILERFLAGS_INIT;

	_c->constCache = alifDict_new();
	if (!_c->constCache) {
		return ERROR;
	}

	_c->stack = alifList_new(0);
	if (!_c->stack) {
		return ERROR;
	}

	_c->filename = ALIF_NEWREF(_filename);
	if (!alifFuture_fromAST(_mod, _filename, &_c->future)) {
		return ERROR;
	}
	if (!_flags) {
		_flags = &localFlags;
	}
	AlifIntT merged = _c->future.features | _flags->flags;
	_c->future.features = merged;
	_flags->flags = merged;
	_c->flags = *_flags;
	_c->optimize = (_optimize == -1) ? alif_getConfig()->optimizationLevel : _optimize;
	_c->saveNestedSeqs = false;

	if (!alifAST_optimize(_mod, _astMem, _c->optimize, merged)) {
		return ERROR;
	}
	_c->st = alifSymtable_build(_mod, _filename, &_c->future);
	if (_c->st == nullptr) {
		if (!alifErr_occurred()) {
			//alifErr_setString(_alifExcSystemError_, "no symtable");
		}
		return ERROR;
	}
	return SUCCESS;
}



static void compiler_free(AlifCompiler* _c) {
	if (_c->st) {
		alifSymtable_free(_c->st);
	}
	ALIF_XDECREF(_c->filename);
	ALIF_XDECREF(_c->constCache);
	ALIF_XDECREF(_c->stack);
	alifMem_dataFree(_c);
}



static AlifCompiler* new_compiler(ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _pflags, AlifIntT _optimize, AlifASTMem* _astMem) {
	AlifCompiler* compiler = (AlifCompiler*)alifMem_dataAlloc(sizeof(AlifCompiler));
	if (compiler == nullptr) {
		return nullptr;
	}
	if (compiler_setup(compiler, _mod, _filename, _pflags, _optimize, _astMem) < 0) {
		compiler_free(compiler);
		return nullptr;
	}
	return compiler;
}



static void compiler_unitFree(CompilerUnit* _u) {
	ALIF_CLEAR(_u->instrSequence);
	ALIF_CLEAR(_u->ste);
	ALIF_CLEAR(_u->metadata.name);
	ALIF_CLEAR(_u->metadata.qualname);
	ALIF_CLEAR(_u->metadata.consts);
	ALIF_CLEAR(_u->metadata.names);
	ALIF_CLEAR(_u->metadata.varnames);
	ALIF_CLEAR(_u->metadata.freevars);
	ALIF_CLEAR(_u->metadata.cellvars);
	ALIF_CLEAR(_u->metadata.fasthidden);
	ALIF_CLEAR(_u->private_);
	ALIF_CLEAR(_u->staticAttributes);
	ALIF_CLEAR(_u->deferredAnnotations);
	alifMem_dataFree(_u);
}



#define CAPSULE_NAME "AlifCompile.cpp AlifCompiler unit"

AlifIntT _alifCompiler_maybeAddStaticAttributeToClass(AlifCompiler* _c, ExprTy _e) { // 6572
	ExprTy attr_value = _e->V.attribute.val;
	if (attr_value->type != ExprK_::NameK or
		_e->V.attribute.ctx != ExprContext_::Store or
		!alifUStr_equalToASCIIString(attr_value->V.name.name, "self"))
	{
		return SUCCESS;
	}
	AlifSizeT stackSize = ALIFLIST_GET_SIZE(_c->stack);
	for (AlifSizeT i = stackSize - 1; i >= 0; i--) {
		AlifObject* capsule = ALIFLIST_GET_ITEM(_c->stack, i);
		CompilerUnit* u = (CompilerUnit*)alifCapsule_getPointer(
			capsule, CAPSULE_NAME);
		if (u->scopeType == ScopeType_::Compiler_Scope_Class) {
			RETURN_IF_ERROR(alifSet_add(u->staticAttributes, _e->V.attribute.attr));
			break;
		}
	}
	return SUCCESS;
}






static AlifIntT compiler_setQualname(AlifCompiler* _c) {
	AlifSizeT stackSize{};
	CompilerUnit* u = _c->u_;
	AlifObject* name{}, * base{};

	base = nullptr;
	stackSize = ALIFLIST_GET_SIZE(_c->stack);
	if (stackSize > 1) {
		AlifIntT scope, forceGlobal = 0;
		CompilerUnit* parent{};
		AlifObject* mangled{}, * capsule{};

		capsule = ALIFLIST_GET_ITEM(_c->stack, stackSize - 1);
		parent = (CompilerUnit*)alifCapsule_getPointer(capsule, CAPSULE_NAME);
		if (parent->scopeType == ScopeType_::Compiler_Scope_Annotations) {
			if (stackSize == 2) {
				u->metadata.qualname = ALIF_NEWREF(u->metadata.name);
				return SUCCESS;
			}
			capsule = ALIFLIST_GET_ITEM(_c->stack, stackSize - 2);
			parent = (CompilerUnit*)alifCapsule_getPointer(capsule, CAPSULE_NAME);
		}

		if (u->scopeType == ScopeType_::Compiler_Scope_Function
			or u->scopeType == ScopeType_::Compiler_Scope_Async_Function
			or u->scopeType == ScopeType_::Compiler_Scope_Class) {
			mangled = alif_mangle(parent->private_, u->metadata.name);
			if (!mangled) {
				return ERROR;
			}

			scope = alifST_getScope(parent->ste, mangled);
			ALIF_DECREF(mangled);
			RETURN_IF_ERROR(scope);
			if (scope == GLOBAL_EXPLICIT) forceGlobal = 1;
		}

		if (!forceGlobal) {
			if (parent->scopeType == ScopeType_::Compiler_Scope_Function
				or parent->scopeType == ScopeType_::Compiler_Scope_Async_Function
				or parent->scopeType == ScopeType_::Compiler_Scope_Lambda)
			{
				base = alifUStr_concat(parent->metadata.qualname,
					&ALIF_STR(DotLocals));
				if (base == nullptr) {
					return ERROR;
				}
			}
			else {
				base = ALIF_NEWREF(parent->metadata.qualname);
			}
		}
	}

	if (base != nullptr) {
		name = alifUStr_concat(base, ALIF_LATIN1_CHR('.'));
		ALIF_DECREF(base);
		if (name == nullptr) {
			return ERROR;
		}
		alifUStr_append(&name, u->metadata.name);
		if (name == nullptr) {
			return ERROR;
		}
	}
	else {
		name = ALIF_NEWREF(u->metadata.name);
	}
	u->metadata.qualname = name;

	return SUCCESS;
}

















static AlifObject* const_cacheInsert(AlifObject* _constCache,
	AlifObject* _o, bool _recursive) {
	if (_o == ALIF_NONE or _o == ALIF_ELLIPSIS) {
		return _o;
	}

	AlifObject* key_ = alifCode_constantKey(_o);
	if (key_ == nullptr) {
		return nullptr;
	}

	AlifObject* t_{};
	AlifIntT res = alifDict_setDefaultRef(_constCache, key_, key_, &t_);
	if (res != 0) {
		ALIF_DECREF(key_);
		return t_;
	}
	ALIF_DECREF(t_);

	if (!_recursive) {
		return key_;
	}

	if (ALIFTUPLE_CHECKEXACT(_o)) {
		AlifSizeT len_ = ALIFTUPLE_GET_SIZE(_o);
		for (AlifSizeT i = 0; i < len_; i++) {
			AlifObject* item = ALIFTUPLE_GET_ITEM(_o, i);
			AlifObject* u_ = const_cacheInsert(_constCache, item, _recursive);
			if (u_ == nullptr) {
				ALIF_DECREF(key_);
				return nullptr;
			}

			AlifObject* v_{};
			if (ALIFTUPLE_CHECKEXACT(u_)) {
				v_ = ALIFTUPLE_GET_ITEM(u_, 1);
			}
			else {
				v_ = u_;
			}
			if (v_ != item) {
				ALIFTUPLE_SET_ITEM(_o, i, ALIF_NEWREF(v_));
				ALIF_DECREF(item);
			}

			ALIF_DECREF(u_);
		}
	}
	else if (ALIFFROZENSET_CHECKEXACT(_o)) {

		AlifSizeT len_ = ALIFSET_GET_SIZE(_o);
		if (len_ == 0) {
			return key_;
		}
		AlifObject* tuple = alifTuple_new(len_);
		if (tuple == nullptr) {
			ALIF_DECREF(key_);
			return nullptr;
		}
		AlifSizeT i_ = 0, pos_ = 0;
		AlifObject* item{};
		AlifHashT hash{};
		while (alifSet_nextEntry(_o, &pos_, &item, &hash)) {
			AlifObject* k_ = const_cacheInsert(_constCache, item, _recursive);
			if (k_ == nullptr) {
				ALIF_DECREF(tuple);
				ALIF_DECREF(key_);
				return nullptr;
			}
			AlifObject* u_{};
			if (ALIFTUPLE_CHECKEXACT(k_)) {
				u_ = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(k_, 1));
				ALIF_DECREF(k_);
			}
			else {
				u_ = k_;
			}
			ALIFTUPLE_SET_ITEM(tuple, i_, u_);  // Steals reference of u.
			i_++;
		}
		AlifObject* new_ = alifFrozenSet_new(tuple);
		ALIF_DECREF(tuple);
		if (new_ == nullptr) {
			ALIF_DECREF(key_);
			return nullptr;
		}
		ALIF_DECREF(_o);
		ALIFTUPLE_SET_ITEM(key_, 1, new_);
	}

	return key_;
}




















static AlifObject* merge_constsRecursive(AlifObject* _constCache, AlifObject* _o) {
	return const_cacheInsert(_constCache, _o, true);
}



AlifSizeT _alifCompiler_dictAddObj(AlifObject* _dict, AlifObject* _o) {
	AlifObject* v{};
	AlifSizeT arg{};

	if (alifDict_getItemRef(_dict, _o, &v) < 0) {
		return ERROR;
	}
	if (!v) {
		arg = ALIFDICT_GET_SIZE(_dict);
		v = alifLong_fromSizeT(arg);
		if (!v) {
			return ERROR;
		}
		if (alifDict_setItem(_dict, _o, v) < 0) {
			ALIF_DECREF(v);
			return ERROR;
		}
	}
	else {
		arg = alifLong_asLong(v);
	}
	ALIF_DECREF(v);
	return arg;
}


AlifSizeT _alifCompiler_addConst(AlifCompiler* _c, AlifObject* _o) {
	AlifObject* key = merge_constsRecursive(_c->constCache, _o);
	if (key == nullptr) {
		return ERROR;
	}

	AlifSizeT arg = _alifCompiler_dictAddObj(_c->u_->metadata.consts, key);
	ALIF_DECREF(key);
	return arg;
}



static AlifObject* list2dict(AlifObject* _list) {
	AlifSizeT i{}, n{};
	AlifObject* v{}, * k{};
	AlifObject* dict = alifDict_new();
	if (!dict) return nullptr;

	n = alifList_size(_list);
	for (i = 0; i < n; i++) {
		v = alifLong_fromSizeT(i);
		if (!v) {
			ALIF_DECREF(dict);
			return nullptr;
		}
		k = ALIFLIST_GET_ITEM(_list, i);
		if (alifDict_setItem(dict, k, v) < 0) {
			ALIF_DECREF(v);
			ALIF_DECREF(dict);
			return nullptr;
		}
		ALIF_DECREF(v);
	}
	return dict;
}











static AlifObject* dict_byType(AlifObject* _src, AlifIntT _scopeType,
	AlifIntT _flag, AlifSizeT _offset) {
	AlifSizeT i = _offset, numKeys, keyI;
	AlifObject* k, * v, * dest = alifDict_new();
	AlifObject* sortedKeys;

	if (dest == nullptr) return nullptr;

	sortedKeys = alifDict_keys(_src);
	if (sortedKeys == nullptr) {
		ALIF_DECREF(dest);
		return nullptr;
	}
	if (alifList_sort(sortedKeys) != 0) {
		ALIF_DECREF(sortedKeys);
		ALIF_DECREF(dest);
		return nullptr;
	}
	numKeys = ALIFLIST_GET_SIZE(sortedKeys);

	for (keyI = 0; keyI < numKeys; keyI++) {
		k = ALIFLIST_GET_ITEM(sortedKeys, keyI);
		v = alifDict_getItemWithError(_src, k);
		if (!v) {
			//if (!alifErr_occurred()) {
			//	alifErr_SetObject(_alifExcKeyError_, k);
			//}
			ALIF_DECREF(sortedKeys);
			ALIF_DECREF(dest);
			return nullptr;
		}
		long vi = alifLong_asLong(v);
		if (vi == -1 /*and alifErr_occurred()*/) {
			ALIF_DECREF(sortedKeys);
			ALIF_DECREF(dest);
			return nullptr;
		}
		if (SYMBOL_TO_SCOPE(vi) == _scopeType or vi & _flag) {
			AlifObject* item = alifLong_fromSizeT(i);
			if (item == nullptr) {
				ALIF_DECREF(sortedKeys);
				ALIF_DECREF(dest);
				return nullptr;
			}
			i++;
			if (alifDict_setItem(dest, k, item) < 0) {
				ALIF_DECREF(sortedKeys);
				ALIF_DECREF(item);
				ALIF_DECREF(dest);
				return nullptr;
			}
			ALIF_DECREF(item);
		}
	}
	ALIF_DECREF(sortedKeys);
	return dest;
}









AlifIntT _alifCompiler_enterScope(AlifCompiler* _c, Identifier _name,
	AlifIntT _scopeType, void* _key, AlifIntT _lineno,
	AlifObject* _private, AlifCompileCodeUnitMetadata* _umd) { // 6797

	CompilerUnit* u{};

	u = (CompilerUnit*)alifMem_dataAlloc(sizeof(CompilerUnit));
	if (!u) {
		//alifErr_noMemory();
		return ERROR;
	}
	u->scopeType = _scopeType;
	if (_umd != nullptr) {
		u->metadata = *_umd;
	}
	else {
		u->metadata.argCount = 0;
		u->metadata.posOnlyArgCount = 0;
		u->metadata.kwOnlyArgCount = 0;
	}
	u->ste = _alifSymtable_lookup(_c->st, _key);
	if (!u->ste) {
		compiler_unitFree(u);
		return ERROR;
	}
	u->metadata.name = ALIF_NEWREF(_name);
	u->metadata.varnames = list2dict(u->ste->varNames);
	if (!u->metadata.varnames) {
		compiler_unitFree(u);
		return ERROR;
	}
	u->metadata.cellvars = dict_byType(u->ste->symbols, CELL, DEF_COMP_CELL, 0);
	if (!u->metadata.cellvars) {
		compiler_unitFree(u);
		return ERROR;
	}
	if (u->ste->needsClassClosure) {
		AlifSizeT res{};
		res = _alifCompiler_dictAddObj(u->metadata.cellvars, &ALIF_ID(__class__));
		if (res < 0) {
			compiler_unitFree(u);
			return ERROR;
		}
	}
	if (u->ste->needsClassDict) {
		AlifSizeT res{};
		res = _alifCompiler_dictAddObj(u->metadata.cellvars, &ALIF_ID(__classDict__));
		if (res < 0) {
			compiler_unitFree(u);
			return ERROR;
		}
	}

	u->metadata.freevars = dict_byType(u->ste->symbols, FREE, DEF_FREE_CLASS,
		ALIFDICT_GET_SIZE(u->metadata.cellvars));
	if (!u->metadata.freevars) {
		compiler_unitFree(u);
		return ERROR;
	}

	u->metadata.fasthidden = alifDict_new();
	if (!u->metadata.fasthidden) {
		compiler_unitFree(u);
		return ERROR;
	}

	u->nfBlocks = 0;
	u->inInlinedComp = 0;
	u->metadata.firstLineno = _lineno;
	u->metadata.consts = alifDict_new();
	if (!u->metadata.consts) {
		compiler_unitFree(u);
		return ERROR;
	}
	u->metadata.names = alifDict_new();
	if (!u->metadata.names) {
		compiler_unitFree(u);
		return ERROR;
	}

	u->deferredAnnotations = nullptr;
	if (_scopeType == ScopeType_::Compiler_Scope_Class) {
		u->staticAttributes = alifSet_new(0);
		if (!u->staticAttributes) {
			compiler_unitFree(u);
			return ERROR;
		}
	}
	else {
		u->staticAttributes = nullptr;
	}

	u->instrSequence = (InstrSequence*)_alifInstructionSequence_new();
	if (!u->instrSequence) {
		compiler_unitFree(u);
		return ERROR;
	}

	/* Push the old compiler_unit on the stack. */
	if (_c->u_) {
		AlifObject* capsule = alifCapsule_new(_c->u_, CAPSULE_NAME, nullptr);
		if (!capsule or alifList_append(_c->stack, capsule) < 0) {
			ALIF_XDECREF(capsule);
			compiler_unitFree(u);
			return ERROR;
		}
		ALIF_DECREF(capsule);
		if (_private == nullptr) {
			_private = _c->u_->private_;
		}
	}

	u->private_ = ALIF_XNEWREF(_private);

	_c->u_ = u;

	if (_scopeType != ScopeType_::Compiler_Scope_Module) {
		RETURN_IF_ERROR(compiler_setQualname(_c));
	}

	return SUCCESS;
}



void _alifCompiler_exitScope(AlifCompiler* _c) { // 6931
	//AlifObject* exc = alifErr_getRaisedException();

	InstrSequence* nestedSeq = nullptr;
	if (_c->saveNestedSeqs) {
		nestedSeq = _c->u_->instrSequence;
		ALIF_INCREF(nestedSeq);
	}
	compiler_unitFree(_c->u_);
	AlifSizeT n = ALIFLIST_GET_SIZE(_c->stack) - 1;
	if (n >= 0) {
		AlifObject* capsule = ALIFLIST_GET_ITEM(_c->stack, n);
		_c->u_ = (CompilerUnit*)alifCapsule_getPointer(capsule, CAPSULE_NAME);
		if (alifSequence_delItem(_c->stack, n) < 0) {
			//alifErr_formatUnraisable("Exception ignored on removing "
			//	"the last compiler stack item");
		}
		if (nestedSeq != nullptr) {
			if (_alifInstructionSequence_addNested(_c->u_->instrSequence, nestedSeq) < 0) {
				//alifErr_formatUnraisable("Exception ignored on appending "
				//	"nested instruction sequence");
			}
		}
	}
	else {
		_c->u_ = nullptr;
	}
	ALIF_XDECREF(nestedSeq);

	//alifErr_setRaisedException(exc);
}








/*
 * Frame block handling functions
 */
AlifIntT _alifCompiler_pushFBlock(AlifCompiler* _c, Location _loc,
 	FBlockType _t, JumpTargetLabel _blockLabel,
 	JumpTargetLabel _exit, void* _datum) {
 	FBlockInfo* f{};
 	if (_c->u_->nfBlocks >= MAXBLOCKS) {
 		//return _alifCompiler_error(_c, _loc, "too many statically nested blocks");
 	}
 	f = &_c->u_->fBlock[_c->u_->nfBlocks++];
 	f->type = _t;
 	f->block = _blockLabel;
 	f->loc = _loc;
 	f->exit = _exit;
 	f->datum = _datum;
 	return SUCCESS;
 }



void _alifCompiler_popFBlock(AlifCompiler* _c,
	FBlockType _t, JumpTargetLabel _blockLabel) {
	CompilerUnit* u = _c->u_;
	u->nfBlocks--;
}





FBlockInfo* _alifCompiler_topFBlock(AlifCompiler* _c) {
	if (_c->u_->nfBlocks == 0) {
		return nullptr;
	}
	return &_c->u_->fBlock[_c->u_->nfBlocks - 1];
}









static Location start_location(ASDLStmtSeq* _stmts) {
	if (ASDL_SEQ_LEN(_stmts) > 0) {
		StmtTy st = (StmtTy)ASDL_SEQ_GET(_stmts, 0);
		return SRC_LOCATION_FROM_AST(st);
	}
	return AlifSourceLocation( 1, 1, 0, 0 );
}








static AlifIntT compiler_codegen(AlifCompiler* _c, ModuleTy _mod) {
	RETURN_IF_ERROR(_alifCodegen_enterAnonymousScope(_c, _mod));
	switch (_mod->type) {
	case ModK_::ModuleK: {
		ASDLStmtSeq* stmts = _mod->V.module.body;
		RETURN_IF_ERROR(_alifCodegen_body(_c, start_location(stmts), stmts, false));
		break;
	}
	case ModK_::InteractiveK: {
		_c->interactive = 1;
		ASDLStmtSeq* stmts = _mod->V.interactive.body;
		RETURN_IF_ERROR(_alifCodegen_body(_c, start_location(stmts), stmts, true));
		break;
	}
	case ModK_::ExpressionK: {
		RETURN_IF_ERROR(_alifCodegen_expression(_c, _mod->V.expression.body));
		break;
	}
	default: {
		//alifErr_format(_alifExcSystemError_,
		//	"module kind %d should not be possible",
		//	_mod->type);
		return ERROR;
	}
	}
	return SUCCESS;
}



static AlifCodeObject* compiler_mod(AlifCompiler* _c, ModuleTy _mod) {
	AlifCodeObject* co = nullptr;
	AlifIntT addNone = _mod->type != ModK_::ExpressionK;
	if (compiler_codegen(_c, _mod) < 0) {
		goto finally;
	}
	co = _alifCompiler_optimizeAndAssemble(_c, addNone);
finally:
	_alifCompiler_exitScope(_c);
	return co;
}



AlifIntT _alifCompiler_getRefType(AlifCompiler* _c, AlifObject* _name) {
	if (_c->u_->scopeType == ScopeType_::Compiler_Scope_Class and
		(alifUStr_equalToASCIIString(_name, "__class__") or
			alifUStr_equalToASCIIString(_name, "__classDict__"))) {
		return CELL;
	}
	SymTableEntry* ste = _c->u_->ste;
	AlifIntT scope = alifST_getScope(ste, _name);
	if (scope == 0) {
		//alifErr_format(_alifExcSystemError_,
		//	"alifST_getScope(name=%R) failed: "
		//	"unknown scope in unit %S (%R); "
		//	"symbols: %R; locals: %R; "
		//	"globals: %R",
		//	_name, _c->u_->metadata.name, ste->id,
		//	ste->symbols, _c->u_->metadata.varnames,
		//	_c->u_->metadata.names);
		return ERROR;
	}
	return scope;
}




static AlifIntT dict_lookupArg(AlifObject* _dict, AlifObject* _name) {
	AlifObject* v = alifDict_getItemWithError(_dict, _name);
	if (v == nullptr) {
		return ERROR;
	}
	return alifLong_asLong(v);
}

AlifIntT _alifCompiler_lookupCellVar(AlifCompiler* _c, AlifObject* _name) {
	return dict_lookupArg(_c->u_->metadata.cellvars, _name);
}






AlifIntT _alifCompiler_lookupArg(AlifCompiler* _c,
	AlifCodeObject* _co, AlifObject* _name) {
	AlifIntT reftype = _alifCompiler_getRefType(_c, _name);
	if (reftype == -1) {
		return ERROR;
	}
	AlifIntT arg{};
	if (reftype == CELL) {
		arg = dict_lookupArg(_c->u_->metadata.cellvars, _name);
	}
	else {
		arg = dict_lookupArg(_c->u_->metadata.freevars, _name);
	}
	if (arg == -1 and !alifErr_occurred()) {
		//AlifObject* freevars = _alifCode_getFreeVars(_co);
		//if (freevars == nullptr) {
		//	alifErr_clear();
		//}
		//alifErr_format(_alifExcSystemError_,
		//	"compiler_lookupArg(name=%R) with reftype=%d failed in %S; "
		//	"freevars of code %S: %R",
		//	_name, reftype, _c->u_->metadata.name, _co->name, freevars);
		//ALIF_DECREF(freevars);
		return ERROR;
	}
	return arg;
}












AlifObject* _alifCompiler_staticAttributesTuple(AlifCompiler* _c) {
	AlifObject* staticAttributesUnsorted = alifSequence_list(_c->u_->staticAttributes);
	if (staticAttributesUnsorted == nullptr) {
		return nullptr;
	}
	if (alifList_sort(staticAttributesUnsorted) != 0) {
		ALIF_DECREF(staticAttributesUnsorted);
		return nullptr;
	}
	AlifObject* static_attributes = alifSequence_tuple(staticAttributesUnsorted);
	ALIF_DECREF(staticAttributesUnsorted);
	return static_attributes;
}




AlifIntT _alifCompiler_resolveNameOp(AlifCompiler* _c, AlifObject* _mangled,
	AlifIntT _scope, AlifCompilerOpType* _optype, AlifSizeT* _arg) {
	AlifObject* dict = _c->u_->metadata.names;
	*_optype = AlifCompilerOpType::Compiler_Op_Name;

	switch (_scope) {
	case FREE:
		dict = _c->u_->metadata.freevars;
		*_optype = AlifCompilerOpType::Compiler_Op_DeRef;
		break;
	case CELL:
		dict = _c->u_->metadata.cellvars;
		*_optype = AlifCompilerOpType::Compiler_Op_DeRef;
		break;
	case LOCAL:
		if (alifST_isFunctionLike(_c->u_->ste)) {
			*_optype = AlifCompilerOpType::Compiler_Op_Fast;
		}
		else {
			AlifObject* item{};
			RETURN_IF_ERROR(alifDict_getItemRef(_c->u_->metadata.fasthidden, _mangled,
				&item));
			if (item == ALIF_TRUE) {
				*_optype = AlifCompilerOpType::Compiler_Op_Fast;
			}
			ALIF_XDECREF(item);
		}
		break;
	case GLOBAL_IMPLICIT:
		if (alifST_isFunctionLike(_c->u_->ste)) {
			*_optype = AlifCompilerOpType::Compiler_Op_Global;
		}
		break;
	case GLOBAL_EXPLICIT:
		*_optype = AlifCompilerOpType::Compiler_Op_Global;
		break;
	default:
		/* scope can be 0 */
		break;
	}
	if (*_optype != AlifCompilerOpType::Compiler_Op_Fast) {
		*_arg = _alifCompiler_dictAddObj(dict, _mangled);
		RETURN_IF_ERROR(*_arg);
	}
	return SUCCESS;
}




AlifIntT _alifCompiler_tweakInlinedComprehensionScopes(AlifCompiler* _c, Location _loc,
	SymTableEntry* _entry, AlifCompilerInlinedComprehensionState* _state) {
	AlifIntT in_class_block = (_c->u_->ste->type == BlockType_::Class_Block)
		and !_c->u_->inInlinedComp;
	_c->u_->inInlinedComp++;

	AlifObject* k{}, * v{};
	AlifSizeT pos = 0;
	while (alifDict_next(_entry->symbols, &pos, &k, &v)) {
		long symbol = alifLong_asLong(v);
		RETURN_IF_ERROR(symbol);
		long scope = SYMBOL_TO_SCOPE(symbol);

		long outsymbol = _alifST_getSymbol(_c->u_->ste, k);
		RETURN_IF_ERROR(outsymbol);
		long outsc = SYMBOL_TO_SCOPE(outsymbol);

		if ((scope != outsc && scope != FREE && !(scope == CELL && outsc == FREE))
			|| in_class_block) {
			if (_state->tempSymbols == nullptr) {
				_state->tempSymbols = alifDict_new();
				if (_state->tempSymbols == nullptr) {
					return ERROR;
				}
			}

			if (alifDict_setItem(_c->u_->ste->symbols, k, v) < 0) {
				return ERROR;
			}
			AlifObject* outv = alifLong_fromLong(outsymbol);
			if (outv == nullptr) {
				return ERROR;
			}
			AlifIntT res = alifDict_setItem(_state->tempSymbols, k, outv);
			ALIF_DECREF(outv);
			RETURN_IF_ERROR(res);
		}
		if ((symbol & DEF_LOCAL and !(symbol & DEF_NONLOCAL)) or in_class_block) {
			if (!alifST_isFunctionLike(_c->u_->ste)) {
				AlifObject* orig{};
				if (alifDict_getItemRef(_c->u_->metadata.fasthidden, k, &orig) < 0) {
					return ERROR;
				}
				if (orig != ALIF_TRUE) {
					if (alifDict_setItem(_c->u_->metadata.fasthidden, k, ALIF_TRUE) < 0) {
						return ERROR;
					}
					if (_state->fastHidden == nullptr) {
						_state->fastHidden = alifSet_new(nullptr);
						if (_state->fastHidden == nullptr) {
							return ERROR;
						}
					}
					if (alifSet_add(_state->fastHidden, k) < 0) {
						return ERROR;
					}
				}
			}
		}
	}
	return SUCCESS;
}


















AlifIntT _alifCompiler_revertInlinedComprehensionScopes(AlifCompiler* _c, Location _loc,
	AlifCompilerInlinedComprehensionState* _state) {
	_c->u_->inInlinedComp--;
	if (_state->tempSymbols) {
		AlifObject* k{}, * v{};
		AlifSizeT pos = 0;
		while (alifDict_next(_state->tempSymbols, &pos, &k, &v)) {
			if (alifDict_setItem(_c->u_->ste->symbols, k, v)) {
				return ERROR;
			}
		}
		ALIF_CLEAR(_state->tempSymbols);
	}
	if (_state->fastHidden) {
		while (alifSet_size(_state->fastHidden) > 0) {
			AlifObject* k = alifSet_pop(_state->fastHidden);
			if (k == nullptr) {
				return ERROR;
			}
			if (alifDict_setItem(_c->u_->metadata.fasthidden, k, ALIF_FALSE)) {
				ALIF_DECREF(k);
				return ERROR;
			}
			ALIF_DECREF(k);
		}
		ALIF_CLEAR(_state->fastHidden);
	}
	return SUCCESS;
}






























































































AlifObject* _alifCompiler_maybeMangle(AlifCompiler* _c, AlifObject* _name) {
	return alif_maybeMangle(_c->u_->private_, _c->u_->ste, _name);
}



InstrSequence* _alifCompiler_instrSequence(AlifCompiler* _c) {
	return _c->u_->instrSequence;
}









AlifSymTable* _alifCompiler_symTable(AlifCompiler* _c) {
	return _c->st;
}



SymTableEntry* _alifCompiler_symTableEntry(AlifCompiler* _c) {
	return _c->u_->ste;
}



AlifIntT _alifCompiler_optimizationLevel(AlifCompiler* _c) {
	return _c->optimize;
}























AlifIntT _alifCompiler_isInInlinedComp(AlifCompiler* _c) {
	return _c->u_->inInlinedComp;
}



AlifObject* _alifCompiler_qualname(AlifCompiler* _c) {
	return _c->u_->metadata.qualname;
}




AlifCompileCodeUnitMetadata* _alifCompiler_metadata(AlifCompiler* _c) {
	return &_c->u_->metadata;
}













AlifIntT _alifCompiler_constCacheMergeOne(AlifObject* _constCache,
	AlifObject** _obj) {

	AlifObject* key = const_cacheInsert(_constCache, *_obj, false);
	if (key == nullptr) {
		return ERROR;
	}
	if (ALIFTUPLE_CHECKEXACT(key)) {
		AlifObject* item = ALIFTUPLE_GET_ITEM(key, 1);
		ALIF_SETREF(*_obj, ALIF_NEWREF(item));
		ALIF_DECREF(key);
	}
	else {
		ALIF_SETREF(*_obj, key);
	}
	return SUCCESS;
}

static AlifObject* constsDict_keysInorder(AlifObject* _dict) {
	AlifObject* consts{}, * k{}, * v{};
	AlifSizeT i{}, pos = 0, size = ALIFDICT_GET_SIZE(_dict);

	consts = alifList_new(size);
	if (consts == nullptr) return nullptr;
	while (alifDict_next(_dict, &pos, &k, &v)) {
		i = alifLong_asLong(v);
		if (ALIFTUPLE_CHECKEXACT(k)) {
			k = ALIFTUPLE_GET_ITEM(k, 1);
		}
		ALIFLIST_SET_ITEM(consts, i, ALIF_NEWREF(k));
	}
	return consts;
}










static AlifIntT compute_codeFlags(AlifCompiler* _c) {
	SymTableEntry* ste = _c->u_->ste;
	AlifIntT flags = 0;
	if (alifST_isFunctionLike(ste)) {
		flags |= CO_NEWLOCALS | CO_OPTIMIZED;
		if (ste->nested)
			flags |= CO_NESTED;
		if (ste->generator and !ste->coroutine)
			flags |= CO_GENERATOR;
		if (ste->generator and ste->coroutine)
			flags |= CO_ASYNC_GENERATOR;
		if (ste->varArgs)
			flags |= CO_VARARGS;
		if (ste->varKeywords)
			flags |= CO_VARKEYWORDS;
	}

	if (ste->coroutine and !ste->generator) {
		flags |= CO_COROUTINE;
	}

	flags |= (_c->flags.flags & ALIFCF_MASK);

	return flags;
}




static AlifCodeObject* optimizeAndAssemble_codeUnit(CompilerUnit* u, AlifObject* const_cache,
	AlifIntT _codeFlags, AlifObject* filename) {
	CFGBuilder* g = nullptr;
	InstrSequence optimizedInstrs{};
	memset(&optimizedInstrs, 0, sizeof(InstrSequence)); //* review //* delete

	AlifIntT nlocals{};
	AlifIntT nparams{};

	AlifIntT stackDepth{};
	AlifIntT nLocalsPlus{};

	AlifCodeObject* co = nullptr;
	AlifObject* consts = constsDict_keysInorder(u->metadata.consts);
	if (consts == nullptr) {
		goto error;
	}
	g = alifCFG_fromInstructionSequence(u->instrSequence);
	if (g == nullptr) {
		goto error;
	}
	nlocals = (AlifIntT)ALIFDICT_GET_SIZE(u->metadata.varnames);
	nparams = (AlifIntT)ALIFLIST_GET_SIZE(u->ste->varNames);

	if (alifCFG_optimizeCodeUnit(g, consts, const_cache, nlocals,
		nparams, u->metadata.firstLineno) < 0) {
		goto error;
	}

	if (alifCFG_optimizedCFGToInstructionSequence(g, &u->metadata, _codeFlags,
		&stackDepth, &nLocalsPlus, &optimizedInstrs) < 0) {
		goto error;
	}

	/** Assembly **/
	co = alifAssemble_makeCodeObject(&u->metadata, const_cache, consts,
		stackDepth, &optimizedInstrs, nLocalsPlus, _codeFlags, filename);

error:
	ALIF_XDECREF(consts);
	alifInstructionSequence_fini(&optimizedInstrs);
	alifCFGBuilder_free(g);
	return co;
}



AlifCodeObject* _alifCompiler_optimizeAndAssemble(AlifCompiler* _c, AlifIntT _addNone) {
	CompilerUnit* u = _c->u_;
	AlifObject* constCache = _c->constCache;
	AlifObject* filename = _c->filename;

	AlifIntT codeFlags = compute_codeFlags(_c);
	if (codeFlags < 0) {
		return nullptr;
	}

	if (_alifCodegen_addReturnAtEnd(_c, _addNone) < 0) {
		return nullptr;
	}

	return optimizeAndAssemble_codeUnit(u, constCache, codeFlags, filename);
}



AlifCodeObject* _alifAST_compile(ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _pFlags, AlifIntT _optimize, AlifASTMem* _astMem) {

	AlifCompiler* compiler = new_compiler(_mod, _filename, _pFlags, _optimize, _astMem);
	if (compiler == nullptr) {
		return nullptr;
	}

	AlifCodeObject* co = compiler_mod(compiler, _mod);
	compiler_free(compiler);
	return co;
}




AlifIntT _alifCompile_astOptimize(ModuleTy _mod, AlifObject* _filename,
	AlifCompilerFlags* _cf, AlifIntT _optimize, AlifASTMem* _astMem) {
	AlifFutureFeatures future{};
	if (!alifFuture_fromAST(_mod, _filename, &future)) {
		return -1;
	}
	AlifIntT flags = future.features | _cf->flags;
	if (_optimize == -1) {
		_optimize = alif_getConfig()->optimizationLevel;
	}
	if (!alifAST_optimize(_mod, _astMem, _optimize, flags)) {
		return -1;
	}
	return 0;
}
