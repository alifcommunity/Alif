#include "alif.h"
#include "OpCode.h"

#include "AlifCore_AST.h"
#include "AlifCore_OpCodeUtils.h"
#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSeq.h"
#include "AlifCore_Intrinsics.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_SymTable.h"



// Forward
class AlifCompiler; // temp
static AlifCodeObject* compiler_module(AlifCompiler*, Module*);
static AlifIntT compiler_body(AlifCompiler*, SourceLocation, StmtSeq*);

typedef AlifFlowGraph AlifFlowGraph;

#define LOC(_a) {_a->lineNo, _a->endLineNo, _a->colOffset, _a->endColOffset}

static const SourceLocation noLocation = {-1, -1, -1, -1};

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

	InstructionSequence* uInstrSequence;

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
	AlifIntT interactive{};
	AlifIntT nestLevel{};

	CompilerUnit* unit{};

	AlifObject* stack{};
	AlifASTMem* astMem{};
};

#define INSTR_SEQUANCE(_c) (_c->unit->uInstrSequence)


#define CAPSULE_NAME L"AlifCompile.cpp AlifCompiler Unit" // 377



#define ADDOP(_c, _loc, _op) {if (codeGen_addOpNoArg(INSTR_SEQUANCE(_c), _op, _loc) == -1) return -1;}
#define ADDOP_I(_c, _loc, _op, _o) { if (codeGen_addOpI(INSTR_SEQUANCE(_c), _op, _o, _loc) == -1) return -1;}
#define ADDOP_BINARY(_c, _loc, _binOp) { if (addOp_binary(_c, _loc, _binOp, TRUE) == -1) return -1;}
#define ADDOP_LOAD_CONST(_c, _loc, _o) { if (compilerAddOp_loadConst(_c->unit, _loc, _o) == -1) return -1;}




#define VISIT(_c, _type, _v) {if (compiler_visit ## _type(_c, _v) == -1) return -1;}


static AlifIntT codeGen_addOpNoArg(InstructionSequence* _seq, AlifIntT _opCode, SourceLocation _loc) { // 822
	return alifInstructionSequance_addOp(_seq, _opCode, 0, _loc);
}

static AlifObject* marge_constsRecursive(AlifObject* _obj) { // 857
	if (_obj == ALIF_NONE) return _obj;


	/*
		long code here
	*/

}

static AlifSizeT compiler_addConst(CompilerUnit* _cu, AlifObject* _obj) { // 965
	AlifObject* key = marge_constsRecursive(_obj);
	if (key == nullptr) return -1;

	AlifSizeT arg = dict_addObject(_cu->uData.consts, key);
	ALIF_DECREF(key);
	return arg;
}

static AlifIntT compilerAddOp_loadConst(CompilerUnit* _cu, SourceLocation _loc, AlifObject* _obj) { // 979
	AlifSizeT arg = compiler_addConst(_cu, _obj);
	if (arg < 0) return -1;

	return codeGen_addOpI(_cu->uInstrSequence, LOAD_CONST, arg, _loc);
}

static AlifIntT codeGen_addOpI(InstructionSequence* _seq,
	AlifIntT _opCode, AlifSizeT _opArg, SourceLocation _loc) { // 1047

	AlifIntT opArg = (AlifIntT)_opArg;
	return alifInstructionSequance_addOp(_seq, _opCode, opArg, _loc);
}



AlifIntT alifCompile_ensureArraySpace(AlifIntT _idx, void** _array, AlifIntT* _alloc, AlifIntT _defaultAlloc, AlifUSizeT _itemSize) { // 155

	void* arr = *_array;
	if (arr == nullptr) {
		AlifIntT newAlloc = _defaultAlloc;
		if (_idx >= newAlloc) {
			newAlloc = _idx + _defaultAlloc;
		}
		arr = alifMem_dataAlloc(_itemSize * newAlloc);
		if (arr == nullptr) {
			// memory error
			return -1;
		}
		*_alloc = newAlloc;
	}
	else if (_idx >= *_alloc) {
		AlifUSizeT oldSize = *_alloc * _itemSize;
		AlifIntT newAlloc = *_alloc << 1;
		if (_idx >= newAlloc) {
			newAlloc = _idx + _defaultAlloc;
		}
		AlifUSizeT newSize = newAlloc * _itemSize;

		if (oldSize > (SIZE_MAX >> 1)) {
			// memory error
			return -1;
		}

		void* tmp = alifMem_dataRealloc(arr, newSize);
		if (tmp == nullptr) {
			// memory error
			return -1;
		}
		*_alloc = newAlloc;
		arr = tmp;
	}

	*_array = arr;
	return 1;
}



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
		if (dict_setItem((AlifDictObject*)dict, k, v) < 0) {
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

	AlifObject* k{}, *v{};
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
		v = dict_getItem(_src, k); // alifDict_getItemWithError
		vi = alifInteger_asLong(v); // ALIFLONG_AS_LONG
		scope = (vi >> SCOPE_OFFSET) & SCOPE_MASK;

		if (scope == _scopeType or vi & _flag) {
			AlifObject* item = alifInteger_fromLongLong(i);
			if (item == nullptr) {
				ALIF_DECREF(sortedKeys);
				ALIF_DECREF(dest);
				return nullptr;
			}
			i++;
			if (dict_setItem((AlifDictObject*)dest, k, item) < 0) {
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

		if (dict_setItem((AlifDictObject*)_dict, _obj, v) < 0) {
			ALIF_DECREF(v);
			return -1;
		}
	}
	else {
		arg = alifInteger_asLong(v);
	}
	ALIF_DECREF(v);
	return arg;
}

static AlifIntT compiler_setQualName(AlifCompiler* _compiler) { // 608

	AlifSizeT stackSize = ALIFLIST_GET_SIZE(_compiler->stack);
	CompilerUnit* cu = _compiler->unit;
	AlifObject* name{}, * base = nullptr;

	if (stackSize > 1) {
		AlifIntT scope{};
		AlifIntT forceGlobal = 0;
		AlifObject* mangled{};

		AlifObject* capsule = ALIFLIST_GET_ITEM(_compiler->stack, stackSize - 1);
		CompilerUnit* parent = (CompilerUnit*)alifCapsule_getPointer(capsule, CAPSULE_NAME);

		if (cu->uScopeType == Compiler_Scope_Function
			or cu->uScopeType == Compiler_Scope_AsyncFunction
			or cu->uScopeType == Compiler_Scope_Class) {
			mangled = alif_mangle(parent->uPrivate, cu->uData.name);
			if (!mangled) return -1;

			scope = alifST_getScope(parent->uSTE, mangled);
			ALIF_DECREF(mangled);
			if (scope == GLOBAL_EXPLICIT) forceGlobal = 1;
		}

		if (!forceGlobal) {
			if (parent->uScopeType == Compiler_Scope_Function
				or parent->uScopeType == Compiler_Scope_AsyncFunction)
			{
				//ALIF_DECLARE_STR(dotLocal, L".<locals>");
				AlifObject* name1 = alifUnicode_decodeStringToUTF8(L"dotLocals");
				base = alifUStr_concat(parent->uData.qualName, name1);
				if (base == nullptr) return -1;
			}
			else {
				base = ALIF_NEWREF(parent->uData.qualName);
			}
		}

	}

	if (base != nullptr) {
		//ALIF_DECLARE_STR(dot, L".");
		AlifObject* name2 = alifUnicode_decodeStringToUTF8(L"dot");
		name = alifUStr_concat(base, name2);
		ALIF_DECREF(base);
		if (name == nullptr) return -1;
		alifUStr_append(&name, cu->uData.name);
		if (name == nullptr) return -1;
	}
	else {
		name = ALIF_NEWREF(cu->uData.name);
	}
	cu->uData.qualName = name;

	return 1;
}



static AlifIntT compiler_setup(AlifCompiler* _compiler, Module* _module,
	AlifObject* _fn, AlifIntT _optimize, AlifASTMem* _astMem) { // 380

	_compiler->stack = alifNew_list(0);
	if (!_compiler->stack) return -1;

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
	if (!cu->uData.freeVars) {
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

	cu->uInstrSequence = (InstructionSequence*)alifInstructionSequance_new();

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

static AlifIntT compiler_codeGenerate(AlifCompiler* _compiler, Module* _module) { // 1671

	SourceLocation loc = {1, 1, 0, 0};
	if (_module->type == ModType::ModuleK) {
		if (compiler_body(_compiler, loc, _module->V.module.body) < 0) {
			return -1;
		}
	}
	else if (_module->type == ModType::InteractiveK) {

	}
	else {
		// error
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
	codeObject = optimize_andAssemble(_compiler, addNone);

done:
	compiler_exitScope(_compiler);
	return codeObject;
}



static AlifIntT addOp_binary(AlifCompiler* _compiler, SourceLocation _loc, Operator _binOp, BOOL _inPlace) { // 4122

	AlifIntT opArg{};

	if (_binOp == Operator::Add) {
		opArg = _inPlace ? NB_INPLACE_ADD : NB_ADD;
	}

	ADDOP_I(_compiler, _loc, BINARY_OP, opArg);

	return 1;
}


static AlifIntT compiler_visitExpression(AlifCompiler* _compiler, Expression* _expr) { // 6341

	SourceLocation loc = LOC(_expr);
	if (_expr->type == ExprType::BinOpK) {
		VISIT(_compiler, Expression, _expr->V.binOp.left);
		VISIT(_compiler, Expression, _expr->V.binOp.right);
		ADDOP_BINARY(_compiler, loc, _expr->V.binOp.op);
	}


	return 1;
}


static AlifIntT compiler_stmtExpr(AlifCompiler* _compiler, SourceLocation _loc, Expression* _val) { // 3988

	if (_compiler->interactive and _compiler->nestLevel <= 1) {
		VISIT(_compiler, Expression, _val);
		ADDOP_I(_compiler, _loc, CALL_INTRINSIC_1, INTRINSIC_PRINT);
		ADDOP(_compiler, noLocation, POP_TOP);
		return 1;
	}

	if (_val->type == ExprType::ConstantK) {
		ADDOP(_compiler, _loc, NOP);
		return 1;
	}

	VISIT(_compiler, Expression, _val);
	ADDOP(_compiler, noLocation, POP_TOP);

	return 1;
}

static AlifIntT compiler_visitStatement(AlifCompiler* _compiler, Statement* _stmt) { // 4009

	if (_stmt->type == StmtType::ExprK) {
		return compiler_stmtExpr(_compiler, LOC(_stmt), _stmt->V.expression.val);
	}


	return 1;
}

static AlifIntT compiler_body(AlifCompiler* _compiler, SourceLocation _loc, StmtSeq* _stmts) { // 1628

	if (_compiler->unit->uScopeType == ScopeType::Compiler_Scope_Module and SEQ_LEN(_stmts)) {
		Statement* stmt = (Statement*)SEQ_GET(_stmts, 0);
		_loc = LOC(stmt);
	}

	if (!SEQ_LEN(_stmts)) return 1;

	AlifSizeT firstInstr = 0;
	AlifObject* docString = alifAST_getDocString(_stmts);
	if (docString) {
		firstInstr = 1;
		//if (_compiler->optimize < 2) {
		//	AlifObject* cleanDoc = alifCompile_cleanDoc(docString);
		//	if (cleanDoc == nullptr) return -1;

		//	Statement* stmt = (Statement*)SEQ_GET(_stmts, 0);
		//	SourceLocation loc = LOC(stmt->V.expression.val);
		//	ADDOP_LOAD_CONST(_compiler, loc, cleanDoc);
		//	ALIF_DECREF(cleanDoc);
		//	AlifObject* name = alifUnicode_decodeStringToUTF8(L"__doc__");
		//	if (compiler_nameOp(_compiler, noLocation, name, ExprCTX::Store) == -1) return -1;
		//}
	}
	for (AlifSizeT i = firstInstr; i < SEQ_LEN(_stmts); i++) {
		VISIT(_compiler, Statement, (Statement*)SEQ_GET(_stmts, i));
	}

	return 1;
}


static AlifIntT addReturn_atEnd(AlifCompiler* _compiler, AlifIntT _addNone) { // 7606
	if (_addNone) {
		ADDOP_LOAD_CONST(_compiler, noLocation, ALIF_NONE);
	}
	ADDOP(_compiler, noLocation, RETURN_VALUE);
	return 1;
}

static AlifObject* constsDict_keysInorder(AlifObject* _dict) { // 7515
	AlifObject* consts{};
	AlifObject* k{};
	AlifObject* v{};
	AlifSizeT i{};
	AlifSizeT pos = 0;
	AlifSizeT size = ALIFDICT_GET_SIZE(_dict);

	consts = alifNew_list(size);
	if (consts == nullptr) return nullptr;

	while (alifDict_next(_dict, &pos, &k, &v, 0)) {
		i = alifInteger_asLongLong(v);
		if (ALIFTUPLE_CHECK(k)) {
			k = ALIFTUPLE_GET_ITEM(k, 1);
		}
		ALIFLIST_SET_ITEM(consts, i, ALIF_NEWREF(k));
	}
	return consts;
}

static AlifCodeObject* optimize_andAssembleCodeUnit(CompilerUnit* _cu, AlifObject* _fn) { // 7619

	AlifFlowGraph* cfg = nullptr;
	InstructionSequence optimizedInstrs{};
	memset(&optimizedInstrs, 0, sizeof(InstructionSequence));

	AlifIntT nLocals{};
	AlifIntT nParams{};
	AlifIntT stackDepth{};
	AlifIntT nLocalsPlus{};

	AlifCodeObject* co = nullptr;
	AlifObject* consts = constsDict_keysInorder(_cu->uData.consts);
	if (consts == nullptr) goto error;

	cfg = instr_sequenceToCFG(_cu->uInstrSequence);
	if (cfg == nullptr) goto error;

	nLocals = (AlifIntT)ALIFDICT_GET_SIZE(_cu->uData.varNames);
	nParams = (AlifIntT)ALIFDICT_GET_SIZE(_cu->uSTE->steVarNames);

	if (alifCFG_optimizeCodeUnit(cfg, consts, nLocals, nParams, _cu->uData.firstLineNo) < 0) {
		goto error;
	}

	if (alifCFG_optimizedCFGToInstructionSeq(cfg, &_cu->uData,
		&stackDepth, &nLocalsPlus, &optimizedInstrs) < 0) {
		goto error;
	}

	/* المجمع */
	co = alifAssemble_makeCodeObject(&_cu->uData, consts,
		stackDepth, &optimizedInstrs, nLocalsPlus, _fn);

error:
	ALIF_XDECREF(consts);
	//alifInstructionsSeq_fini(&optimizedInstrs);
	//alifCFGBuilder_free(cfg);
	return co;
}

static AlifCodeObject* optimize_andAssemble(AlifCompiler* _compiler, AlifIntT _addNone) { // 7666
	CompilerUnit* cu = _compiler->unit;
	AlifObject* fn = _compiler->fileName;

	// flags of code here

	if (addReturn_atEnd(_compiler, _addNone) < 1) {
		return nullptr;
	}

	return optimize_andAssembleCodeUnit(cu, fn);
}
