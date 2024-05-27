#include "alif.h"


#include "AlifCore_AST.h"
#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSeq.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_SymTable.h"



enum FBlockType {
	WHILE_LOOP, FOR_LOOP, WITH, ASYNC_WITH, HANDLER_CLEANUP,
	POP_VALUE, ASYNC_COMPREHENSION_GENERATOR, STOP_ITERATION
};

class FBlockInfo {
public:
	FBlockType type{};
	JumpTargetLable block{};
	JumpTargetLable exit{};
	void* datum{};
};


enum ScopeType{
	Compiler_Scope_Module,
	Compiler_Scope_Class,
	Compiler_Scope_Function,
	Compiler_Scope_AsyncFunction,
	Compiler_Scope_Comprehension,
};


class CompilerUnit { // 245
public:
	AlifSTEntryObject* uSTE;

	AlifIntT uScopeType;

	AlifObject* uPrivate;
	AlifObject* uStaticAttributes; 

	InstructionSequance* uInstrSequence;

	AlifIntT uNFBlocks;
	AlifIntT uInInlinedComp;

	FBlockInfo uFBlock[CO_MAXBLOCKS];

	AlifCompileCodeUnitData uData;
};

class AlifCompiler { // 275
public:
	AlifObject* fileName{};
	AlifSymTable* symTable{};

	AlifIntT optimize{};
	AlifIntT nestLevel{};

	CompilerUnit* unit{};

	AlifObject* stack{};
	AlifASTMem* astMem{};
};

static AlifObject* list_toDict(AlifObject* _list) { // 485

	AlifObject* v{};
	AlifObject* k{};
	AlifObject* dict = alifNew_dict();
	if (!dict) return nullptr;

	AlifSizeT n = alifList_size(_list);
	for (AlifSizeT i = 0; i < n; i++) {
		v = alifInteger_fromLongLong(i);
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

static AlifObject* dict_byType(AlifObject* _src, AlifIntT _scopeType, AlifIntT _flag, AlifSizeT _offset) { // 519

	AlifSizeT i = _offset;
	AlifSizeT scope{};

	AlifObject* k{}, v{};
	AlifObject* dest = alifNew_dict();
	if (dest == nullptr) return nullptr;

	AlifObject* sortedKeys = alifDict_keys(_src);
	if (sortedKeys == nullptr) return nullptr;

	if (alifList_sort(sortedKeys) != 0) {
		ALIF_DECREF(sortedKeys);
		return nullptr;
	}
	AlifSizeT numKeys = ALIFLIST_GET_SIZE(sortedKeys);

	for (AlifSizeT keyIdx = 0; keyIdx < numKeys; keyIdx++) {
		long vi{};
		k = ALIFLIST_GET_ITEM(sortedKeys, keyIdx);
		v = alifDict_getItemWithError(_src, k);
		vi = ALIFLONG_AS_LONG(v);
		scope = (vi >> SCOPE_OFFSET) & SCOPE_MASK;

		if (scope == _scopeType or vi & _flag) {
			AlifObject* item = alifInteger_fromLongLong(i);
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

static AlifSizeT dict_addObject(AlifObject* _dict, AlifObject* _obj) { // 830

	AlifObject* v{};
	AlifSizeT arg{};

	if (alifDict_getItemRef(_dict, _obj, &v) < 0) {
		return -1;
	}

	if (!v) {
		arg = ALIFDICT_GET_SIZE(_dict);
		v = alifInteger_fromLongLong(arg);
		if (!v) return -1;

		if (alifDict_setItem(_dict, _obj, v) < 0) {
			ALIF_DECREF(v);
			return -1;
		}
	}
	else {
		arg = alifLong_asLong(v);
	}
	ALIF_DECREF(v);
	return arg;
}

static AlifIntT compiler_setup(AlifCompiler* _compiler, Module* _module,
	AlifObject* _fn, AlifIntT _optimize, AlifASTMem* _astMem) { // 380

	_compiler->stack = alifNew_list(0);
	if (_compiler->stack) return -1;

	_compiler->fileName = alif_newRef(_fn); // need Change to MACRO
	_compiler->astMem = _astMem;

	_compiler->optimize = (_optimize == -1) ? alifConfig_get()->optimizationLevel : _optimize;
	_compiler->nestLevel = 0;

	if (!alifAST_optimize(_module, _astMem, _compiler->optimize)) return -1;

	_compiler->symTable = alifSymTable_build(_module, _fn);
	if (_compiler->symTable == nullptr) {
		// error
		return -1;
	}

	return 1;
}

static AlifCompiler* compiler_new(Module* _module, AlifObject* _fn, AlifIntT _optimize, AlifASTMem* _astMem) { // 425

	AlifCompiler* compiler_ = (AlifCompiler*)alifMem_dataAlloc(sizeof(AlifCompiler));
	if (compiler_ == nullptr) return nullptr;

	if (compiler_setup(compiler_, _module, _fn, _optimize, _astMem) < 0) {
		//compiler_free(compiler_);
		return nullptr;
	}

	return compiler_;
}

AlifCodeObject* alifAST_compile(Module* _module, AlifObject* _fn, AlifIntT _optimize, AlifASTMem* _astMem) { // 440

	AlifCompiler* compiler_ = compiler_new(_module, _fn, _optimize, _astMem);
	if (compiler_ == nullptr) return nullptr;

	AlifCodeObject* codeObject = compiler_module(compiler_, _module);
	//compiler_free(compiler_);

	return codeObject;
}

static AlifIntT compiler_enterScope(AlifCompiler* _compiler,
	AlifObject* _name, AlifIntT _scopeType, void* _key, AlifIntT _lineNo) { // 1169

	SourceLocation location = {_lineNo, _lineNo, 0, 0 };

	CompilerUnit* cu = (CompilerUnit*)alifMem_dataAlloc(sizeof(CompilerUnit));
	if (!cu) {
		// error
		return -1;
	}

	cu->uScopeType = _scopeType;
	cu->uData.argCount = 0;
	cu->uData.posOnlyArgCount = 0;
	cu->uData.kwOnlyArgCount = 0;
	cu->uSTE = alifSymTable_lookup(_compiler->symTable, _key);
	if (!cu->uSTE) {
		// error
		return -1;
	}

	cu->uData.name = ALIF_NEWREF(_name);
	cu->uData.varNames = list_toDict(cu->uSTE->steVarNames);
	if (!cu->uData.varNames) {
		//compilerUnit_free(cu);
		return -1;
	}

	cu->uData.cellVars = dict_byType(cu->uSTE->steSymbols, CELL, DEF_COMP_CELL, 0);
	if (!cu->uData.cellVars) {
		//compilerUnit_free(cu);
		return -1;
	}

	if (cu->uSTE->steNeedsClassClosure) {
		AlifSizeT res{};
		AlifObject* name = alifUnicode_decodeStringToUTF8(L"__class__");
		res = dict_addObject(cu->uData.cellVars, name);
		if (res < 0) {
			//compilerUnit_free(cu);
			return -1;
		}
	}

	if (cu->uSTE->steNeedsClassDict) {
		AlifSizeT res{};
		AlifObject* name = alifUnicode_decodeStringToUTF8(L"__classdict__");
		res = dict_addObject(cu->uData.cellVars, name);
		if (res < 0) {
			//compilerUnit_free(cu);
			return -1;
		}
	}

	cu->uData.freeVars = dict_byType(cu->uSTE->steSymbols, FREE, DEF_FREE_CLASS, ALIFDICT_GET_SIZE(cu->uData.cellVars));
	if (cu->uData.freeVars) {
		//compilerUnit_free(cu);
		return -1;
	}

	cu->uNFBlocks = 0;
	cu->uInInlinedComp = 0;
	cu->uData.firstLineNo = _lineNo;
	cu->uData.consts = alifNew_dict();
	if (!cu->uData.consts) {
		//compilerUnit_free(cu);
		return -1;
	}

	cu->uData.names = alifNew_dict();
	if (!cu->uData.names) {
		//compilerUnit_free(cu);
		return -1;
	}

	cu->uPrivate = nullptr;
	if (_scopeType == ScopeType::Compiler_Scope_Class) {
		cu->uStaticAttributes = alifNew_set(0);
		if (!cu->uStaticAttributes) {
			//compilerUnit_free(cu);
			return -1;
		}
	}
	else {
		cu->uStaticAttributes = nullptr;
	}

	cu->uInstrSequence = (InstructionSequance*)alifInstructionSequance_new();

	if (_compiler->unit) {
		AlifObject* capsule = alifCapsule_new(_compiler->unit, CAPSULE_NAME, nullptr);
		if (!capsule or alifList_append(_compiler->stack, capsule) < 0) {
			ALIF_XDECREF(capsule);
			//compilerUnit_free(cu);
			return -1;
		}
		ALIF_DECREF(capsule);
		cu->uPrivate = ALIF_XNEWREF(_compiler->unit->uPrivate);
	}
	_compiler->unit = cu;

	_compiler->nestLevel++;

	if (cu->uScopeType == ScopeType::Compiler_Scope_Module) {
		location.lineNo = 0;
	}
	else {
		if (compiler_setQualName(_compiler) < 0) {
			return -1;
		}
	}
	ADDOP_I(_compiler, location, RESUME, RESUME_ATFUNC_START);

	if (cu->uScopeType == ScopeType::Compiler_Scope_Module) {
		location.lineNo = -1;
	}

	return 1;
}

static AlifIntT compiler_enterAnonymousScope(AlifCompiler* _compiler, Module* _module) { // 1700

	AlifObject* name = alifUnicode_decodeStringToUTF8(L"<module>");
	if (compiler_enterScope(_compiler, name, ScopeType::Compiler_Scope_Module, _module, 1) == -1) {
		return -1;
	}

	return 1;
}

static AlifCodeObject* compiler_module(AlifCompiler* _compiler, Module* _module) { // 1710

	AlifCodeObject* codeObject = nullptr;
	AlifIntT addNone = _module->type != ExpressionK;
	if (compiler_enterAnonymousScope(_compiler, _module) < 0) {
		return nullptr;
	}
	if (compiler_codeGenerate(_compiler, _module) < 0) {
		goto done;
	}
	codeObject = optimize_assemble(_compiler, addNone);

done:
	compiler_exitScope(_compiler);
	return codeObject;
}
