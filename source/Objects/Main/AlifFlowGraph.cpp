#include "alif.h"

#include "AlifCore_FlowGraph.h"
#include "AlifCore_Compile.h"
#include "AlifCore_Memory.h"

#include "AlifCore_OpCodeUtils.h"
#include "AlifCore_OpCodeData.h"

#define ERROR -1

// Forward
static AlifIntT resolve_lineNumbers(AlifFlowGraph*, AlifIntT);


#define DEFAULT_BLOCK_SIZE 16

static const SourceLocation noLocation = { -1, -1, -1, -1 };


class AlifCFGInstruction {
public:
	AlifIntT opCode{};
	AlifIntT opArg{};
	SourceLocation loc{};
	class AlifCFGBasicBlock* target{};
};

class AlifCFGBasicBlock { // 36
public:
	AlifCFGBasicBlock* list{};
	JumpTargetLable label{};

	AlifCFGInstruction* instr{};

	AlifCFGBasicBlock* next{};

	AlifIntT iUsed{};
	AlifIntT ialloc{};
	AlifIntT predecessors{};
	unsigned visited : 1;
	unsigned cold : 1;
};

class AlifFlowGraph { // 74
public:
	AlifCFGBasicBlock* entryBlock{};
	AlifCFGBasicBlock* blockList{};
	AlifCFGBasicBlock* curBlock{};

	JumpTargetLable curLabel{};
};

static const JumpTargetLable noLabel = { -1 }; // 89

#define SAME_LABEL(_l1, _l2) ((_l1).id == (_l2).id)
#define IS_LABEL(_l) (!SAME_LABEL((_l), (noLabel)))

/* One arg*/
#define INSTR_SET_OP1(_i, _op, _arg) \
    do { \
        AlifCFGInstruction *_instr__ptr_ = (_i); \
        _instr__ptr_->opCode = (_op); \
        _instr__ptr_->opArg = (_arg); \
    } while (0);
/* No args*/
#define INSTR_SET_OP0(_i, _op) \
    do { \
        AlifCFGInstruction* _instr__ptr_ = (_i); \
        _instr__ptr_->opCode = (_op); \
        _instr__ptr_->opArg = 0; \
    } while (0);


static inline AlifIntT is_jump(AlifCFGInstruction* _i) { // 104
	return OPCODE_HAS_JUMP(_i->opCode);
}

static AlifIntT basicBlock_nextInstr(AlifCFGBasicBlock* _block) { // 134
	if (alifCompile_ensureArraySpace(_block->iUsed + 1,
		(void**)&_block->instr,
		&_block->ialloc,
		DEFAULT_BLOCK_SIZE,
		sizeof(AlifCFGInstruction)) == -1) {
		return -1;
	}
	return _block->iUsed++;
}

static AlifCFGBasicBlock* alifFlowGraph_newBlock(AlifFlowGraph* _cfg) { // 162

	AlifCFGBasicBlock* block = (AlifCFGBasicBlock*)alifMem_dataAlloc(sizeof(AlifCFGBasicBlock));
	if (block == nullptr) {
		// memory error
		return nullptr;
	}

	block->list = _cfg->blockList;
	_cfg->blockList = block;
	block->label = noLabel;

	return block;
}


static AlifCFGInstruction* basicBlock_lastInstr(const AlifCFGBasicBlock* _block) { // 148
	if (_block->iUsed > 0) {
		return &_block->instr[_block->iUsed - 1];
	}

	return nullptr;
}

static AlifIntT basicBlock_addOp(AlifCFGBasicBlock* _block,
	AlifIntT _opCode, AlifIntT _opArg, SourceLocation _loc) { // 177
	
	AlifIntT off = basicBlock_nextInstr(_block);
	if (off < 0) return -1;

	AlifCFGInstruction* instr = &_block->instr[off];
	instr->opCode = _opCode;
	instr->opArg = _opArg;
	instr->target = nullptr;
	instr->loc = _loc;

	return 1;
}

static AlifIntT basicBlock_addJump(AlifCFGBasicBlock* _block, AlifIntT _opCode,
	AlifCFGBasicBlock* _target, SourceLocation _loc) { // 198
	AlifCFGInstruction* last = basicBlock_lastInstr(_block);
	if (last and is_jump(last)) return -1;

	if(basicBlock_addOp(_block, _opCode, _target->label.id, _loc) == -1) return -1;
	last = basicBlock_lastInstr(_block);
	last->target = _target;
	return 1;
}

static inline AlifIntT basicBlock_appendInstructions(AlifCFGBasicBlock* _to, AlifCFGBasicBlock* _from) { // 214
	for (AlifIntT i = 0; i < _from->iUsed; i++) {
		AlifIntT n = basicBlock_nextInstr(_to);
		if (n < 0) {
			return 0;
		}
		_to->instr[n] = _from->instr[i];
	}
	return 1;
}

static inline AlifIntT basicBlock_noFallThrough(const AlifCFGBasicBlock* _b) {
	AlifCFGInstruction* last = basicBlock_lastInstr(_b);
	return (last and
		(ISSCOPE_EXIT_OPCODE(last->opCode) or ISUNCONDITIONAL_JUMP_OPCODE(last->opCode)));
}

#define BB_NO_FALLTHROUGH(_b) (basicBlock_noFallThrough(_b))
#define BB_HAS_FALLTHROUGH(_b) (!basicBlock_noFallThrough(_b))

static AlifCFGBasicBlock* copy_basicBlock(AlifFlowGraph* _cfg, AlifCFGBasicBlock* _block) { // 238
	/* Cannot copy a block if it has a fallthrough, since
	 * a block can only have one fallthrough predecessor.
	 */
	AlifCFGBasicBlock* result = alifFlowGraph_newBlock(_cfg);
	if (result == nullptr) {
		return nullptr;
	}
	if (basicBlock_appendInstructions(result, _block) < 0) {
		return nullptr;
	}
	return result;
}

static AlifCFGBasicBlock* alifFlowGraph_useNextBlock(AlifFlowGraph* _cfg, AlifCFGBasicBlock* _block) { // 321
	_cfg->curBlock->next = _block;
	_cfg->curBlock = _block;
	return _block;
}

static inline AlifIntT basicBlock_exitsScope(const AlifCFGBasicBlock* _b) { // 329
	AlifCFGInstruction* last = basicBlock_lastInstr(_b);
	return last and ISSCOPE_EXIT_OPCODE(last->opCode);
}

static inline AlifIntT basicBlock_hasEvalBreak(const AlifCFGBasicBlock* _b) { // 335
	for (AlifIntT i = 0; i < _b->iUsed; i++) {
		if (OPCODE_HAS_EVAL_BREAK(_b->instr[i].opCode)) {
			return true;
		}
	}
	return false;
}

static bool alifFlowGraph_currentBlockIsTerminated(AlifFlowGraph* _cfg) { // 345
	AlifCFGInstruction* last = basicBlock_lastInstr(_cfg->curBlock);
	if (last and IS_TERMINATOR_OPCODE(last->opCode)) {
		return true;
	}
	if (IS_LABEL(_cfg->curLabel)) {
		if (last or IS_LABEL(_cfg->curBlock->label)) {
			return true;
		}
		else {
			_cfg->curBlock->label = _cfg->curLabel;
			_cfg->curLabel = noLabel;
		}
	}

	return false;
}

static AlifIntT alifFlowGraph_maybeStartNewBlock(AlifFlowGraph* _cfg) { // 365

	if (alifFlowGraph_currentBlockIsTerminated(_cfg)) {
		AlifCFGBasicBlock* block = alifFlowGraph_newBlock(_cfg);
		if (block == nullptr) {
			return -1;
		}
		block->label = _cfg->curLabel;
		_cfg->curLabel = noLabel;
		alifFlowGraph_useNextBlock(_cfg, block);
	}

	return 1;
}

static AlifIntT init_alifFlowGraph(AlifFlowGraph* _cfg) { // 401

	_cfg->blockList = nullptr;
	AlifCFGBasicBlock* block = alifFlowGraph_newBlock(_cfg);
	if (block == nullptr) return -1;

	_cfg->curBlock = _cfg->entryBlock = block;
	_cfg->curLabel = noLabel;

	return 1;
}


AlifFlowGraph* alifFlowGraph_new() { // 414
	
	AlifFlowGraph* cfg = (AlifFlowGraph*)alifMem_dataAlloc(sizeof(AlifFlowGraph));
	if (cfg == nullptr) {
		// memory error
		return nullptr;
	}
	if (init_alifFlowGraph(cfg) < 0) {
		alifMem_dataFree(cfg);
		return nullptr;
	}

	return cfg;
}

AlifIntT alifFlowGraph_checkSize(AlifFlowGraph* _cfg) { // 449
	AlifIntT nBlocks = 0;
	for (AlifCFGBasicBlock* b = _cfg->blockList; b != nullptr; b = b->list) {
		nBlocks++;
	}
	if ((AlifUSizeT)nBlocks > SIZE_MAX / sizeof(AlifCFGBasicBlock*)) {
		// memory error
		return -1;
	}

	return 1;
}

AlifIntT alifFlowGraph_useLable(AlifFlowGraph* _cfg, JumpTargetLable _lable) { // 463

	_cfg->curLabel = _lable;
	return alifFlowGraph_maybeStartNewBlock(_cfg);
}


AlifIntT alifFlowGraph_addOp(AlifFlowGraph* _cfg, AlifIntT _opCode, AlifIntT _opArg, SourceLocation _loc) { // 470

	if (alifFlowGraph_maybeStartNewBlock(_cfg) == -1) {
		return -1;
	}
	return basicBlock_addOp(_cfg->curBlock, _opCode, _opArg, _loc);
}

static AlifCFGBasicBlock* next_nonEmptyBlock(AlifCFGBasicBlock* _block) { // 478
	while (_block and _block->iUsed == 0) {
		_block = _block->next;
	}
	return _block;
}

static AlifIntT normalize_jumpsInBlock(AlifFlowGraph* _cfg, AlifCFGBasicBlock* _block) { // 540
	AlifCFGInstruction* last = basicBlock_lastInstr(_block);
	if (last == nullptr or !is_jump(last) or ISUNCONDITIONAL_JUMP_OPCODE(last->opCode)) {
		return 1;
	}

	bool isForward = last->target->visited == 0;
	if (isForward) {
		return 1;
	}

	AlifIntT reversedOpCode = 0;
	switch (last->opCode) {
	case POP_JUMPIF_NOTNONE:
		reversedOpCode = POP_JUMPIF_NONE;
		break;
	case POP_JUMPIF_NONE:
		reversedOpCode = POP_JUMPIF_NOTNONE;
		break;
	case POP_JUMPIF_FALSE:
		reversedOpCode = POP_JUMPIF_TRUE;
		break;
	case POP_JUMPIF_TRUE:
		reversedOpCode = POP_JUMPIF_FALSE;
		break;
	}

	AlifCFGBasicBlock* target = last->target;
	AlifCFGBasicBlock* backwardsJump = alifFlowGraph_newBlock(_cfg);
	if (backwardsJump == nullptr) return -1;

	if(basicBlock_addJump(backwardsJump, JUMP, target, last->loc) == -1) return -1;
	last->opCode = reversedOpCode;
	last->target = _block->next;

	backwardsJump->cold = _block->cold;
	backwardsJump->next = _block->next;
	_block->next = backwardsJump;
	return 1;
}

static AlifIntT normalize_jumps(AlifFlowGraph* _cfg) { // 590
	AlifCFGBasicBlock* entryblock = _cfg->entryBlock;
	for (AlifCFGBasicBlock* b = entryblock; b != nullptr; b = b->next) {
		b->visited = 0;
	}
	for (AlifCFGBasicBlock* b = entryblock; b != nullptr; b = b->next) {
		b->visited = 1;
		if(normalize_jumpsInBlock(_cfg, b) == -1) return -1;
	}
	return 1;
}

static AlifIntT get_maxLabel(AlifCFGBasicBlock* _entryBlock) { // 622
	AlifIntT label = -1;
	for (AlifCFGBasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		if (b->label.id > label) {
			label = b->label.id;
		}
	}
	return label;
}

static AlifIntT basicBlock_removeRedundantNops(AlifCFGBasicBlock* _cfgb) { // 1013
	/* Remove NOPs when legal to do so. */
	AlifIntT dest = 0;
	AlifIntT prevLineNo = -1;
	for (AlifIntT src = 0; src < _cfgb->iUsed; src++) {
		AlifIntT lineno = _cfgb->instr[src].loc.lineNo;
		if (_cfgb->instr[src].opCode == NOP) {
			/* Eliminate no-op if it doesn't have a line number */
			if (lineno < 0) {
				continue;
			}
			/* or, if the previous instruction had the same line number. */
			if (prevLineNo == lineno) {
				continue;
			}
			/* or, if the next instruction has same line number or no line number */
			if (src < _cfgb->iUsed - 1) {
				AlifIntT nextLineNo = _cfgb->instr[src + 1].loc.lineNo;
				if (nextLineNo == lineno) {
					continue;
				}
				if (nextLineNo < 0) {
					_cfgb->instr[src + 1].loc = _cfgb->instr[src].loc;
					continue;
				}
			}
			else {
				AlifCFGBasicBlock* next = next_nonEmptyBlock(_cfgb->next);
				/* or if last instruction in _cfgb and next _cfgb has same line number */
				if (next) {
					SourceLocation nextLoc = noLocation;
					for (AlifIntT next_i = 0; next_i < next->iUsed; next_i++) {
						AlifCFGInstruction* instr = &next->instr[next_i];
						if (instr->opCode == NOP && instr->loc.lineNo == noLocation.lineNo) {
							/* Skip over NOPs without location, they will be removed */
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
		if (dest != src) {
			_cfgb->instr[dest] = _cfgb->instr[src];
		}
		dest++;
		prevLineNo = lineno;
	}
	AlifIntT numRemoved = _cfgb->iUsed - dest;
	_cfgb->iUsed = dest;
	return numRemoved;
}

static AlifIntT remove_redundantNopsAndPairs(AlifCFGBasicBlock* _entryBlock) { // 1084
	bool done = false;

	while (!done) {
		done = true;
		AlifCFGInstruction* prevInstr = nullptr;
		AlifCFGInstruction* instr = nullptr;
		for (AlifCFGBasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
			if(basicBlock_removeRedundantNops(b) == -1) return -1;
			if (IS_LABEL(b->label)) {
				instr = nullptr;
			}
			for (AlifIntT i = 0; i < b->iUsed; i++) {
				prevInstr = instr;
				instr = &b->instr[i];
				AlifIntT prevOpCode = prevInstr ? prevInstr->opCode : 0;
				AlifIntT prevOpArg = prevInstr ? prevInstr->opArg : 0;
				AlifIntT opCode = instr->opCode;
				bool isRedundantPair = false;
				if (opCode == POP_TOP) {
					if (prevOpCode == LOAD_CONST) {
						isRedundantPair = true;
					}
					else if (prevOpCode == COPY && prevOpArg == 1) {
						isRedundantPair = true;
					}
				}
				if (isRedundantPair) {
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
	return 1;
}

static inline bool basicBlock_hasNoLineNo(AlifCFGBasicBlock* _b) { // 1162
	for (AlifIntT i = 0; i < _b->iUsed; i++) {
		if (_b->instr[i].loc.lineNo >= 0) {
			return false;
		}
	}
	return true;
}

static AlifObject* get_constValue(AlifIntT _opCode, AlifIntT _opArg, AlifObject* _consts) { // 1256
	AlifObject* constant = nullptr;
	if (_opCode == LOAD_CONST) {
		constant = ALIFLIST_GET_ITEM(_consts, _opArg);
	}

	if (constant == nullptr) {
		// error
		return nullptr;
	}
	return ALIF_NEWREF(constant);
}

static AlifIntT add_const(AlifObject* _newConst, AlifObject* _consts) { // 1274
	//if (alifCompile_constCacheMergeOne(const_cache, &_newConst) < 0) {
	//	ALIF_DECREF(_newConst);
	//	return -1;
	//}

	AlifSizeT index{};
	for (index = 0; index < ALIFLIST_GET_SIZE(_consts); index++) {
		if (ALIFLIST_GET_ITEM(_consts, index) == _newConst) {
			break;
		}
	}
	if (index == ALIFLIST_GET_SIZE(_consts)) {
		if ((size_t)index >= (size_t)INT_MAX - 1) {
			// error
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

static AlifIntT basicBlock_optimizeLoadConst(AlifCFGBasicBlock* _cfgb, AlifObject* _consts) { // 1542
	AlifIntT opcode = 0;
	AlifIntT oparg = 0;
	for (AlifIntT i = 0; i < _cfgb->iUsed; i++) {
		AlifCFGInstruction* inst = &_cfgb->instr[i];
		bool is_copy_of_load_const = (opcode == LOAD_CONST and
			inst->opCode == COPY and
			inst->opArg == 1);
		if (!is_copy_of_load_const) {
			opcode = inst->opCode;
			oparg = inst->opArg;
		}

		if (opcode != LOAD_CONST) {
			continue;
		}
		AlifIntT nextop = i + 1 < _cfgb->iUsed ? _cfgb->instr[i + 1].opCode : 0;
		switch (nextop) {
		case POP_JUMPIF_FALSE:
		case POP_JUMPIF_TRUE:
		{
			/* Remove LOAD_CONST const; conditional jump */
			AlifObject* cnt = get_constValue(opcode, oparg, _consts);
			if (cnt == nullptr) return -1;

			AlifIntT is_true = alifObject_isTrue(cnt);
			ALIF_DECREF(cnt);
			if (is_true == -1) return -1;

			INSTR_SET_OP0(inst, NOP);
			AlifIntT jumpIfTrue = nextop == POP_JUMPIF_TRUE;
			if (is_true == jumpIfTrue) {
				_cfgb->instr[i + 1].opCode = JUMP;
			}
			else {
				INSTR_SET_OP0(&_cfgb->instr[i + 1], NOP);
			}
			break;
		}
		case IS_OP:
		{
			// Fold to POP_JUMP_IF_NONE:
			// - LOAD_CONST(None) IS_OP(0) POP_JUMP_IF_TRUE
			// - LOAD_CONST(None) IS_OP(1) POP_JUMP_IF_FALSE
			// - LOAD_CONST(None) IS_OP(0) TO_BOOL POP_JUMP_IF_TRUE
			// - LOAD_CONST(None) IS_OP(1) TO_BOOL POP_JUMP_IF_FALSE
			// Fold to POP_JUMP_IF_NOT_NONE:
			// - LOAD_CONST(None) IS_OP(0) POP_JUMP_IF_FALSE
			// - LOAD_CONST(None) IS_OP(1) POP_JUMP_IF_TRUE
			// - LOAD_CONST(None) IS_OP(0) TO_BOOL POP_JUMP_IF_FALSE
			// - LOAD_CONST(None) IS_OP(1) TO_BOOL POP_JUMP_IF_TRUE
			AlifObject* cnt = get_constValue(opcode, oparg, _consts);
			if (cnt == nullptr) return ERROR;

			if (!alif_isNone(cnt)) {
				ALIF_DECREF(cnt);
				break;
			}
			if (_cfgb->iUsed <= i + 2) {
				break;
			}
			AlifCFGInstruction* isInstr = &_cfgb->instr[i + 1];
			AlifCFGInstruction* jumpInstr = &_cfgb->instr[i + 2];
			// Get rid of TO_BOOL regardless:
			if (jumpInstr->opCode == TO_BOOL) {
				INSTR_SET_OP0(jumpInstr, NOP);
				if (_cfgb->iUsed <= i + 3) {
					break;
				}
				jumpInstr = &_cfgb->instr[i + 3];
			}
			bool invert = isInstr->opArg;
			if (jumpInstr->opCode == POP_JUMPIF_FALSE) {
				invert = !invert;
			}
			else if (jumpInstr->opCode != POP_JUMPIF_TRUE) {
				break;
			}
			INSTR_SET_OP0(inst, NOP);
			INSTR_SET_OP0(isInstr, NOP);
			jumpInstr->opCode = invert ? POP_JUMPIF_NOTNONE : POP_JUMPIF_NONE;
			break;
		}
		case RETURN_VALUE:
		{
			INSTR_SET_OP0(inst, NOP);
			INSTR_SET_OP1(&_cfgb->instr[++i], RETURN_CONST, oparg);
			break;
		}
		case TO_BOOL:
		{
			AlifObject* cnt = get_constValue(opcode, oparg, _consts);
			if (cnt == nullptr) return -1;

			AlifIntT isTrue = alifObject_isTrue(cnt);
			ALIF_DECREF(cnt);
			if (isTrue == -1) return -1;

			cnt = alifBool_fromInteger(isTrue);
			AlifIntT index = add_const(cnt, _consts);
			if (index < 0) return -1;

			INSTR_SET_OP0(inst, NOP);
			INSTR_SET_OP1(&_cfgb->instr[i + 1], LOAD_CONST, index);
			break;
		}
		}
	}
	return 1;
}

static AlifIntT optimize_loadConst(AlifFlowGraph* _cfg, AlifObject* _consts) { // 1664

	for (AlifCFGBasicBlock* b = _cfg->entryBlock; b != nullptr; b = b->next) {
		if (basicBlock_optimizeLoadConst(b, _consts) == -1) return -1;
	}
	return 1;
}

static AlifIntT optimize_alifFlowGraph(AlifFlowGraph* _cfg, AlifObject* _consts, AlifIntT _firstLineNo) { // 1839

	if (resolve_lineNumbers(_cfg, _firstLineNo) == -1) return -1;
	if (optimize_loadConst(_cfg, _consts) == -1) return -1;

	if (remove_redundantNopsAndPairs(_cfg->entryBlock) == -1) return -1;


	return 1; // need review
}

static AlifIntT remove_unusedConsts(AlifCFGBasicBlock* _entryBlock, AlifObject* _consts) { // 2029

	AlifSizeT nConsts = ALIFLIST_GET_SIZE(_consts);
	if (nConsts == 0) return 1;

	AlifSizeT* indexMap = nullptr;
	AlifSizeT* revIndexMap = nullptr;
	AlifIntT ret = -1;
	AlifSizeT nUsedConsts{};

	indexMap = (AlifSizeT*)alifMem_dataAlloc(nConsts * sizeof(AlifSizeT));
	if (indexMap == nullptr) {
		goto end;
	}
	for (AlifSizeT i = 1; i < nConsts; i++) {
		indexMap[i] = -1;
	}
	indexMap[0] = 0;

	for (AlifCFGBasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		for (AlifIntT i = 0; i < b->iUsed; i++) {
			if (OPCODE_HAS_CONST(b->instr[i].opCode)) {
				AlifIntT index = b->instr->opArg;
				indexMap[index] = index;
			}
		}
	}

	nUsedConsts = 0;
	for (AlifIntT i = 0; i < nConsts; i++) {
		if (indexMap[i] != -1) {
			indexMap[nUsedConsts++] = indexMap[i];
		}
	}
	if (nUsedConsts == nConsts) {
		ret = 1;
		goto end;
	}

	for (AlifSizeT i = 0; i < nUsedConsts; i++) {
		AlifSizeT oldIndex = indexMap[i];
		if (i != oldIndex) {
			AlifObject* value = ALIFLIST_GET_ITEM(_consts, indexMap[i]);
			alifList_setItem(_consts, i, ALIF_NEWREF(value));
		}
	}

	//if (alifList_setSlice(_consts, nUsedConsts, nConsts, nullptr) < 0) {
	//	goto end;
	//}

	revIndexMap = (AlifSizeT*)alifMem_dataAlloc(nConsts * sizeof(AlifSizeT));
	if (revIndexMap == nullptr) {
		goto end;
	}
	for (AlifSizeT i = 0; i < nConsts; i++) {
		revIndexMap[i] = -1;
	}
	for (AlifSizeT i = 0; i < nUsedConsts; i++) {
		revIndexMap[indexMap[i]] = i;
	}

	for (AlifCFGBasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		for (int i = 0; i < b->iUsed; i++) {
			if (OPCODE_HAS_CONST(b->instr[i].opCode)) {
				int index = b->instr[i].opArg;
				b->instr[i].opArg = (int)revIndexMap[index];
			}
		}
	}

	ret = 1;

end:
	alifMem_dataFree(indexMap);
	//alifMem_dataFree(revIndexMap);
	return ret;
}

static inline bool isExit_orEvalCheckWithoutLineNo(AlifCFGBasicBlock* _b) { // 2365
	if (basicBlock_exitsScope(_b) or basicBlock_hasEvalBreak(_b)) {
		return basicBlock_hasNoLineNo(_b);
	}
	else {
		return false;
	}
}

static AlifIntT duplicateExits_withoutLineNo(AlifFlowGraph* _cfg) { // 2385
	AlifIntT nextLabel = get_maxLabel(_cfg->entryBlock) + 1;

	/* Copy all exit blocks without line number that are targets of a jump.
	 */
	AlifCFGBasicBlock* entryblock = _cfg->entryBlock;
	for (AlifCFGBasicBlock* b = entryblock; b != nullptr; b = b->next) {
		AlifCFGInstruction* last = basicBlock_lastInstr(b);
		if (last == nullptr) continue;

		if (is_jump(last)) {
			AlifCFGBasicBlock* target = next_nonEmptyBlock(last->target);
			if (isExit_orEvalCheckWithoutLineNo(target) and target->predecessors > 1) {
				AlifCFGBasicBlock* newTarget = copy_basicBlock(_cfg, target);
				if (newTarget == nullptr) {
					return ERROR;
				}
				newTarget->instr[0].loc = last->loc;
				last->target = newTarget;
				target->predecessors--;
				newTarget->predecessors = 1;
				newTarget->next = target->next;
				newTarget->label.id = nextLabel++;
				target->next = newTarget;
			}
		}
	}

	/* Any remaining reachable exit blocks without line number can only be reached by
	 * fall through, and thus can only have a single predecessor */
	for (AlifCFGBasicBlock* b = entryblock; b != nullptr; b = b->next) {
		if (BB_HAS_FALLTHROUGH(b) && b->next && b->iUsed > 0) {
			if (isExit_orEvalCheckWithoutLineNo(b->next)) {
				AlifCFGInstruction* last = basicBlock_lastInstr(b);
				b->next->instr[0].loc = last->loc;
			}
		}
	}
	return 1;
}

static void propagate_lineNumbers(AlifCFGBasicBlock* _entryBlock) {
	for (AlifCFGBasicBlock* b = _entryBlock; b != nullptr; b = b->next) {
		AlifCFGInstruction* last = basicBlock_lastInstr(b);
		if (last == nullptr) continue;

		SourceLocation prevLocation = noLocation;
		for (AlifIntT i = 0; i < b->iUsed; i++) {
			if (b->instr[i].loc.lineNo < 0) {
				b->instr[i].loc = prevLocation;
			}
			else {
				prevLocation = b->instr[i].loc;
			}
		}
		if (BB_HAS_FALLTHROUGH(b) && b->next->predecessors == 1) {
			if (b->next->iUsed > 0) {
				if (b->next->instr[0].loc.lineNo < 0) {
					b->next->instr[0].loc = prevLocation;
				}
			}
		}
		if (is_jump(last)) {
			AlifCFGBasicBlock* target = last->target;
			if (target->predecessors == 1) {
				if (target->instr[0].loc.lineNo < 0) {
					target->instr[0].loc = prevLocation;
				}
			}
		}
	}
}

static AlifIntT resolve_lineNumbers(AlifFlowGraph* _cfg, AlifIntT _firstLineNo) { // 2473
	if (duplicateExits_withoutLineNo(_cfg) == -1) return -1;
	propagate_lineNumbers(_cfg->entryBlock);
	return 1;
}

AlifIntT alifCFG_optimizeCodeUnit(AlifFlowGraph* _cfg, AlifObject* _consts,
	AlifIntT _nLocals, AlifIntT _nParams, AlifIntT _firstLineNo) { // 2481

	/* --- Preprocessing --- */

	/* --- Optimization --- */
	if (optimize_alifFlowGraph(_cfg, _consts, _firstLineNo) == -1) return -1;
	if (remove_unusedConsts(_cfg->entryBlock, _consts) == -1) return -1;


	return 1;
}




AlifIntT alifCFG_toInstructionSeq(AlifFlowGraph* _cfg, InstructionSequence* _seq) { // 2714
	AlifIntT label = 0;
	for (AlifCFGBasicBlock* b = _cfg->entryBlock; b != nullptr; b = b->next) {
		b->label = { label };
		label += 1;
	}
	for (AlifCFGBasicBlock* b = _cfg->entryBlock; b != nullptr; b = b->next) {
		if(alifInstructionSequence_useLabel(_seq, b->label.id) == -1) return -1;
		for (AlifIntT i = 0; i < b->iUsed; i++) {
			AlifCFGInstruction* instr = &b->instr[i];
			if (HAS_TARGET(instr->opCode)) {
				/* Set oparg to the label id (it will later be mapped to an offset) */
				instr->opArg = instr->target->label.id;
			}
			if(alifInstructionSequence_addOp(_seq, instr->opCode, instr->opArg, instr->loc));

			//AlifExceptHandlerInfo* hi = &seq->instrs[seq->used - 1].exceptHandlerInfo;
			//if (instr->except != nullptr) {
			//	hi->label = instr->except->label.id;
			//	hi->startDepth = instr->except->b_startdepth;
			//	hi->preserveLastI = instr->except->preserveLastI;
			//}
			//else {
			//	hi->label = -1;
			//}
		}
	}
	return 1;
}


AlifIntT alifCFG_optimizedCFGToInstructionSeq(AlifFlowGraph* _cfg, AlifCompileCodeUnitData* _cud,
	AlifIntT* _stackDepth, AlifIntT* _nLocalsPlus, InstructionSequence* _seq) { // 2749

	//*_stackDepth = calculate_stackDepth(_cfg);
	//if (*_stackDepth < 0) return -1;

	//*_nLocalsPlus = prePare_localsPlus(_cud, _cfg);
	//if (*_nLocalsPlus < 0) return -1;

	//if (convert_pseudoOps(_cfg) == -1) return -1;

	if (normalize_jumps(_cfg) == -1) return -1;

	if (alifCFG_toInstructionSeq(_cfg, _seq) < 0) {
		return -1;
	}

	return 1;
}
