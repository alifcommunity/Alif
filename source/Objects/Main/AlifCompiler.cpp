#include "alif.h"
#include "OpCode.h"

#include "AlifCore_AST.h"
#include "AlifCore_OpCodeUtils.h"
#include "AlifCore_Compile.h"
#include "AlifCore_FlowGraph.h"
#include "AlifCore_InstructionSeq.h"
#include "AlifCore_Intrinsics.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_SymTable.h"

#define NEED_OPCODE_DATA
#include "AlifCore_OpCodeData.h"
#undef NEED_OPCODE_DATA

// Forward
class AlifCompiler; // temp
static AlifCodeObject* compiler_module(AlifCompiler*, Module*);
static AlifIntT compiler_body(AlifCompiler*, SourceLocation, StmtSeq*);
static AlifSizeT dict_addObject(AlifObject*, AlifObject*);
static AlifIntT codeGen_addOpI(InstructionSequence*, AlifIntT, AlifSizeT, SourceLocation);
static AlifCodeObject* optimize_andAssemble(AlifCompiler*, AlifIntT);
static AlifIntT compiler_visitExpression(AlifCompiler*, Expression*);


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
	AlifSTEntryObject* symTableEntry{};

	AlifIntT scopeType{};

	AlifObject* private_{};
	AlifObject* staticAttributes{};

	InstructionSequence* instrSequence{};

	AlifIntT nfBlocks{};
	AlifIntT inInlinedComp{};

	FBlockInfo ufBlock[CO_MAXBLOCKS]{};

	AlifCompileCodeUnitData data{};
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

	bool saveNestedSeqs{};
};

#define INSTR_SEQUANCE(_c) (_c->unit->instrSequence)


#define CAPSULE_NAME L"AlifCompile.cpp AlifCompiler Unit" // 377



#define ADDOP(_c, _loc, _op) {if (codeGen_addOpNoArg(INSTR_SEQUANCE(_c), _op, _loc) == -1) return -1;}
#define ADDOP_I(_c, _loc, _op, _o) { if (codeGen_addOpI(INSTR_SEQUANCE(_c), _op, _o, _loc) == -1) return -1;}
#define ADDOP_BINARY(_c, _loc, _binOp) { if (addOp_binary(_c, _loc, _binOp, TRUE) == -1) return -1;}
#define ADDOP_LOAD_CONST(_c, _loc, _o) { if (compilerAddOp_loadConst(_c->unit, _loc, _o) == -1) return -1;}




#define VISIT(_c, _type, _v) {if (compiler_visit ## _type(_c, _v) == -1) return -1;}


static AlifIntT codeGen_addOpNoArg(InstructionSequence* _seq, AlifIntT _opCode, SourceLocation _loc) { // 822
	return alifInstructionSequence_addOp(_seq, _opCode, 0, _loc);
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

	AlifSizeT arg = dict_addObject(_cu->data.consts, key);
	ALIF_DECREF(key);
	return arg;
}

static AlifIntT compilerAddOp_loadConst(CompilerUnit* _cu, SourceLocation _loc, AlifObject* _obj) { // 979
	AlifSizeT arg = compiler_addConst(_cu, _obj);
	if (arg < 0) return -1;

	return codeGen_addOpI(_cu->instrSequence, LOAD_CONST, arg, _loc);
}

static AlifIntT codeGen_addOpI(InstructionSequence* _seq,
	AlifIntT _opCode, AlifSizeT _opArg, SourceLocation _loc) { // 1047

	AlifIntT opArg = (AlifIntT)_opArg;
	return alifInstructionSequence_addOp(_seq, _opCode, opArg, _loc);
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

		if (cu->scopeType == Compiler_Scope_Function
			or cu->scopeType == Compiler_Scope_AsyncFunction
			or cu->scopeType == Compiler_Scope_Class) {
			mangled = alif_mangle(parent->private_, cu->data.name);
			if (!mangled) return -1;

			scope = alifST_getScope(parent->symTableEntry, mangled);
			ALIF_DECREF(mangled);
			if (scope == GLOBAL_EXPLICIT) forceGlobal = 1;
		}

		if (!forceGlobal) {
			if (parent->scopeType == Compiler_Scope_Function
				or parent->scopeType == Compiler_Scope_AsyncFunction)
			{
				//ALIF_DECLARE_STR(dotLocal, L".<locals>");
				AlifObject* name1 = alifUStr_decodeStringToUTF8(L"dotLocals");
				base = alifUStr_concat(parent->data.qualName, name1);
				if (base == nullptr) return -1;
			}
			else {
				base = ALIF_NEWREF(parent->data.qualName);
			}
		}

	}

	if (base != nullptr) {
		//ALIF_DECLARE_STR(dot, L".");
		AlifObject* name2 = alifUStr_decodeStringToUTF8(L"dot");
		name = alifUStr_concat(base, name2);
		ALIF_DECREF(base);
		if (name == nullptr) return -1;
		alifUStr_append(&name, cu->data.name);
		if (name == nullptr) return -1;
	}
	else {
		name = ALIF_NEWREF(cu->data.name);
	}
	cu->data.qualName = name;

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

	cu->scopeType = _scopeType;
	cu->data.argCount = 0;
	cu->data.posOnlyArgCount = 0;
	cu->data.kwOnlyArgCount = 0;
	cu->symTableEntry = alifSymTable_lookup(_compiler->symTable, _key);
	if (!cu->symTableEntry) {
		// error
		return -1;
	}

	cu->data.name = ALIF_NEWREF(_name);
	cu->data.varNames = list_toDict(cu->symTableEntry->steVarNames);
	if (!cu->data.varNames) {
		//compilerUnit_free(cu);
		return -1;
	}

	cu->data.cellVars = dict_byType(cu->symTableEntry->steSymbols, CELL, DEF_COMP_CELL, 0);
	if (!cu->data.cellVars) {
		//compilerUnit_free(cu);
		return -1;
	}

	if (cu->symTableEntry->steNeedsClassClosure) {
		AlifSizeT res{};
		AlifObject* name = alifUStr_decodeStringToUTF8(L"__class__");
		res = dict_addObject(cu->data.cellVars, name);
		if (res < 0) {
			//compilerUnit_free(cu);
			return -1;
		}
	}

	if (cu->symTableEntry->steNeedsClassDict) {
		AlifSizeT res{};
		AlifObject* name = alifUStr_decodeStringToUTF8(L"__classdict__");
		res = dict_addObject(cu->data.cellVars, name);
		if (res < 0) {
			//compilerUnit_free(cu);
			return -1;
		}
	}

	cu->data.freeVars = dict_byType(cu->symTableEntry->steSymbols, FREE, DEF_FREE_CLASS, ALIFDICT_GET_SIZE(cu->data.cellVars));
	if (!cu->data.freeVars) {
		//compilerUnit_free(cu);
		return -1;
	}

	cu->nfBlocks = 0;
	cu->inInlinedComp = 0;
	cu->data.firstLineNo = _lineNo;
	cu->data.consts = alifNew_dict();
	if (!cu->data.consts) {
		//compilerUnit_free(cu);
		return -1;
	}

	cu->data.names = alifNew_dict();
	if (!cu->data.names) {
		//compilerUnit_free(cu);
		return -1;
	}

	cu->private_ = nullptr;
	if (_scopeType == ScopeType::Compiler_Scope_Class) {
		cu->staticAttributes = alifNew_set(0);
		if (!cu->staticAttributes) {
			//compilerUnit_free(cu);
			return -1;
		}
	}
	else {
		cu->staticAttributes = nullptr;
	}

	cu->instrSequence = (InstructionSequence*)alifInstructionSequance_new();

	if (_compiler->unit) {
		AlifObject* capsule = alifCapsule_new(_compiler->unit, CAPSULE_NAME, nullptr);
		if (!capsule or alifList_append(_compiler->stack, capsule) < 0) {
			ALIF_XDECREF(capsule);
			//compilerUnit_free(cu);
			return -1;
		}
		ALIF_DECREF(capsule);
		cu->private_ = ALIF_XNEWREF(_compiler->unit->private_);
	}
	_compiler->unit = cu;

	_compiler->nestLevel++;

	if (cu->scopeType == ScopeType::Compiler_Scope_Module) {
		location.lineNo = 0;
	}
	else {
		if (compiler_setQualName(_compiler) < 0) {
			return -1;
		}
	}
	ADDOP_I(_compiler, location, RESUME, RESUME_AT_FUNC_START);

	if (cu->scopeType == ScopeType::Compiler_Scope_Module) {
		location.lineNo = -1;
	}

	return 1;
}

static void compiler_exitScope(AlifCompiler* _compiler) { // 1293
	//AlifObject* exc = alifErr_getRaisedException();

	InstructionSequence* nestedSeq = nullptr;
	if (_compiler->saveNestedSeqs) {
		nestedSeq = _compiler->unit->instrSequence;
		ALIF_INCREF(nestedSeq);
	}
	_compiler->nestLevel--;
	//compiler_unitFree(_compiler->unit);
	/* Restore c->u to the parent unit. */
	AlifSizeT n = ALIFLIST_GET_SIZE(_compiler->stack) - 1;
	if (n >= 0) {
		AlifObject* capsule = ALIFLIST_GET_ITEM(_compiler->stack, n);
		_compiler->unit = (CompilerUnit*)alifCapsule_getPointer(capsule, CAPSULE_NAME);

		if (alifSequence_delItem(_compiler->stack, n) < 0) {
			// error
		}
		if (nestedSeq != nullptr) {
			if (alifInstructionSequence_addNested(_compiler->unit->instrSequence, nestedSeq) < 0) {
				// error
			}
		}
	}
	else {
		_compiler->unit = nullptr;
	}
	ALIF_XDECREF(nestedSeq);

	//alifErr_setRaisedException(exc);
}

static AlifIntT compiler_enterAnonymousScope(AlifCompiler* _compiler, Module* _module) { // 1700

	AlifObject* name = alifUStr_decodeStringToUTF8(L"<module>");
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

static AlifIntT compiler_nameOp(AlifCompiler* _compiler, SourceLocation _loc, AlifObject* _name, ExprCTX _ctx) { // 4187

	enum { FastOp, GlobalOp, DeRefOp, NameOp } opType;

	AlifObject* dict = _compiler->unit->data.names;

	AlifObject* mangled = alif_mangle(_compiler->unit->private_, _name);
	if (!mangled) return -1;

	AlifIntT op = 0;
	opType = NameOp;
	AlifIntT scope = alifST_getScope(_compiler->unit->symTableEntry, mangled);
	switch (scope) {
		// code here
	default:
		break;
	}

	switch (opType)
	{
	case NameOp:
		switch (_ctx)
		{
		case Load:
			op = (_compiler->unit->symTableEntry->steType == AlifBlockType::ClassBlock
				and _compiler->unit->inInlinedComp) ? LOAD_GLOBAL : LOAD_NAME;
			break;
		case Store:
			op = STORE_NAME;
			break;
		case Del:
			op = DELETE_NAME;
			break;
		}
		break;
	}

	AlifSizeT arg = dict_addObject(dict, mangled);
	ALIF_DECREF(mangled);
	if (arg < 0) return -1;

	return codeGen_addOpI(INSTR_SEQUANCE(_compiler), op, arg, _loc);
}

static AlifIntT maybeOptimize_methodCall(AlifCompiler* _compiler, Expression* _expr) { // 4931
	AlifSizeT argsl{}, i{}, kwdsl{};
	Expression* meth = _expr->V.call.func;
	ExprSeq* args = _expr->V.call.args;
	KeywordSeq* kwds = _expr->V.call.keywords;

	/* Check that the call node is an attribute access */
	if (meth->type != ExprType::AttributeK or meth->V.attribute.ctx != ExprCTX::Load) {
		return 0;
	}

	/*
	// code here
	*/
}

static AlifIntT compiler_callHelper(AlifCompiler* _compiler, SourceLocation _loc, AlifIntT _n,
	ExprSeq* _args, KeywordSeq* _keywords) { // 5189

	//if(validate_keywords(_compiler, _keywords) == -1) return -1;

	AlifSizeT nElts = SEQ_LEN(_args);
	AlifSizeT nKwElts = SEQ_LEN(_keywords);

	// code here


	for (AlifSizeT i = 0; i < nElts; i++) {
		Expression* elt = SEQ_GET(_args, i);
		VISIT(_compiler, Expression, elt);
	}
	if (nKwElts) {
		// code here
	}
	else {
		ADDOP_I(_compiler, _loc, CALL, _n + nElts);
	}

	return 1;

exCall:

	return 0; // temp
}

static AlifIntT compiler_call(AlifCompiler* _compiler, Expression* _expr) { // 5026

	//if(validate_keywords(_compiler, _expr->V.call.keywords) == -1) return -1;
	AlifIntT ret = maybeOptimize_methodCall(_compiler, _expr);
	if (ret < 0) {
		return -1;
	}
	if (ret == 1) {
		return 1;
	}

	//if(check_caller(_compiler, _expr->V.call.func) == -1) return -1;

	VISIT(_compiler, Expression, _expr->V.call.func);
	SourceLocation loc = LOC(_expr->V.call.func);
	ADDOP(_compiler, loc, PUSH_NULL);
	loc = LOC(_expr);
	return compiler_callHelper(_compiler, loc, 0, _expr->V.call.args, _expr->V.call.keywords);
}

static AlifIntT compiler_visitExpression(AlifCompiler* _compiler, Expression* _expr) { // 6341

	SourceLocation loc = LOC(_expr);
	if (_expr->type == ExprType::BinOpK) {
		VISIT(_compiler, Expression, _expr->V.binOp.left);
		VISIT(_compiler, Expression, _expr->V.binOp.right);
		ADDOP_BINARY(_compiler, loc, _expr->V.binOp.op);
	}
	else if (_expr->type == ExprType::CallK) {
		return compiler_call(_compiler, _expr);
	}
	else if (_expr->type == ExprType::ConstantK) {
		ADDOP_LOAD_CONST(_compiler, loc, _expr->V.constant.val);
	}
	else if (_expr->type == ExprType::NameK) {
		return compiler_nameOp(_compiler, loc, _expr->V.name.name, _expr->V.name.ctx);
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

	if (_compiler->unit->scopeType == ScopeType::Compiler_Scope_Module and SEQ_LEN(_stmts)) {
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
		i = alifInteger_asLong(v);
		if (ALIFTUPLE_CHECKEXACT(k)) {
			k = ALIFTUPLE_GET_ITEM(k, 1);
		}
		ALIFLIST_SETITEM(consts, i, ALIF_NEWREF(k));
	}
	return consts;
}

static AlifFlowGraph* instrSequence_toCFG(InstructionSequence* _seq) { // 200

	if (alifInstructionSeq_applyLableMap(_seq) < 0) {
		return nullptr;
	}

	AlifFlowGraph* cfg = alifFlowGraph_new();
	if (cfg == nullptr) return nullptr;

	for (AlifIntT i = 0; i < _seq->used; i++) {
		_seq->instructions[i].target = 0;
	}
	for (AlifIntT i = 0; i < _seq->used; i++) {
		AlifInstruction* instr = &_seq->instructions[i];
		if (HAS_TARGET(instr->opCode)) {
			_seq->instructions[instr->opArg].target = 1;
		}
	}
	for (AlifIntT i = 0; i < _seq->used; i++) {
		AlifInstruction* instr = &_seq->instructions[i];
		if (instr->target) {
			JumpTargetLable lable = { i };
			if (alifFlowGraph_useLable(cfg, lable) < 0) {
				goto error;
			}
		}
		AlifIntT opCode = instr->opCode;
		AlifIntT opArg = instr->opArg;
		if (alifFlowGraph_addOp(cfg, opCode, opArg, instr->loc) < 0) {
			goto error;
		}
	}
	if (alifFlowGraph_checkSize(cfg) < 0) {
		goto error;
	}

	return cfg;

error:
	//alifFlowGraph_free(cfg);
	return nullptr;
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
	AlifObject* consts = constsDict_keysInorder(_cu->data.consts);
	if (consts == nullptr) goto error;

	cfg = instrSequence_toCFG(_cu->instrSequence);
	if (cfg == nullptr) goto error;

	nLocals = (AlifIntT)ALIFDICT_GET_SIZE(_cu->data.varNames);
	nParams = (AlifIntT)ALIFDICT_GET_SIZE(_cu->symTableEntry->steVarNames);

	if (alifCFG_optimizeCodeUnit(cfg, consts, nLocals, nParams, _cu->data.firstLineNo) < 0) {
		goto error;
	}

	if (alifCFG_optimizedCFGToInstructionSeq(cfg, &_cu->data,
		&stackDepth, &nLocalsPlus, &optimizedInstrs) < 0) {
		goto error;
	}

	/* المجمع */
	co = alifAssemble_makeCodeObject(&_cu->data, consts,
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
