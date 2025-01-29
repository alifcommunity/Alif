#pragma once

#include "AlifCore_AST.h"
#include "AlifCore_SymTable.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_Memory.h"












AlifCodeObject* _alifAST_compile(Module*, AlifObject*,
	AlifCompilerFlags*, AlifIntT, AlifASTMem*);






extern AlifIntT alifAST_optimize(Module*, AlifASTMem*, AlifIntT, AlifIntT);















class AlifCompileCodeUnitMetadata {
public:
	AlifObject* name{};
	AlifObject* qualname{};

	AlifObject* consts{};    
	AlifObject* names{};     
	AlifObject* varnames{};  
	AlifObject* cellvars{};  
	AlifObject* freevars{};  
	AlifObject* fasthidden{};

	AlifSizeT argCount{};
	AlifSizeT posOnlyArgCount{};
	AlifSizeT kwOnlyArgCount{};

	AlifIntT firstLineno;
};






class AlifCompiler;

enum AlifCompilerOpType {
	Compiler_Op_Fast,
	Compiler_Op_Global,
	Compiler_Op_DeRef,
	Compiler_Op_Name,
};








enum AlifCompileFBlockType {
	Compiler_FBlock_While_Loop,
	Compiler_FBlock_For_Loop,
	Compiler_FBlock_Try_Except,
	Compiler_FBlock_Finally_Try,
	Compiler_FBlock_Finally_End,
	Compiler_FBlock_With,
	Compiler_FBlock_Async_With,
	Compiler_FBlock_Handler_Cleanup,
	Compiler_FBlock_Pop_Value,
	Compiler_FBlock_Exception_Handler,
	Compiler_FBlock_Exception_Group_Handler,
	Compiler_FBlock_Async_Comprehension_Generator,
	Compiler_FBlock_Stop_Iteration,
};

class AlifCompileFBlockInfo {
public:
	AlifCompileFBlockType type{};
	AlifJumpTargetLabel block{};
	AlifSourceLocation loc{};
	AlifJumpTargetLabel exit{};
	void* datum{};
};



AlifIntT _alifCompiler_pushFBlock(AlifCompiler*, Location,
	AlifCompileFBlockType, AlifJumpTargetLabel, AlifJumpTargetLabel, void*);


void _alifCompiler_popFBlock(AlifCompiler*, AlifCompileFBlockType, AlifJumpTargetLabel);

class AlifCompileFBlockInfo* _alifCompiler_topFBlock(AlifCompiler*);

AlifIntT _alifCompiler_enterScope(AlifCompiler*, Identifier, AlifIntT,
	void*, AlifIntT, AlifObject*, AlifCompileCodeUnitMetadata*);

void _alifCompiler_exitScope(AlifCompiler*);
AlifSizeT _alifCompiler_addConst(AlifCompiler*, AlifObject*);
AlifInstructionSequence* _alifCompiler_instrSequence(AlifCompiler*);



AlifObject* _alifCompiler_maybeMangle(AlifCompiler*, AlifObject*);
AlifIntT _alifCompiler_maybeAddStaticAttributeToClass(AlifCompiler*, ExprTy);
AlifIntT _alifCompiler_getRefType(AlifCompiler*, AlifObject*);
AlifIntT _alifCompiler_lookupCellVar(AlifCompiler*, AlifObject*);
AlifIntT _alifCompiler_resolveNameOp(AlifCompiler*, AlifObject*, AlifIntT,
	AlifCompilerOpType*, AlifSizeT*);




AlifIntT _alifCompiler_isInInlinedComp(AlifCompiler*);

AlifIntT _alifCompiler_optimizationLevel(AlifCompiler*);
AlifIntT _alifCompiler_lookupArg(AlifCompiler*, AlifCodeObject*, AlifObject*);
AlifObject* _alifCompiler_qualname(AlifCompiler*);
AlifCompileCodeUnitMetadata* _alifCompiler_metadata(AlifCompiler*);
AlifObject* _alifCompiler_staticAttributesTuple(AlifCompiler*);

AlifSymTable* _alifCompiler_symTable(AlifCompiler*);
SymTableEntry* _alifCompiler_symTableEntry(AlifCompiler*);

enum ScopeType_ {
	Compiler_Scope_Module,
	Compiler_Scope_Class,
	Compiler_Scope_Function,
	Compiler_Scope_Async_Function,
	Compiler_Scope_Lambda,
	Compiler_Scope_Comprehension,
	Compiler_Scope_Annotations,
};


class AlifCompilerInlinedComprehensionState {
public:
	AlifObject* pushedLocals{};
	AlifObject* tempSymbols{};
	AlifObject* fastHidden{};
	AlifJumpTargetLabel cleanup{};
};
AlifIntT _alifCompiler_tweakInlinedComprehensionScopes(AlifCompiler*,
	AlifSourceLocation, SymTableEntry*, AlifCompilerInlinedComprehensionState*);

AlifIntT _alifCompiler_revertInlinedComprehensionScopes(AlifCompiler*,
	AlifSourceLocation, AlifCompilerInlinedComprehensionState*);


AlifIntT _alifCodegen_addReturnAtEnd(AlifCompiler*, AlifIntT);
AlifIntT _alifCodegen_enterAnonymousScope(AlifCompiler*, ModuleTy);
AlifIntT _alifCodegen_expression(AlifCompiler*, ExprTy);
AlifIntT _alifCodegen_body(AlifCompiler*, AlifSourceLocation, ASDLStmtSeq*, bool);



AlifIntT _alifCompiler_ensureArrayLargeEnough(AlifIntT,
	void**, AlifIntT*, AlifIntT, AlifUSizeT);





AlifIntT _alifCompiler_constCacheMergeOne(AlifObject*, AlifObject**);

AlifCodeObject* _alifCompiler_optimizeAndAssemble(AlifCompiler*, AlifIntT);

AlifSizeT _alifCompiler_dictAddObj(AlifObject*, AlifObject*);
