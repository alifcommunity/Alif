#include "AlifParserEngine.h"


/*
	محلل لغة ألف نـ5



	دلالات الدوال في ملف المحلل اللغوي :
	nameOfFunc_rule()	: هذه الدالة تعبر عن حالة من حالات قواعد المحلل اللغوي
	alif000()			: هذه الدالة هي دالة مساعدة، غالبا ما تكون متصلة بدالة التكرار او دالة تجميع
	alif000_loop0()		: هذه الدالة تعبر عن عدم تكرار او تكرار مرة واكثر كالتالي س* ـ
	alif000_loop1()		: هذه الدالة تعبر عن تكرار مرة أو اكثر كالتالي س+ ـ
	alif000_gather()	: هذه الدالة تعبر عن الحالة "س"."ص"+ والتي تختصر الحالة ص (ص س)+ ـ
*/



#define MAXSTACK 6000


static const int nKeywordList = 7;
static KeywordToken* reservedKeywords[7] = {
	new (KeywordToken[1]) { {nullptr, -1} },  // 0 char
	new (KeywordToken[3]) { {"ك", 501}, {"و", 502}, {nullptr, -1} },  // 1 char
	new (KeywordToken[7]) { {"في", 511}, {"او", 512}, {"من", 513},  {"مع", 514}, {"صح", 515}, {"هل", 516}, {nullptr, -1} },  // 2 chars
	new (KeywordToken[9]) { {"إذا", 521}, {"ليس", 522}, {"مرر", 523}, {"عدم", 524}, {"ولد", 525},  {"صنف", 526},  {"خطا", 527},  {"عام", 528}, {nullptr, -1} },  // 3 chars
	new (KeywordToken[8]) { {"احذف", 531}, {"دالة", 532}, {"لاجل", 533},  {"والا", 534}, {"توقف", 535}, {"نطاق", 536}, {"ارجع", 537}, {nullptr, -1}},  // 4 chars
	new (KeywordToken[5]) { {"اواذا", 541}, {"بينما", 542},  {"انتظر", 543}, {"استمر", 544}, {nullptr, -1}},  // 5 chars
	new (KeywordToken[3]) { {"مزامنة", 551}, {"استورد", 552}, {nullptr, -1}}  // 6 chars
};
// ^
// |
// |
#define AS_KW 501
#define AND_KW 502
#define IN_KW 511
#define OR_KW 512
#define FROM_KW 513
#define WITH_KW 514
#define TRUE_KW 515
#define IS_KW 516
#define IF_KW 521
#define NOT_KW 522
#define PASS_KW 523
#define NONE_KW 524
#define YIELD_KW 525
#define CLASS_KW 526
#define FALSE_KW 527
#define GLOBAL_KW 528
#define DEL_KW 531
#define FUNC_KW 532
#define FOR_KW 533
#define ELSE_KW 534
#define BREAK_KW 535
#define NONLOCALE_KW 536
#define RETURN_KW 537
#define ELIF_KW 541
#define WHILE_KW 542
#define AWAIT_KW 543
#define CONTINUE_KW 544
#define ASYNC_KW 551
#define IMPORT_KW 552


// الكلمات المفتاحية السياقية: هي الكلمة المفتاحية التي يمكن إستخدامها كاسم متغير
static char* softKeywords[] = { 
	(char*)"_", (char*)"نوع", nullptr,
};


#define SIMPLE_STMTS_TYPE 1001
#define SIMPLE_STMT_TYPE 1002
#define DOTTED_NAME_TYPE 1003
#define BLOCK_TYPE 1004
#define EXPRESSIONS_TYPE 1005
#define EXPRESSION_TYPE 1006
#define STAR_EXPRESSION_TYPE 1007
#define START_EXPRESSIONS_TYPE 1008
#define DISJUCTION_TYPE 1009
#define CONJUCTION_TYPE 1010
#define INVERSION_TYPE 1011
#define COMPARISON_TYPE 1012
#define BITWISE_OR_TYPE 1013
#define BITWISE_XOR_TYPE 1014
#define BITWISE_AND_TYPE 1015
#define SHIFT_EXPR_TYPE 1016
#define SUM_TYPE 1017
#define TERM_TYPE 1018
#define FACTOR_TYPE 1019
#define POWER_TYPE 1020
#define AWAIT_PRIMARY_TYPE 1021
#define PRIMARY_TYPE 1022
#define SLICES_TYPE 1023
#define SLICE_TYPE 1024
#define ATOM_TYPE 1025
#define STRING_TYPE 1026
#define STRINGS_TYPE 1027
#define ARGUMENTS_TYPE 1028
#define STAR_TARGET_TYPE 1029
#define TARGETWITH_STARATOM_TYPE 1030
#define T_PRIMARY_TYPE 1031
#define DEL_TARGET_TYPE 1032




static ExprTy expression_rule(AlifParser*);
static ExprTy disjunction_rule(AlifParser*);
static ExprTy sum_rule(AlifParser*);
static ExprTy starExpression_rule(AlifParser*);
static ExprTy starSubExpression_rule(AlifParser*);
static ASDLExprSeq* starSubExpressions_rule(AlifParser*);
static ExprTy primary_rule(AlifParser*);
static ExprTy atom_rule(AlifParser*);
static ExprTy slices_rule(AlifParser*);
static ExprTy targetWithStarAtom_rule(AlifParser*);
static ExprTy arguments_rule(AlifParser*);
static ExprTy tPrimary_rule(AlifParser*);
static ExprTy starTarget_rule(AlifParser*);
static ExprTy alif7(AlifParser*);
static ExprTy dottedName_rule(AlifParser*);
static ASDLStmtSeq* simpleStmts_rule(AlifParser*);
static ASDLStmtSeq* statements_rule(AlifParser*);
static ASDLStmtSeq* block_rule(AlifParser*);
static ExprTy bitwiseOr_rule(AlifParser*);
static ExprTy bitwiseXOr_rule(AlifParser*);
static ExprTy shiftExpr_rule(AlifParser*);
static ExprTy term_rule(AlifParser*);
static ExprTy factor_rule(AlifParser*);
static ExprTy starExpressions_rule(AlifParser*);
static ExprTy yieldExpr_rule(AlifParser*);
static ExprTy fStringReplacementField_rule(AlifParser*);
static KeyValuePair* kvPair_rule(AlifParser*);
static void* alif23(AlifParser*);
static void* tLookahead_rule(AlifParser*);
static ASDLExprSeq* delTargets_rule(AlifParser*);
static ExprTy delTarget_rule(AlifParser*);











// del_t_atom: *اسم* > "(" del_target ")" > "(" del_targets? ")" > "[" del_targets? "]"
static ExprTy delTAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprContext_::Del);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "(" del_target ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = delTarget_rule(_p)) // del_target
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprContext_::Del);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "(" del_target? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ASDLExprSeq* a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = delTargets_rule(_p), !_p->errorIndicator) // del_targeta?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_tuple(a_, ExprContext_::Del, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "(" del_target? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ASDLExprSeq* a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(a_ = delTargets_rule(_p), !_p->errorIndicator) // del_targeta?
			and
			(literal1 = alifParserEngine_expectToken(_p, RSQR)) // "["
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_list(a_, ExprContext_::Del, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	del_target:
		> t_primary "." *اسم* !t_lookahead
		> t_primary "[" slices "]" !t_lookahead
		> del_t_atom
*/
static ExprTy delTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, DEL_TARGET_TYPE, &res_)) {
		_p->level--;
		return res_;
	}
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // t_primary "." *اسم* !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !t_lookahead
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_attribute(a_, b_->V.name.name, ExprContext_::Del, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // t_primary "[" slices "]" !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // slices
			and
			(literal_ = alifParserEngine_expectToken(_p, RSQR)) // "]"
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !t_lookahead
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_subScript(a_, b_, ExprContext_::Del, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // del_t_atom
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = delTAtom_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, DEL_TARGET_TYPE, res_);
	_p->level--;
	return res_;
}


// alif31_loop0: "," del_target
static ASDLSeq* alif31_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," del_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = delTarget_rule(_p)) // del_target
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif12_gather: del_target alif31_loop0
static ASDLSeq* alif12_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // del_target alif31_loop0
		ExprTy element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = delTarget_rule(_p)) // del_target
			and
			(seq_ = alif31_loop0(_p)) // alif31_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// del_targets: ",".del_target+ ","?
static ASDLExprSeq* delTargets_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // ",".del_target+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		ASDLExprSeq* a_{};
		if (
			(a_ = (ASDLExprSeq*)alif12_gather(_p)) // ",".del_target+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// t_lookahead: "(" > "[" > "."
static void* tLookahead_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "("
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, LPAR))) // "("
		{
			res_ = literal_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "["
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, LSQR))) // "["
		{
			res_ = literal_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "."
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, DOT))) // "."
		{
			res_ = literal_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


static ExprTy tPrimary_raw(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // t_primary "." *اسم* &t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p)
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_attribute(a_, b_->V.name.name, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // t_primary "[" slices "]" &t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // slices
			and
			(literal_1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p)
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_subScript(a_, b_, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // t_primary "(" arguments? ")" &t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(b_ = arguments_rule(_p), !_p->errorIndicator) // arguments?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p)
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_call(a_, b_ ? b_->V.call.args : nullptr, b_ ? b_->V.call.keywords : nullptr, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // atom &t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = atom_rule(_p)) // atom
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p) // &t_lookahead
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
/*
	Left-recursive
	t_primary:
		> t_primary "." *اسم* &t_lookahead
		> t_primary "[" slices "]" &t_lookahead
		> t_primary "(" arguments? ")" &t_lookahead
		> atom &t_lookahead
*/
static ExprTy tPrimary_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, T_PRIMARY_TYPE, &res_)) {
		_p->level--;
		return res_;
	}
	AlifIntT mark_ = _p->mark;
	int resMark = _p->mark;

	while (true) {
		int var = alifParserEngine_updateMemo(_p, mark_, T_PRIMARY_TYPE, res_);
		if (var) { _p->level--; return res_; }

		_p->mark = mark_;
		ExprTy raw = tPrimary_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMark)  break;

		resMark = _p->mark;
		res_ = raw;
	}

	_p->mark = resMark;
	_p->level--;

	return res_;
}


/*
	single_subscript_attribute_target:
		> t_primary "." *اسم* !t_lookahead
		> t_primary "[" slices "]" !t_lookahead
*/
static ExprTy singleSubScriptAttributeTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // t_primary "." *اسم* !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !t_lookahead
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_attribute(a_, b_->V.name.name, ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // t_primary "[" slices "]" !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // slices
			and
			(literal_ = alifParserEngine_expectToken(_p, RSQR)) // "]"
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !t_lookahead
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_subScript(a_, b_, ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// single_target: single_subscript_attribute_target > *اسم* > "(" single_target ")"
static ExprTy singleTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // single_subscript_attribute_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = singleSubScriptAttributeTarget_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprCTX::Store);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "(" single_target ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = singleTarget_rule(_p)) // single_target
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif27: !"*" star_target
static ExprTy alif27(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // !"*" star_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, STAR) // "*"
			and
			(a_ = starTarget_rule(_p)) // star_target
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// star_target: "*" (!"*" star_target) > target_with_star_atom
static ExprTy starTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, STAR_TARGET_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "*" (!"*" star_target)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = alif27(_p)) // !"*" star_target
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_star((ExprTy)alifParserEngine_setExprContext(_p, a_, ExprCTX::Store), ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // target_with_star_atom
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = targetWithStarAtom_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, STAR_TARGET_TYPE, res_);
	_p->level--;
	return res_;
}


// alif30_loop0: "," star_target
static ASDLSeq* alif30_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," star_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = starTarget_rule(_p)) // star_target
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif11_gather: star_target alif30_loop0
static ASDLSeq* alif11_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // star_target alif30_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = starTarget_rule(_p)) // star_target
			and
			(seq_ = alif30_loop0(_p)) // alif30_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// star_targets_list_seq: ",".star_target+ ","?
static ASDLExprSeq* starTargetsListSeq_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // ",".star_target+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		ASDLExprSeq* a_{};
		if (
			(a_ = (ASDLExprSeq*)alif11_gather(_p)) // ",".star_target+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif26: "," star_target
static void* alif26(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "," star_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starTarget_rule(_p)) // star_target
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif29_loop1: ("," star_target)
static ASDLSeq* alif29_loop1(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // ("," star_target)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_{};
		while (var_ = alif26(_p))
		{
			res_ = var_;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// star_targets_tuple_seq: star_target ("," star_target)+ ","? > star_target ","
static ASDLExprSeq* starTargetsTupleSeq_rule(AlifParser* _p) {
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // star_target ("," star_target)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = starTarget_rule(_p)) // star_target
			and
			(b_ = alif29_loop1(_p)) // ("," star_target)+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res_ = (ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_target ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(a_ = starTarget_rule(_p)) // star_target
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			res_ = (ASDLExprSeq*)alifParserEngine_singletonSeq(_p, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	star_atom:
		> *اسم*
		> "(" target_with_star_atom ")"
		> "(" star_targets_tuple_seq? ")"
		> "[" star_targets_list_seq? "]"
*/
static ExprTy starAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p))) // *اسم*
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprCTX::Store);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "(" target_with_star_atom ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = targetWithStarAtom_rule(_p)) // target_with_star_atom
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprCTX::Store);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "(" star_targets_tuple_seq? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ASDLExprSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = starTargetsTupleSeq_rule(_p), !_p->errorIndicator) // star_targets_tuple_seq?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_tuple(a_, ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "[" star_targets_list_seq? "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ASDLExprSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(a_ = starTargetsListSeq_rule(_p), !_p->errorIndicator) // star_targets_list_seq?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_list(a_, ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;
done:
	_p->level--;
	return res_;
}


/*
	target_with_star_atom:
		> t_primary "." *اسم* !t_lookahead
		> t_primary "[" slices "]" !t_lookahead
		> star_atom
*/
static ExprTy targetWithStarAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, TARGETWITH_STARATOM_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // t_primary "." *اسم* !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !t_lookahead
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_attribute(a_, b_->V.name.name, ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // t_primary "[" slices "]" !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // slices
			and
			(literal_1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !t_lookahead
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_subScript(a_, b_, ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_atom
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy starAtomVar{};
		if ((starAtomVar = starAtom_rule(_p))) // star_atom
		{
			res_ = starAtomVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, TARGETWITH_STARATOM_TYPE, res_);
	_p->level--;
	return res_;
}


// alif25: "," star_target
static void* alif25(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "," star_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starTarget_rule(_p)) // star_target
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif28_loop0: ("," star_target)
static ASDLSeq* alif28_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // ("," star_target)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_{};
		while ((var_ = alif25(_p)))
		{
			res_ = var_;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// star_targets: star_target !"," > star_target ("," star_target)* ","?
static ExprTy starTargets_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // star_target !","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = starTarget_rule(_p)) // star_target
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, COMMA) // !","
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_target ("," star_target)* ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = starTarget_rule(_p)) // star_target
			and
			(b_ = alif28_loop0(_p)) // ("," star_target)*
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) {
				_p->level--;
				return nullptr;
			}
			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_tuple((ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// kwarg_or_double_starred: *اسم* "=" expression > "**" expression
static KeywordOrStar* kwArgOrDoubleStar_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeywordOrStar* res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // *اسم* "=" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(literal_ = alifParserEngine_expectToken(_p, EQUAL)) // "="
			and
			(b_ = expression_rule(_p)) // expression
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_keywordOrStarred(_p, (Keyword*)alifAST_keyword(a_->V.name.name, b_, EXTRA), 1);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "**" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, DOUBLESTAR)) // "**"
			and
			(a_ = expression_rule(_p)) // expression
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_keywordOrStarred(_p, (Keyword*)alifAST_keyword(nullptr, a_, EXTRA), 1);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// kwarg_or_starred: *اسم* "=" expression > starred_expression
static KeywordOrStar* kwArgOrStar_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeywordOrStar* res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // *اسم* "=" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(literal_ = alifParserEngine_expectToken(_p, EQUAL)) // "="
			and
			(b_ = expression_rule(_p)) // expression
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_keywordOrStarred(_p, (Keyword*)alifAST_keyword(a_->V.name.name, b_, EXTRA), 1);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = expression_rule(_p))) // expression
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_keywordOrStarred(_p, alifAST_star(a_, ExprContext_::Load, EXTRA), 0);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif27_loop0: "," kwarg_or_double_star
static ASDLSeq* alif27_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," kwarg_or_double_star
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		KeywordOrStar* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = kwArgOrDoubleStar_rule(_p)) // kwarg_or_double_star
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif10_gather: kwarg_or_double_star alif27_loop0
static ASDLSeq* alif10_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // kwarg_or_double_star alif27_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeywordOrStar* element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = kwArgOrDoubleStar_rule(_p)) // kwarg_or_double_star
			and
			(seq_ = alif27_loop0(_p)) // alif27_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif26_loop0: "," kwarg_or_star
static ASDLSeq* alif26_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," kwarg_or_star
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		KeywordOrStar* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = kwArgOrStar_rule(_p)) // kwarg_or_star
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif9_gather: kwarg_or_star alif26_loop0
static ASDLSeq* alif9_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // kwarg_or_starred alif26_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeywordOrStar* element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = kwArgOrStar_rule(_p)) // kwarg_or_star
			and
			(seq_ = alif26_loop0(_p)) // alif26_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
/*
	kwargs:
		> ",".kwarg_or_starred+ "," ",".kwarg_or_double_starred+
		> ",".kwarg_or_starred+
		> ",".kwarg_or_double_starred+
*/
static ASDLSeq* kwArgs_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // ",".kwarg_or_starred+ "," ",".kwarg_or_double_starred+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ASDLSeq* a_{};
		ASDLSeq* b_{};
		if (
			(a_ = alif9_gather(_p)) // ",".kwarg_or_starred+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(b_ = alif10_gather(_p)) // ",".kwarg_or_double_starred+
			)
		{
			res_ = alifParserEngine_joinSequences(_p, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // ",".kwarg_or_starred+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLSeq* a_{};
		if ((a_ = alif9_gather(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // ",".kwarg_or_double_starred+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLSeq* a_{};
		if ((a_ = alif10_gather(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif24: "," kwargs
static void* alif24(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "," kwArgs
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ASDLSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = kwArgs_rule(_p)) // kwargs
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif25_loop0: "," (starred_expression > expression !"=")
static ASDLSeq* alif25_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," (starred_expression > expression !"=")
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		void* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = alif23(_p)) // (starred_expression > expression !"=")
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif23: starred_expression > expression !"="
static void* alif23(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // starred_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = starExpression_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // expression !"="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = expression_rule(_p)) // expression
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, NOTEQUAL) // !"="
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif8_gather: (starred_expression > expression !"=") alif25_loop0
static ASDLSeq* alif8_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // (starred_expression > expression !"=") alif25_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = alif23(_p)) // (starred_expression > expression !"=")
			and
			(seq_ = alif25_loop0(_p)) // alif25_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
/*
	args:
		> ",".(star_expression > expression !"=")+ ["," kwargs]
		> kwargs
*/
static ExprTy args_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // ",".(star_expression > expression !"=")+ ["," kwargs]
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLExprSeq* a_{};
		void* b_{};
		if (
			(a_ = (ASDLExprSeq*)alif8_gather(_p)) // ",".(star_expression > expression !"=")+
			and
			(b_ = alif24(_p), !_p->errorIndicator) // ["," kwargs]
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_collectCallSeqs(_p, a_, (ASDLSeq*)b_, EXTRA); // b_ casted to ASDLSeq*
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // kwargs
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLSeq* a_{};
		if ((a_ = kwArgs_rule(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_call(alifParserEngine_dummyName(), (ASDLExprSeq*)alifParserEngine_seqExtractStarExprs(_p, a_), (KeywordSeq*)alifParserEngine_seqDeleteStarExprs(_p, a_), EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// arguments: args ","? &")"
static ExprTy arguments_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, ARGUMENTS_TYPE, &res_)) {
		_p->level--;
		return res_;
	}
	AlifIntT mark_ = _p->mark;

	{ // args ","? &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ExprTy a_{};
		if (
			(a_ = args_rule(_p)) // args
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RPAR) // ")"
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	// error

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, ARGUMENTS_TYPE, res_);
	_p->level--;
	return res_;
}


// alif22: "if" disjuction
static void* alif22(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "if" disjuction
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "if"
			and
			(a_ = disjunction_rule(_p)) // disjuction
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif24_loop0: ("if" disjunction)
static ASDLSeq* alif24_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;
	{ // ("if" disjunction)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_{};
		while ((var_ = alif22(_p)))
		{
			res_ = var_;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq;
}
// ^
// |
// |
/*
	for_if_clause:
		> "مزامنة" "for" star_targets "in" ~ disjunction ("if" disjunction)*
		> "for" star_targets "in" ~ disjunction ("if" disjunction)*
		> "مزامنة"? "for" (bitwise_or ("," bitwise_or)* ","?) !"in"
*/
static Comprehension* forIfClause_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Comprehension* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "مزامنة" "for" star_targets "in" ~ disjunction ("if" disjunction)*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		int cutVar = 0;
		AlifPToken* keyword{};
		AlifPToken* keyword_1{};
		AlifPToken* keyword_2{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLExprSeq* c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, ASYNC_KW)) // "مزامنة"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, FOR_KW)) // "for"
			and
			(a_ = starTargets_rule(_p)) // star_targets
			and
			(keyword_2 = alifParserEngine_expectToken(_p, IN_KW)) // "in"
			and
			(cutVar = 1)
			and
			(b_ = disjunction_rule(_p)) // disjunction
			and
			(c_ = (ASDLExprSeq*)alif24_loop0(_p)) // ("if" disjunction)*
			)
		{
			//res_ = CHECK_VERSION(Comprehension*, 6, L"المزامنة الضمنية", alifAST_comprehension(a_, b_, c, 1, _p->astMem));
			res_ = alifAST_comprehension(a_, b_, c_, 1, _p->astMem);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
		if (cutVar) { _p->level--; return nullptr; }
	}
	{ // "for" star_targets "in" ~ disjunction ("if" disjunction)*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		int cutVar = 0;
		AlifPToken* keyword{};
		AlifPToken* keyword_1{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLExprSeq* c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, FOR_KW)) // "for"
			and
			(a_ = starTargets_rule(_p)) // star_targets
			and
			(keyword_1 = alifParserEngine_expectToken(_p, IN_KW)) // "in"
			and
			(cutVar = 1)
			and
			(b_ = disjunction_rule(_p)) // disjunction
			and
			(c_ = (ASDLExprSeq*)alif24_loop0(_p)) // ("if" disjunction)*
			)
		{
			//res_ = CHECK_VERSION(Comprehension*, 6, L"المزامنة الضمنية", alifAST_comprehension(a_, b_, c, 1, _p->astMem));
			res_ = alifAST_comprehension(a_, b_, c_, 1, _p->astMem);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
		if (cutVar) { _p->level--; return nullptr; }
	}
	/*
		هذا القسم لا يرجع أي قيم
		يبدو أنه لا يزال قيد التطوير
	*/
	//{ // "مزامنة"? "for" (bitwise_or ("," bitwise_or)* ","?) !"in"
	//	if (_p->errorIndicator) { _p->level--; return nullptr; }
	// 
	//	AlifPToken* keyword{};
	//	void* optVar{};
	//	void* a_{};
	//	if (
	//		(optVar = alifParserEngine_expectToken(_p, 673), !_p->errorIndicator) // "مزامنة"?
	//		and
	//		(keyword = alifParserEngine_expectToken(_p, 671)) // "for"
	//		and
	//		(a_ = alif122(_p)) // (bitwise_or ("," bitwise_or)* ","?)
	//		and
	//		(alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, 672)) // "in"
	//		)
	//	{
	//		res_ = RAISE_SYNTAX_ERROR(L"يتوقع وجود 'في' بعد متغيرات حلقة لاجل");
	//		if (res_ == nullptr
	//			/* and
	//			error occurred and stored in ThreadState->currentException */)
	//		{
	//			_p->errorIndicator = 1;
	//			_p->level--;
	//			return nullptr;
	//		}
	//		goto done;
	//	}
	//	_p->mark = mark_;
	//}


	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif23_loop1: for_if_clause
static ASDLSeq* alif23_loop1(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;
	{ // for_if_clause
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Comprehension* a_{};
		while ((a_ = forIfClause_rule(_p)))
		{
			res_ = a_;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// for_if_clauses: for_if_clause+
static CompSeq* forIfClauses_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // for_if_clause+
		CompSeq* a_{};
		if ((a_ = (CompSeq*)alif23_loop1(_p)))
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// dictcomp: "{" kvpair for_if_clauses "}"
static ExprTy dictComp_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "{" kvpair for_if_clauses "}"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		KeyValuePair* a_{};
		CompSeq* b_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LBRACE)) // "{"
			and
			(a_ = kvPair_rule(_p)) // kvpair
			and
			(b_ = forIfClauses_rule(_p)) // for_if_clauses
			and
			(literal_1 = alifParserEngine_expectToken(_p, RBRACE)) // "}"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_dictComp(a_->key_, a_->val_, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// listcomp: "[" expression for_if_clauses "]"
static ExprTy listComp_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "[" expression for_if_clauses "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		CompSeq* b_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(a_ = expression_rule(_p)) // expression
			and
			(b_ = forIfClauses_rule(_p)) // for_if_clauses
			and
			(literal_1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_listComp(a_, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// kvpair: expression ":" expression
static KeyValuePair* kvPair_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeyValuePair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // expression ":" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = expression_rule(_p)) // expression
			)
		{
			res_ = alifParserEngine_keyValuePair(_p, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// double_starred_kvpair: "**" bitwise_or > kvpair
static KeyValuePair* doubleStarKVPair_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeyValuePair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "**" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};

		if (
			(literal_ = alifParserEngine_expectToken(_p, DOUBLESTAR)) // "**"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_keyValuePair(_p, nullptr, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // kvpair
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeyValuePair* a_{};
		if ((a_ = kvPair_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif22_loop0: "," double_starred_kvpair
static ASDLSeq* alif22_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," double_starred_kvpair
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		KeyValuePair* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = doubleStarKVPair_rule(_p)) // double_starred_kvpair
			)
		{
			res_ = element_;
			if (!res_) {
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif7_gather: double_starred_kvpair alif22_loop0
static ASDLSeq* alif7_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // double_starred_kvpair alif22_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeyValuePair* element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = doubleStarKVPair_rule(_p)) // double_starred_kvpair
			and
			(seq_ = alif22_loop0(_p)) // alif22_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// double_starred_kvpairs: ",".double_starred_kvpair+ ","?
static ASDLSeq* doubleStarKVPairs_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // ",".double_starred_kvpair+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		ASDLSeq* a_{};
		if (
			(a_ = alif7_gather(_p)) // ",".double_starred_kvpair+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// dict: "{" double_starred_kvpairs? "}"
static ExprTy dict_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "{" double_starred_kvpairs? "}"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ASDLSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LBRACE)) // "{"
			and
			(a_ = doubleStarKVPairs_rule(_p), !_p->errorIndicator) // double_starred_kvpairs?
			and
			(literal_ = alifParserEngine_expectToken(_p, RBRACE)) // "}"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_dict((ASDLExprSeq*)alifParserEngine_getKeys(_p, a_), (ASDLExprSeq*)alifParserEngine_getValues(_p, a_), EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif21: star_sub_expression "," star_sub_expressions?
static ASDLExprSeq* alif21(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // star_sub_Expressions "," star_sub_Expressions?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ASDLExprSeq* b_{};
		if (
			(a_ = starSubExpression_rule(_p)) // star_sub_expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(b_ = starSubExpressions_rule(_p), !_p->errorIndicator) // star_sub_expression?
			)
		{
			res_ = (ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, (ASDLSeq*)b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// tuple: "(" [star_sub_expression "," star_sub_expressions?] ")"
static ExprTy tuple_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "(" [star_sub_expression "," star_sub_expression?] ")"
		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ASDLExprSeq* a_{}; // casted to ASDLExprSeq*
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = alif21(_p), !_p->errorIndicator) // [starSubExpression ',' starSubExpressions?] // casted to ExpreSeq*
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_tuple(a_, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// list: "[" star_sub_expressions? "]"
static ExprTy list_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "[" star_sub_expressions? "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ASDLExprSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(a_ = starSubExpressions_rule(_p), !_p->errorIndicator)
			and
			(literal_1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_list(a_, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// fstring_format_spec: FSTRING_MIDDLE > fstring_replacement_field
static ExprTy fStringFormatSpec_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // FSTRING_MIDDLE
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, FSTRINGMIDDLE)))
		{
			res_ = alifParserEngine_decodeConstantFromToken(_p, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // fstring_replacement_field
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = fStringReplacementField_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif21_loop0: fstring_format_spec
static ASDLSeq* alif21_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // fstring_format_spec
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy var_1{};
		while ((var_1 = fStringFormatSpec_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// fstring_full_format_spec: ":" fstring_format_spec*
static ResultTokenWithMetadata* fStringFullFormatSpec_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ResultTokenWithMetadata* res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // ":" fstring_format_spec*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ASDLSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(a_ = alif21_loop0(_p)) // // fstring_format_spec*
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_setupFullFormatSpec(_p, literal_, (ASDLExprSeq*)a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// fstring_conversion: "!" *اسم*
static ResultTokenWithMetadata* fStringConversion_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ResultTokenWithMetadata* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "!" *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, EXCLAMATION)) // "!"
			and
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			)
		{
			res_ = alifParserEngine_checkFStringConversion(_p, literal_, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif20: yield_expr > star_expressions
static ExprTy alif20(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // yield_expr
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = yieldExpr_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_expressions
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = starExpressions_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// fstring_replacement_field: "{" (yield_expr > star_expressions) "="? fstring_conversion? fstring_full_format_spec? "}"
static ExprTy fStringReplacementField_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "{" (yield_expr > star_expressions) "="? fstring_conversion? fstring_full_format_spec? "}"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		ResultTokenWithMetadata* b_{};
		ResultTokenWithMetadata* c_{};
		AlifPToken* d_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LBRACE)) // "{"
			and
			(a_ = alif20(_p)) // yield_expr > star_expression
			and
			(d_ = alifParserEngine_expectToken(_p, EQUAL), !_p->errorIndicator) // "="?
			and
			(c_ = fStringConversion_rule(_p), !_p->errorIndicator) // fstring_conversion?
			and
			(b_ = fStringFullFormatSpec_rule(_p), !_p->errorIndicator) // fstring_full_format_spec?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RBRACE)) // "}"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_formattedValue(_p, a_, d_, c_, b_, literal_1, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// fstring_middle: fstring_replacement_field > FSTRING_MIDDLE
static ExprTy fStringMiddle_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // fstring_replacement_field
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = fStringReplacementField_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // FSTRING_MIDDLE
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, FSTRINGMIDDLE)))
		{
			res_ = alifParserEngine_constantFromToken(_p, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif20_loop0: fstring_middle
static ASDLSeq* alif20_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // (fstring > string)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy var_1{};
		while ((var_1 = fStringMiddle_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// fstring: FSTRING_START fstring_middle* FSTRING_END
static ExprTy fString_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // FSTRING_START fstring_middle* FSTRING_END
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		ASDLSeq* b_{};
		AlifPToken* c_{};
		if (
			(a_ = alifParserEngine_expectToken(_p, FSTRINGSTART)) // "FSTRING_START"
			and
			(b_ = alif20_loop0(_p)) // fstring_middle*
			and
			(c_ = alifParserEngine_expectToken(_p, FSTRINGEND)) // "FSTRING_END"
			)
		{
			res_ = alifParserEngine_joinedStr(_p, a_, (ASDLExprSeq*)b_, c_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// string: STRING
static ExprTy string_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // STRING
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = (AlifPToken*)alifParserEngine_stringToken(_p)))
		{
			res_ = alifParserEngine_constantFromString(_p, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif19: fstring > string
static void* alif19(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // fstring
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = fString_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // string
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = string_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif19_loop1: (fstring > string)
static ASDLSeq* alif19_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // (fstring > string)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_1{};
		while ((var_1 = alif19(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// strings: (fstring > string)+
static ExprTy strings_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, STRINGS_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // (fstring > string)+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLExprSeq* a_{};
		if ((a_ = (ASDLExprSeq*)alif19_loop1(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_combineStrings(_p, a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, STRINGS_TYPE, res_);
	_p->level--;
	return res_;

}


// alif18: dict > dictcomp
static ExprTy alif18(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	{ // dict
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy dictVar{};
		if ((dictVar = dict_rule(_p)))
		{
			res_ = dictVar;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // dictcomp
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy dictCompVar{};
		if ((dictCompVar = dictComp_rule(_p)))
		{
			res_ = dictCompVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif17: list > listcomp
static ExprTy alif17(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	{ // list
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy listVar{};
		if ((listVar = list_rule(_p)))
		{
			res_ = listVar;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // listcomp
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy listCompVar{};
		if ((listCompVar = listComp_rule(_p)))
		{
			res_ = listCompVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif16 : STRING > FSTRING_START
static void* alif16(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // STRING
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* stringVar{};
		if (stringVar = alifParserEngine_stringToken(_p))
		{
			res_ = stringVar;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // FSTRING_START
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* fStringStart{};
		if ((fStringStart = alifParserEngine_expectToken(_p, FSTRINGSTART)))
		{
			res_ = fStringStart;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
/*
	atom:
		> *اسم*
		> "True"
		> "False"
		> "None"
		> &(STRING > FSTRING_START) string
		> NUMBER
		> &"(" tuble
		> &"[" (list > listcomp)
		> &"{" (dict > dictcomp)
*/
static ExprTy atom_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy nameVar{};
		if ((nameVar = alifParserEngine_nameToken(_p)))
		{
			res_ = nameVar;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "True"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, TRUE_KW)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_constant(ALIF_TRUE, nullptr, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "False"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, FALSE_KW)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_constant(ALIF_FALSE, nullptr, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "None"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, NONE_KW)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_constant(ALIF_NONE, nullptr, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // &(STRING > FSTRING_START) strings
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy stringsVar{};
		if (
			alifParserEngine_lookahead(1, alif16, _p)
			and
			(stringsVar = strings_rule(_p)) // strings
			)
		{
			res_ = stringsVar;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // NUMBER
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy numberVar{};
		if ((numberVar = alifParserEngine_numberToken(_p)))
		{
			res_ = numberVar;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // &"(" tuple
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy tupleVar{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LPAR) // "("
			and
			(tupleVar = tuple_rule(_p)) // tuple
			)
		{
			res_ = tupleVar;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // &"[" (list > listcomp)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy listVar{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LSQR) // "["
			and
			(listVar = alif17(_p)) // list > listcomp
			)
		{
			res_ = listVar;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // &"{" (dict > dictcomp)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy dictVar{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LBRACE) // "{"
			and
			(dictVar = alif18(_p)) // dict > dictcomp
			)
		{
			res_ = dictVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;

}


// alif15: ":" expression?
static ExprTy alif15(AlifParser* _p) { 
	// return type is changed to Expression

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{}; // changed to Expression
	AlifIntT mark_ = _p->mark;
	{ // ":" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{}; // changed to Expression
		if (
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(a_ = expression_rule(_p), !_p->errorIndicator) // expression?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// slice: expression? ":" expression? [":" expression?] > expression
static ExprTy slice_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{}; // changed to ExprTy
		ExprTy b_{}; // changed to ExprTy
		ExprTy c_{}; // changed to ExprTy
		if (
			(a_ = expression_rule(_p), !_p->errorIndicator) // expression?
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = expression_rule(_p), !_p->errorIndicator) // expression?
			and
			(c_ = alif15(_p), !_p->errorIndicator) // [":" expression?]
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_slice(a_, b_, c_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = expression_rule(_p)))
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif14: slice > starred_expression
static void* alif14(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // slice
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = slice_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = starExpression_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// alif18_loop0: "," (slice > starred_expression)
static ASDLSeq* alif18_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// error
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;
	{ // "," (slice > star_expression)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		void* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = alif14(_p)) // slice > star_expression
			)
		{
			res_ = element_;
			if (res_ == nullptr) {
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// error
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// error
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif6_gather: (slice > starred_expression) alif18_loop0
static ASDLSeq* alif6_gather(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // (slice > starred_expression) alif18_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = alif14(_p)) // slice > star_expression
			and
			(seq_ = alif18_loop0(_p)) // alif18_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// ^
// |
// |
// slices: slice !"," > ",".(slice > starred_expression)+ ","?
static ExprTy slices_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // slice !","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = slice_rule(_p)) // slice
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, COMMA) // ","
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // ",".(slice > starred_expression)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ASDLExprSeq* a_{};
		if (
			(a_ = (ASDLExprSeq*)alif6_gather(_p)) // ",".(slice > starred_expression)+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_tuple(a_, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;

}


static ExprTy primary_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // primary "." *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = primary_rule(_p)) // primary
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_attribute(a_, b_->V.name.name, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // primary "(" arguments? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		void* b_{};
		if (
			(a_ = primary_rule(_p)) // primary
			and
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(b_ = arguments_rule(_p), !_p->errorIndicator) // arguments?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_call(a_, b_ ? ((ExprTy)b_)->V.call.args : nullptr,
				b_ ? ((ExprTy)b_)->V.call.keywords : nullptr, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // primary "[" slices "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = primary_rule(_p)) // primary
			and
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // slices
			and
			(literal_1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_subScript(a_, b_, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // atom
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy atom{};
		if (atom = atom_rule(_p)) {
			res_ = atom;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
/*
	Left-recursive
	primary:
		> primary "." *اسم*
		> primary "(" arguments? ")"
		> primary "[" slices "]"
		> atom
*/
static ExprTy primary_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, PRIMARY_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	AlifIntT resMark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	while (true)
	{
		int var_1 = alifParserEngine_updateMemo(_p, mark_, PRIMARY_TYPE, res_);
		if (var_1) { _p->level--; return res_; }

		_p->mark = mark_;
		ExprTy raw_ = primary_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res_ = raw_;
	}

	_p->mark = resMark;
	_p->level--;

	return res_;
}


// await_primary: "await" primary > primary
static ExprTy awaitPrimary_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, AWAIT_PRIMARY_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "await" primary
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, AWAIT_KW)) // "await"
			and
			(a_ = primary_rule(_p)) // primary
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			//res_ = CHECK_VERSION(ExprTy, 5, L"تعبير إنتظر", alifAST_await(a_, EXTRA)); // تمت إضافته فقط للإستفادة منه في المستقبل في حال الحاجة
			res_ = alifAST_await(a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // primary
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy primaryVar{};
		if (primaryVar = primary_rule(_p)) {
			res_ = primaryVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, AWAIT_PRIMARY_TYPE, res_);
	_p->level--;
	return res_;
}


// sqrt: "/^" sqrt > await_primary
static ExprTy sqrt_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "/^" sqrt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, SLASHCIRCUMFLEX)) // "/^"
			and
			(a_ = sqrt_rule(_p)) // sqrt
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_unaryOp(UnaryOp_::Sqrt, a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;

	}
	{ // await_primary
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy awaitPrimary{};
		if (awaitPrimary = awaitPrimary_rule(_p)) {
			res_ = awaitPrimary;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// power: sqrt "^" factor > sqrt
static ExprTy power_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // sqrt "^" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = sqrt_rule(_p)) // sqrt
			and
			(literal_ = alifParserEngine_expectToken(_p, CIRCUMFLEX)) // "^"
			and
			(b_ = factor_rule(_p)) // factor
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::Pow, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // sqrt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy sqrtVar{};
		if (sqrtVar = sqrt_rule(_p)) {
			res_ = sqrtVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// factor: "+" factor > "-" factor > power
static ExprTy factor_rule(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, FACTOR_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "+" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, PLUS)) // "+"
			and
			(a_ = factor_rule(_p)) // factor
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_unaryOp(UnaryOp_::UAdd, a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "-" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, MINUS)) // "-"
			and
			(a_ = factor_rule(_p)) // factor
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_unaryOp(UnaryOp_::USub, a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // power
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy power{};
		if (power = power_rule(_p)) {
			res_ = power;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark_, FACTOR_TYPE, res_);
	_p->level--;
	return res_;
}


static ExprTy term_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // term "*" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = term_rule(_p)) // term
			and
			(literal_ = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(b_ = factor_rule(_p)) // factor
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::Mult, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // term "/" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = term_rule(_p)) // term
			and
			(literal_ = alifParserEngine_expectToken(_p, SLASH)) // "/"
			and
			(b_ = factor_rule(_p)) // factor
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::Div, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // term "/*" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = term_rule(_p)) // term
			and
			(literal_ = alifParserEngine_expectToken(_p, SLASHSTAR)) // "/*"
			and
			(b_ = factor_rule(_p)) // factor
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::FloorDiv, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // term "//" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = term_rule(_p)) // term
			and
			(literal_ = alifParserEngine_expectToken(_p, DOUBLESLASH)) // "//"
			and
			(b_ = factor_rule(_p)) // factor
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::Mod, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy factorVar{};
		if (factorVar = factor_rule(_p)) {
			res_ = factorVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
/*
	Left-recursive
	term:
		> term "*" factor
		> term "/" factor
		> term "/*" factor
		> term "//" factor
		> factor
*/
static ExprTy term_rule(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, TERM_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	AlifIntT resMark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	while (true) {
		int var_1 = alifParserEngine_updateMemo(_p, mark_, TERM_TYPE, res_);
		if (var_1) { _p->level--; return res_; }

		_p->mark = mark_;
		ExprTy raw_ = term_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res_ = raw_;
	}

	_p->mark = resMark;
	_p->level--;

	return res_;
}


static ExprTy sum_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // sum "+" term
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = sum_rule(_p)) // sum
			and
			(literal_ = alifParserEngine_expectToken(_p, PLUS))  // "+"
			and
			(b_ = term_rule(_p)) // term
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::Add, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // sum "-" term
		if (_p->errorIndicator) {
			_p->level--;
			return nullptr;
		}
		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = sum_rule(_p)) // sum
			and
			(literal_ = alifParserEngine_expectToken(_p, MINUS))  // "-"
			and
			(b_ = term_rule(_p)) // term
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::Sub, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // term
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy termVar{};
		if (termVar = term_rule(_p))
		{
			res_ = termVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// sum: sum "+" term > sum "-" term > term
static ExprTy sum_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, SUM_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	AlifIntT resMark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	while (true) {
		int var_1 = alifParserEngine_updateMemo(_p, mark_, SUM_TYPE, res_);
		if (var_1) { _p->level--; return res_; }

		_p->mark = mark_;
		ExprTy raw_ = sum_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res_ = raw_;
	}

	_p->mark = resMark;
	_p->level--;

	return res_;
}


static ExprTy shiftExpr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // shift_expr "<<" sum
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = shiftExpr_rule(_p)) // shift_expr
			and
			(literal_ = alifParserEngine_expectToken(_p, LSHIFT))  // "<<"
			and
			(b_ = sum_rule(_p)) // sum
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::LShift, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // shift_expr ">>" sum
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = shiftExpr_rule(_p)) // shift_expr
			and
			(literal_ = alifParserEngine_expectToken(_p, RSHIFT))  // ">>"
			and
			(b_ = sum_rule(_p)) // sum
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::RShift, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // sum
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy sumVar{};
		if (sumVar = sum_rule(_p))
		{
			res_ = sumVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// shift_expr: shift_expr "<<" sum > shift_expr ">>" sum > sum
static ExprTy shiftExpr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, SHIFT_EXPR_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	AlifIntT resMark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	while (true) {
		int var_1 = alifParserEngine_updateMemo(_p, mark_, SHIFT_EXPR_TYPE, res_);
		if (var_1) { _p->level--; return res_; }

		_p->mark = mark_;
		ExprTy raw_ = shiftExpr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res_ = raw_;
	}

	_p->mark = resMark;
	_p->level--;

	return res_;
}


static ExprTy bitwiseAnd_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // bitwise_and "&" shift_expr
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = shiftExpr_rule(_p)) // bitwise_and
			and
			(literal_ = alifParserEngine_expectToken(_p, AMPER))  // "&"
			and
			(b_ = shiftExpr_rule(_p)) // shift_expr
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::BitAnd, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // shift_expr
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy shiftExpr{};
		if (shiftExpr = shiftExpr_rule(_p))
		{
			res_ = shiftExpr;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// bitwise_and: bitwise_and "&" shift_expr > shift_expr
static ExprTy bitwiseAnd_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, BITWISE_AND_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	AlifIntT resMark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	while (true) {
		int var_1 = alifParserEngine_updateMemo(_p, mark_, BITWISE_AND_TYPE, res_);
		if (var_1) { _p->level--; return res_; }

		_p->mark = mark_;
		ExprTy raw_ = bitwiseAnd_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res_ = raw_;
	}

	_p->mark = resMark;
	_p->level--;

	return res_;
}


static ExprTy bitwiseXOr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // bitwise_xor "*|" bitwise_and
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = bitwiseXOr_rule(_p)) // bitwise_xor
			and
			(literal_ = alifParserEngine_expectToken(_p, STARVBAR))  // "*|"
			and
			(b_ = bitwiseAnd_rule(_p)) // bitwise_and
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::BitXor, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // bitwise_and
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy bitwiseAnd{};
		if (bitwiseAnd = bitwiseAnd_rule(_p))
		{
			res_ = bitwiseAnd;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// bitwise_xor: bitwise_xor "*|" bitwise_and > bitwise_and
static ExprTy bitwiseXOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, BITWISE_XOR_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	AlifIntT resMark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	while (true) {
		int var_1 = alifParserEngine_updateMemo(_p, mark_, BITWISE_XOR_TYPE, res_);
		if (var_1) { _p->level--; return res_; }

		_p->mark = mark_;
		ExprTy raw_ = bitwiseXOr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res_ = raw_;
	}

	_p->mark = resMark;
	_p->level--;

	return res_;
}


static ExprTy bitwiseOr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // bitwise_or "|" bitwise_xor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			and
			(literal_ = alifParserEngine_expectToken(_p, VBAR))  // "|"
			and
			(b_ = bitwiseXOr_rule(_p)) // bitwise_xor
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_binOp(a_, Operator_::BitOr, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // bitwise_xor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy bitwiseXOr{};
		if (bitwiseXOr = bitwiseXOr_rule(_p)) {
			res_ = bitwiseXOr;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// bitwise_or: bitwise_or "|" bitwise_xor > bitwise_xor
static ExprTy bitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, BITWISE_OR_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	AlifIntT resMark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	while (true) {
		int var_1 = alifParserEngine_updateMemo(_p, mark_, BITWISE_OR_TYPE, res_);
		if (var_1) { _p->level--; return res_; }

		_p->mark = mark_;
		ExprTy raw_ = bitwiseOr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res_ = raw_;
	}

	_p->mark = resMark;
	_p->level--;

	return res_;
}


// is_bitwise_or: "is" bitwise_or
static CompExprPair* isBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "is" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IS_KW))  // "is"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::Is, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// is_bitwise_or: "is" "not" bitwise_or
static CompExprPair* isNotBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "is" "not" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IS_KW))  // "is"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, NOT_KW))  // "not"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::IsNot, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// in_bitwise_or: "in" bitwise_or
static CompExprPair* inBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "in" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IN_KW))  // "in"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::In, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// notin_bitwise_or: "not" "in" bitwise_or
static CompExprPair* notInBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "not" "in" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, NOT_KW))  // "not"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, IN_KW))  // "in"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::NotIn, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// gt_bitwise_or: ">" bitwise_or
static CompExprPair* greaterThanBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // ">" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LESSTHAN))  // ">"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::GreaterThan, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// gte_bitwise_or: ">=" bitwise_or
static CompExprPair* greaterThanEqBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // ">=" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, GREATEREQUAL))  // ">="
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::GreaterThanEq, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// lt_bitwise_or: "<" bitwise_or
static CompExprPair* lessThanBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "<" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, GREATERTHAN))  // "<"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::LessThan, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// lte_bitwise_or: "<=" bitwise_or
static CompExprPair* lessThanEqBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "<=" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LESSEQUAL))  // "<="
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::LessThanEq, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// noteq_bitwise_or: "!=" bitwise_or
static CompExprPair* notEqBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "!=" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, NOTEQUAL))  // "!=" // from _tmp_89_rule but it's not nessesary
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::NotEq, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// eq_bitwise_or: "==" bitwise_or
static CompExprPair* eqBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "==" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, EQUALEQUAL))  // "=="
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp_::Equal, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	compare_op_bitwise_or_pair:
		> eq_bitwise_or
		> noteq_bitwise_or
		> lte_bitwise_or
		> lt_bitwise_or
		> gte_bitwise_or
		> gt_bitwise_or
		> notin_bitwise_or
		> in_bitwise_or
		> isnot_bitwise_or
		> is_bitwise_or
*/
static CompExprPair* compareOpBitwiseOrPair_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // eq_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* eqBitwise{};
		if ((eqBitwise = eqBitwiseOr_rule(_p)))
		{
			res_ = eqBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // noteq_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* notEqBitwise{};
		if ((notEqBitwise = notEqBitwiseOr_rule(_p)))
		{
			res_ = notEqBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // lte_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* lteBitwise{};
		if ((lteBitwise = lessThanEqBitwiseOr_rule(_p)))
		{
			res_ = lteBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // lt_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* ltBitwise{};
		if ((ltBitwise = lessThanBitwiseOr_rule(_p)))
		{
			res_ = ltBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // gte_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* gteBitwise{};
		if ((gteBitwise = greaterThanEqBitwiseOr_rule(_p)))
		{
			res_ = gteBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // gt_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* gtBitwise{};
		if ((gtBitwise = greaterThanBitwiseOr_rule(_p)))
		{
			res_ = gtBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // notin_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* notInBitwise{};
		if ((notInBitwise = notInBitwiseOr_rule(_p)))
		{
			res_ = notInBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // in_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* inBitwise{};
		if ((inBitwise = inBitwiseOr_rule(_p)))
		{
			res_ = inBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // isnot_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* isNotBitwise{};
		if ((isNotBitwise = isNotBitwiseOr_rule(_p)))
		{
			res_ = isNotBitwise;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // is_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* isBitwise{};
		if ((isBitwise = isBitwiseOr_rule(_p)))
		{
			res_ = isBitwise;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif17_loop1: compare_op_bitwise_or_pair
static ASDLSeq* alif17_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // compare_op_bitwise_or_pair
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* coboPair{};
		while ((coboPair = compareOpBitwiseOrPair_rule(_p)))
		{
			res_ = coboPair;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// comparison: bitwise_or compare_op_bitwise_or_pair+ > bitwise_or
static ExprTy comparison_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // bitwise_or compare_op_bitwise_or_pair+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			and
			(b_ = alif17_loop1(_p)) // bitwise_or compare_op_bitwise_or_pair+
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_compare(a_, (ASDLIntSeq*)alifParserEngine_getCmpOps(_p, b_),
				alifParserEngine_getExprs(_p, b_), EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy bitwiseOr{};
		if ((bitwiseOr = bitwiseOr_rule(_p)))
		{
			res_ = bitwiseOr;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// inversion: "not" inversion > comparison
static ExprTy inversion_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, INVERSION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "not" inversion
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, NOT_KW)) // "not"
			and
			(a_ = inversion_rule(_p)) // inversion
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_unaryOp(UnaryOp_::Not, a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // compression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy comparisonVar{};
		if (comparisonVar = comparison_rule(_p))
		{
			res_ = comparisonVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, INVERSION_TYPE, res_);
	_p->level--;
	return res_;
}


// alif13: "and" inversion
static void* alif13(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "and" inversion
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, AND_KW)) // "and"
			and
			(a_ = inversion_rule(_p)) // inversion
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// alif16_loop1: ("and" inversion)
static ASDLSeq* alif16_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // ("and" inversion)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_1{};
		while ((var_1 = alif13(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// conjunction: inversion ("and" inversion)+ > inversion
static ExprTy conjuction_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, CONJUCTION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // inversion ("and" inversion)+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = inversion_rule(_p)) // inversion
			and
			(b_ = alif16_loop1(_p)) // ("and" inversion)+
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_boolOp(BoolOp::And, (ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // inversion
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy inversionVar{};
		if (inversionVar = inversion_rule(_p))
		{
			res_ = inversionVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, CONJUCTION_TYPE, res_);
	_p->level--;
	return res_;
}


// alif12: "or" conjuction
static void* alif12(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "or" conjuction
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, OR_KW)) // "or"
			and
			(a_ = conjuction_rule(_p)) // conjuction
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// alif15_loop1: ("or" conjuction)
static ASDLSeq* alif15_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // ("or" conjuction)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_1{};
		while ((var_1 = alif12(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// disjunction: conjunction ("or" conjunction)+ > conjunction
static ExprTy disjunction_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, DISJUCTION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // conjuction ("or" conjuction)+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = conjuction_rule(_p)) // conjuction
			and
			(b_ = alif15_loop1(_p)) // ("or" conjuction)+
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_boolOp(BoolOp::Or, (ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // conjuction
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy conjuctionVar{};
		if (conjuctionVar = conjuction_rule(_p))
		{
			res_ = conjuctionVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, DISJUCTION_TYPE, res_);
	_p->level--;
	return res_;
}


/*
	expression:
		> disjunction "if" disjunction "else" expression
		> disjunction
*/
static ExprTy expression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, EXPRESSION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // disjunction "if" disjunction "else" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		ExprTy a_{};
		ExprTy b_{};
		ExprTy c_{};
		if (
			(a_ = disjunction_rule(_p)) // disjunction
			and
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "if"
			and
			(b_ = disjunction_rule(_p)) // disjunction
			and
			(keyword_1 = alifParserEngine_expectToken(_p, ELSE_KW)) // "else"
			and
			(c_ = expression_rule(_p)) // expression
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_ifExpr(b_, a_, c_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // disjunction
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy disjunctionVar{};
		if (disjunctionVar = disjunction_rule(_p)) {
			res_ = disjunctionVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, EXPRESSION_TYPE, res_);
	_p->level--;
	return res_;
}


// alif11: "," expression
static void* alif11(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "," expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};

		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = expression_rule(_p)) // expression
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// alif14_loop1: ("," expression)
static ASDLSeq* alif14_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // ("," expression)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_1{};
		while ((var_1 = alif11(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// expressions: expression ("," expression)+ ","? > expression "," > expression
static ExprTy expressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // expression("," expression)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = expression_rule(_p)) // expression
			and
			(b_ = alif14_loop1(_p)) // ("," expression)+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_tuple((ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // expression ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_tuple((ASDLExprSeq*)alifParserEngine_singletonSeq(_p, a_), ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // expression
		if (_p->errorIndicator) {
			_p->level--;
			return nullptr;
		}
		ExprTy expressionVar{};
		if ((expressionVar = expression_rule(_p))) // expression
		{
			res_ = expressionVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// star_sub_expression: "*" bitwise_or > expression
static ExprTy starSubExpression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "*" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_star(a_, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy exprVar{};
		if ((exprVar = expression_rule(_p)))
		{
			res_ = exprVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif13_loop0: "," star_sub_expression
static ASDLSeq* alif13_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," star_sub_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = starSubExpression_rule(_p)) // star_sub_expression
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// alif5_gather: star_sub_expression alif13_loop0
static ASDLSeq* alif5_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // star_sub_expression alif13_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = starSubExpression_rule(_p)) // star_sub_expression
			and
			(seq_ = alif13_loop0(_p)) // alif13_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// star_sub_expressions: ",".star_sub_expression+ ","?
static ASDLExprSeq* starSubExpressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // ",".star_sub_expression+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ASDLExprSeq* a_{};
		if (
			(a_ = (ASDLExprSeq*)alif5_gather(_p)) // ",".starSubExpression+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// star_expression: "*" bitwise_or > expression
static ExprTy starExpression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	if (alifParserEngine_isMemorized(_p, STAR_EXPRESSION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "*" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_star(a_, ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy expressionVar{};
		if (expressionVar = expression_rule(_p))
		{
			res_ = expressionVar;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, STAR_EXPRESSION_TYPE, res_);
	_p->level--;
	return res_;
}


// alif10: "," star_expression
static void* alif10(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "," star_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};

		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starExpression_rule(_p)) // star_expression
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// alif12_loop1: ("," star_expression)
static ASDLSeq* alif12_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // ("," star_expression)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_1{};
		while ((var_1 = alif10(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
/*
	star_expressions:
		> star_expression ("," star_expression)+ ","?
		> star_expression ","
		> star_expression
*/
static ExprTy starExpressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // star_expression ("," star_expression)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = starExpression_rule(_p)) // star_expression
			and
			(b_ = alif12_loop1(_p)) // ("," star_expression)+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_tuple((ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_expression ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(a_ = starExpression_rule(_p)) // star_expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_tuple((ASDLExprSeq*)alifParserEngine_singletonSeq(_p, a_), ExprContext_::Load, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy starExpression{};
		if (starExpression = starExpression_rule(_p))
		{
			res_ = starExpression;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// yield_expr: "yield" "from" expression > "yield" star_expressions?
static ExprTy yieldExpr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "yield" "from" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, YIELD_KW)) // "yield"
			and
			(keyword_ = alifParserEngine_expectToken(_p, FROM_KW)) // "from"
			and
			(a_ = expression_rule(_p)) // expression
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_yieldFrom(a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "yield" star_expressions?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, YIELD_KW)) // "yield"
			and
			(a_ = starExpressions_rule(_p), !_p->errorIndicator) // star_expressions?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_yield(a_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif9: "," > ")" > ":"
static void* alif9(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	AlifPToken* res_{};
	AlifIntT mark_ = _p->mark;

	{ // ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, COMMA)))
		{
			res_ = literal_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, RPAR)))
		{
			res_ = literal_;
			goto done;
		}
		_p->mark = mark_;
	}
	{ // ":"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, COLON)))
		{
			res_ = literal_;
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
/*
	with_item:
		> expression "as" star_target &("," > ")" > ":")
		> expression
*/
static WithItemTy withItem_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	WithItemTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // expression "as" star_target &("," > ")" > ":")
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = expression_rule(_p)) // expression
			and
			(keyword_ = alifParserEngine_expectToken(_p, AS_KW)) // "as"
			and
			(b_ = starTarget_rule(_p)) // star_target
			and
			alifParserEngine_lookahead(1, alif9, _p)
			)
		{
			res_ = alifAST_withItem(a_, b_, _p->astMem);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = expression_rule(_p)))
		{
			res_ = alifAST_withItem(a_, nullptr, _p->astMem);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif11_loop0: "," with_item
static ASDLSeq* alif11_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," with_item
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		WithItemTy element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = withItem_rule(_p)) // with_item
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				alifMem_dataFree(children_);
				_p->level--;
				return nullptr;
			}
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// alif4_gather: with_item alif11_loop0
static ASDLSeq* alif4_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // with_item alif11_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		WithItemTy element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = withItem_rule(_p)) // with_item
			and
			(seq_ = alif11_loop0(_p))
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
/*
	with_stmt:
		> "with" "(" ",".with_item+ ","? ")" ":" كتلة
		> "with" ",".with_item+ ":" كتلة
		> "مزامنة" "with" "(" ",".with_item+ ","? ")" ":" كتلة
		> "مزامنة" "with" ",".with_item+ ":" كتلة
*/
static StmtTy withStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "with" "(" ",".with_item+ ","? ")" ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		AlifPToken* literal1{};
		AlifPToken* literal2{};
		void* optVar;
		ASDLWithItemSeq* a_{};
		ASDLStmtSeq* b_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, WITH_KW)) // "with"
			and
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = (ASDLWithItemSeq*)alif4_gather(_p)) // ",".with_item+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			(literal2 = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_with(a_, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "with" ",".with_item+ ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ASDLWithItemSeq* a_{};
		ASDLStmtSeq* b_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, WITH_KW)) // "with"
			and
			(a_ = (ASDLWithItemSeq*)alif4_gather(_p)) // ",".with_item+
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_with(a_, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "مزامنة" "with" "(" ",".with_item+ ","? ")" ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* literal_{};
		AlifPToken* literal1{};
		AlifPToken* literal2{};
		void* optVar;
		ASDLWithItemSeq* a_{};
		ASDLStmtSeq* b_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW)) // "مزامنة"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, WITH_KW)) // "with"
			and
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = (ASDLWithItemSeq*)alif4_gather(_p)) // ",".with_item+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			(literal2 = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_asyncWith(a_, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "مزامنة" "with" ",".with_item+ ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* literal_{};
		ASDLWithItemSeq* a_{};
		ASDLStmtSeq* b_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW)) // "مزامنة"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, WITH_KW)) // "with"
			and
			(a_ = (ASDLWithItemSeq*)alif4_gather(_p)) // ",".with_item+
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_asyncWith(a_, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	for_stmt:
		> "for" star_targets "in" ~ star_expressions ":" كتلة
		> "مزامنة" "for" star_targets "in" ~ star_expressions ":" كتلة
*/
static StmtTy forStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "for" star_targets "in" ~ star_expressions ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLStmtSeq* c_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, FOR_KW)) // "for"
			and
			(a_ = starTargets_rule(_p)) // star_targets
			and
			(keyword_1 = alifParserEngine_expectToken(_p, IN_KW)) // "in"
			and
			(cutVar = 1)
			and
			(b_ = starExpressions_rule(_p)) // star_expressions
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(c_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_for(a_, b_, c_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
		if (cutVar) { _p->level--; return nullptr; }
	}
	{ // "مزامنة" "for" star_targets "in" ~ star_expressions ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* keyword_2{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLStmtSeq* c_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW)) // "مزامنة"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, FOR_KW)) // "for"
			and
			(a_ = starTargets_rule(_p)) // star_targets
			and
			(keyword_2 = alifParserEngine_expectToken(_p, IN_KW)) // "in"
			and
			(cutVar = 1)
			and
			(b_ = starExpressions_rule(_p)) // star_expressions
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(c_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_asyncFor(a_, b_, c_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
		if (cutVar) { _p->level--; return nullptr; }
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// while_stmt: "while" expression ":" كتلة
static StmtTy whileStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "while" expression ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, WHILE_KW)) // "while"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_while(a_, b_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// else_block: "else" &&":" كتلة
static ASDLStmtSeq* elseBlock_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLStmtSeq* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "else" &&":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ASDLStmtSeq* b_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, ELSE_KW)) // "else"
			and
			(literal_ = alifParserEngine_expectTokenForced(_p, COLON, L":")) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			res_ = b_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	elif_stmt:
		> "elif" expression ":" كتلة elif_stmt
		> "elif" expression ":" كتلة else_block?
*/
static StmtTy elifStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "elif" expression ":" كتلة elif_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		StmtTy c_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, ELIF_KW)) // "elif"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			and
			(c_ = elifStmt_rule(_p)) // elif_stmt
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			alifAST_if(a_, b_, (ASDLStmtSeq*)alifParserEngine_singletonSeq(_p, c_), EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "elif" expression ":" كتلة else_stmt?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		ASDLStmtSeq* c_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, ELIF_KW)) // "elif"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			and
			(c_ = elseBlock_rule(_p), !_p->errorIndicator) // else_block?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			alifAST_if(a_, b_, c_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	if_stmt:
		> "if" expression ":" كتلة elif_stmt
		> "if" expression ":" كتلة else_block?
*/
static StmtTy ifStmt_rule(AlifParser* _p) {  

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "if" expression ":" كتلة elif_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		StmtTy c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "if"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			and
			(c_ = elifStmt_rule(_p)) // elif_stmt
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_if(a_, b_, (ASDLStmtSeq*)alifParserEngine_singletonSeq(_p, c_), EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "if" expression ":" كتلة else_block?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		ASDLStmtSeq* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "if"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			and
			(c_ = elseBlock_rule(_p), !_p->errorIndicator) // else_block?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_if(a_, b_, c_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// default: "=" expression
static ExprTy default_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // "=" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, EQUAL)) // "="
			and
			(a_ = expression_rule(_p)) // expression
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// param: *اسم*
static ArgTy param_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ArgTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_arg(a_->V.name.name, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	param_maybe_default:
		> param default? ","
		> param default? &")"
*/
static NameDefaultPair* paramMaybeDefault_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	NameDefaultPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // param default? ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ArgTy a_{};
		ExprTy b_{};
		if (
			(a_ = param_rule(_p)) // param
			and
			(b_ = default_rule(_p), !_p->errorIndicator) // default?
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			res_ = alifParserEngine_nameDefaultPair(_p, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // param default? &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy a_{};
		ExprTy b_{};
		if (
			(a_ = param_rule(_p)) // param
			and
			(b_ = default_rule(_p), !_p->errorIndicator) // default?
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RPAR) // ")"
			)
		{
			res_ = alifParserEngine_nameDefaultPair(_p, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// param_with_default: param default "," > param default &")"
static NameDefaultPair* paramWithDefault_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	NameDefaultPair* res_{};
	AlifIntT mark_ = _p->mark;

	{ // param default ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ArgTy a_{};
		ExprTy b_{};
		if (
			(a_ = param_rule(_p)) // param
			and
			(b_ = default_rule(_p)) // default
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			res_ = alifParserEngine_nameDefaultPair(_p, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // param default &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy a_{};
		ExprTy b_{};
		if (
			(a_ = param_rule(_p)) // param
			and
			(b_ = default_rule(_p)) // default
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RPAR) // ")"
			)
		{
			res_ = alifParserEngine_nameDefaultPair(_p, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// param_no_default: param "," > param &")"
static ArgTy paramNoDefault_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ArgTy res_{};
	AlifIntT mark_ = _p->mark;

	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // param ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ArgTy a_{};
		if (
			(a_ = param_rule(_p)) // param
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_arg(a_->arg, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // param &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy a_{};
		if (
			(a_ = param_rule(_p)) // param
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RPAR) // ")"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_arg(a_->arg, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// kwds: "**" param_no_default
static ArgTy kwds_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ArgTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // "**" param_no_default
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ArgTy a_{};

		if (
			(literal_ = alifParserEngine_expectToken(_p, DOUBLESTAR)) // "**"
			and
			(a_ = paramNoDefault_rule(_p)) // param_no_default
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif10_loop1: param_maybe_default
static ASDLSeq* alif10_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // param_with_default
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		NameDefaultPair* var_1{};
		while ((var_1 = paramWithDefault_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif9_loop0: param_maybe_default
static ASDLSeq* alif9_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // param_maybe_default
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		NameDefaultPair* var_1{};
		while ((var_1 = paramMaybeDefault_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
/*
	star_etc:
		> "*" param_no_default param_maybe_default* kwds?
		> "*" "," param_maybe_default+ kwds?
		> kwds
*/
static StarEtc* starEtc_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StarEtc* res_{};
	AlifIntT mark_ = _p->mark;

	{ // "*" param_no_default param_maybe_default* kwds?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ArgTy a_{};
		ASDLSeq* b_{};
		ArgTy c_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = paramNoDefault_rule(_p)) // param_no_default
			and
			(b_ = alif9_loop0(_p)) // param_maybe_default*
			and
			(c_ = kwds_rule(_p), !_p->errorIndicator) // kwds?
			)
		{
			res_ = alifParserEngine_starEtc(_p, a_, b_, c_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // "*" param_maybe_default+ kwds?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ASDLSeq* a_{};
		ArgTy b_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(literal_1 = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = alif10_loop1(_p)) // param_maybe_default+
			and
			(b_ = kwds_rule(_p), !_p->errorIndicator) // kwds?
			)
		{
			res_ = alifParserEngine_starEtc(_p, nullptr, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // kwds
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy a_{};
		if ((a_ = kwds_rule(_p)))
		{
			res_ = alifParserEngine_starEtc(_p, nullptr, nullptr, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif8_loop1: param_with_default
static ASDLSeq* alif8_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // param_with_default
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		NameDefaultPair* var_1{};
		while ((var_1 = paramWithDefault_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif7_loop0: param_with_default
static ASDLSeq* alif7_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // param_with_default
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		NameDefaultPair* var_1{};
		while ((var_1 = paramWithDefault_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif6_loop1: param_no_default
static ASDLSeq* alif6_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // param_no_default
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy var_1{};
		while ((var_1 = paramNoDefault_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// alifErr_noMemory();
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark;
		}
		_p->mark = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq_ = (ASDLSeq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) ASDL_SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
/*
parameters:
	> param_no_default+ param_with_default* star_etc?
	> param_with_default+ star_etc?
	> star_etc
*/
static ArgumentsTy parameters_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ArgumentsTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // param_no_default+ param_with_default* star_etc?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLArgSeq* a_{};
		ASDLSeq* b_{};
		StarEtc* c_{};
		if (
			(a_ = (ASDLArgSeq*)alif6_loop1(_p)) // param_no_default+
			and
			(b_ = alif7_loop0(_p)) // param_with_default*
			and
			(c_ = starEtc_rule(_p), !_p->errorIndicator) // star_etc? 
			)
		{
			res_ = alifParserEngine_makeArguments(_p, nullptr, nullptr, a_, b_, c_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // param_with_default+ star_etc?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLSeq* a_{};
		StarEtc* b_{};
		if (
			(a_ = alif8_loop1(_p)) // param_with_default+
			and
			(b_ = starEtc_rule(_p), !_p->errorIndicator) // star_etc? 
			)
		{
			res_ = alifParserEngine_makeArguments(_p, nullptr, nullptr, nullptr, a_, b_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}
	{ // star_etc?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StarEtc* a_{};
		if ((a_ = starEtc_rule(_p), !_p->errorIndicator))
		{
			res_ = alifParserEngine_makeArguments(_p, nullptr, nullptr, nullptr, nullptr, a_);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// ألف8: "استورد" > "من"
static void* alif8(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "استورد"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, IMPORT_KW)))
		{
			res = keyword;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "من"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, FROM_KW)))
		{
			res = keyword;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// كتلة: *سطر* *مسافة_طويلة* حالات *مسافة_راجعة* > حالات_بسيطة
static ASDLStmtSeq* block_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLStmtSeq* res{};
	if (alifParserEngine_isMemorized(_p, BLOCK_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;

	{ // *سطر* *مسافة_طويلة* حالات *مسافة_راجعة*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLStmtSeq* a_{};
		AlifPToken* newline{};
		AlifPToken* indent{};
		AlifPToken* dedent{};
		if (
			(newline = alifParserEngine_expectToken(_p, NEWLINE)) // "*سطر*"
			and
			(indent = alifParserEngine_expectToken(_p, INDENT)) // "*مسافة_طويلة*"
			and
			(a_ = statements_rule(_p)) // حالات
			and
			(dedent = alifParserEngine_expectToken(_p, DEDENT)) // "*مسافة_راجعة*"
			)
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // حالات_بسيطة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLStmtSeq* a_{};
		if ((a_ = simpleStmts_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, BLOCK_TYPE, res);
	_p->level--;
	return res;
}


static ExprTy dottedName_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark{};

	{ // اسم_نقطة "." *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};

		if (
			(a_ = dottedName_rule(_p)) // اسم_نقطة
			and
			(literal = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			)
		{
			res = alifParserEngine_joinNamesWithDot(_p, a_, b_);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}

// Left-recursive
// اسم_نقطة: اسم_نقطة "." *اسم* > *اسم*
static ExprTy dottedName_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, DOTTED_NAME_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
	AlifIntT resMar = _p->mark;

	while (true) {
		AlifIntT var1 = alifParserEngine_updateMemo(_p, mark, DOTTED_NAME_TYPE, res);
		if (var1) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw = dottedName_raw(_p);
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMar) break;

		resMar = _p->mark;
		res = raw;
	}

	_p->mark = resMar;
	_p->level--;
	return res;
}


// اسماء_كـ_نقطة: اسم_نقطة ["ك" *اسم*]
static AliasTy dottedAsName_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	AliasTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // اسم_نقطة ["ك" *اسم*]
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = dottedName_rule(_p)) // اسم_نقطة
			and
			(b_ = alif7(_p), !_p->errorIndicator) // ["ك" *اسم*] // _tmp_31_rule same _tmp_28_rule!
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_alias(a_->V.name.name, b_ ? b_->V.name.name : nullptr, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// ألف5_حلقة0: "," اسم_كـ_نقطة
static ASDLSeq* alif5_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	AlifPArray children{};
	if (!children.data) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}

	{ // "," اسم_كـ_نقطة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AliasTy element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = dottedAsName_rule(_p)) // اسم_كـ_نقطة
			)
		{
			res = element_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			if (!children.push_back(res)) {
				_p->errorIndicator = 1;
				// alifErr_noMemory();
				_p->level--;
				return nullptr;
			}
			mark = _p->mark;
		}
		_p->mark = mark;
	}

	AlifUSizeT size = children.size;
	ASDLSeq* seq = (ASDLSeq*)alifNew_genericSeq(size, _p->astMem);
	if (!seq) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < size; i++) ASDL_SEQ_SETUNTYPED(seq, i, children[i]);

	_p->level--;
	return seq;
}
//	^
//	|
//	|
// ألف3_تجميع: اسم_كـ_نقطة ألف5_حلقة0
static ASDLSeq* alif3_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // اسم_كـ_نقطة ألف5_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AliasTy element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = dottedAsName_rule(_p)) // اسم_كـ_نقطة
			and
			(seq_ = alif5_loop0(_p)) // ألف5_حلقة0
			)
		{
			res = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
//	^
//	|
//	|
// اسماء_كـ_نقطة: ",".اسم_كـ_نقطة+
static ASDLAliasSeq* dottedAsNames_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLAliasSeq* res{};
	AlifIntT mark = _p->mark;

	{ // ",".اسماء_كـ_نقطة+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLAliasSeq* a_{};
		if ((a_ = (ASDLAliasSeq*)alif3_gather(_p)))
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// ألف7: "ك" *اسم*
static ExprTy alif7(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // "ك" *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, AS_KW)) // "ك"
			and
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			)
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
//	^
//	|
//	|
// استورد_كـ_اسم_من: *اسم* ["ك" *اسم*]
static AliasTy importFromAsName_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	AliasTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // *اسم*["ك" *اسم*]
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(b_ = alif7(_p), !_p->errorIndicator) // ["ك" *اسم*]
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;

			res = alifAST_alias(a_->V.name.name, b_ ? b_->V.name.name : nullptr, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// ألف4_حلقة0: "," استورد_كـ_اسم_من
static ASDLSeq* alif4_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	AlifPArray children{};
	if (!children.data) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}

	{ // "," استورد_كـ_اسم_من
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AliasTy element{};
		while (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element = importFromAsName_rule(_p)) // استورد_كـ_اسم_من
			)
		{
			res = element;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			if (!children.push_back(res)) {
				_p->errorIndicator = 1;
				// alifErr_noMemory();
				_p->level--;
				return nullptr;
			}
			mark = _p->mark;
		}
		_p->mark = mark;
	}

	AlifUSizeT size = children.size;
	ASDLSeq* seq = (ASDLSeq*)alifNew_genericSeq(size, _p->astMem);
	if (!seq) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < size; i++) ASDL_SEQ_SETUNTYPED(seq, i, children[i]);

	_p->level--;
	return seq;
}
//	^
//	|
//	|
// ألف2_تجميع: استورد_كـ_اسم_من ألف4_حلقة0
static ASDLSeq* alif2_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // استورد_كـ_اسم_من ألف4_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AliasTy element{};
		ASDLSeq* seq_{};

		if (
			(element = importFromAsName_rule(_p)) // استورد_كـ_اسم_من
			and
			(seq_ = alif4_loop0(_p)) // ألف4_حلقة0
			)
		{
			res = alifParserEngine_seqInsertInFront(_p, element, seq_);
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
//	^
//	|
//	|
// استورد_كـ_اسماء_من: ",".استورد_كـ_اسم_من+
static ASDLAliasSeq* importFromAsNames_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLAliasSeq* res{};
	AlifIntT mark = _p->mark;

	{ // ",".استورد_كـ_اسم_من+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLAliasSeq* a_{};
		if ((a_ = (ASDLAliasSeq*)alif2_gather(_p)))
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


/*
	استورد_من_أهداف:
		> "(" استورد_كـ_اسماء_من ","? ")"
		> استورد_كـ_اسماء_من !","
		> "*"
*/
static ASDLAliasSeq* importFromTargets_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLAliasSeq* res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // "(" استورد_كـ_اسماء_من ","? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		void* optVar{};
		ASDLAliasSeq* a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = importFromAsNames_rule(_p)) // استورد_كـ_اسماء_من
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // استورد_كـ_اسماء_من !","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLAliasSeq* a_{};
		if (
			(a_ = importFromAsNames_rule(_p)) // استورد_كـ_اسماء_من
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, COMMA) // ","?
			)
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "*"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, STAR))) // "*"
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;

			res = (ASDLAliasSeq*)alifParserEngine_singletonSeq(_p, (AliasTy)alifParserEngine_aliasForStar(_p, EXTRA));
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// استورد_من: "من" اسم_نقطة "استورد" استورد_من_أهداف
static StmtTy importFrom_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // "من" اسم_نقطة "استورد" استورد_من_أهداف
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		ExprTy a_{};
		ASDLAliasSeq* b_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, FROM_KW)) // "من"
			and
			(a_ = dottedName_rule(_p)) // اسم_نقطة
			and
			(keyword1 = alifParserEngine_expectToken(_p, IMPORT_KW)) // "استورد"
			and
			(b_ = importFromTargets_rule(_p)) // استورد_من_أهداف
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_importFrom(a_->V.name.name, b_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// استورد_اسم: "استورد" اسماء_كـ_نقطة
static StmtTy importName_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // "استورد" اسماء_نقطة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ASDLAliasSeq* a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, IMPORT_KW)) // "استورد"
			and
			(a_ = dottedAsNames_rule(_p)) // اسماء_كـ_نقطة
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_import(a_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// حالة_استورد: > استورد_اسم > استورد_من
static StmtTy importStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	{ // استورد_اسم
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if ((a_ = importName_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // استورد_من
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if ((a_ = importFrom_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// حالة_ولد: تعبير_ولد
static StmtTy yieldStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // تعبير_ولد
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = yieldExpr_rule(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_expr(a_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// حالة_حذف: "حذف" حذف_أهداف &(*سطر*)
static StmtTy delStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // "حذف" حذف_أهداف &(*سطر* )
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ASDLExprSeq* a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, DEL_KW)) // "حذف"
			and
			(a_ = delTargets_rule(_p)) // حذف_أهداف
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, NEWLINE) // "*سطر*" // instade of _tmp_22_rule that return ";" or "NEWLINE"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_delete(a_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// ألف3_حلقة0: "," *اسم*
static ASDLSeq* alif3_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	AlifPArray children{};
	if (!children.data) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}

	{ // "," *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = alifParserEngine_nameToken(_p)) // *اسم*
			)
		{
			res = element_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			if (!children.push_back(res)) {
				_p->errorIndicator = 1;
				// alifErr_noMemory();
				_p->level--;
				return nullptr;
			}
			mark = _p->mark;
		}
		_p->mark = mark;
	}

	AlifUSizeT size = children.size;
	ASDLSeq* seq = (ASDLSeq*)alifNew_genericSeq(size, _p->astMem);
	if (!seq) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < size; i++) ASDL_SEQ_SETUNTYPED(seq, i, children[i]);

	_p->level--;
	return seq;
}
//	^
//	|
//	|
static ASDLSeq* alif1_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // *اسم* ألف3_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		ExprTy element{};
		ASDLSeq* seq{};
		if (
			(element = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(seq = alif3_loop0(_p)) // ألف3_حلقة0
			)
		{
			res = alifParserEngine_seqInsertInFront(_p, element, seq);
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
//	^
//	|
//	|
// حالة_عام: "عام" ",".*اسم*+
static StmtTy globalStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // "عام" ",".*اسم*+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ASDLExprSeq* a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, GLOBAL_KW)) // "عام"
			and
			(a_ = (ASDLExprSeq*)alif1_gather(_p)) // ",".*اسم*+
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_global((ASDLIdentifierSeq*)alifParserEngine_mapNamesToIds(_p, a_), EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}

// حالة_نطاق: "نطاق" ",".*اسم*+
static StmtTy nonlocalStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // "نطاق" ",".*اسم*+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ASDLExprSeq* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, NONLOCALE_KW)) // "نطاق"
			and
			(a_ = (ASDLExprSeq*)alif1_gather(_p)) // ",".*اسم*+
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_nonlocal((ASDLIdentifierSeq*)alifParserEngine_mapNamesToIds(_p, a_), EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}

// حالة_ارجع: "ارجع" تعبيرات_نجمة?
static StmtTy returnStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // "ارجع" تعبيرات_نجمة?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, RETURN_KW)) // "ارجع"
			and
			(a_ = starExpressions_rule(_p), !_p->errorIndicator) // تعبيرات_نجمة?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_return(a_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}

/*
	إسناد_رجعي:
		> "+="
		> "-="
		> "*="
		> "/="
		> "^="
		> "//="
		> "/*="
		> "&="
		> "|="
		> "<<="
		> ">>="
*/
static AugOperator* augAssign_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	AugOperator* res{};
	AlifIntT mark = _p->mark;

	{ // "+="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, PLUSEQUAL))) // "+="
		{
			res = alifParserEngine_augOperator(_p, Operator_::Add);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "-="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, MINUSEQUAL))) // "-="
		{
			res = alifParserEngine_augOperator(_p, Operator_::Sub);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "*="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, STAREQUAL))) // "*="
		{
			res = alifParserEngine_augOperator(_p, Operator_::Mult);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "/="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, SLASHEQUAL))) // "/="
		{
			res = alifParserEngine_augOperator(_p, Operator_::Div);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "^="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, CIRCUMFLEXEQUAL))) // "^="
		{
			res = alifParserEngine_augOperator(_p, Operator_::Pow);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "/*="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, SLASHSTAREQUAL))) // "/*="
		{
			res = alifParserEngine_augOperator(_p, Operator_::Mod);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "//="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, DOUBLESLASHEQUAL))) // "//="
		{
			res = alifParserEngine_augOperator(_p, Operator_::FloorDiv);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "&="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, AMPEREQUAL))) // "&="
		{
			res = alifParserEngine_augOperator(_p, Operator_::BitAnd);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "|="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, VBAREQUAL))) // "|="
		{
			res = alifParserEngine_augOperator(_p, Operator_::BitOr);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "^^="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, DOUBLECIRCUMFLEXEQUAL))) // "^^="
		{
			res = alifParserEngine_augOperator(_p, Operator_::BitXor);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "<<="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, LSHIFTEQUAL))) // "<<="
		{
			res = alifParserEngine_augOperator(_p, Operator_::LShift);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // ">>="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, RSHIFTEQUAL))) // ">>="
		{
			res = alifParserEngine_augOperator(_p, Operator_::RShift);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}


// ألف6: أهداف_نجمة "="
static void* alif6(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // أهداف_نجمة "="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(a_ = starTargets_rule(_p)) // أهداف_نجمة
			and
			(literal = alifParserEngine_expectToken(_p, EQUAL)) // "="
			)
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}
//	^
//	|
//	|
// ألف2_حلقة1: (أهداف_نجمة "=")
static ASDLSeq* alif2_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	AlifPArray children{};
	if (!children.data) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}

	{ // (أهداف_نجمة "=")
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var1{};
		while ((var1 = alif6(_p))) // أهداف_نجمة "="
		{
			res = var1;

			if (!children.push_back(res)) {
				_p->errorIndicator = 1;
				// alifErr_noMemory();
				_p->level--;
				return nullptr;
			}
			mark = _p->mark;
		}
		_p->mark = mark;
	}

	AlifUSizeT size = children.size;
	if (size == 0 or _p->errorIndicator) {
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq = (ASDLSeq*)alifNew_genericSeq(size, _p->astMem);
	if (!seq) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < size; i++) ASDL_SEQ_SETUNTYPED(seq, i, children[i]);

	_p->level--;
	return seq;
}
//	^
//	|
//	|
// ألف5: تعبير_ولد > تعبيرات_نجمة
static ExprTy alif5(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // تعبير_ولد
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = yieldExpr_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // تعبيرات_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = starExpressions_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
//	^
//	|
//	|
/*
إسناد:
	> (أهداف_نجمة "=")+ (تعبير_ولد > تعبيرات_نجمة) !"="
	> هدف إسناد_رجعي ~ (تعبير_ولد > تعبيرات_نجمة)
*/
static StmtTy assignment_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // (تعبيرات_نجمة "=") + (تعبير_ولد > تعبيرات_نجمة) !"="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLExprSeq* a_{};
		ExprTy b_{};
		if (
			(a_ = (ASDLExprSeq*)alif2_loop1(_p)) // (أهداف_نجمة "=")+
			and
			(b_ = alif5(_p)) // تعبير_ولد > تعبيرات_نجمة
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, EQUAL) // !"="
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_assign(a_, b_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // هدف إسناد_رجعي ~ (تعبير_ولد > تعبيرات_نجمة)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		ExprTy a_{};
		AugOperator* b_{};
		ExprTy c_{};
		if (
			(a_ = singleTarget_rule(_p)) // هدف
			and
			(b_ = augAssign_rule(_p)) // إسناد_رجعي
			and
			(cutVar = 1)
			and
			(c_ = alif5(_p)) // تعبير_ولد > تعبير_نجمة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_augAssign(a_, b_->type, c_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}


/*
	تعريف_دالة:
		> "دالة" *اسم* "(" معاملات_? ")" ":" كتلة
		> "مزامنة" "دالة" *اسم* "(" معاملات_? ")" ":" كتلة
*/
static StmtTy functionDef_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // "دالة" *اسم* "(" معاملات_? ")" ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* literal{};
		AlifPToken* literal1{};
		AlifPToken* literal2{};
		ExprTy a_{};
		ArgumentsTy b_{};
		ASDLStmtSeq* c_{};

		if (
			(keyword = alifParserEngine_expectToken(_p, FUNC_KW)) // "دالة"
			and
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(literal = alifParserEngine_expectTokenForced(_p, LPAR, "(")) // "("
			and
			(b_ = parameters_rule(_p)) // معاملات?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			(literal2 = alifParserEngine_expectTokenForced(_p, COLON, ":")) // ":"
			and
			(c_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_functionDef(a_->V.name.name, b_ ? b_ : (ArgumentsTy)alifParserEngine_emptyArguments(_p), c_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "مزامنة" "دالة" *اسم* "(" معاملات_؟ ")" ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		AlifPToken* literal_2{};
		ExprTy a_{};
		ArgumentsTy b_{};
		ASDLStmtSeq* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, CLASS_KW)) // "مزامنة"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, FUNC_KW)) // "دالة"
			and
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(literal_ = alifParserEngine_expectTokenForced(_p, LPAR, "(")) // "("
			and
			(b_ = parameters_rule(_p)) // معاملات؟
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			(literal_2 = alifParserEngine_expectTokenForced(_p, COLON, ":")) // ":"
			and
			(c_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_asyncFunctionDef(a_->V.name.name, b_ ? b_ : (ArgumentsTy)alifParserEngine_emptyArguments(_p), c_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}


// ألف4: "(" وسيطات? ")"
static ExprTy alif4(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // "(" وسيطات? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = arguments_rule(_p), !_p->errorIndicator) // وسيطات?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}
//	^
//	|
//	|
/*
	تعريف_صنف:
		> "صنف" *اسم* ["(" وسيطات? ")"] ":" كتلة
*/
static StmtTy classDef_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res_{};
	AlifIntT mark_ = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "صنف" *اسم* ["(" وسيطات? ")"] ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLStmtSeq* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, CLASS_KW)) // "صنف"
			and
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(b_ = alif4(_p), !_p->errorIndicator) // ["(" وسيطات? ")"]
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(c_ = block_rule(_p), !_p->errorIndicator) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_classDef(a_->V.name.name, b_ ? b_->V.call.args : nullptr, b_ ? b_->V.call.keywords : nullptr, c_, EXTRA);
			if (res_ == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	حالة_بسيطة:
		> إسناد
		> تعبيرات_نجمة
		> &"ارجع" حالة_ارجع
		> &("استورد" > "من") حالة_استورد
		> "مرر"
		> &"حذف" حالة_حذف
		> &"ولد" حالة_ولد
		> "توقف"
		> "استمر"
		> &"عام" حالة_عام
		> &"نطاق" حالة_نطاق
*/
static StmtTy simpleStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	if (alifParserEngine_isMemorized(_p, SIMPLE_STMT_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
	if (_p->mark == _p->fill
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark]->colOffset;

	{ // إسناد
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy assignmentVar{};
		if ((assignmentVar = assignment_rule(_p)))
		{
			res = assignmentVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // تعبيرات_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = starExpressions_rule(_p)))
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_expr(a_, EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"ارجع" حالة_ارجع
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RETURN_KW) // "return"
			and
			(a_ = returnStmt_rule(_p)) // حالة_ارجع
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &("استورد" > "من") حالة_استورد
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookahead(1, alif8, _p)
			and
			(a_ = importStmt_rule(_p)) // حالة_استورد
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "مرر"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, PASS_KW))) // "مرر"
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_pass(EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"حذف" حالة_حذف
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, DEL_KW) // "del"
			and
			(a_ = delStmt_rule(_p)) // حالة_حذف
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"ولد" حالة_ولد
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, YIELD_KW) // "yield"
			and
			(a_ = yieldStmt_rule(_p)) // حالة_ولد
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "توقف"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, BREAK_KW))) // "توقف"
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_break(EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // "استمر"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, CONTINUE_KW))) // "استمر"
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_continue(EXTRA);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"عام" حالة_عام
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, GLOBAL_KW) // "global"
			and
			(a_ = globalStmt_rule(_p)) // حالة_عام
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"نطاق" حالة_نطاق
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, NONLOCALE_KW) // "nonlocal"
			and
			(a_ = nonlocalStmt_rule(_p)) // حالة_نطاق
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark, SIMPLE_STMT_TYPE, res);
	_p->level--;
	return res;
}


// حالات_بسيطة: حالة_بسيطة *سطر*
static ASDLStmtSeq* simpleStmts_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLStmtSeq* res{};
	AlifIntT mark = _p->mark;

	{ // حالة_بسيطة *سطر*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		AlifPToken* newline{};
		if (
			(a_ = simpleStmt_rule(_p)) // حالة_بسيطة
			and
			(newline = alifParserEngine_expectToken(_p, NEWLINE)) // *سطر*
			)
		{
			res = (ASDLStmtSeq*)alifParserEngine_singletonSeq(_p, a_);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}


// ألف3: "دالة" > "مزامنة"
static void* alif3(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "دالة"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, FUNC_KW))) // "دالة"
		{
			res = keyword;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "مزامنة"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, ASYNC_KW))) // "مزامنة"
		{
			res = keyword;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}

// ألف2: "عند" > "مزامنة"
static void* alif2(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "عند"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, WITH_KW))) // "عند"
		{
			res = keyword;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "مزامنة"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, ASYNC_KW))) // "مزامنة"
		{
			res = keyword;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}

// ألف1: "لاجل" > "مزامنة"
static void* alif1(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "لاجل"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, FOR_KW))) // "لاجل"
		{
			res = keyword;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "مزامنة"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW))) // "مزامنة"
		{
			res = keyword_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}


/*
	حالة_مركبة:
		> &("دالة" > "مزامنة") تعريف_دالة
		> &"اذا" حالة_اذا
		> &"صنف" تعريف_صنف
		> &("عند" > "مزامنة") حالة_عند
		> &("لاجل" > "مزامنة") حالة_لاجل
		> &'بينما' حالة_بينما
*/
static StmtTy compoundStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtTy res{};
	AlifIntT mark = _p->mark;

	{ // &("دالة" > "مزامنة") تعريف_دالة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookahead(1, alif3, _p)
			and
			(a_ = functionDef_rule(_p)) // تعريف_دالة
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"اذا" حالة_اذا
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, IF_KW) // "if"
			and
			(a_ = ifStmt_rule(_p)) // حالة_اذا
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"صنف" تعريف_صنف
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, CLASS_KW) // "class"
			and
			(a_ = classDef_rule(_p)) // تعريف_صنف
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &("عند" > "مزامنة") حالة_عند
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookahead(1, alif2, _p)
			and
			(a_ = withStmt_rule(_p)) // حالة_عند
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &("لاجل" > "مزامنة") حالة_لاجل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookahead(1, alif1, _p)
			and
			(a_ = forStmt_rule(_p)) // حالة_لاجل
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"بينما" حالة_بينما
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, WHILE_KW) // "while"
			and
			(a_ = whileStmt_rule(_p)) // حالة_بينما
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}


// حالة: حالة_مركبة > حالة_بسيطة
static ASDLStmtSeq* statement_rule(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLStmtSeq* res{};
	AlifIntT mark = _p->mark;

	{ // حالة_مركبة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtTy a_{};
		if ((a_ = compoundStmt_rule(_p)))
		{
			res = (ASDLStmtSeq*)alifParserEngine_singletonSeq(_p, a_);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}
	{ // حالة_بسيطة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLStmtSeq* a_{};
		if ((a_ = (ASDLStmtSeq*)simpleStmts_rule(_p)))
		{
			res = a_;
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;

done:
	_p->level--;
	return res;
}


// ألف1_حلقة1: حالة
static ASDLSeq* alif1_loop1(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;
	AlifPArray children{};
	if (!children.data) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}

	{ // حالة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLStmtSeq* statementVar{};
		while ((statementVar = statement_rule(_p))) // حالة
		{
			res = statementVar;

			if (!children.push_back(res)) {
				_p->errorIndicator = 1;
				// alifErr_noMemory();
				_p->level--;
				return nullptr;
			}
			mark = _p->mark;
		}
		_p->mark = mark;
	}

	AlifUSizeT size = children.size;
	if (size == 0 or _p->errorIndicator) {
		_p->level--;
		return nullptr;
	}
	ASDLSeq* seq = (ASDLSeq*)alifNew_genericSeq(size, _p->astMem);
	if (!seq) {
		_p->errorIndicator = 1;
		// alifErr_noMemory();
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < size; i++) ASDL_SEQ_SETUNTYPED(seq, i, children[i]);

	_p->level--;
	return seq;
}
//	^
//	|
//	|
// حالات: حالة+ـ
static ASDLStmtSeq* statements_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLStmtSeq* res{};
	AlifIntT mark = _p->mark;
	{ // حالات: حالة+ـ
		ASDLSeq* a_{};
		if (a_ = alif1_loop1(_p)) // حالة+ـ
		{
			res = (ASDLStmtSeq*)alifParserEngine_seqFlatten(_p, a_);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}

// طرفية: حالة_سطر
static ModuleTy interactiveRun_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ModuleTy res = nullptr;
	AlifIntT mark = _p->mark;
	{ // طرفية: حالة_سطر
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLStmtSeq* a_{};
		//if (
		//	(a_ = statementNewline_rule(_p))  // طرفية: حالة_سطر
		//	)
		//{
		//	res_ = alifAST_interactive(a_, _p->astMem);
		//	if (res_ == nullptr
		//		/*and alifErr_occurred()*/)
		//	{
		//		_p->errorIndicator = 1;
		//		_p->level--;
		//		return nullptr;
		//	}
		//	goto done;
		//}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}

static ModuleTy evalRun_rule(AlifParser* _p) {
	return nullptr;//
}
static ModuleTy funcRun_rule(AlifParser* _p) {
	return nullptr;//
}


// ملف: حالات؟ $ـ
static ModuleTy fileRun_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ModuleTy res{};
	AlifIntT mark = _p->mark;
	{ // ملف: حالات؟ $ـ
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLStmtSeq* a_{};
		AlifPToken* endMarkerVar{};
		if (
			(a_ = statements_rule(_p), !_p->errorIndicator)  // حالات؟
			and
			(endMarkerVar = alifParserEngine_expectToken(_p, ENDMARKER)) // ENDMARKER
			)
		{
			res = alifParserEngine_makeModule(_p, a_);
			if (res == nullptr
				/*and alifErr_occurred()*/)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


void* alifParserEngine_parse(AlifParser* _p) { 
	// تهيئة الكلمات المفتاحية
	_p->keywords = reservedKeywords;
	_p->nKeywordList = nKeywordList;
	_p->softKeyword = softKeywords;

	// بدأ تحليل النص
	void* result{};

	if (_p->startRule == ALIF_FILE_INPUT) {
		result = fileRun_rule(_p);
	}
	else if (_p->startRule == ALIF_SINGLE_INPUT) {
		result = interactiveRun_rule(_p);
	}
	else if (_p->startRule == ALIF_EVAL_INPUT) {
		result = evalRun_rule(_p);
	}
	else if (_p->startRule == ALIF_FUNC_TYPE_INPUT) {
		result = funcRun_rule(_p);
	}

	return result;
}
