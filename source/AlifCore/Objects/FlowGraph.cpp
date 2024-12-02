#include "alif.h"

#include "AlifCore_FlowGraph.h"
#include "AlifCore_Compile.h"


 // 13
#undef SUCCESS
#undef ERROR
#define SUCCESS 0
#define ERROR -1

#define RETURN_IF_ERROR(X)  \
    if ((X) == -1) {        \
        return ERROR;       \
    }
 // 21




class AlifCFGInstruction { // 28
public:
	AlifIntT opcode{};
	AlifIntT oparg{};
	AlifSourceLocation loc{};
	class AlifCFGBasicBlock* target{}; /* target block (if jump instruction) */
	class AlifCFGBasicBlock* except{}; /* target block when exception is raised */
};


typedef AlifCFGInstruction CFGInstr; // alif


class AlifCFGBasicBlock { // 36
public:
	AlifCFGBasicBlock* list{};
	AlifJumpTargetLabel label{};
	class AlifCFGExceptStack* exceptStack{};
	CFGInstr* instr{};
	AlifCFGBasicBlock* next{};
	AlifIntT iused{};
	AlifIntT ialloc{};
	uint64_t unsafeLocalsMask{};
	AlifIntT predecessors{};
	AlifIntT startDepth{};
	unsigned preserveLasti : 1;
	unsigned visited : 1;
	unsigned except_handler : 1;
	unsigned cold : 1;
	unsigned warm : 1;
};


typedef AlifCFGBasicBlock BasicBlock; // alif



class AlifCFGBuilder { // 74
public:
	/* The entryblock, at which control flow begins. All blocks of the
	   CFG are reachable through the b_next links */
	AlifCFGBasicBlock* entryBlock{};
	/* Pointer to the most recently allocated block.  By following
	   list links, you can reach all allocated blocks. */
	AlifCFGBasicBlock* blockList{};
	/* pointer to the block currently being constructed */
	AlifCFGBasicBlock* curBlock{};
	/* label for the next instruction to be placed */
	AlifJumpTargetLabel currentLabel{};
};







typedef class AlifCFGBuilder CFGBuilder; // 87



































void alifCFGBuilder_free(CFGBuilder* _g) { // 430
	if (_g == nullptr) {
		return;
	}
	BasicBlock* b = _g->blockList;
	while (b != nullptr) {
		if (b->instr) {
			alifMem_dataFree((void*)b->instr);
		}
		BasicBlock* next = b->list;
		alifMem_dataFree((void*)b);
		b = next;
	}
	alifMem_dataFree(_g);
}






























class AlifCFGExceptStack { // 687
	BasicBlock* handlers[MAXBLOCKS + 2]{};
	AlifIntT depth{};
};

















































AlifIntT alifCFG_optimizeCodeUnit(CFGBuilder* _g, AlifObject* _consts,
	AlifObject* _constCache, AlifIntT _nlocals,
	AlifIntT _nparams, AlifIntT _firstLineno) { // 2511
	RETURN_IF_ERROR(translateJump_labelsToTargets(_g->entryBlock));
	RETURN_IF_ERROR(markExcept_handlers(_g->entryBlock));
	RETURN_IF_ERROR(labelException_targets(_g->entryBlock));

	/** Optimization **/
	RETURN_IF_ERROR(optimize_cfg(_g, _consts, _constCache, _firstLineno));
	RETURN_IF_ERROR(remove_unusedConsts(_g->entryblock, _consts));
	RETURN_IF_ERROR(
		addChecksFor_loadsOfUninitializedVariables(
			_g->entryBlock, _nlocals, _nparams));
	RETURN_IF_ERROR(insert_superInstructions(_g));

	RETURN_IF_ERROR(pushCold_blocksToEnd(_g));
	RETURN_IF_ERROR(resolve_lineNumbers(_g, _firstLineno));
	return SUCCESS;
}



























AlifIntT alifCFG_optimizedCFGToInstructionSequence(CFGBuilder* _g,
	AlifCompileCodeUnitMetadata* _umd, AlifIntT _codeFlags, AlifIntT* _stackDepth,
	AlifIntT* _nLocalsPlus, AlifInstructionSequence* _seq) { // 2825
	*_stackDepth = calculate_stackDepth(_g);
	if (*_stackDepth < 0) {
		return ERROR;
	}

	*_nLocalsPlus = prepare_localsPlus(_umd, _g, _codeFlags);
	if (*_nLocalsPlus < 0) {
		return ERROR;
	}

	RETURN_IF_ERROR(convert_pseudoOps(_g));

	/* Order of basic blocks must have been determined by now */

	RETURN_IF_ERROR(normalize_jumps(_g));

	/* Can't modify the bytecode after computing jump offsets. */
	if (alifCFG_toInstructionSequence(_g, _seq) < 0) {
		return ERROR;
	}

	return SUCCESS;
}
