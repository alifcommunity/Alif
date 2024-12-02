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


typedef AlifJumpTargetLabel JumpTargetLabel; // 26

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

static const JumpTargetLabel _noLable_ = { -1 }; // 89



static BasicBlock* cfgBuilder_newBlock(CFGBuilder* _g) { // 163
	BasicBlock* b_ = (BasicBlock*)alifMem_objAlloc(sizeof(BasicBlock));
	if (b_ == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	b_->list = _g->blockList;
	_g->blockList = b_;
	b_->label = _noLable_;
	return b_;
}



















static AlifIntT init_cfgBuilder(CFGBuilder* _g) { // 402
	_g->blockList = nullptr;
	BasicBlock* block = cfgBuilder_newBlock(_g);
	if (block == nullptr) {
		return ERROR;
	}
	_g->curBlock = _g->entryBlock = block;
	_g->currentLabel = _noLable_;
	return SUCCESS;
}

CFGBuilder* _alifCfgBuilder_new(void) { // 415
	CFGBuilder* g_ = (CFGBuilder*)alifMem_objAlloc(sizeof(CFGBuilder));
	if (g_ == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	memset(g_, 0, sizeof(CFGBuilder));
	if (init_cfgBuilder(g_) < 0) {
		alifMem_objFree(g_);
		return nullptr;
	}
	return g_;
}

void _alifCfgBuilder_Free(CFGBuilder* _g) { // 431
	if (_g == nullptr) {
		return;
	}
	BasicBlock* b_ = _g->blockList;
	while (b_ != nullptr) {
		if (b_->instr) {
			alifMem_objFree((void*)b_->instr);
		}
		BasicBlock* next = b_->list;
		alifMem_objFree((void*)b_);
		b_ = next;
	}
	alifMem_objFree(_g);
}












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















CFGBuilder* _alifCfg_fromInstructionSequence(AlifInstructionSequence* _seq) { // 2745
	if (_alifInstructionSequence_applyLabelMap(_seq) < 0) {
		return nullptr;
	}
	CFGBuilder* g_ = _alifCfgBuilder_new();
	if (g_ == nullptr) {
		return nullptr;
	}
	for (AlifIntT i = 0; i < _seq->used; i++) {
		_seq->instrs[i].target = 0;
	}
	for (AlifIntT i = 0; i < _seq->used; i++) {
		AlifInstruction* instr = &_seq->instrs[i];
		if (HAS_TARGET(instr->opcode)) {
			_seq->instrs[instr->oparg].target = 1;
		}
	}
	for (AlifIntT i = 0; i < _seq->used; i++) {
		AlifInstruction* instr = &_seq->instrs[i];
		if (instr->target) {
			JumpTargetLabel lbl_ = { i };
			if (_alifCfgBuilder_useLabel(g_, lbl_) < 0) {
				goto error;
			}
		}
		AlifIntT opcode = instr->opcode;
		AlifIntT oparg = instr->oparg;
		if (_alifCfgBuilder_Addop(g_, opcode, oparg, instr->loc) < 0) {
			goto error;
		}
	}
	if (_alifCfgBuilder_checkSize(g_) < 0) {
		goto error;
	}
	return g_;
error:
	_alifCfgBuilder_free(g_);
	return nullptr;











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
