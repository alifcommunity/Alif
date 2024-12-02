#include "alif.h"

#include "AlifCore_FlowGraph.h"
#include "AlifCore_Compile.h"

#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_OpcodeMetaData.h"


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







typedef AlifCFGBuilder CFGBuilder; // 87

static const JumpTargetLabel _noLable_ = { -1 }; // 89

// 91
#define SAME_LABEL(_l1, _l2) ((_l1).id == (_l2).id)
#define IS_LABEL(_l) (!SAME_LABEL((_l), (_noLable_)))

static CFGInstr* basicBlock_lastInstr(const BasicBlock* _b) { // 149
	if (_b->iused > 0) {
		return &_b->instr[_b->iused - 1];
	}
	return nullptr;
}

static BasicBlock* cfgBuilder_newBlock(CFGBuilder* _g) { // 163
	BasicBlock* b_ = (BasicBlock*)alifMem_dataAlloc(sizeof(BasicBlock));
	if (b_ == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	b_->list = _g->blockList;
	_g->blockList = b_;
	b_->label = _noLable_;
	return b_;
}







static BasicBlock* cfgBuilder_useNextBlock(CFGBuilder* _g, BasicBlock* _block) { // 321
	_g->curBlock->next = _block;
	_g->curBlock = _block;
	return _block;
}



static bool cfgBuilder_currentBlockIsTerminated(CFGBuilder* _g) { // 346
	CFGInstr* last = basicBlock_lastInstr(_g->curBlock);
	if (last and IS_TERMINATOR_OPCODE(last->opcode)) {
		return true;
	}
	if (IS_LABEL(_g->currentLabel)) {
		if (last or IS_LABEL(_g->curBlock->label)) {
			return true;
		}
		else {
			_g->curBlock->label = _g->currentLabel;
			_g->currentLabel = _noLable_;
		}
	}
	return false;
}

static AlifIntT cfgBuilderMaybe_startNewBlock(CFGBuilder* _g) { // 366
	if (cfgBuilder_currentBlockIsTerminated(_g)) {
		BasicBlock* b = cfgBuilder_newBlock(_g);
		if (b == NULL) {
			return ERROR;
		}
		b->label = _g->currentLabel;
		_g->currentLabel = _noLable_;
		cfgBuilder_useNextBlock(_g, b);
	}
	return SUCCESS;
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

CFGBuilder* _alifCFGBuilder_new(void) { // 415
	CFGBuilder* g_ = (CFGBuilder*)alifMem_dataAlloc(sizeof(CFGBuilder));
	if (g_ == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	memset(g_, 0, sizeof(CFGBuilder));
	if (init_cfgBuilder(g_) < 0) {
		alifMem_dataFree(g_);
		return nullptr;
	}
	return g_;
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



AlifIntT _alifCfgBuilder_useLabel(CFGBuilder* _g, JumpTargetLabel _lbl) { // 464
	_g->currentLabel = _lbl;
	return cfgBuilderMaybe_startNewBlock(_g);
}










static AlifIntT get_maxLabel(BasicBlock* _entryBlock) { // 622
	AlifIntT lbl = -1;
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		if (b->label.id > lbl) {
			lbl = b->label.id;
		}
	}
	return lbl;
}



static AlifIntT translateJump_labelsToTargets(BasicBlock* _entryBlock) { // 635
	AlifIntT maxLabel = get_maxLabel(_entryBlock);
	AlifUSizeT mapSize = sizeof(BasicBlock*) * (maxLabel + 1);
	BasicBlock** label2block = (BasicBlock**)alifMem_dataAlloc(mapSize);
	if (!label2block) {
		//alifErr_noMemory();
		return ERROR;
	}
	memset(label2block, 0, mapSize);
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		if (b->label.id >= 0) {
			label2block[b->label.id] = b;
		}
	}
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			if (HAS_TARGET(instr->opcode)) {
				AlifIntT lbl = instr->oparg;
				instr->target = label2block[lbl];
			}
		}
	}
	alifMem_dataFree(label2block);
	return SUCCESS;
}











class AlifCFGExceptStack { // 687
public:
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
	RETURN_IF_ERROR(remove_unusedConsts(_g->entryBlock, _consts));
	RETURN_IF_ERROR(
		addChecksFor_loadsOfUninitializedVariables(
			_g->entryBlock, _nlocals, _nparams));
	RETURN_IF_ERROR(insert_superInstructions(_g));

	RETURN_IF_ERROR(pushCold_blocksToEnd(_g));
	RETURN_IF_ERROR(resolve_lineNumbers(_g, _firstLineno));
	return SUCCESS;
}















CFGBuilder* alifCFG_fromInstructionSequence(AlifInstructionSequence* _seq) { // 2745
	if (_alifInstructionSequence_applyLabelMap(_seq) < 0) {
		return nullptr;
	}
	CFGBuilder* g_ = _alifCFGBuilder_new();
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
			if (_alifCFGBuilder_useLabel(g_, lbl_) < 0) {
				goto error;
			}
		}
		AlifIntT opcode = instr->opcode;
		AlifIntT oparg = instr->oparg;
		if (_alifCFGBuilder_addOp(g_, opcode, oparg, instr->loc) < 0) {
			goto error;
		}
	}
	if (_alifCFGBuilder_checkSize(g_) < 0) {
		goto error;
	}
	return g_;
error:
	alifCFGBuilder_free(g_);
	return nullptr;
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
