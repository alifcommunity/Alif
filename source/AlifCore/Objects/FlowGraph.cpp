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
	unsigned preserveLastI : 1;
	unsigned visited : 1;
	unsigned exceptHandler : 1;
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

#define LOCATION(_lno, _endLno, _col, _endCol) {_lno, _endLno, _col, _endCol} // 94

static inline AlifIntT is_blockPush(CFGInstr* _i) { // 97
	return IS_BLOCK_PUSH_OPCODE(_i->opcode);
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
	b_->label = _noLable_;
	return b_;
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

static AlifIntT cfgBuilder_maybeStartNewBlock(CFGBuilder* _g) { // 366
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






static AlifIntT normalize_jumps(CFGBuilder* _g) { // 590
	BasicBlock* entryBlock = _g->entryBlock;
	for (BasicBlock* b = entryBlock; b != nullptr; b = b->next) {
		b->visited = 0;
	}
	for (BasicBlock* b = entryBlock; b != nullptr; b = b->next) {
		b->visited = 1;
		RETURN_IF_ERROR(normalize_jumpsInBlock(_g, b));
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





static AlifIntT remove_unusedConsts(BasicBlock* _entryBlock, AlifObject* _consts) { // 2063
	AlifSizeT nconsts = ALIFLIST_GET_SIZE(_consts);
	if (nconsts == 0) {
		return SUCCESS;  /* nothing to do */
	}

	AlifSizeT* indexMap = nullptr;
	AlifSizeT* reverseIndexMap = nullptr;
	AlifIntT err = ERROR;

	AlifSizeT nUsedConsts{}; // alif

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
		for (int i = 0; i < b->iused; i++) {
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
		for (int i = 0; i < b->iused; i++) {
			if (OPCODE_HAS_CONST(b->instr[i].opcode)) {
				int index = b->instr[i].oparg;
				b->instr[i].oparg = (int)reverseIndexMap[index];
			}
		}
	}

	err = SUCCESS;
end:
	alifMem_dataFree(indexMap);
	alifMem_dataFree(reverseIndexMap);
	return err;
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
		if (b->cold && BB_HAS_FALLTHROUGH(b) and b->next and b->next->warm) {
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



static AlifIntT convert_pseudoOps(CFGBuilder* _g) { // 2372
	BasicBlock* entryblock = _g->entryBlock;
	for (BasicBlock* b = entryblock; b != nullptr; b = b->next) {
		for (int i = 0; i < b->iused; i++) {
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
	//RETURN_IF_ERROR(labelException_targets(_g->entryBlock));

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
		if (argoffset == -1 /*and alifErr_occurred()*/) {
			goto error;
		}

		AlifIntT oldindex = alifLong_asInt(cellindex);
		if (oldindex == -1 /*and alifErr_occurred()*/) {
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
		AlifIntT* sorted = (AlifIntT*)alifMem_dataAlloc(nvars * sizeof(int));
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
		for (int i = 0; i < b->iused; i++) {
			CFGInstr* inst = &b->instr[i];
			// This is called before extended args are generated.
			int oldoffset = inst->oparg;
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
	AlifIntT nlocals = (int)ALIFDICT_GET_SIZE(_umd->varnames);
	AlifIntT ncellvars = (int)ALIFDICT_GET_SIZE(_umd->cellvars);
	AlifIntT nfreevars = (int)ALIFDICT_GET_SIZE(_umd->freevars);
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
		for (int i = 0; i < b->iused; i++) {
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
