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
		if (b == nullptr) {
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

AlifIntT _alifCFGBuilder_checkSize(CFGBuilder* _g) { // 450
	AlifIntT nBlocks = 0;
	for (BasicBlock* b = _g->blockList; b != nullptr; b = b->list) {
		nBlocks++;
	}
	if ((size_t)nBlocks > SIZE_MAX / sizeof(BasicBlock*)) {
		//alifErr_noMemory();
		return ERROR;
	}
	return SUCCESS;
}


AlifIntT _alifCFGBuilder_useLabel(CFGBuilder* _g, JumpTargetLabel _lbl) { // 464
	_g->currentLabel = _lbl;
	return cfgBuilderMaybe_startNewBlock(_g);
}


AlifIntT _alifCFGBuilder_addOp(CFGBuilder* _g, AlifIntT _opcode, AlifIntT _oparg, Location _loc) { // 470
	RETURN_IF_ERROR(cfgBuilder_maybeStartNewBlock(_g));
	return basicBlock_addOp(_g->curBlock, _opcode, _oparg, _loc);
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



static BasicBlock** makeCfg_traversalStack(BasicBlock* _entryBlock) { // 741
	AlifIntT nBlocks = 0;
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		b->visited = 0;
		nBlocks++;
	}
	BasicBlock** stack = (BasicBlock**)alifMem_objAlloc(sizeof(BasicBlock*) * nBlocks);
	if (!stack) {
		//alifErr_noMemory();
	}
	return stack;
}

ALIF_LOCAL(AlifIntT) stack_effect(AlifIntT _opcode, AlifIntT _oparg, AlifIntT _jump) { // 764
	if (_opcode < 0) {
		return ALIF_INVALID_STACK_EFFECT;
	}
	if ((_opcode <= MAX_REAL_OPCODE) and (_alifOpcode_deopt[_opcode] != _opcode)) {
		return ALIF_INVALID_STACK_EFFECT;
	}
	AlifIntT popped = _alifOpcode_num_popped(_opcode, _oparg);
	AlifIntT pushed = _alifOpcode_num_pushed(_opcode, _oparg);
	if (popped < 0 or pushed < 0) {
		return ALIF_INVALID_STACK_EFFECT;
	}
	if (IS_BLOCK_PUSH_OPCODE(_opcode) and !_jump) {
		return 0;
	}
	return pushed - popped;
}

ALIF_LOCAL_INLINE(AlifIntT) stackDepth_push(BasicBlock*** _sp, BasicBlock* _b, AlifIntT _depth) { // 785
	if (!(_b->startDepth < 0 or _b->startDepth == _depth)) {
		//alifErr_format(_alifExcValueError_, "Invalid CFG, inconsistent stackdepth");
		return ERROR;
	}
	if (_b->startDepth < _depth and _b->startDepth < 100) {
		_b->startDepth = _depth;
		*(*_sp)++ = _b;
	}
	return SUCCESS;
}



static AlifIntT calculate_stackdepth(CFGBuilder* _g) { // 803
	BasicBlock* entryBlock = _g->entryBlock;
	for (BasicBlock* b = entryBlock; b != nullptr; b = b->next) {
		b->startDepth = INT_MIN;
	}
	BasicBlock** stack = makeCfg_traversalStack(entryBlock);
	if (!stack) {
		return ERROR;
	}


	AlifIntT stackDepth = -1;
	AlifIntT maxDepth = 0;
	BasicBlock** sp_ = stack;
	if (stackDepth_push(&sp_, entryBlock, 0) < 0) {
		goto error;
	}
	while (sp_ != stack) {
		BasicBlock* b_ = *--sp_;
		AlifIntT depth = b_->startDepth;
		BasicBlock* next = b_->next;
		for (AlifIntT i = 0; i < b_->iused; i++) {
			CFGInstr* instr_ = &b_->instr[i];
			AlifIntT effect = stack_effect(instr_->opcode, instr_->oparg, 0);
			if (effect == ALIF_INVALID_STACK_EFFECT) {
				//alifErr_format(_alifExcSystemError_,
					//"Invalid stack effect for opcode=%d, arg=%i",
					//instr->opcode, instr->oparg);
				goto error;
			}
			AlifIntT newDepth = depth + effect;
			if (newDepth < 0) {
				//alifErr_format(_alifExcValueError_,
					//"Invalid CFG, stack underflow");
				goto error;
			}
			if (newDepth > maxDepth) {
				maxDepth = newDepth;
			}
			if (HAS_TARGET(instr_->opcode)) {
				effect = stack_effect(instr_->opcode, instr_->oparg, 1);
				if (effect == ALIF_INVALID_STACK_EFFECT) {
					//alifErr_format(_alifExcSystemError_,
						//"Invalid stack effect for opcode=%d, arg=%i",
						//instr->opcode, instr->oparg);
					goto error;
				}
				AlifIntT targetDepth = depth + effect;
				if (targetDepth > maxDepth) {
					maxDepth = targetDepth;
				}
				if (stackDepth_push(&sp_, instr_->target, targetDepth) < 0) {
					goto error;
				}
			}
			depth = newDepth;
			if (IS_UNCONDITIONAL_JUMP_OPCODE(instr_->opcode) or
				IS_SCOPE_EXIT_OPCODE(instr_->i_opcode))
			{
				next = nullptr;
				break;
			}
		}
		if (next != nullptr) {
			if (stackDepth_push(&sp_, next, depth) < 0) {
				goto error;
			}
		}
	}
	stackDepth = maxDepth;
error:
	alifMem_objFree(stack);
	return stackDepth;
}












































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
