#pragma once


#include "AlifCore_Compile.h"
#include "AlifCore_InstructionSequence.h"
#include "AlifCore_OpcodeUtils.h"




class AlifCFGBuilder;


void alifCFGBuilder_free(AlifCFGBuilder*); // 21


AlifIntT alifCFG_optimizeCodeUnit(AlifCFGBuilder*, AlifObject*,
	AlifObject*, AlifIntT, AlifIntT, AlifIntT); // 24




AlifIntT alifCFG_optimizedCFGToInstructionSequence(CFGBuilder*, AlifCompileCodeUnitMetadata*,
	AlifIntT, AlifIntT*, AlifIntT*, AlifInstructionSequence*); // 29






AlifCodeObject* alifAssemble_makeCodeObject(AlifCompileCodeUnitMetadata*, AlifObject*,
	AlifObject*, AlifIntT, AlifInstructionSequence*, AlifIntT, AlifIntT, AlifObject*); // 34
