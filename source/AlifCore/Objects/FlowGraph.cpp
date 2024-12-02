#include "alif.h"

#include "AlifCore_FlowGraph.h"
#include "AlifCore_Compile.h"



// 15
#define SUCCESS 0
#define ERROR -1


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











void _alifCfgBuilder_free(CFGBuilder* _g) { // 431
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
}
