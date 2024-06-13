#pragma once




#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSeq.h"
#include "AlifCore_OpCodeUtils.h"





class AlifFlowGraph;


AlifIntT alifFlowGraph_useLable(AlifFlowGraph*, JumpTargetLable);
AlifIntT alifFlowGraph_addOp(AlifFlowGraph*, AlifIntT, AlifIntT, SourceLocation);

AlifFlowGraph* alifFlowGraph_new();

AlifIntT alifFlowGraph_checkSize(AlifFlowGraph*);

AlifIntT alifCFG_optimizeCodeUnit(AlifFlowGraph*, AlifObject*, AlifIntT, AlifIntT, AlifIntT);

AlifIntT alifCFG_toInstructionSeq(AlifFlowGraph*, InstructionSequence*);
AlifIntT alifCFG_optimizedCFGToInstructionSeq(AlifFlowGraph*, AlifCompileCodeUnitData*,
	AlifIntT*, AlifIntT*, InstructionSequence*);

AlifCodeObject* alifAssemble_makeCodeObject(AlifCompileCodeUnitData*,
	AlifObject*, AlifIntT, InstructionSequence*, AlifIntT, AlifObject*);