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

#define DEFAULT_BLOCK_SIZE 16 // 23


typedef AlifJumpTargetLabel JumpTargetLabel; // 26

class AlifCFGInstruction { // 28
public:
	AlifIntT opcode{};
	AlifIntT oparg{};
	AlifSourceLocation loc{};
	class AlifCFGBasicBlock* target{}; /* target block (if jump instruction) */
	class AlifCFGBasicBlock* except{}; /* target block when exception is raised */
};


typedef AlifCFGInstruction CFGInstr; //* alif


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
	unsigned preserveLastI : 1;
	unsigned visited : 1;
	unsigned exceptHandler : 1;
	unsigned cold : 1;
	unsigned warm : 1;
};


typedef AlifCFGBasicBlock BasicBlock; //* alif



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


// 91
#define SAME_LABEL(_l1, _l2) ((_l1).id == (_l2).id)
#define IS_LABEL(_l) (!SAME_LABEL((_l), (NO_LABEL)))

#define LOCATION(_lno, _endLno, _col, _endCol) {_lno, _endLno, _col, _endCol} // 94

static inline AlifIntT is_blockPush(CFGInstr* _i) { // 97
	return IS_BLOCK_PUSH_OPCODE(_i->opcode);
}



static inline AlifIntT is_jump(CFGInstr* _i) { // 105
	return OPCODE_HAS_JUMP(_i->opcode);
}

 // 110
/* One arg*/
#define INSTR_SET_OP1(_i, _op, _arg) \
    do { \
        CFGInstr *instrPtr = _i; \
        instrPtr->opcode = _op; \
        instrPtr->oparg = _arg; \
    } while (0);


 // 120
/* No args*/
#define INSTR_SET_OP0(_i, _op) \
    do { \
        CFGInstr *instrPtr = _i; \
        instrPtr->opcode = _op; \
        instrPtr->oparg = 0; \
    } while (0);


static AlifIntT basicBlock_nextInstr(BasicBlock* _b) { // 135
	RETURN_IF_ERROR(
		_alifCompiler_ensureArrayLargeEnough(
			_b->iused + 1,
			(void**)&_b->instr,
			&_b->ialloc,
			DEFAULT_BLOCK_SIZE,
			sizeof(CFGInstr)));
	return _b->iused++;
}

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
	b_->label = NO_LABEL;
	return b_;
}

static AlifIntT basicBlock_addOp(BasicBlock* _b, AlifIntT _opcode,
	AlifIntT _oparg, Location _loc) { // 178
	AlifIntT off_ = basicBlock_nextInstr(_b);
	if (off_ < 0) {
		return ERROR;
	}
	CFGInstr* i = &_b->instr[off_];
	i->opcode = _opcode;
	i->oparg = _oparg;
	i->target = nullptr;
	i->loc = _loc;

	return SUCCESS;
}

static AlifIntT basicBlock_addJump(BasicBlock* _b, AlifIntT _opcode,
	BasicBlock* _target, Location _loc) { // 199
	CFGInstr* last = basicBlock_lastInstr(_b);
	if (last and is_jump(last)) {
		return ERROR;
	}

	RETURN_IF_ERROR(
		basicBlock_addOp(_b, _opcode, _target->label.id, _loc));
	last = basicBlock_lastInstr(_b);
	last->target = _target;
	return SUCCESS;
}


static inline AlifIntT basicBlock_appendInstructions(BasicBlock* _to, BasicBlock* _from) { // 215
	for (AlifIntT i = 0; i < _from->iused; i++) {
		AlifIntT n = basicBlock_nextInstr(_to);
		if (n < 0) {
			return ERROR;
		}
		_to->instr[n] = _from->instr[i];
	}
	return SUCCESS;
}

static inline AlifIntT basicBlock_noFallThrough(const BasicBlock* _b) { // 227
	CFGInstr* last = basicBlock_lastInstr(_b);
	return (last and
		(IS_SCOPE_EXIT_OPCODE(last->opcode) or
			IS_UNCONDITIONAL_JUMP_OPCODE(last->opcode)));
}

#define BB_NO_FALLTHROUGH(_b) (basicBlock_noFallThrough(_b))
#define BB_HAS_FALLTHROUGH(_b) (!basicBlock_noFallThrough(_b)) // 236

static BasicBlock* copy_basicBlock(CFGBuilder* _g, BasicBlock* _block) { // 238
	BasicBlock* result = cfgBuilder_newBlock(_g);
	if (result == nullptr) {
		return nullptr;
	}
	if (basicBlock_appendInstructions(result, _block) < 0) {
		return nullptr;
	}
	return result;
}

static AlifIntT basicBlock_insertInstruction(BasicBlock* _block,
	AlifIntT _pos, CFGInstr* _instr) { // 255
	RETURN_IF_ERROR(basicBlock_nextInstr(_block));
	for (AlifIntT i = _block->iused - 1; i > _pos; i--) {
		_block->instr[i] = _block->instr[i - 1];
	}
	_block->instr[_pos] = *_instr;
	return SUCCESS;
}




static BasicBlock* cfgBuilder_useNextBlock(CFGBuilder* _g, BasicBlock* _block) { // 321
	_g->curBlock->next = _block;
	_g->curBlock = _block;
	return _block;
}

static inline AlifIntT basicBlock_exitsScope(const BasicBlock* _b) { // 330
	CFGInstr* last = basicBlock_lastInstr(_b);
	return last and IS_SCOPE_EXIT_OPCODE(last->opcode);
}

static inline AlifIntT basicBlock_hasEvalBreak(const BasicBlock* _b) { // 335
	for (AlifIntT i = 0; i < _b->iused; i++) {
		if (OPCODE_HAS_EVAL_BREAK(_b->instr[i].opcode)) {
			return true;
		}
	}
	return false;
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
			_g->currentLabel = NO_LABEL;
		}
	}
	return false;
}

static AlifIntT cfgBuilder_maybeStartNewBlock(CFGBuilder* _g) { // 366
	if (cfgBuilder_currentBlockIsTerminated(_g)) {
		BasicBlock* b = cfgBuilder_newBlock(_g);
		if (b == nullptr) {
			return ERROR;
		}
		b->label = _g->currentLabel;
		_g->currentLabel = NO_LABEL;
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
	_g->currentLabel = NO_LABEL;
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
	if ((AlifUSizeT)nBlocks > SIZE_MAX / sizeof(BasicBlock*)) {
		//alifErr_noMemory();
		return ERROR;
	}
	return SUCCESS;
}


AlifIntT _alifCFGBuilder_useLabel(CFGBuilder* _g, JumpTargetLabel _lbl) { // 464
	_g->currentLabel = _lbl;
	return cfgBuilder_maybeStartNewBlock(_g);
}


AlifIntT _alifCFGBuilder_addOp(CFGBuilder* _g, AlifIntT _opcode,
	AlifIntT _oparg, Location _loc) { // 470
	RETURN_IF_ERROR(cfgBuilder_maybeStartNewBlock(_g));
	return basicBlock_addOp(_g->curBlock, _opcode, _oparg, _loc);
}

static BasicBlock* next_nonemptyBlock(BasicBlock* _b) { // 478
	while (_b and _b->iused == 0) {
		_b = _b->next;
	}
	return _b;
}

static AlifIntT normalizeJumps_inBlock(CFGBuilder* _g, BasicBlock* _b) { // 540
	CFGInstr* last = basicBlock_lastInstr(_b);
	if (last == nullptr or !is_jump(last) or IS_UNCONDITIONAL_JUMP_OPCODE(last->opcode)) {
		return SUCCESS;
	}

	bool isForward = last->target->visited == 0;
	if (isForward) {
		return SUCCESS;
	}

	AlifIntT reversedOpcode = 0;
	switch (last->opcode) {
	case POP_JUMP_IF_NOT_NONE:
		reversedOpcode = POP_JUMP_IF_NONE;
		break;
	case POP_JUMP_IF_NONE:
		reversedOpcode = POP_JUMP_IF_NOT_NONE;
		break;
	case POP_JUMP_IF_FALSE:
		reversedOpcode = POP_JUMP_IF_TRUE;
		break;
	case POP_JUMP_IF_TRUE:
		reversedOpcode = POP_JUMP_IF_FALSE;
		break;
	}

	BasicBlock* target = last->target;
	BasicBlock* backwardsJump = cfgBuilder_newBlock(_g);
	if (backwardsJump == nullptr) {
		return ERROR;
	}
	RETURN_IF_ERROR(
		basicBlock_addJump(backwardsJump, JUMP, target, last->loc));
	last->opcode = reversedOpcode;
	last->target = _b->next;

	backwardsJump->cold = _b->cold;
	backwardsJump->next = _b->next;
	_b->next = backwardsJump;
	return SUCCESS;
}



static AlifIntT normalize_jumps(CFGBuilder* _g) { // 590
	BasicBlock* entryBlock = _g->entryBlock;
	for (BasicBlock* b = entryBlock; b != nullptr; b = b->next) {
		b->visited = 0;
	}
	for (BasicBlock* b = entryBlock; b != nullptr; b = b->next) {
		b->visited = 1;
		RETURN_IF_ERROR(normalizeJumps_inBlock(_g, b));
	}
	return SUCCESS;
}


static AlifIntT check_cfg(CFGBuilder* _g) { // 605
	for (BasicBlock* b_ = _g->entryBlock; b_ != nullptr; b_ = b_->next) {
		for (AlifIntT i = 0; i < b_->iused; i++) {
			AlifIntT opcode = b_->instr[i].opcode;
			if (IS_TERMINATOR_OPCODE(opcode)) {
				if (i != b_->iused - 1) {
					//alifErr_setString(_alifExcSystemError_, "malformed control flow graph.");
					return ERROR;
				}
			}
		}
	}
	return SUCCESS;
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
	BasicBlock** label2block = (BasicBlock**)alifMem_dataAlloc(mapSize ? mapSize : 1); //* alif
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





static AlifIntT mark_exceptHandlers(BasicBlock* _entryBlock) { // 668
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			if (is_blockPush(instr)) {
				instr->target->exceptHandler = 1;
			}
		}
	}
	return SUCCESS;
}





class AlifCFGExceptStack { // 687
public:
	BasicBlock* handlers[MAXBLOCKS + 2]{};
	AlifIntT depth{};
};


static BasicBlock* push_exceptBlock(AlifCFGExceptStack* stack, CFGInstr* setup) { // 693
	AlifIntT opcode = setup->opcode;
	BasicBlock* target = setup->target;
	if (opcode == SETUP_WITH or opcode == SETUP_CLEANUP) {
		target->preserveLastI = 1;
	}
	stack->handlers[++stack->depth] = target;
	return target;
}

static BasicBlock* pop_exceptBlock(AlifCFGExceptStack* _stack) { // 706
	return _stack->handlers[--_stack->depth];
}

static BasicBlock* except_stackTop(AlifCFGExceptStack* _stack) { // 712
	return _stack->handlers[_stack->depth];
}

static AlifCFGExceptStack* make_exceptStack(void) { // 717
	AlifCFGExceptStack* new_ = (AlifCFGExceptStack*)alifMem_dataAlloc(sizeof(AlifCFGExceptStack));
	if (new_ == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	new_->depth = 0;
	new_->handlers[0] = nullptr;
	return new_;
}

static AlifCFGExceptStack* copy_exceptStack(AlifCFGExceptStack* _stack) { // 729
	AlifCFGExceptStack* copy = (AlifCFGExceptStack*)alifMem_dataAlloc(sizeof(AlifCFGExceptStack));
	if (copy == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	memcpy(copy, _stack, sizeof(AlifCFGExceptStack));
	return copy;
}


static BasicBlock** makeCFG_traversalStack(BasicBlock* _entryBlock) { // 741
	AlifIntT nBlocks = 0;
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		b->visited = 0;
		nBlocks++;
	}
	BasicBlock** stack = (BasicBlock**)alifMem_dataAlloc(sizeof(BasicBlock*) * nBlocks);
	if (!stack) {
		//alifErr_noMemory();
	}
	return stack;
}

ALIF_LOCAL(AlifIntT) stack_effect(AlifIntT _opcode, AlifIntT _oparg, AlifIntT _jump) { // 764
	if (_opcode < 0) {
		return ALIF_INVALID_STACK_EFFECT;
	}
	if ((_opcode <= MAX_REAL_OPCODE) and (_alifOpcodeDeopt_[_opcode] != _opcode)) {
		return ALIF_INVALID_STACK_EFFECT;
	}
	AlifIntT popped = _alifOpcode_numPopped(_opcode, _oparg);
	AlifIntT pushed = _alifOpcode_numPushed(_opcode, _oparg);
	if (popped < 0 or pushed < 0) {
		return ALIF_INVALID_STACK_EFFECT;
	}
	if (IS_BLOCK_PUSH_OPCODE(_opcode) and !_jump) {
		return 0;
	}
	return pushed - popped;
}

ALIF_LOCAL_INLINE(AlifIntT) stackDepth_push(BasicBlock*** _sp,
	BasicBlock* _b, AlifIntT _depth) { // 785
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



static AlifIntT calculate_stackDepth(CFGBuilder* _g) { // 803
	BasicBlock* entryBlock = _g->entryBlock;
	for (BasicBlock* b = entryBlock; b != nullptr; b = b->next) {
		b->startDepth = INT_MIN;
	}
	BasicBlock** stack = makeCFG_traversalStack(entryBlock);
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
				IS_SCOPE_EXIT_OPCODE(instr_->opcode))
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
	alifMem_dataFree(stack);
	return stackDepth;
}


static AlifIntT label_exceptionTargets(BasicBlock* entryblock) { // 884
	BasicBlock** todo_stack = makeCFG_traversalStack(entryblock);
	if (todo_stack == nullptr) {
		return ERROR;
	}
	AlifCFGExceptStack* except_stack = make_exceptStack();
	if (except_stack == nullptr) {
		alifMem_dataFree(todo_stack);
		//alifErr_noMemory();
		return ERROR;
	}
	except_stack->depth = 0;
	todo_stack[0] = entryblock;
	entryblock->visited = 1;
	entryblock->exceptStack = except_stack;
	BasicBlock** todo = &todo_stack[1];
	BasicBlock* handler = nullptr;
	while (todo > todo_stack) {
		todo--;
		BasicBlock* b = todo[0];
		except_stack = b->exceptStack;
		b->exceptStack = nullptr;
		handler = except_stackTop(except_stack);
		AlifIntT last_yield_except_depth = -1;
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			if (is_blockPush(instr)) {
				if (!instr->target->visited) {
					AlifCFGExceptStack* copy = copy_exceptStack(except_stack);
					if (copy == nullptr) {
						goto error;
					}
					instr->target->exceptStack = copy;
					todo[0] = instr->target;
					instr->target->visited = 1;
					todo++;
				}
				handler = push_exceptBlock(except_stack, instr);
			}
			else if (instr->opcode == POP_BLOCK) {
				handler = pop_exceptBlock(except_stack);
				INSTR_SET_OP0(instr, NOP);
			}
			else if (is_jump(instr)) {
				instr->except = handler;
				if (!instr->target->visited) {
					if (BB_HAS_FALLTHROUGH(b)) {
						AlifCFGExceptStack* copy = copy_exceptStack(except_stack);
						if (copy == nullptr) {
							goto error;
						}
						instr->target->exceptStack = copy;
					}
					else {
						instr->target->exceptStack = except_stack;
						except_stack = nullptr;
					}
					todo[0] = instr->target;
					instr->target->visited = 1;
					todo++;
				}
			}
			else if (instr->opcode == YIELD_VALUE) {
				instr->except = handler;
				last_yield_except_depth = except_stack->depth;
			}
			else if (instr->opcode == RESUME) {
				instr->except = handler;
				if (instr->oparg != RESUME_AT_FUNC_START) {
					if (last_yield_except_depth == 1) {
						instr->oparg |= RESUME_OPARG_DEPTH1_MASK;
					}
					last_yield_except_depth = -1;
				}
			}
			else {
				instr->except = handler;
			}
		}
		if (BB_HAS_FALLTHROUGH(b) and !b->next->visited) {
			b->next->exceptStack = except_stack;
			todo[0] = b->next;
			b->next->visited = 1;
			todo++;
		}
		else if (except_stack != nullptr) {
			alifMem_dataFree(except_stack);
		}
	}

	alifMem_dataFree(todo_stack);
	return SUCCESS;
error:
	alifMem_dataFree(todo_stack);
	alifMem_dataFree(except_stack);
	return ERROR;
}


static AlifIntT remove_unreachable(BasicBlock* _entryBlock) { // 995
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		b->predecessors = 0;
	}
	BasicBlock** stack = makeCFG_traversalStack(_entryBlock);
	if (stack == nullptr) {
		return ERROR;
	}
	BasicBlock** sp_ = stack;
	_entryBlock->predecessors = 1;
	*sp_++ = _entryBlock;
	_entryBlock->visited = 1;
	while (sp_ > stack) {
		BasicBlock* b_ = *(--sp_);
		if (b_->next and BB_HAS_FALLTHROUGH(b_)) {
			if (!b_->next->visited) {
				*sp_++ = b_->next;
				b_->next->visited = 1;
			}
			b_->next->predecessors++;
		}
		for (AlifIntT i = 0; i < b_->iused; i++) {
			BasicBlock* target{};
			CFGInstr* instr = &b_->instr[i];
			if (is_jump(instr) or is_blockPush(instr)) {
				target = instr->target;
				if (!target->visited) {
					*sp_++ = target;
					target->visited = 1;
				}
				target->predecessors++;
			}
		}
	}
	alifMem_dataFree(stack);

	/* Delete unreachable instructions */
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		if (b->predecessors == 0) {
			b->iused = 0;
			b->exceptHandler = 0;
		}
	}
	return SUCCESS;
}

static AlifIntT basicBlock_removeRedundantNops(BasicBlock* _bb) { // 1042
	AlifIntT dest = 0;
	AlifIntT prevLineNo = -1;
	for (AlifIntT src_ = 0; src_ < _bb->iused; src_++) {
		AlifIntT lineno = _bb->instr[src_].loc.lineNo;
		if (_bb->instr[src_].opcode == NOP) {
			if (lineno < 0) {
				continue;
			}
			if (prevLineNo == lineno) {
				continue;
			}
			if (src_ < _bb->iused - 1) {
				AlifIntT next_lineno = _bb->instr[src_ + 1].loc.lineNo;
				if (next_lineno == lineno) {
					continue;
				}
				if (next_lineno < 0) {
					_bb->instr[src_ + 1].loc = _bb->instr[src_].loc;
					continue;
				}
			}
			else {
				BasicBlock* next = next_nonemptyBlock(_bb->next);
				if (next) {
					Location nextLoc = _noLocation_;
					for (AlifIntT nextI = 0; nextI < next->iused; nextI++) {
						CFGInstr* instr = &next->instr[nextI];
						if (instr->opcode == NOP and instr->loc.lineNo == _noLocation_.lineNo) {
							continue;
						}
						nextLoc = instr->loc;
						break;
					}
					if (lineno == nextLoc.lineNo) {
						continue;
					}
				}
			}

		}
		if (dest != src_) {
			_bb->instr[dest] = _bb->instr[src_];
		}
		dest++;
		prevLineNo = lineno;
	}
	AlifIntT numRemoved = _bb->iused - dest;
	_bb->iused = dest;
	return numRemoved;
}

static AlifIntT remove_redundantNops(CFGBuilder* _g) { // 1101
	AlifIntT changes = 0;
	for (BasicBlock* b = _g->entryBlock; b != nullptr; b = b->next) {
		AlifIntT change = basicBlock_removeRedundantNops(b);
		RETURN_IF_ERROR(change);
		changes += change;
	}
	return changes;
}


static AlifIntT remove_redundantNopsAndPairs(BasicBlock* _entryBlock) { // 1112
	bool done = false;

	while (!done) {
		done = true;
		CFGInstr* prevInstr = nullptr;
		CFGInstr* instr = nullptr;
		for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
			RETURN_IF_ERROR(basicBlock_removeRedundantNops(b));
			if (IS_LABEL(b->label)) {
				/* this block is a jump target, forget instr */
				instr = nullptr;
			}
			for (AlifIntT i = 0; i < b->iused; i++) {
				prevInstr = instr;
				instr = &b->instr[i];
				AlifIntT prev_opcode = prevInstr ? prevInstr->opcode : 0;
				AlifIntT prev_oparg = prevInstr ? prevInstr->oparg : 0;
				AlifIntT opcode = instr->opcode;
				bool is_redundant_pair = false;
				if (opcode == POP_TOP) {
					if (prev_opcode == LOAD_CONST) {
						is_redundant_pair = true;
					}
					else if (prev_opcode == COPY and prev_oparg == 1) {
						is_redundant_pair = true;
					}
				}
				if (is_redundant_pair) {
					INSTR_SET_OP0(prevInstr, NOP);
					INSTR_SET_OP0(instr, NOP);
					done = false;
				}
			}
			if ((instr and is_jump(instr)) or !BB_HAS_FALLTHROUGH(b)) {
				instr = nullptr;
			}
		}
	}
	return SUCCESS;
}



static AlifIntT remove_redundantJumps(CFGBuilder* _g) { // 1156
	AlifIntT changes = 0;
	for (BasicBlock* b = _g->entryBlock; b != nullptr; b = b->next) {
		CFGInstr* last = basicBlock_lastInstr(b);
		if (last == nullptr) {
			continue;
		}
		if (IS_UNCONDITIONAL_JUMP_OPCODE(last->opcode)) {
			BasicBlock* jumpTarget = next_nonemptyBlock(last->target);
			if (jumpTarget == nullptr) {
				//alifErr_setString(_alifExcSystemError_, "jump with nullptr target");
				return ERROR;
			}
			BasicBlock* next = next_nonemptyBlock(b->next);
			if (jumpTarget == next) {
				changes++;
				INSTR_SET_OP0(last, NOP);
			}
		}
	}

	return changes;
}



static inline bool basicBlock_hasNoLineno(BasicBlock* b) { // 1191
	for (AlifIntT i = 0; i < b->iused; i++) {
		if (b->instr[i].loc.lineNo >= 0) {
			return false;
		}
	}
	return true;
}


#define MAX_COPY_SIZE 4 // 1201


static AlifIntT basicBlock_inlineSmallOrNoLinenoBlocks(BasicBlock* _bb) { // 1209
	CFGInstr* last = basicBlock_lastInstr(_bb);
	if (last == nullptr) {
		return 0;
	}
	if (!IS_UNCONDITIONAL_JUMP_OPCODE(last->opcode)) {
		return 0;
	}
	BasicBlock* target = last->target;
	bool smallExitBlock = (basicBlock_exitsScope(target) and
		target->iused <= MAX_COPY_SIZE);
	bool noLinenoNoFallThrough = (basicBlock_hasNoLineno(target) and
		!BB_HAS_FALLTHROUGH(target));
	if (smallExitBlock or noLinenoNoFallThrough) {
		AlifIntT removedJumpOpcode = last->opcode;
		INSTR_SET_OP0(last, NOP);
		RETURN_IF_ERROR(basicBlock_appendInstructions(_bb, target));
		if (noLinenoNoFallThrough) {
			last = basicBlock_lastInstr(_bb);
			if (IS_UNCONDITIONAL_JUMP_OPCODE(last->opcode) and
				removedJumpOpcode == JUMP)
			{
				last->opcode = JUMP;
			}
		}
		target->predecessors--;
		return 1;
	}
	return 0;
}

static AlifIntT inline_smallOrNoLinenoBlocks(BasicBlock* _entryBlock) { // 1243
	bool changes{};
	do {
		changes = false;
		for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
			AlifIntT res_ = basicBlock_inlineSmallOrNoLinenoBlocks(b);
			RETURN_IF_ERROR(res_);
			if (res_) {
				changes = true;
			}
		}
	} while (changes); /* every change removes a jump, ensuring convergence */
	return changes;
}



static bool jump_thread(BasicBlock* _bb, CFGInstr* _inst,
	CFGInstr* _target, AlifIntT _opcode) { // 1261
	if (_inst->target != _target->target) {
		INSTR_SET_OP0(_inst, NOP);

		RETURN_IF_ERROR(
			basicBlock_addJump(_bb, _opcode, _target->target, _target->loc));

		return true;
	}
	return false;
}

static AlifObject* get_constValue(AlifIntT _opcode, AlifIntT _oparg, AlifObject* _coConsts) { // 1285
	AlifObject* constant = nullptr;
	if (_opcode == LOAD_CONST) {
		constant = ALIFLIST_GET_ITEM(_coConsts, _oparg);
	}

	if (constant == nullptr) {
		//alifErr_setString(_alifExcSystemError_,
			//"Internal error: failed to get value of a constant");
		return nullptr;
	}
	return ALIF_NEWREF(constant);
}

static AlifIntT add_const(AlifObject* _newConst, AlifObject* _consts, AlifObject* _constCache) { // 1302
	if (_alifCompiler_constCacheMergeOne(_constCache, &_newConst) < 0) {
		ALIF_DECREF(_newConst);
		return -1;
	}

	AlifSizeT index{};
	for (index = 0; index < ALIFLIST_GET_SIZE(_consts); index++) {
		if (ALIFLIST_GET_ITEM(_consts, index) == _newConst) {
			break;
		}
	}
	if (index == ALIFLIST_GET_SIZE(_consts)) {
		if ((AlifUSizeT)index >= (AlifUSizeT)INT_MAX - 1) {
			//alifErr_setString(_alifExcOverflowError_, "too many constants");
			ALIF_DECREF(_newConst);
			return -1;
		}
		if (alifList_append(_consts, _newConst)) {
			ALIF_DECREF(_newConst);
			return -1;
		}
	}
	ALIF_DECREF(_newConst);
	return (AlifIntT)index;
}

static AlifIntT foldTuple_onConstants(AlifObject* _constCache,
	CFGInstr* _inst, AlifIntT _n, AlifObject* _consts) { // 1337
	for (AlifIntT i = 0; i < _n; i++) {
		if (!OPCODE_HAS_CONST(_inst[i].opcode)) {
			return SUCCESS;
		}
	}

	/* Buildup new tuple of constants */
	AlifObject* newconst = alifTuple_new(_n);
	if (newconst == nullptr) {
		return ERROR;
	}
	for (AlifIntT i = 0; i < _n; i++) {
		AlifIntT op = _inst[i].opcode;
		AlifIntT arg = _inst[i].oparg;
		AlifObject* constant = get_constValue(op, arg, _consts);
		if (constant == nullptr) {
			return ERROR;
		}
		ALIFTUPLE_SET_ITEM(newconst, i, constant);
	}
	AlifIntT index = add_const(newconst, _consts, _constCache);
	if (index < 0) {
		return ERROR;
	}
	for (AlifIntT i = 0; i < _n; i++) {
		INSTR_SET_OP0(&_inst[i], NOP);
	}
	INSTR_SET_OP1(&_inst[_n], LOAD_CONST, index);
	return SUCCESS;
}

#define VISITED (-1) // 1379


static AlifIntT swaptimize(BasicBlock* _block, AlifIntT* _ix) { // 1383
	CFGInstr* instructions = &_block->instr[*_ix];

	AlifIntT depth = instructions[0].oparg;
	AlifIntT len = 0;
	AlifIntT more = false;
	AlifIntT limit = _block->iused - *_ix;
	while (++len < limit) {
		AlifIntT opcode = instructions[len].opcode;
		if (opcode == SWAP) {
			depth = ALIF_MAX(depth, instructions[len].oparg);
			more = true;
		}
		else if (opcode != NOP) {
			break;
		}
	}
	if (!more) {
		return SUCCESS;
	}
	// Create an array with elements {0, 1, 2, ..., depth - 1}:
	AlifIntT* stack = (AlifIntT*)alifMem_dataAlloc(depth * sizeof(AlifIntT));
	if (stack == nullptr) {
		//alifErr_noMemory();
		return ERROR;
	}
	for (AlifIntT i = 0; i < depth; i++) {
		stack[i] = i;
	}

	for (AlifIntT i = 0; i < len; i++) {
		if (instructions[i].opcode == SWAP) {
			AlifIntT oparg = instructions[i].oparg;
			AlifIntT top = stack[0];
			// SWAPs are 1-indexed:
			stack[0] = stack[oparg - 1];
			stack[oparg - 1] = top;
		}
	}

	AlifIntT current = len - 1;
	for (AlifIntT i = 0; i < depth; i++) {
		if (stack[i] == VISITED or stack[i] == i) {
			continue;
		}
		AlifIntT j = i;
		while (true) {
			if (j) {
				instructions[current].opcode = SWAP;
				instructions[current--].oparg = j + 1;
			}
			if (stack[j] == VISITED) {
				break;
			}
			AlifIntT next_j = stack[j];
			stack[j] = VISITED;
			j = next_j;
		}
	}
	while (0 <= current) {
		INSTR_SET_OP0(&instructions[current--], NOP);
	}
	alifMem_dataFree(stack);
	*_ix += len - 1;
	return SUCCESS;
}





// 1484
#define SWAPPABLE(_opcode) \
    ((_opcode) == STORE_FAST or \
     (_opcode) == STORE_FAST_MAYBE_NULL or \
     (_opcode) == POP_TOP)

// 1489
#define STORES_TO(_instr) \
    (((_instr).opcode == STORE_FAST or \
      (_instr).opcode == STORE_FAST_MAYBE_NULL) \
     ? (_instr).oparg : -1)


static AlifIntT next_swappableInstruction(BasicBlock* _block, AlifIntT _i, AlifIntT _lineNo) { // 1495
	while (++_i < _block->iused) {
		CFGInstr* instruction = &_block->instr[_i];
		if (0 <= _lineNo and instruction->loc.lineNo != _lineNo) {
			return -1;
		}
		if (instruction->opcode == NOP) {
			continue;
		}
		if (SWAPPABLE(instruction->opcode)) {
			return _i;
		}
		return -1;
	}
	return -1;
}

static void apply_staticSwaps(BasicBlock* _block, AlifIntT _i) { // 1518
	for (; 0 <= _i; _i--) {
		CFGInstr* swap = &_block->instr[_i];
		if (swap->opcode != SWAP) {
			if (swap->opcode == NOP or SWAPPABLE(swap->opcode)) {
				continue;
			}
			return;
		}
		AlifIntT j = next_swappableInstruction(_block, _i, -1);
		if (j < 0) {
			return;
		}
		AlifIntT k = j;
		AlifIntT lineno = _block->instr[j].loc.lineNo;
		for (AlifIntT count = swap->oparg - 1; 0 < count; count--) {
			k = next_swappableInstruction(_block, k, lineno);
			if (k < 0) {
				return;
			}
		}

		AlifIntT storeJ = STORES_TO(_block->instr[j]);
		AlifIntT storeK = STORES_TO(_block->instr[k]);
		if (storeJ >= 0 or storeK >= 0) {
			if (storeJ == storeK) {
				return;
			}
			for (AlifIntT idx = j + 1; idx < k; idx++) {
				AlifIntT storeIDx = STORES_TO(_block->instr[idx]);
				if (storeIDx >= 0 and (storeIDx == storeJ or storeIDx == storeK)) {
					return;
				}
			}
		}

		// Success!
		INSTR_SET_OP0(swap, NOP);
		CFGInstr temp = _block->instr[j];
		_block->instr[j] = _block->instr[k];
		_block->instr[k] = temp;
	}
}





static AlifIntT basicBlock_optimizeLoadConst(AlifObject* const_cache,
	BasicBlock* bb, AlifObject* consts) { // 1570
	AlifIntT opcode = 0;
	AlifIntT oparg = 0;
	for (AlifIntT i = 0; i < bb->iused; i++) {
		CFGInstr* inst = &bb->instr[i];
		bool isCopyOfLoadConst = (opcode == LOAD_CONST and
			inst->opcode == COPY and
			inst->oparg == 1);
		if (!isCopyOfLoadConst) {
			opcode = inst->opcode;
			oparg = inst->oparg;
		}
		if (opcode != LOAD_CONST) {
			continue;
		}
		AlifIntT nextop = i + 1 < bb->iused ? bb->instr[i + 1].opcode : 0;
		switch (nextop) {
		case POP_JUMP_IF_FALSE:
		case POP_JUMP_IF_TRUE:
		case JUMP_IF_FALSE:
		case JUMP_IF_TRUE:
		{
			AlifObject* cnt = get_constValue(opcode, oparg, consts);
			if (cnt == nullptr) {
				return ERROR;
			}
			AlifIntT isTrue = alifObject_isTrue(cnt);
			ALIF_DECREF(cnt);
			if (isTrue == -1) {
				return ERROR;
			}
			if (alifCompile_opcodeStackEffect(nextop, 0) == -1) {
				INSTR_SET_OP0(inst, NOP);
			}
			AlifIntT jumpIfTrue = (nextop == POP_JUMP_IF_TRUE or nextop == JUMP_IF_TRUE);
			if (isTrue == jumpIfTrue) {
				bb->instr[i + 1].opcode = JUMP;
			}
			else {
				INSTR_SET_OP0(&bb->instr[i + 1], NOP);
			}
			break;
		}
		case IS_OP:
		{
			AlifObject* cnt = get_constValue(opcode, oparg, consts);
			if (cnt == nullptr) {
				return ERROR;
			}
			if (!ALIF_ISNONE(cnt)) {
				ALIF_DECREF(cnt);
				break;
			}
			if (bb->iused <= i + 2) {
				break;
			}
			CFGInstr* isInstr = &bb->instr[i + 1];
			CFGInstr* jumpInstr = &bb->instr[i + 2];
			if (jumpInstr->opcode == TO_BOOL) {
				INSTR_SET_OP0(jumpInstr, NOP);
				if (bb->iused <= i + 3) {
					break;
				}
				jumpInstr = &bb->instr[i + 3];
			}
			bool invert = isInstr->oparg;
			if (jumpInstr->opcode == POP_JUMP_IF_FALSE) {
				invert = !invert;
			}
			else if (jumpInstr->opcode != POP_JUMP_IF_TRUE) {
				break;
			}
			INSTR_SET_OP0(inst, NOP);
			INSTR_SET_OP0(isInstr, NOP);
			jumpInstr->opcode = invert ? POP_JUMP_IF_NOT_NONE
				: POP_JUMP_IF_NONE;
			break;
		}
		case RETURN_VALUE:
		{
			INSTR_SET_OP0(inst, NOP);
			INSTR_SET_OP1(&bb->instr[++i], RETURN_CONST, oparg);
			break;
		}
		case TO_BOOL:
		{
			AlifObject* cnt = get_constValue(opcode, oparg, consts);
			if (cnt == nullptr) {
				return ERROR;
			}
			AlifIntT isTrue = alifObject_isTrue(cnt);
			ALIF_DECREF(cnt);
			if (isTrue == -1) {
				return ERROR;
			}
			cnt = alifBool_fromLong(isTrue);
			AlifIntT index = add_const(cnt, consts, const_cache);
			if (index < 0) {
				return ERROR;
			}
			INSTR_SET_OP0(inst, NOP);
			INSTR_SET_OP1(&bb->instr[i + 1], LOAD_CONST, index);
			break;
		}
		}
	}
	return SUCCESS;
}

static AlifIntT optimize_loadConst(AlifObject* _constCache,
	CFGBuilder* _g, AlifObject* _consts) { // 1692
	for (BasicBlock* b = _g->entryBlock; b != nullptr; b = b->next) {
		RETURN_IF_ERROR(basicBlock_optimizeLoadConst(_constCache, b, _consts));
	}
	return SUCCESS;
}

static AlifIntT optimize_basicBlock(AlifObject* _constCache,
	BasicBlock* _bb, AlifObject* _consts) { // 1700
	CFGInstr nop{};
	INSTR_SET_OP0(&nop, NOP);
	for (AlifIntT i = 0; i < _bb->iused; i++) {
		CFGInstr* inst = &_bb->instr[i];
		CFGInstr* target;
		AlifIntT opcode = inst->opcode;
		AlifIntT oparg = inst->oparg;
		if (HAS_TARGET(opcode)) {
			target = &inst->target->instr[0];
		}
		else {
			target = &nop;
		}
		AlifIntT nextop = i + 1 < _bb->iused ? _bb->instr[i + 1].opcode : 0;
		switch (opcode) {
		case BUILD_TUPLE:
			if (nextop == UNPACK_SEQUENCE and oparg == _bb->instr[i + 1].oparg) {
				switch (oparg) {
				case 1:
					INSTR_SET_OP0(inst, NOP);
					INSTR_SET_OP0(&_bb->instr[i + 1], NOP);
					continue;
				case 2:
				case 3:
					INSTR_SET_OP0(inst, NOP);
					_bb->instr[i + 1].opcode = SWAP;
					continue;
				}
			}
			if (i >= oparg) {
				if (foldTuple_onConstants(_constCache, inst - oparg, oparg, _consts)) {
					goto error;
				}
			}
			break;
		case POP_JUMP_IF_NOT_NONE:
		case POP_JUMP_IF_NONE:
			switch (target->opcode) {
			case JUMP:
				i -= jump_thread(_bb, inst, target, inst->opcode);
			}
			break;
		case POP_JUMP_IF_FALSE:
			switch (target->opcode) {
			case JUMP:
				i -= jump_thread(_bb, inst, target, POP_JUMP_IF_FALSE);
			}
			break;
		case POP_JUMP_IF_TRUE:
			switch (target->opcode) {
			case JUMP:
				i -= jump_thread(_bb, inst, target, POP_JUMP_IF_TRUE);
			}
			break;
		case JUMP_IF_FALSE:
			switch (target->opcode) {
			case JUMP:
			case JUMP_IF_FALSE:
				i -= jump_thread(_bb, inst, target, JUMP_IF_FALSE);
				continue;
			case JUMP_IF_TRUE:
				inst->target = inst->target->next;
				i--;
				continue;
			}
			break;
		case JUMP_IF_TRUE:
			switch (target->opcode) {
			case JUMP:
			case JUMP_IF_TRUE:
				i -= jump_thread(_bb, inst, target, JUMP_IF_TRUE);
				continue;
			case JUMP_IF_FALSE:
				inst->target = inst->target->next;
				i--;
				continue;
			}
			break;
		case JUMP:
		case JUMP_NO_INTERRUPT:
			switch (target->opcode) {
			case JUMP:
				i -= jump_thread(_bb, inst, target, JUMP);
				continue;
			case JUMP_NO_INTERRUPT:
				i -= jump_thread(_bb, inst, target, opcode);
				continue;
			}
			break;
		case FOR_ITER:
			if (target->opcode == JUMP) {
				/* This will not work now because the jump (at target) could
				 * be forward or backward and FOR_ITER only jumps forward. We
				 * can re-enable this if ever we implement a backward version
				 * of FOR_ITER.
				 */
				 /*
				 i -= jump_thread(bb, inst, target, FOR_ITER);
				 */
			}
			break;
		case STORE_FAST:
			if (opcode == nextop and
				oparg == _bb->instr[i + 1].oparg and
				_bb->instr[i].loc.lineNo == _bb->instr[i + 1].loc.lineNo) {
				_bb->instr[i].opcode = POP_TOP;
				_bb->instr[i].oparg = 0;
			}
			break;
		case SWAP:
			if (oparg == 1) {
				INSTR_SET_OP0(inst, NOP);
			}
			break;
		case LOAD_GLOBAL:
			if (nextop == PUSH_NULL and (oparg & 1) == 0) {
				INSTR_SET_OP1(inst, LOAD_GLOBAL, oparg | 1);
				INSTR_SET_OP0(&_bb->instr[i + 1], NOP);
			}
			break;
		case COMPARE_OP:
			if (nextop == TO_BOOL) {
				INSTR_SET_OP0(inst, NOP);
				INSTR_SET_OP1(&_bb->instr[i + 1], COMPARE_OP, oparg | 16);
				continue;
			}
			break;
		case CONTAINS_OP:
		case IS_OP:
			if (nextop == TO_BOOL) {
				INSTR_SET_OP0(inst, NOP);
				INSTR_SET_OP1(&_bb->instr[i + 1], opcode, oparg);
				continue;
			}
			break;
		case TO_BOOL:
			if (nextop == TO_BOOL) {
				INSTR_SET_OP0(inst, NOP);
				continue;
			}
			break;
		case UNARY_NOT:
			if (nextop == TO_BOOL) {
				INSTR_SET_OP0(inst, NOP);
				INSTR_SET_OP0(&_bb->instr[i + 1], UNARY_NOT);
				continue;
			}
			if (nextop == UNARY_NOT) {
				INSTR_SET_OP0(inst, NOP);
				INSTR_SET_OP0(&_bb->instr[i + 1], NOP);
				continue;
			}
			break;
		}
	}

	for (AlifIntT i = 0; i < _bb->iused; i++) {
		CFGInstr* inst = &_bb->instr[i];
		if (inst->opcode == SWAP) {
			if (swaptimize(_bb, &i) < 0) {
				goto error;
			}
			apply_staticSwaps(_bb, i);
		}
	}
	return SUCCESS;
error:
	return ERROR;
}



static AlifIntT resolve_lineNumbers(CFGBuilder*, AlifIntT); // 1858


static AlifIntT remove_redundantNopsAndJumps(CFGBuilder* _g) { // 1860
	AlifIntT removedNops{}, removedJumps{};
	do {
		removedNops = remove_redundantNops(_g);
		RETURN_IF_ERROR(removedNops);
		removedJumps = remove_redundantJumps(_g);
		RETURN_IF_ERROR(removedJumps);
	} while (removedNops + removedJumps > 0);
	return SUCCESS;
}


/* Perform optimizations on a control flow graph.
   The consts object should still be in list form to allow new constants
   to be appended.

   Code trasnformations that reduce code size initially fill the gaps with
   NOPs.  Later those NOPs are removed.
*/
static AlifIntT optimize_cfg(CFGBuilder* _g, AlifObject* _consts,
	AlifObject* _constCache, AlifIntT _firstLineno) { // 1884
	RETURN_IF_ERROR(check_cfg(_g));
	RETURN_IF_ERROR(inline_smallOrNoLinenoBlocks(_g->entryBlock));
	RETURN_IF_ERROR(remove_unreachable(_g->entryBlock));
	RETURN_IF_ERROR(resolve_lineNumbers(_g, _firstLineno));
	RETURN_IF_ERROR(optimize_loadConst(_constCache, _g, _consts));
	for (BasicBlock* b = _g->entryBlock; b != nullptr; b = b->next) {
		RETURN_IF_ERROR(optimize_basicBlock(_constCache, b, _consts));
	}
	RETURN_IF_ERROR(remove_redundantNopsAndPairs(_g->entryBlock));
	RETURN_IF_ERROR(remove_unreachable(_g->entryBlock));
	RETURN_IF_ERROR(remove_redundantNopsAndJumps(_g));
	return SUCCESS;
}


static inline void maybe_push(BasicBlock* _b,
	uint64_t _unsafeMask, BasicBlock*** _sp) { // 1951
	// Push b if the unsafe mask is giving us any new information.
	// To avoid overflowing the stack, only allow each block once.
	// Use b->visited=1 to mean that b is currently on the stack.
	uint64_t both = _b->unsafeLocalsMask | _unsafeMask;
	if (_b->unsafeLocalsMask != both) {
		_b->unsafeLocalsMask = both;
		// More work left to do.
		if (!_b->visited) {
			// not on the stack, so push it.
			*(*_sp)++ = _b;
			_b->visited = 1;
		}
	}
}


static void scanBlock_forLocals(BasicBlock* b, BasicBlock*** sp) { // 1969
	uint64_t unsafe_mask = b->unsafeLocalsMask;
	for (AlifIntT i = 0; i < b->iused; i++) {
		CFGInstr* instr = &b->instr[i];
		if (instr->except != nullptr) {
			maybe_push(instr->except, unsafe_mask, sp);
		}
		if (instr->oparg >= 64) {
			continue;
		}
		uint64_t bit = (uint64_t)1 << instr->oparg;
		switch (instr->opcode) {
		case DELETE_FAST:
		case LOAD_FAST_AND_CLEAR:
		case STORE_FAST_MAYBE_NULL:
			unsafe_mask |= bit;
			break;
		case STORE_FAST:
			unsafe_mask &= ~bit;
			break;
		case LOAD_FAST_CHECK:
			unsafe_mask &= ~bit;
			break;
		case LOAD_FAST:
			if (unsafe_mask & bit) {
				instr->opcode = LOAD_FAST_CHECK;
			}
			unsafe_mask &= ~bit;
			break;
		}
	}
	if (b->next and BB_HAS_FALLTHROUGH(b)) {
		maybe_push(b->next, unsafe_mask, sp);
	}
	CFGInstr* last = basicBlock_lastInstr(b);
	if (last and is_jump(last)) {
		maybe_push(last->target, unsafe_mask, sp);
	}
}


static AlifIntT fastScan_manyLocals(BasicBlock* _entryBlock, AlifIntT _nlocals) { // 2016
	AlifSizeT* states = (AlifSizeT*)alifMem_dataAlloc((_nlocals - 64) * sizeof(AlifSizeT));
	if (states == nullptr) {
		//alifErr_noMemory();
		return ERROR;
	}
	AlifSizeT blocknum = 0;
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		blocknum++;
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			AlifIntT arg = instr->oparg;
			if (arg < 64) {
				continue;
			}
			switch (instr->opcode) {
			case DELETE_FAST:
			case LOAD_FAST_AND_CLEAR:
			case STORE_FAST_MAYBE_NULL:
				states[arg - 64] = blocknum - 1;
				break;
			case STORE_FAST:
				states[arg - 64] = blocknum;
				break;
			case LOAD_FAST:
				if (states[arg - 64] != blocknum) {
					instr->opcode = LOAD_FAST_CHECK;
				}
				states[arg - 64] = blocknum;
				break;
				ALIF_UNREACHABLE();
			}
		}
	}
	alifMem_dataFree(states);
	return SUCCESS;
}


static AlifIntT remove_unusedConsts(BasicBlock* _entryBlock, AlifObject* _consts) { // 2063
	AlifSizeT nconsts = ALIFLIST_GET_SIZE(_consts);
	if (nconsts == 0) {
		return SUCCESS;  /* nothing to do */
	}

	AlifSizeT* indexMap = nullptr;
	AlifSizeT* reverseIndexMap = nullptr;
	AlifIntT err = ERROR;

	AlifSizeT nUsedConsts{}; //* alif

	indexMap = (AlifSizeT*)alifMem_dataAlloc(nconsts * sizeof(AlifSizeT));
	if (indexMap == nullptr) {
		goto end;
	}
	for (AlifSizeT i = 1; i < nconsts; i++) {
		indexMap[i] = -1;
	}
	// The first constant may be docstring; keep it always.
	indexMap[0] = 0;

	/* mark used consts */
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		for (AlifIntT i = 0; i < b->iused; i++) {
			if (OPCODE_HAS_CONST(b->instr[i].opcode)) {
				AlifIntT index = b->instr[i].oparg;
				indexMap[index] = index;
			}
		}
	}
	/* now index_map[i] == i if consts[i] is used, -1 otherwise */
	/* condense consts */
	nUsedConsts = 0;
	for (AlifSizeT i = 0; i < nconsts; i++) {
		if (indexMap[i] != -1) {
			indexMap[nUsedConsts++] = indexMap[i];
		}
	}
	if (nUsedConsts == nconsts) {
		/* nothing to do */
		err = SUCCESS;
		goto end;
	}

	/* move all used consts to the beginning of the consts list */
	for (AlifSizeT i = 0; i < nUsedConsts; i++) {
		AlifSizeT oldIndex = indexMap[i];
		if (i != oldIndex) {
			AlifObject* value = ALIFLIST_GET_ITEM(_consts, indexMap[i]);
			alifList_setItem(_consts, i, ALIF_NEWREF(value));
		}
	}

	/* truncate the consts list at its new size */
	if (alifList_setSlice(_consts, nUsedConsts, nconsts, nullptr) < 0) {
		goto end;
	}
	/* adjust const indices in the bytecode */
	reverseIndexMap = (AlifSizeT*)alifMem_dataAlloc(nconsts * sizeof(AlifSizeT));
	if (reverseIndexMap == nullptr) {
		goto end;
	}
	for (AlifSizeT i = 0; i < nconsts; i++) {
		reverseIndexMap[i] = -1;
	}
	for (AlifSizeT i = 0; i < nUsedConsts; i++) {
		reverseIndexMap[indexMap[i]] = i;
	}

	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		for (AlifIntT i = 0; i < b->iused; i++) {
			if (OPCODE_HAS_CONST(b->instr[i].opcode)) {
				AlifIntT index = b->instr[i].oparg;
				b->instr[i].oparg = (AlifIntT)reverseIndexMap[index];
			}
		}
	}

	err = SUCCESS;
end:
	alifMem_dataFree(indexMap);
	alifMem_dataFree(reverseIndexMap);
	return err;
}



static AlifIntT addChecksFor_loadsOfUninitializedVariables(BasicBlock* _entryBlock,
	AlifIntT _nlocals, AlifIntT _nparams) { // 2160
	if (_nlocals == 0) {
		return SUCCESS;
	}
	if (_nlocals > 64) {
		if (fastScan_manyLocals(_entryBlock, _nlocals) < 0) {
			return ERROR;
		}
		_nlocals = 64;
	}
	BasicBlock** stack = makeCFG_traversalStack(_entryBlock);
	if (stack == nullptr) {
		return ERROR;
	}
	BasicBlock** sp = stack;

	uint64_t startMask = 0;
	for (AlifIntT i = _nparams; i < _nlocals; i++) {
		startMask |= (uint64_t)1 << i;
	}
	maybe_push(_entryBlock, startMask, &sp);

	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		scanBlock_forLocals(b, &sp);
	}

	while (sp > stack) {
		BasicBlock* b = *--sp;
		b->visited = 0;
		scanBlock_forLocals(b, &sp);
	}
	alifMem_dataFree(stack);
	return SUCCESS;
}


static AlifIntT mark_warm(BasicBlock* _entryBlock) { // 2210
	BasicBlock** stack = makeCFG_traversalStack(_entryBlock);
	if (stack == nullptr) {
		return ERROR;
	}
	BasicBlock** sp = stack;

	*sp++ = _entryBlock;
	_entryBlock->visited = 1;
	while (sp > stack) {
		BasicBlock* b = *(--sp);
		b->warm = 1;
		BasicBlock* next = b->next;
		if (next and BB_HAS_FALLTHROUGH(b) and !next->visited) {
			*sp++ = next;
			next->visited = 1;
		}
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			if (is_jump(instr) and !instr->target->visited) {
				*sp++ = instr->target;
				instr->target->visited = 1;
			}
		}
	}
	alifMem_dataFree(stack);
	return SUCCESS;
}

static AlifIntT mark_cold(BasicBlock* _entryBlock) { // 2241
	if (mark_warm(_entryBlock) < 0) {
		return ERROR;
	}

	BasicBlock** stack = makeCFG_traversalStack(_entryBlock);
	if (stack == nullptr) {
		return ERROR;
	}

	BasicBlock** sp = stack;
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		if (b->exceptHandler) {
			*sp++ = b;
			b->visited = 1;
		}
	}

	while (sp > stack) {
		BasicBlock* b = *(--sp);
		b->cold = 1;
		BasicBlock* next = b->next;
		if (next && BB_HAS_FALLTHROUGH(b)) {
			if (!next->warm and !next->visited) {
				*sp++ = next;
				next->visited = 1;
			}
		}
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			if (is_jump(instr)) {
				BasicBlock* target = b->instr[i].target;
				if (!target->warm and !target->visited) {
					*sp++ = target;
					target->visited = 1;
				}
			}
		}
	}
	alifMem_dataFree(stack);
	return SUCCESS;
}


static AlifIntT push_coldBlocksToEnd(CFGBuilder* g) { // 2291
	BasicBlock* entryblock = g->entryBlock;
	if (entryblock->next == nullptr) {
		/* single BasicBlock, no need to reorder */
		return SUCCESS;
	}
	RETURN_IF_ERROR(mark_cold(entryblock));

	AlifIntT nextLbl = get_maxLabel(g->entryBlock) + 1;

	for (BasicBlock* b = entryblock; b != nullptr; b = b->next) {
		if (b->cold and BB_HAS_FALLTHROUGH(b) and b->next and b->next->warm) {
			BasicBlock* explicit_jump = cfgBuilder_newBlock(g);
			if (explicit_jump == nullptr) {
				return ERROR;
			}
			if (!IS_LABEL(b->next->label)) {
				b->next->label.id = nextLbl++;
			}
			basicBlock_addOp(explicit_jump, JUMP_NO_INTERRUPT, b->next->label.id,
				_noLocation_);
			explicit_jump->cold = 1;
			explicit_jump->next = b->next;
			explicit_jump->predecessors = 1;
			b->next = explicit_jump;

			/* set target */
			CFGInstr* last = basicBlock_lastInstr(explicit_jump);
			last->target = explicit_jump->next;
		}
	}

	BasicBlock* coldBlocks = nullptr;
	BasicBlock* coldBlocksTail = nullptr;

	BasicBlock* b = entryblock;
	while (b->next) {
		while (b->next and !b->next->cold) {
			b = b->next;
		}
		if (b->next == nullptr) {
			/* no more cold blocks */
			break;
		}

		BasicBlock* b_end = b->next;
		while (b_end->next and b_end->next->cold) {
			b_end = b_end->next;
		}

		if (coldBlocks == nullptr) {
			coldBlocks = b->next;
		}
		else {
			coldBlocksTail->next = b->next;
		}
		coldBlocksTail = b_end;
		b->next = b_end->next;
		b_end->next = nullptr;
	}
	b->next = coldBlocks;

	if (coldBlocks != nullptr) {
		RETURN_IF_ERROR(remove_redundantNopsAndJumps(g));
	}
	return SUCCESS;
}



static AlifIntT convert_pseudoConditionalJumps(CFGBuilder* g) { // 2405
	BasicBlock* entryblock = g->entryBlock;
	for (BasicBlock* b = entryblock; b != nullptr; b = b->next) {
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			if (instr->opcode == JUMP_IF_FALSE or instr->opcode == JUMP_IF_TRUE) {
				instr->opcode = instr->opcode == JUMP_IF_FALSE ?
					POP_JUMP_IF_FALSE : POP_JUMP_IF_TRUE;
				Location loc = instr->loc;
				CFGInstr copy = {
							.opcode = COPY,
							.oparg = 1,
							.loc = loc,
							.target = nullptr,
				};
				RETURN_IF_ERROR(basicBlock_insertInstruction(b, i++, &copy));
				CFGInstr toBool = {
							.opcode = TO_BOOL,
							.oparg = 0,
							.loc = loc,
							.target = nullptr,
				};
				RETURN_IF_ERROR(basicBlock_insertInstruction(b, i++, &toBool));
			}
		}
	}
	return SUCCESS;
}


static AlifIntT convert_pseudoOps(CFGBuilder* _g) { // 2372
	BasicBlock* entryblock = _g->entryBlock;
	for (BasicBlock* b = entryblock; b != nullptr; b = b->next) {
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			if (is_blockPush(instr)) {
				INSTR_SET_OP0(instr, NOP);
			}
			else if (instr->opcode == LOAD_CLOSURE) {
				instr->opcode = LOAD_FAST;
			}
			else if (instr->opcode == STORE_FAST_MAYBE_NULL) {
				instr->opcode = STORE_FAST;
			}
		}
	}
	return remove_redundantNopsAndJumps(_g);
}

static inline bool isExit_orEvalCheckWithoutLineno(BasicBlock* b) { // 2395
	if (basicBlock_exitsScope(b) or basicBlock_hasEvalBreak(b)) {
		return basicBlock_hasNoLineno(b);
	}
	else {
		return false;
	}
}

static AlifIntT duplicateExits_withoutLineno(CFGBuilder* _g) { // 2415
	AlifIntT nextLbl = get_maxLabel(_g->entryBlock) + 1;

	BasicBlock* entryblock = _g->entryBlock;
	for (BasicBlock* b = entryblock; b != nullptr; b = b->next) {
		CFGInstr* last = basicBlock_lastInstr(b);
		if (last == nullptr) {
			continue;
		}
		if (is_jump(last)) {
			BasicBlock* target = next_nonemptyBlock(last->target);
			if (isExit_orEvalCheckWithoutLineno(target) and target->predecessors > 1) {
				BasicBlock* new_target = copy_basicBlock(_g, target);
				if (new_target == nullptr) {
					return ERROR;
				}
				new_target->instr[0].loc = last->loc;
				last->target = new_target;
				target->predecessors--;
				new_target->predecessors = 1;
				new_target->next = target->next;
				new_target->label.id = nextLbl++;
				target->next = new_target;
			}
		}
	}

	for (BasicBlock* b = entryblock; b != nullptr; b = b->next) {
		if (BB_HAS_FALLTHROUGH(b) and b->next and b->iused > 0) {
			if (isExit_orEvalCheckWithoutLineno(b->next)) {
				CFGInstr* last = basicBlock_lastInstr(b);
				b->next->instr[0].loc = last->loc;
			}
		}
	}
	return SUCCESS;
}




static void propagate_lineNumbers(BasicBlock* _entryBlock) { // 2468
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		CFGInstr* last = basicBlock_lastInstr(b);
		if (last == nullptr) {
			continue;
		}

		Location prevLocation = _noLocation_;
		for (AlifIntT i = 0; i < b->iused; i++) {
			if (b->instr[i].loc.lineNo < 0) {
				b->instr[i].loc = prevLocation;
			}
			else {
				prevLocation = b->instr[i].loc;
			}
		}
		if (BB_HAS_FALLTHROUGH(b) and b->next->predecessors == 1) {
			if (b->next->iused > 0) {
				if (b->next->instr[0].loc.lineNo < 0) {
					b->next->instr[0].loc = prevLocation;
				}
			}
		}
		if (is_jump(last)) {
			BasicBlock* target = last->target;
			if (target->predecessors == 1) {
				if (target->instr[0].loc.lineNo < 0) {
					target->instr[0].loc = prevLocation;
				}
			}
		}
	}
}


static AlifIntT resolve_lineNumbers(CFGBuilder* _g, AlifIntT _firstLineno) { // 2503
	RETURN_IF_ERROR(duplicateExits_withoutLineno(_g));
	propagate_lineNumbers(_g->entryBlock);
	return SUCCESS;
}



AlifIntT alifCFG_optimizeCodeUnit(CFGBuilder* _g, AlifObject* _consts,
	AlifObject* _constCache, AlifIntT _nlocals,
	AlifIntT _nparams, AlifIntT _firstLineno) { // 2511
	RETURN_IF_ERROR(translateJump_labelsToTargets(_g->entryBlock));
	RETURN_IF_ERROR(mark_exceptHandlers(_g->entryBlock));
	RETURN_IF_ERROR(label_exceptionTargets(_g->entryBlock));

	/** Optimization **/
	RETURN_IF_ERROR(optimize_cfg(_g, _consts, _constCache, _firstLineno));
	RETURN_IF_ERROR(remove_unusedConsts(_g->entryBlock, _consts));
	RETURN_IF_ERROR(
		addChecksFor_loadsOfUninitializedVariables(
			_g->entryBlock, _nlocals, _nparams));
	//RETURN_IF_ERROR(insert_superInstructions(_g));

	RETURN_IF_ERROR(push_coldBlocksToEnd(_g));
	RETURN_IF_ERROR(resolve_lineNumbers(_g, _firstLineno));
	return SUCCESS;
}


static AlifIntT* build_cellFixedOffsets(AlifCompileCodeUnitMetadata* _umd) { // 2536
	AlifIntT nlocals = (AlifIntT)ALIFDICT_GET_SIZE(_umd->varnames);
	AlifIntT ncellvars = (AlifIntT)ALIFDICT_GET_SIZE(_umd->cellvars);
	AlifIntT nfreevars = (AlifIntT)ALIFDICT_GET_SIZE(_umd->freevars);

	AlifIntT noffsets = ncellvars + nfreevars;
	noffsets ? noffsets : noffsets = 1; //* alif
	AlifIntT* fixed = (AlifIntT*)alifMem_dataAlloc(noffsets * sizeof(AlifIntT));
	if (fixed == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	for (AlifIntT i = 0; i < noffsets; i++) {
		fixed[i] = nlocals + i;
	}

	AlifObject* varname{}, * cellindex{};
	AlifSizeT pos = 0;
	while (alifDict_next(_umd->cellvars, &pos, &varname, &cellindex)) {
		AlifObject* varindex{};
		if (alifDict_getItemRef(_umd->varnames, varname, &varindex) < 0) {
			goto error;
		}
		if (varindex == nullptr) {
			continue;
		}

		AlifIntT argoffset = alifLong_asInt(varindex);
		ALIF_DECREF(varindex);
		if (argoffset == -1 and alifErr_occurred()) {
			goto error;
		}

		AlifIntT oldindex = alifLong_asInt(cellindex);
		if (oldindex == -1 and alifErr_occurred()) {
			goto error;
		}
		fixed[oldindex] = argoffset;
	}
	return fixed;

error:
	alifMem_dataFree(fixed);
	return nullptr;
}



#define IS_GENERATOR(_cf) (_cf & (CO_GENERATOR | CO_COROUTINE | CO_ASYNC_GENERATOR)) // 2583

static AlifIntT insert_prefixInstructions(AlifCompileCodeUnitMetadata* _umd,
	BasicBlock* _entryBlock, AlifIntT* _fixed,
	AlifIntT _nFreeVars, AlifIntT _codeFlags) { // 2586
	if (IS_GENERATOR(_codeFlags)) {
		Location loc = LOCATION(_umd->firstLineno, _umd->firstLineno, -1, -1);
		CFGInstr make_gen = {
			.opcode = RETURN_GENERATOR,
			.oparg = 0,
			.loc = loc,
			.target = nullptr,
		};
		RETURN_IF_ERROR(basicBlock_insertInstruction(_entryBlock, 0, &make_gen));
		CFGInstr pop_top = {
			.opcode = POP_TOP,
			.oparg = 0,
			.loc = loc,
			.target = nullptr,
		};
		RETURN_IF_ERROR(basicBlock_insertInstruction(_entryBlock, 1, &pop_top));
	}

	const AlifIntT ncellvars = (AlifIntT)ALIFDICT_GET_SIZE(_umd->cellvars);
	if (ncellvars) {
		const AlifIntT nvars = ncellvars + (AlifIntT)ALIFDICT_GET_SIZE(_umd->varnames);
		AlifIntT* sorted = (AlifIntT*)alifMem_dataAlloc(nvars * sizeof(AlifIntT));
		if (sorted == nullptr) {
			//alifErr_noMemory();
			return ERROR;
		}
		for (AlifIntT i = 0; i < ncellvars; i++) {
			sorted[_fixed[i]] = i + 1;
		}
		for (AlifIntT i = 0, ncellsused = 0; ncellsused < ncellvars; i++) {
			AlifIntT oldindex = sorted[i] - 1;
			if (oldindex == -1) {
				continue;
			}
			CFGInstr make_cell = {
				.opcode = MAKE_CELL,
				.oparg = oldindex,
				.loc = _noLocation_,
				.target = nullptr,
			};
			if (basicBlock_insertInstruction(_entryBlock, ncellsused, &make_cell) < 0) {
				alifMem_dataFree(sorted);
				return ERROR;
			}
			ncellsused += 1;
		}
		alifMem_dataFree(sorted);
	}

	if (_nFreeVars) {
		CFGInstr copy_frees = {
			.opcode = COPY_FREE_VARS,
			.oparg = _nFreeVars,
			.loc = _noLocation_,
			.target = nullptr,
		};
		RETURN_IF_ERROR(basicBlock_insertInstruction(_entryBlock, 0, &copy_frees));
	}

	return SUCCESS;
}


static AlifIntT fix_cellOffsets(AlifCompileCodeUnitMetadata* _umd,
	BasicBlock* _entryBlock, AlifIntT* _fixedMap) { // 2665
	AlifIntT nlocals = (AlifIntT)ALIFDICT_GET_SIZE(_umd->varnames);
	AlifIntT ncellvars = (AlifIntT)ALIFDICT_GET_SIZE(_umd->cellvars);
	AlifIntT nfreevars = (AlifIntT)ALIFDICT_GET_SIZE(_umd->freevars);
	AlifIntT noffsets = ncellvars + nfreevars;

	// First deal with duplicates (arg cells).
	AlifIntT numdropped = 0;
	for (AlifIntT i = 0; i < noffsets; i++) {
		if (_fixedMap[i] == i + nlocals) {
			_fixedMap[i] -= numdropped;
		}
		else {
			// It was a duplicate (cell/arg).
			numdropped += 1;
		}
	}

	// Then update offsets, either relative to locals or by cell2arg.
	for (BasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* inst = &b->instr[i];
			// This is called before extended args are generated.
			AlifIntT oldoffset = inst->oparg;
			switch (inst->opcode) {
			case MAKE_CELL:
			case LOAD_CLOSURE:
			case LOAD_DEREF:
			case STORE_DEREF:
			case DELETE_DEREF:
			case LOAD_FROM_DICT_OR_DEREF:
				inst->oparg = _fixedMap[oldoffset];
			}
		}
	}

	return numdropped;
}



static AlifIntT prepare_localsPlus(AlifCompileCodeUnitMetadata* _umd,
	CFGBuilder* _g, AlifIntT _codeFlags) { // 2710
	AlifIntT nlocals = (AlifIntT)ALIFDICT_GET_SIZE(_umd->varnames);
	AlifIntT ncellvars = (AlifIntT)ALIFDICT_GET_SIZE(_umd->cellvars);
	AlifIntT nfreevars = (AlifIntT)ALIFDICT_GET_SIZE(_umd->freevars);
	AlifIntT nlocalsplus = nlocals + ncellvars + nfreevars;
	AlifIntT* cellFixedOffsets = build_cellFixedOffsets(_umd);
	if (cellFixedOffsets == nullptr) {
		return ERROR;
	}

	// This must be called before fix_cellOffsets().
	if (insert_prefixInstructions(_umd, _g->entryBlock, cellFixedOffsets, nfreevars, _codeFlags)) {
		alifMem_dataFree(cellFixedOffsets);
		return ERROR;
	}

	AlifIntT numdropped = fix_cellOffsets(_umd, _g->entryBlock, cellFixedOffsets);
	alifMem_dataFree(cellFixedOffsets);  // At this point we're done with it.
	cellFixedOffsets = nullptr;
	if (numdropped < 0) {
		return ERROR;
	}

	nlocalsplus -= numdropped;
	return nlocalsplus;
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


AlifIntT alifCFG_toInstructionSequence(CFGBuilder* _g, AlifInstructionSequence* _seq) { // 2787
	AlifIntT lbl = 0;
	for (BasicBlock* b = _g->entryBlock; b != nullptr; b = b->next) {
		b->label = { lbl };
		lbl += 1;
	}
	for (BasicBlock* b = _g->entryBlock; b != nullptr; b = b->next) {
		RETURN_IF_ERROR(_alifInstructionSequence_useLabel(_seq, b->label.id));
		for (AlifIntT i = 0; i < b->iused; i++) {
			CFGInstr* instr = &b->instr[i];
			if (HAS_TARGET(instr->opcode)) {
				instr->oparg = instr->target->label.id;
			}
			RETURN_IF_ERROR(
				_alifInstructionSequence_addOp(_seq, instr->opcode, instr->oparg, instr->loc));

			AlifExceptHandlerInfo* hi = &_seq->instrs[_seq->used - 1].exceptHandlerInfo;
			if (instr->except != nullptr) {
				hi->label = instr->except->label.id;
				hi->startDepth = instr->except->startDepth;
				hi->preserveLastI = instr->except->preserveLastI;
			}
			else {
				hi->label = -1;
			}
		}
	}
	if (_alifInstructionSequence_applyLabelMap(_seq) < 0) {
		return ERROR;
	}
	return SUCCESS;
}



AlifIntT alifCFG_optimizedCFGToInstructionSequence(CFGBuilder* _g,
	AlifCompileCodeUnitMetadata* _umd, AlifIntT _codeFlags, AlifIntT* _stackDepth,
	AlifIntT* _nLocalsPlus, AlifInstructionSequence* _seq) { // 2825

	RETURN_IF_ERROR(convert_pseudoConditionalJumps(_g));

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









AlifIntT alifCompile_opcodeStackEffect(AlifIntT _opcode, AlifIntT _oparg) { // 2951
	return stack_effect(_opcode, _oparg, -1);
}
