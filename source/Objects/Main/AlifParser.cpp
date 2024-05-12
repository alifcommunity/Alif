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
	new (KeywordToken[3]) { {L"ك", 501}, {L"و", 502}, {nullptr, -1} },  // 1 char
	new (KeywordToken[7]) { {L"في", 511}, {L"او", 512}, {L"من", 513},  {L"مع", 514}, {L"صح", 515}, {L"هل", 516}, {nullptr, -1} },  // 2 chars
	new (KeywordToken[9]) { {L"إذا", 521}, {L"ليس", 522}, {L"مرر", 523}, {L"عدم", 524}, {L"ولد", 525},  {L"صنف", 526},  {L"خطا", 527},  {L"عام", 528}, {nullptr, -1} },  // 3 chars
	new (KeywordToken[8]) { {L"احذف", 531}, {L"دالة", 532}, {L"لاجل", 533},  {L"والا", 534}, {L"توقف", 535}, {L"نطاق", 536}, {L"ارجع", 537}, {nullptr, -1}},  // 4 chars
	new (KeywordToken[5]) { {L"اواذا", 541}, {L"بينما", 542},  {L"انتظر", 543}, {L"استمر", 544}, {nullptr, -1}},  // 5 chars
	new (KeywordToken[3]) { {L"مزامنة", 551}, {L"استورد", 552}, {nullptr, -1}}  // 6 chars
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
static wchar_t* softKeywords[] = { 
	(wchar_t*)L"_", (wchar_t*)L"نوع", nullptr,
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



int alifParserEngine_fillToken(AlifParser*);
Expression* alifParserEngine_nameToken(AlifParser*);
AlifObject* alifParserEngine_newIdentifier(AlifParser*, const wchar_t*);
AlifPToken* alifParserEngine_expectToken(AlifParser*, int);
AlifPToken* alifParserEngine_getLastNonWhitespaceToken(AlifParser*);
Expression* alifParserEngine_numberToken(AlifParser*);
int alifParserEngine_insertMemo(AlifParser*, int, int, void*);
void* alifParserEngine_stringToken(AlifParser*);
int alifParserEngine_isMemorized(AlifParser*, int, void*);
int alifParserEngine_lookahead(int, void* (_func)(AlifParser*), AlifParser*);
int alifParserEngine_updateMemo(AlifParser*, int, int, void*);
int alifParserEngine_lookaheadWithInt(int, AlifPToken* (_func)(AlifParser*, int), AlifParser*, int);
AlifPToken* alifParserEngine_expectTokenForced(AlifParser*, int, const wchar_t*);

static Expression* expression_rule(AlifParser*);
static Expression* disjunction_rule(AlifParser*);
static Expression* sum_rule(AlifParser*);
static Expression* starExpression_rule(AlifParser*);
static Expression* starSubExpression_rule(AlifParser*);
static ExprSeq* starSubExpressions_rule(AlifParser*);
static Expression* primary_rule(AlifParser*);
static Expression* atom_rule(AlifParser*);
static Expression* slices_rule(AlifParser*);
static Expression* targetWithStarAtom_rule(AlifParser*);
static Expression* arguments_rule(AlifParser*);
static Expression* tPrimary_rule(AlifParser*);
static Expression* starTarget_rule(AlifParser*);
static Expression* alif7(AlifParser*);
static Expression* dottedName_rule(AlifParser*);
static StmtSeq* simpleStmts_rule(AlifParser*);
static StmtSeq* statements_rule(AlifParser*);
static StmtSeq* block_rule(AlifParser*);
static Expression* bitwiseOr_rule(AlifParser*);
static Expression* bitwiseXOr_rule(AlifParser*);
static Expression* shiftExpr_rule(AlifParser*);
static Expression* term_rule(AlifParser*);
static Expression* factor_rule(AlifParser*);
static Expression* starExpressions_rule(AlifParser*);
static Expression* yieldExpr_rule(AlifParser*);
static Expression* fStringReplacementField_rule(AlifParser*);
static KeyValuePair* kvPair_rule(AlifParser*);
static void* alif23(AlifParser*);
static void* tLookahead_rule(AlifParser*);
static ExprSeq* delTargets_rule(AlifParser*);
static Expression* delTarget_rule(AlifParser*);







//////////////////////////////////////////////////// مؤقت
/*
	هذه الدالة مؤقتة
	حيث تقوم بإرجاع اسم فارغ
	يتم إستخدامه مؤقتاً حتى يتوفر الأسم الحقيقي
	--> مثال: تستخدم مع استدعاء الدوال
*/
AlifObject* theName = new AlifObject{};

Expression* dummyName = new Expression{};
#define INIT_DUMMY_NAME { \
	dummyName->type = NameK; \
	dummyName->V.name.name = theName; \
	dummyName->V.name.ctx = Load; \
	dummyName->lineNo = 1; \
	dummyName->colOffset = 0; \
	dummyName->endLineNo = 1; \
	dummyName->endColOffset = 0; \
} \

static Expression* alifParserEngine_dummyName() {
	INIT_DUMMY_NAME;
	return dummyName;
	//return &_alifRuntime_.parser.dummyName;
}
//////////////////////////////////////////////////// مؤقت











// del_t_atom: NAME > "(" del_target ")" > "(" del_targets? ")" > "[" del_targets? "]"
static Expression* delTAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprCTX::Del);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "(" del_target ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = delTarget_rule(_p)) // del_target
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprCTX::Del);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "(" del_target? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = delTargets_rule(_p), !_p->errorIndicator) // del_targeta?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_tuple(a_, ExprCTX::Del, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "(" del_target? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(a_ = delTargets_rule(_p), !_p->errorIndicator) // del_targeta?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RSQR)) // "["
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_list(a_, ExprCTX::Del, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	del_target:
		> t_primary "." NAME !t_lookahead
		> t_primary "[" slices "]" !t_lookahead
		> del_t_atom
*/
static Expression* delTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, DEL_TARGET_TYPE, &res_)) {
		_p->level--;
		return res_;
	}
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // t_primary "." NAME !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // NAME
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !t_lookahead
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_attribute(a_, b_->V.name.name, ExprCTX::Del, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // t_primary "[" slices "]" !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_subScript(a_, b_, ExprCTX::Del, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // del_t_atom
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = delTAtom_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, DEL_TARGET_TYPE, res_);
	_p->level--;
	return res_;
}


// alif31_loop0: "," del_target
static Seq* alif31_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," del_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = delTarget_rule(_p)) // del_target
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif12_gather: del_target alif31_loop0
static Seq* alif12_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // del_target alif31_loop0
		Expression* element_{};
		Seq* seq_{};
		if (
			(element_ = delTarget_rule(_p)) // del_target
			and
			(seq_ = alif31_loop0(_p)) // alif31_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
static ExprSeq* delTargets_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // ",".del_target+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		ExprSeq* a_{};
		if (
			(a_ = (ExprSeq*)alif12_gather(_p)) // ",".del_target+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "("
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, LPAR))) // "("
		{
			res_ = literal_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "["
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, LSQR))) // "["
		{
			res_ = literal_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "."
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, DOT))) // "."
		{
			res_ = literal_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


static Expression* tPrimary_raw(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // t_primary "." NAME &t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // NAME
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p)
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_attribute(a_, b_->V.name.name, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // t_primary "[" slices "]" &t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_subScript(a_, b_, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // t_primary "(" arguments? ")" &t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // atom &t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if (
			(a_ = atom_rule(_p)) // atom
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p) // &t_lookahead
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
/*
	Left-recursive
	t_primary:
		> t_primary "." NAME &t_lookahead
		> t_primary "[" slices "]" &t_lookahead
		> t_primary "(" arguments? ")" &t_lookahead
		> atom &t_lookahead
*/
static Expression* tPrimary_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, T_PRIMARY_TYPE, &res_)) {
		_p->level--;
		return res_;
	}
	AlifIntT mark_ = _p->mark_;
	int resMark = _p->mark_;

	while (true) {
		int var = alifParserEngine_updateMemo(_p, mark_, T_PRIMARY_TYPE, res_);
		if (var) { _p->level--; return res_; }

		_p->mark_ = mark_;
		Expression* raw = tPrimary_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark_ <= resMark)  break;

		resMark = _p->mark_;
		res_ = raw;
	}

	_p->mark_ = resMark;
	_p->level--;

	return res_;
}


/*
	single_subscript_attribute_target:
		> t_primary "." NAME !t_lookahead
		> t_primary "[" slices "]" !t_lookahead
*/
static Expression* singleSubScriptAttributeTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // t_primary "." NAME !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // NAME
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // t_primary "[" slices "]" !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// single_target: single_subscript_attribute_target > NAME > "(" single_target ")"
static Expression* singleTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // single_subscript_attribute_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = singleSubScriptAttributeTarget_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprCTX::Store);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "(" single_target ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif27: !"*" star_target
static Expression* alif27(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // !"*" star_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if (
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, STAR) // "*"
			and
			(a_ = starTarget_rule(_p)) // star_target
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* starTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, STAR_TARGET_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
			res_ = alifAST_star((Expression*)alifParserEngine_setExprContext(_p, a_, ExprCTX::Store), ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // target_with_star_atom
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = targetWithStarAtom_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, STAR_TARGET_TYPE, res_);
	_p->level--;
	return res_;
}


// alif30_loop0: "," star_target
static Seq* alif30_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," star_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = starTarget_rule(_p)) // star_target
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif11_gather: star_target alif30_loop0
static Seq* alif11_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // star_target alif30_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* element_{};
		Seq* seq_{};
		if (
			(element_ = starTarget_rule(_p)) // star_target
			and
			(seq_ = alif30_loop0(_p)) // alif30_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
static ExprSeq* starTargetsListSeq_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // ",".star_target+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		ExprSeq* a_{};
		if (
			(a_ = (ExprSeq*)alif11_gather(_p)) // ",".star_target+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "," star_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starTarget_rule(_p)) // star_target
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif29_loop1(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// star_targets_tuple_seq: star_target ("," star_target)+ ","? > star_target ","
static ExprSeq* starTargetsTupleSeq_rule(AlifParser* _p) {
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // star_target ("," star_target)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		Expression* a_{};
		Seq* b_{};
		if (
			(a_ = starTarget_rule(_p)) // star_target
			and
			(b_ = alif29_loop1(_p)) // ("," star_target)+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res_ = (ExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_target ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(a_ = starTarget_rule(_p)) // star_target
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			res_ = (ExprSeq*)alifParserEngine_singletonSeq(_p, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	star_atom:
		> NAME
		> "(" target_with_star_atom ")"
		> "(" star_targets_tuple_seq? ")"
		> "[" star_targets_list_seq? "]"
*/
static Expression* starAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = alifParserEngine_nameToken(_p))) // NAME
		{
			res_ = alifParserEngine_setExprContext(_p, a_, ExprCTX::Store);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "(" target_with_star_atom ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "(" star_targets_tuple_seq? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprSeq* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "[" star_targets_list_seq? "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		ExprSeq* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;
done:
	_p->level--;
	return res_;
}


/*
	target_with_star_atom:
		> t_primary "." NAME !t_lookahead
		> t_primary "[" slices "]" !t_lookahead
		> star_atom
*/
static Expression* targetWithStarAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, TARGETWITH_STARATOM_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // t_primary "." NAME !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = tPrimary_rule(_p)) // t_primary
			and
			(alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // NAME
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // t_primary "[" slices "]" !t_lookahead
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_atom
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* starAtomVar{};
		if ((starAtomVar = starAtom_rule(_p))) // star_atom
		{
			res_ = starAtomVar;
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "," star_target
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starTarget_rule(_p)) // star_target
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif28_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// star_targets: star_target !"," > star_target ("," star_target)* ","?
static Expression* starTargets_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		Expression* a_{};
		if (
			(a_ = starTarget_rule(_p)) // star_target
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, COMMA) // ","
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_target ("," star_target)* ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		Expression* a_{};
		Seq* b_{};
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
			res_ = alifAST_tuple((ExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), ExprCTX::Store, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// kwarg_or_double_starred: NAME "=" expression > "**" expression
static KeywordOrStar* kwArgOrDoubleStar_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeywordOrStar* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // NAME "=" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = alifParserEngine_nameToken(_p)) // NAME
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "**" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// kwarg_or_starred: NAME "=" expression > starred_expression
static KeywordOrStar* kwArgOrStar_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeywordOrStar* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // NAME "=" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = alifParserEngine_nameToken(_p)) // NAME
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = expression_rule(_p))) // expression
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_keywordOrStarred(_p, alifAST_star(a_, ExprCTX::Load, EXTRA), 0);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif27_loop0: "," kwarg_or_double_star
static Seq* alif27_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif10_gather: kwarg_or_double_star alif27_loop0
static Seq* alif10_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // kwarg_or_double_star alif27_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeywordOrStar* element_{};
		Seq* seq_{};
		if (
			(element_ = kwArgOrDoubleStar_rule(_p)) // kwarg_or_double_star
			and
			(seq_ = alif27_loop0(_p)) // alif27_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif26_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif9_gather: kwarg_or_star alif26_loop0
static Seq* alif9_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // kwarg_or_starred alif26_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeywordOrStar* element_{};
		Seq* seq_{};
		if (
			(element_ = kwArgOrStar_rule(_p)) // kwarg_or_star
			and
			(seq_ = alif26_loop0(_p)) // alif26_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* kwArgs_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // ",".kwarg_or_starred+ "," ",".kwarg_or_double_starred+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Seq* a_{};
		Seq* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // ",".kwarg_or_starred+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Seq* a_{};
		if ((a_ = alif9_gather(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // ",".kwarg_or_double_starred+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Seq* a_{};
		if ((a_ = alif10_gather(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "," kwArgs
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Seq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = kwArgs_rule(_p)) // kwargs
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif25_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

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
	AlifIntT mark_ = _p->mark_;

	{ // starred_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = starExpression_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // expression !"="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if (
			(a_ = expression_rule(_p)) // expression
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, NOTEQUAL) // !"="
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif8_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // (starred_expression > expression !"=") alif25_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* element_{};
		Seq* seq_{};
		if (
			(element_ = alif23(_p)) // (starred_expression > expression !"=")
			and
			(seq_ = alif25_loop0(_p)) // alif25_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* args_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		ExprSeq* a_{};
		void* b_{};
		if (
			(a_ = (ExprSeq*)alif8_gather(_p)) // ",".(star_expression > expression !"=")+
			and
			(b_ = alif24(_p), !_p->errorIndicator) // ["," kwargs]
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_collectCallSeqs(_p, a_, (Seq*)b_, EXTRA); // b_ casted to Seq*
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // kwargs
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Seq* a_{};
		if ((a_ = kwArgs_rule(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_call(alifParserEngine_dummyName(), (ExprSeq*)alifParserEngine_seqExtractStarExprs(_p, a_), (KeywordSeq*)alifParserEngine_seqDeleteStarExprs(_p, a_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// arguments: args ","? &")"
static Expression* arguments_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, ARGUMENTS_TYPE, &res_)) {
		_p->level--;
		return res_;
	}
	AlifIntT mark_ = _p->mark_;

	{ // args ","? &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		Expression* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "if" disjuction
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "if"
			and
			(a_ = disjunction_rule(_p)) // disjuction
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif24_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq;
}
// ^
// |
// |
/*
	for_if_clause:
		> "async" "for" star_targets "in" ~ disjunction ("if" disjunction)*
		> "for" star_targets "in" ~ disjunction ("if" disjunction)*
		> "async"? "for" (bitwise_or ("," bitwise_or)* ","?) !"in"
*/
static Comprehension* forIfClause_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Comprehension* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "async" "for" star_targets "in" ~ disjunction ("if" disjunction)*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		int cutVar = 0;
		AlifPToken* keyword{};
		AlifPToken* keyword_1{};
		AlifPToken* keyword_2{};
		Expression* a_{};
		Expression* b_{};
		ExprSeq* c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, ASYNC_KW)) // "async"
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
			(c_ = (ExprSeq*)alif24_loop0(_p)) // ("if" disjunction)*
			)
		{
			//res_ = CHECK_VERSION(Comprehension*, 6, L"المزامنة الضمنية", alifAST_comprehension(a_, b_, c, 1, _p->astMem));
			res_ = alifAST_comprehension(a_, b_, c_, 1, _p->astMem);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
		if (cutVar) { _p->level--; return nullptr; }
	}
	{ // "for" star_targets "in" ~ disjunction ("if" disjunction)*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		int cutVar = 0;
		AlifPToken* keyword{};
		AlifPToken* keyword_1{};
		Expression* a_{};
		Expression* b_{};
		ExprSeq* c_{};
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
			(c_ = (ExprSeq*)alif24_loop0(_p)) // ("if" disjunction)*
			)
		{
			//res_ = CHECK_VERSION(Comprehension*, 6, L"المزامنة الضمنية", alifAST_comprehension(a_, b_, c, 1, _p->astMem));
			res_ = alifAST_comprehension(a_, b_, c_, 1, _p->astMem);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
		if (cutVar) { _p->level--; return nullptr; }
	}
	/*
		هذا القسم لا يرجع أي قيم
		يبدو أنه لا يزال قيد التطوير
	*/
	//{ // "async"? "for" (bitwise_or ("," bitwise_or)* ","?) !"in"
	//	if (_p->errorIndicator) { _p->level--; return nullptr; }
	// 
	//	AlifPToken* keyword{};
	//	void* optVar{};
	//	void* a_{};
	//	if (
	//		(optVar = alifParserEngine_expectToken(_p, 673), !_p->errorIndicator) // "async"?
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
	//	_p->mark_ = mark_;
	//}


	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif23_loop1: for_if_clause
static Seq* alif23_loop1(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (int i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

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
	AlifIntT mark_ = _p->mark_;

	{ // for_if_clause+
		CompSeq* a_{};
		if ((a_ = (CompSeq*)alif23_loop1(_p)))
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// dictcomp: "{" kvpair for_if_clauses "}"
static Expression* dictComp_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// listcomp: "[" expression for_if_clauses "]"
static Expression* listComp_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // expression ":" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "**" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};

		if (
			(literal_ = alifParserEngine_expectToken(_p, DOUBLESTAR)) // "**"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_keyValuePair(_p, nullptr, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // kvpair
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeyValuePair* a_{};
		if ((a_ = kvPair_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif22_loop0: "," double_starred_kvpair
static Seq* alif22_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif7_gather: double_starred_kvpair alif22_loop0
static Seq* alif7_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // double_starred_kvpair alif22_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeyValuePair* element_{};
		Seq* seq_{};
		if (
			(element_ = doubleStarKVPair_rule(_p)) // double_starred_kvpair
			and
			(seq_ = alif22_loop0(_p)) // alif22_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* doubleStarKVPairs_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // ",".double_starred_kvpair+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal_{};
		Seq* a_{};
		if (
			(a_ = alif7_gather(_p)) // ",".double_starred_kvpair+
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// dict: "{" double_starred_kvpairs? "}"
static Expression* dict_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Seq* a_{};
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
			res_ = alifAST_dict((ExprSeq*)alifParserEngine_getKeys(_p, a_), (ExprSeq*)alifParserEngine_getValues(_p, a_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif21: star_sub_expression "," star_sub_expressions?
static ExprSeq* alif21(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // star_sub_Expressions "," star_sub_Expressions?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		ExprSeq* b_{};
		if (
			(a_ = starSubExpression_rule(_p)) // star_sub_expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(b_ = starSubExpressions_rule(_p), !_p->errorIndicator) // star_sub_expression?
			)
		{
			res_ = (ExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, (Seq*)b_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* tuple_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		ExprSeq* a_{}; // casted to ExprSeq* from void*
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = alif21(_p), !_p->errorIndicator) // [starSubExpression ',' starSubExpressions?] // casted to ExpreSeq* from void*
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_tuple(a_, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// list: "[" star_sub_expressions? "]"
static Expression* list_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		ExprSeq* a_{};
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
			res_ = alifAST_list(a_, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// fstring_format_spec: FSTRING_MIDDLE > fstring_replacement_field
static Expression* fStringFormatSpec_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // FSTRING_MIDDLE
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, FSTRINGMIDDLE)))
		{
			res_ = alifParserEngine_decodeConstantFromToken(_p, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // fstring_replacement_field
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = fStringReplacementField_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif21_loop0: fstring_format_spec
static Seq* alif21_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // fstring_format_spec
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* var_1{};
		while ((var_1 = fStringFormatSpec_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

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
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Seq* a_{};
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
			res_ = alifParserEngine_setupFullFormatSpec(_p, literal_, (ExprSeq*)a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// fstring_conversion: "!" NAME
static ResultTokenWithMetadata* fStringConversion_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ResultTokenWithMetadata* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "!" NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, EXCLAMATION)) // "!"
			and
			(a_ = alifParserEngine_nameToken(_p)) // NAME
			)
		{
			res_ = alifParserEngine_checkFStringConversion(_p, literal_, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif20: yield_expr > star_expressions
static Expression* alif20(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // yield_expr
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = yieldExpr_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_expressions
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = starExpressions_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* fStringReplacementField_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// fstring_middle: fstring_replacement_field > FSTRING_MIDDLE
static Expression* fStringMiddle_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // fstring_replacement_field
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = fStringReplacementField_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // FSTRING_MIDDLE
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, FSTRINGMIDDLE)))
		{
			res_ = alifParserEngine_constantFromToken(_p, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif20_loop0: fstring_middle
static Seq* alif20_loop0(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // (fstring > string)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* var_1{};
		while ((var_1 = fStringMiddle_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// fstring: FSTRING_START fstring_middle* FSTRING_END
static Expression* fString_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // FSTRING_START fstring_middle* FSTRING_END
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		Seq* b_{};
		AlifPToken* c_{};
		if (
			(a_ = alifParserEngine_expectToken(_p, FSTRINGSTART)) // "FSTRING_START"
			and
			(b_ = alif20_loop0(_p)) // fstring_middle*
			and
			(c_ = alifParserEngine_expectToken(_p, FSTRINGEND)) // "FSTRING_END"
			)
		{
			res_ = alifParserEngine_joinedStr(_p, a_, (ExprSeq*)b_, c_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// string: STRING
static Expression* string_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // STRING
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = (AlifPToken*)alifParserEngine_stringToken(_p)))
		{
			res_ = alifParserEngine_constantFromString(_p, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // fstring
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = fString_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // string
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = string_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif19_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// strings: (fstring > string)+
static Expression* strings_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, STRINGS_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		ExprSeq* a_{};
		if ((a_ = (ExprSeq*)alif19_loop1(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifParserEngine_combineStrings(_p, a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, STRINGS_TYPE, res_);
	_p->level--;
	return res_;

}


// alif18: dict > dictcomp
static Expression* alif18(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	{ // dict
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* dictVar{};
		if ((dictVar = dict_rule(_p)))
		{
			res_ = dictVar;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // dictcomp
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* dictCompVar{};
		if ((dictCompVar = dictComp_rule(_p)))
		{
			res_ = dictCompVar;
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* alif17(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	{ // list
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* listVar{};
		if ((listVar = list_rule(_p)))
		{
			res_ = listVar;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // listcomp
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* listCompVar{};
		if ((listCompVar = listComp_rule(_p)))
		{
			res_ = listCompVar;
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // STRING
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* stringVar{};
		if (stringVar = alifParserEngine_stringToken(_p))
		{
			res_ = stringVar;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // FSTRING_START
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* fStringStart{};
		if ((fStringStart = alifParserEngine_expectToken(_p, FSTRINGSTART)))
		{
			res_ = fStringStart;
			goto done;
		}
		_p->mark_ = mark_;
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
		> NAME
		> "True"
		> "False"
		> "None"
		> &(STRING > FSTRING_START) string
		> NUMBER
		> &"(" tuble
		> &"[" (list > listcomp)
		> &"{" (dict > dictcomp)
*/
static Expression* atom_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* nameVar{};
		if ((nameVar = alifParserEngine_nameToken(_p)))
		{
			res_ = nameVar;
			goto done;
		}
		_p->mark_ = mark_;
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &(STRING > FSTRING_START) strings
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* stringsVar{};
		if (
			alifParserEngine_lookahead(1, alif16, _p)
			and
			(stringsVar = strings_rule(_p)) // strings
			)
		{
			res_ = stringsVar;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // NUMBER
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* numberVar{};
		if ((numberVar = alifParserEngine_numberToken(_p)))
		{
			res_ = numberVar;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"(" tuple
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* tupleVar{}; // casted to Expression* from void*
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LPAR) // "("
			and
			(tupleVar = tuple_rule(_p)) // tuple
			)
		{
			res_ = tupleVar;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"[" (list > listcomp)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* listVar{}; // casted to Expression* from void*
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LSQR) // "["
			and
			(listVar = alif17(_p)) // list > listcomp
			)
		{
			res_ = listVar;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"{" (dict > dictcomp)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* dictVar{}; // casted to Expression* from void*
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LBRACE) // "{"
			and
			(dictVar = alif18(_p)) // dict > dictcomp
			)
		{
			res_ = dictVar;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;

}


// alif15: ":" expression?
static Expression* alif15(AlifParser* _p) { 
	// return type is changed to Expression from void*

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{}; // changed to Expression from void*
	AlifIntT mark_ = _p->mark_;
	{ // ":" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{}; // changed to Expression from void*
		if (
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(a_ = expression_rule(_p), !_p->errorIndicator) // expression?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* slice_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{}; // changed to Expression* from void*
		Expression* b_{}; // changed to Expression* from void*
		Expression* c_{}; // changed to Expression* from void*
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = expression_rule(_p)))
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // slice
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = slice_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = starExpression_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif18_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
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
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// error
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif6_gather: (slice > starred_expression) alif18_loop0
static Seq* alif6_gather(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // (slice > starred_expression) alif18_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* element_{};
		Seq* seq_{};
		if (
			(element_ = alif14(_p)) // slice > star_expression
			and
			(seq_ = alif18_loop0(_p)) // alif18_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* slices_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		Expression* a_{};
		if (
			(a_ = slice_rule(_p)) // slice
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, COMMA) // ","
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // ",".(slice > starred_expression)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ExprSeq* a_{};
		if (
			(a_ = (ExprSeq*)alif6_gather(_p)) // ",".(slice > starred_expression)+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res_ = alifAST_tuple(a_, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;

}


static Expression* primary_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}

	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // primary "." NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = primary_rule(_p)) // primary
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // NAME
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_attribute(a_, b_->V.name.name, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // primary "(" arguments? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
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
			res_ = alifAST_call(a_, b_ ? ((Expression*)b_)->V.call.args : nullptr,
				b_ ? ((Expression*)b_)->V.call.keywords : nullptr, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // primary "[" slices "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_subScript(a_, b_, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // atom
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* atom{};
		if (atom = atom_rule(_p)) {
			res_ = atom;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
/*
	Left-recursive
	primary:
		> primary "." NAME
		> primary "(" arguments? ")"
		> primary "[" slices "]"
		> atom
*/
static Expression* primary_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, PRIMARY_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	AlifIntT resMark = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		_p->mark_ = mark_;
		Expression* raw_ = primary_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark_ <= resMark) break;

		resMark = _p->mark_;
		res_ = raw_;
	}

	_p->mark_ = resMark;
	_p->level--;

	return res_;
}


// await_primary: "await" primary > primary
static Expression* awaitPrimary_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, AWAIT_PRIMARY_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};

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
			//res_ = CHECK_VERSION(Expression*, 5, L"تعبير إنتظر", alifAST_await(a_, EXTRA)); // تمت إضافته فقط للإستفادة منه في المستقبل في حال الحاجة
			res_ = alifAST_await(a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // primary
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* primaryVar{};
		if (primaryVar = primary_rule(_p)) {
			res_ = primaryVar;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, AWAIT_PRIMARY_TYPE, res_);
	_p->level--;
	return res_;
}


// sqrt: "/^" sqrt > await_primary
static Expression* sqrt_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
			res_ = alifAST_unaryOp(UnaryOp::Sqrt, a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;

	}
	{ // await_primary
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* awaitPrimary{};
		if (awaitPrimary = awaitPrimary_rule(_p)) {
			res_ = awaitPrimary;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// power: sqrt "^" factor > sqrt
static Expression* power_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::Pow, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // sqrt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* sqrtVar{};
		if (sqrtVar = sqrt_rule(_p)) {
			res_ = sqrtVar;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// factor: "+" factor > "-" factor > power
static Expression* factor_rule(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, FACTOR_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
			res_ = alifAST_unaryOp(UnaryOp::UAdd, a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "-" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
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
			res_ = alifAST_unaryOp(UnaryOp::USub, a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // power
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* power{};
		if (power = power_rule(_p)) {
			res_ = power;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark_, FACTOR_TYPE, res_);
	_p->level--;
	return res_;
}


static Expression* term_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::Mult, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // term "/" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::Div, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // term "/*" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::FloorDiv, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // term "//" factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::Mod, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // factor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* factorVar{};
		if (factorVar = factor_rule(_p)) {
			res_ = factorVar;
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* term_rule(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, TERM_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	AlifIntT resMark = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		_p->mark_ = mark_;
		Expression* raw_ = term_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark_ <= resMark) break;

		resMark = _p->mark_;
		res_ = raw_;
	}

	_p->mark_ = resMark;
	_p->level--;

	return res_;
}


static Expression* sum_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::Add, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // sum "-" term
		if (_p->errorIndicator) {
			_p->level--;
			return nullptr;
		}
		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::Sub, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // term
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* termVar{};
		if (termVar = term_rule(_p))
		{
			res_ = termVar;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// sum: sum "+" term > sum "-" term > term
static Expression* sum_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, SUM_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	AlifIntT resMark = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		_p->mark_ = mark_;
		Expression* raw_ = sum_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark_ <= resMark) break;

		resMark = _p->mark_;
		res_ = raw_;
	}

	_p->mark_ = resMark;
	_p->level--;

	return res_;
}


static Expression* shiftExpr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::LShift, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // shift_expr ">>" sum
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::RShift, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // sum
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* sumVar{};
		if (sumVar = sum_rule(_p))
		{
			res_ = sumVar;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// shift_expr: shift_expr "<<" sum > shift_expr ">>" sum > sum
static Expression* shiftExpr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, SHIFT_EXPR_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	AlifIntT resMark = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		_p->mark_ = mark_;
		Expression* raw_ = shiftExpr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark_ <= resMark) break;

		resMark = _p->mark_;
		res_ = raw_;
	}

	_p->mark_ = resMark;
	_p->level--;

	return res_;
}


static Expression* bitwiseAnd_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::BitAnd, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // shift_expr
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* shiftExpr{};
		if (shiftExpr = shiftExpr_rule(_p))
		{
			res_ = shiftExpr;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// bitwise_and: bitwise_and "&" shift_expr > shift_expr
static Expression* bitwiseAnd_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, BITWISE_AND_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	AlifIntT resMark = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		_p->mark_ = mark_;
		Expression* raw_ = bitwiseAnd_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark_ <= resMark) break;

		resMark = _p->mark_;
		res_ = raw_;
	}

	_p->mark_ = resMark;
	_p->level--;

	return res_;
}


static Expression* bitwiseXOr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::BitXor, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // bitwise_and
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* bitwiseAnd{};
		if (bitwiseAnd = bitwiseAnd_rule(_p))
		{
			res_ = bitwiseAnd;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// bitwise_xor: bitwise_xor "*|" bitwise_and > bitwise_and
static Expression* bitwiseXOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, BITWISE_XOR_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	AlifIntT resMark = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		_p->mark_ = mark_;
		Expression* raw_ = bitwiseXOr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark_ <= resMark) break;

		resMark = _p->mark_;
		res_ = raw_;
	}

	_p->mark_ = resMark;
	_p->level--;

	return res_;
}


static Expression* bitwiseOr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Expression* b_{};
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
			res_ = alifAST_binOp(a_, Operator::BitOr, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // bitwise_xor
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* bitwiseXOr{};
		if (bitwiseXOr = bitwiseXOr_rule(_p)) {
			res_ = bitwiseXOr;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
// Left-recursive
// bitwise_or: bitwise_or "|" bitwise_xor > bitwise_xor
static Expression* bitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, BITWISE_OR_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	AlifIntT resMark = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		_p->mark_ = mark_;
		Expression* raw_ = bitwiseOr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark_ <= resMark) break;

		resMark = _p->mark_;
		res_ = raw_;
	}

	_p->mark_ = resMark;
	_p->level--;

	return res_;
}


// is_bitwise_or: "is" bitwise_or
static CompExprPair* isBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "is" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IS_KW))  // "is"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::Is, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "is" "not" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		Expression* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IS_KW))  // "is"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, NOT_KW))  // "not"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::IsNot, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "in" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IN_KW))  // "in"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::In, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "not" "in" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		Expression* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, NOT_KW))  // "not"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, IN_KW))  // "in"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::NotIn, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // ">" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LESSTHAN))  // ">"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::GreaterThan, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // ">=" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, GREATEREQUAL))  // ">="
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::GreaterThanEq, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "<" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, GREATERTHAN))  // "<"
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::LessThan, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "<=" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LESSEQUAL))  // "<="
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::LessThanEq, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "!=" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, NOTEQUAL))  // "!=" // from _tmp_89_rule but it's not nessesary
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::NotEq, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "==" bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, EQUALEQUAL))  // "=="
			and
			(a_ = bitwiseOr_rule(_p)) // bitwise_or
			)
		{
			res_ = alifParserEngine_compExprPair(_p, CmpOp::Equal, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		_p->mark_ = mark_;
	}
	{ // noteq_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* notEqBitwise{};
		if ((notEqBitwise = notEqBitwiseOr_rule(_p)))
		{
			res_ = notEqBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // lte_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* lteBitwise{};
		if ((lteBitwise = lessThanEqBitwiseOr_rule(_p)))
		{
			res_ = lteBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // lt_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* ltBitwise{};
		if ((ltBitwise = lessThanBitwiseOr_rule(_p)))
		{
			res_ = ltBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // gte_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* gteBitwise{};
		if ((gteBitwise = greaterThanEqBitwiseOr_rule(_p)))
		{
			res_ = gteBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // gt_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* gtBitwise{};
		if ((gtBitwise = greaterThanBitwiseOr_rule(_p)))
		{
			res_ = gtBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // notin_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* notInBitwise{};
		if ((notInBitwise = notInBitwiseOr_rule(_p)))
		{
			res_ = notInBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // in_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* inBitwise{};
		if ((inBitwise = inBitwiseOr_rule(_p)))
		{
			res_ = inBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // isnot_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* isNotBitwise{};
		if ((isNotBitwise = isNotBitwiseOr_rule(_p)))
		{
			res_ = isNotBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // is_bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* isBitwise{};
		if ((isBitwise = isBitwiseOr_rule(_p)))
		{
			res_ = isBitwise;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif17_loop1: compare_op_bitwise_or_pair
static Seq* alif17_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// comparison: bitwise_or compare_op_bitwise_or_pair+ > bitwise_or
static Expression* comparison_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		Expression* a_{};
		Seq* b_{};
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
			res_ = alifAST_compare(a_, (IntSeq*)alifParserEngine_getCmpOps(_p, b_),
				alifParserEngine_getExprs(_p, b_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // bitwise_or
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* bitwiseOr{};
		if ((bitwiseOr = bitwiseOr_rule(_p)))
		{
			res_ = bitwiseOr;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// inversion: "not" inversion > comparison
static Expression* inversion_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, INVERSION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
			res_ = alifAST_unaryOp(UnaryOp::Not, a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // compression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* comparisonVar{};
		if (comparisonVar = comparison_rule(_p))
		{
			res_ = comparisonVar;
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "and" inversion
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, AND_KW)) // "and"
			and
			(a_ = inversion_rule(_p)) // inversion
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif16_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// conjunction: inversion ("and" inversion)+ > inversion
static Expression* conjuction_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, CONJUCTION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		Expression* a_{};
		Seq* b_{};
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
			res_ = alifAST_boolOp(BoolOp::And, (ExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // inversion
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* inversionVar{};
		if (inversionVar = inversion_rule(_p))
		{
			res_ = inversionVar;
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "or" conjuction
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, OR_KW)) // "or"
			and
			(a_ = conjuction_rule(_p)) // conjuction
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif15_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// disjunction: conjunction ("or" conjunction)+ > conjunction
static Expression* disjunction_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, DISJUCTION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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

		Expression* a_{};
		Seq* b_{};
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
			res_ = alifAST_boolOp(BoolOp::Or, (ExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // conjuction
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* conjuctionVar{};
		if (conjuctionVar = conjuction_rule(_p))
		{
			res_ = conjuctionVar;
			goto done;
		}
		_p->mark_ = mark_;
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
static Expression* expression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, EXPRESSION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Expression* b_{};
		Expression* c_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // disjunction
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* disjunctionVar{};
		if (disjunctionVar = disjunction_rule(_p)) {
			res_ = disjunctionVar;
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "," expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};

		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = expression_rule(_p)) // expression
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif14_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// expressions: expression ("," expression)+ ","? > expression "," > expression
static Expression* expressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Seq* b_{};
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
			res_ = alifAST_tuple((ExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // expression ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
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
			res_ = alifAST_tuple((ExprSeq*)alifParserEngine_singletonSeq(_p, a_), ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // expression
		if (_p->errorIndicator) {
			_p->level--;
			return nullptr;
		}
		Expression* expressionVar{};
		if ((expressionVar = expression_rule(_p))) // expression
		{
			res_ = expressionVar;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// star_sub_expression: "*" bitwise_or > expression
static Expression* starSubExpression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
			res_ = alifAST_star(a_, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* exprVar{};
		if ((exprVar = expression_rule(_p)))
		{
			res_ = exprVar;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif13_loop0: "," star_sub_expression
static Seq* alif13_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," star_sub_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = starSubExpression_rule(_p)) // star_sub_expression
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// alif5_gather: star_sub_expression alif13_loop0
static Seq* alif5_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // star_sub_expression alif13_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* element_{};
		Seq* seq_{};
		if (
			(element_ = starSubExpression_rule(_p)) // star_sub_expression
			and
			(seq_ = alif13_loop0(_p)) // alif13_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
static ExprSeq* starSubExpressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // ",".star_sub_expression+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ExprSeq* a_{};
		if (
			(a_ = (ExprSeq*)alif5_gather(_p)) // ",".starSubExpression+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// star_expression: "*" bitwise_or > expression
static Expression* starExpression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, STAR_EXPRESSION_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
			res_ = alifAST_star(a_, ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* expressionVar{};
		if (expressionVar = expression_rule(_p))
		{
			res_ = expressionVar;
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // "," star_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};

		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starExpression_rule(_p)) // star_expression
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
static Seq* alif12_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

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
static Expression* starExpressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
		Seq* b_{};
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
			res_ = alifAST_tuple((ExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_expression ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
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
			res_ = alifAST_tuple((ExprSeq*)alifParserEngine_singletonSeq(_p, a_), ExprCTX::Load, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* starExpression{};
		if (starExpression = starExpression_rule(_p))
		{
			res_ = starExpression;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// yield_expr: "yield" "from" expression > "yield" star_expressions?
static Expression* yieldExpr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
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
		Expression* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "yield" star_expressions?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, COMMA)))
		{
			res_ = literal_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, RPAR)))
		{
			res_ = literal_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // ":"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, COLON)))
		{
			res_ = literal_;
			goto done;
		}
		_p->mark_ = mark_;
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
static WithItem* withItem_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	WithItem* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // expression "as" star_target &("," > ")" > ":")
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = expression_rule(_p)))
		{
			res_ = alifAST_withItem(a_, nullptr, _p->astMem);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif11_loop0: "," with_item
static Seq* alif11_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," with_item
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		WithItem* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = withItem_rule(_p)) // with_item
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// alif4_gather: with_item alif11_loop0
static Seq* alif4_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // with_item alif11_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		WithItem* element_{};
		Seq* seq_{};
		if (
			(element_ = withItem_rule(_p)) // with_item
			and
			(seq_ = alif11_loop0(_p))
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
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
		> "with" "(" ",".with_item+ ","? ")" ":" block
		> "with" ",".with_item+ ":" block
		> "async" "with" "(" ",".with_item+ ","? ")" ":" block
		> "async" "with" ",".with_item+ ":" block
*/
static Statement* withStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "with" "(" ",".with_item+ ","? ")" ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		AlifPToken* literal1{};
		AlifPToken* literal2{};
		void* optVar;
		WithItemSeq* a_{};
		StmtSeq* b_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, WITH_KW)) // "with"
			and
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = (WithItemSeq*)alif4_gather(_p)) // ",".with_item+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			(literal2 = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_with(a_, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "with" ",".with_item+ ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		WithItemSeq* a_{};
		StmtSeq* b_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, WITH_KW)) // "with"
			and
			(a_ = (WithItemSeq*)alif4_gather(_p)) // ",".with_item+
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_with(a_, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "async" "with" "(" ",".with_item+ ","? ")" ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* literal_{};
		AlifPToken* literal1{};
		AlifPToken* literal2{};
		void* optVar;
		WithItemSeq* a_{};
		StmtSeq* b_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW)) // "async"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, WITH_KW)) // "with"
			and
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = (WithItemSeq*)alif4_gather(_p)) // ",".with_item+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			(literal2 = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_asyncWith(a_, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "async" "with" ",".with_item+ ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* literal_{};
		WithItemSeq* a_{};
		StmtSeq* b_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW)) // "async"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, WITH_KW)) // "with"
			and
			(a_ = (WithItemSeq*)alif4_gather(_p)) // ",".with_item+
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_asyncWith(a_, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	for_stmt:
		> "for" star_targets "in" ~ star_expressions ":" block
		> "async" "for" star_targets "in" ~ star_expressions ":" block
*/
static Statement* forStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "for" star_targets "in" ~ star_expressions ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		StmtSeq* c_{};
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
			(c_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_for(a_, b_, c_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
		if (cutVar) { _p->level--; return nullptr; }
	}
	{ // "async" "for" star_targets "in" ~ star_expressions ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* keyword_2{};
		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		StmtSeq* c_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW)) // "async"
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
			(c_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_asyncFor(a_, b_, c_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
		if (cutVar) { _p->level--; return nullptr; }
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// while_stmt: "while" expression ":" block
static Statement* whileStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "while" expression ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		Expression* a_{};
		StmtSeq* b_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, WHILE_KW)) // "while"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_while(a_, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// else_block: "else" &&":" block
static StmtSeq* elseBlock_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "else" &&":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		StmtSeq* b_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, ELSE_KW)) // "else"
			and
			(literal_ = alifParserEngine_expectTokenForced(_p, COLON, L":")) // ":"
			and
			(b_ = block_rule(_p)) // block
			)
		{
			res_ = b_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	elif_stmt:
		> "elif" expression ":" block elif_stmt
		> "elif" expression ":" block else_block?
*/
static Statement* elifStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "elif" expression ":" block elif_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		Expression* a_{};
		StmtSeq* b_{};
		Statement* c_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, ELIF_KW)) // "elif"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
			and
			(c_ = elifStmt_rule(_p)) // elif_stmt
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			alifAST_if(a_, b_, (StmtSeq*)alifParserEngine_singletonSeq(_p, c_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "elif" expression ":" block else_stmt?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		Expression* a_{};
		StmtSeq* b_{};
		StmtSeq* c_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, ELIF_KW)) // "elif"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	if_stmt:
		> "if" expression ":" block elif_stmt
		> "if" expression ":" block else_block?
*/
static Statement* ifStmt_rule(AlifParser* _p) {  

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "if" expression ":" block elif_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		Expression* a_{};
		StmtSeq* b_{};
		Statement* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "if"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
			and
			(c_ = elifStmt_rule(_p)) // elif_stmt
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_if(a_, b_, (StmtSeq*)alifParserEngine_singletonSeq(_p, c_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "if" expression ":" block else_block?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		Expression* a_{};
		StmtSeq* b_{};
		StmtSeq* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "if"
			and
			(a_ = expression_rule(_p)) // expression
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // block
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// default: "=" expression
static Expression* default_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "=" expression
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, EQUAL)) // "="
			and
			(a_ = expression_rule(_p)) // expression
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// param: NAME
static Arg* param_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Arg* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_arg(a_->V.name.name, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // param default? ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Arg* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // param default? &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Arg* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	AlifIntT mark_ = _p->mark_;

	{ // param default ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Arg* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // param default &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Arg* a_{};
		Expression* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// param_no_default: param "," > param &")"
static Arg* paramNoDefault_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Arg* res_{};
	AlifIntT mark_ = _p->mark_;

	if (_p->mark_ == _p->fill_
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
		Arg* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // param &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Arg* a_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// kwds: "**" param_no_default
static Arg* kwds_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Arg* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "**" param_no_default
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Arg* a_{};

		if (
			(literal_ = alifParserEngine_expectToken(_p, DOUBLESTAR)) // "**"
			and
			(a_ = paramNoDefault_rule(_p)) // param_no_default
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif10_loop1: param_maybe_default
static Seq* alif10_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif9_loop0: param_maybe_default
static Seq* alif9_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

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
	AlifIntT mark_ = _p->mark_;

	{ // "*" param_no_default param_maybe_default* kwds?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Arg* a_{};
		Seq* b_{};
		Arg* c_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "*" param_maybe_default+ kwds?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Seq* a_{};
		Arg* b_{};
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
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // kwds
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Arg* a_{};
		if ((a_ = kwds_rule(_p)))
		{
			res_ = alifParserEngine_starEtc(_p, nullptr, nullptr, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif8_loop1: param_with_default
static Seq* alif8_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif7_loop0: param_with_default
static Seq* alif7_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
// ^
// |
// |
// alif6_loop1: param_no_default
static Seq* alif6_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // param_no_default
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Arg* var_1{};
		while ((var_1 = paramNoDefault_rule(_p)))
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

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
static Arguments* parameters_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Arguments* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // param_no_default+ param_with_default* star_etc?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgSeq* a_{};
		Seq* b_{};
		StarEtc* c_{};
		if (
			(a_ = (ArgSeq*)alif6_loop1(_p)) // param_no_default+
			and
			(b_ = alif7_loop0(_p)) // param_with_default*
			and
			(c_ = starEtc_rule(_p), !_p->errorIndicator) // star_etc? 
			)
		{
			res_ = alifParserEngine_makeArguments(_p, nullptr, nullptr, a_, b_, c_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // param_with_default+ star_etc?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Seq* a_{};
		StarEtc* b_{};
		if (
			(a_ = alif8_loop1(_p)) // param_with_default+
			and
			(b_ = starEtc_rule(_p), !_p->errorIndicator) // star_etc? 
			)
		{
			res_ = alifParserEngine_makeArguments(_p, nullptr, nullptr, nullptr, a_, b_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_etc?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StarEtc* a_{};
		if ((a_ = starEtc_rule(_p), !_p->errorIndicator))
		{
			res_ = alifParserEngine_makeArguments(_p, nullptr, nullptr, nullptr, nullptr, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif8: "import" > "from"
static void* alif8(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "import"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, IMPORT_KW)))
		{
			res_ = keyword_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "from"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, FROM_KW)))
		{
			res_ = keyword_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// block: NEWLINE INDENT statements DEDENT > simple_stmts
static StmtSeq* block_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtSeq* res_{};
	if (alifParserEngine_isMemorized(_p, BLOCK_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;

	{ // NEWLINE INDENT statements DEDENT
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtSeq* a_{};
		AlifPToken* newline_{};
		AlifPToken* indent_{};
		AlifPToken* dedent_{};
		if (
			(newline_ = alifParserEngine_expectToken(_p, NEWLINE)) // "NEWLINE"
			and
			(indent_ = alifParserEngine_expectToken(_p, INDENT)) // "INDENT"
			and
			(a_ = statements_rule(_p)) // statements
			and
			(dedent_ = alifParserEngine_expectToken(_p, DEDENT)) // "DEDENT"
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // simple_stmts
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtSeq* a_{};
		if ((a_ = simpleStmts_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, BLOCK_TYPE, res_);
	_p->level--;
	return res_;
}


static Expression* dottedName_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_{};

	{ // dotted_name "." NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};

		if (
			(a_ = dottedName_rule(_p)) // dotted_name
			and
			(literal_ = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // NAME
			)
		{
			res_ = alifParserEngine_joinNamesWithDot(_p, a_, b_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};

		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}

// Left-recursive
// dotted_name: dotted_name "." NAME > NAME
static Expression* dottedName_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	if (alifParserEngine_isMemorized(_p, DOTTED_NAME_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	AlifIntT resMark = _p->mark_;

	while (true) {
		AlifIntT var_1 = alifParserEngine_updateMemo(_p, mark_, DOTTED_NAME_TYPE, res_);
		if (var_1) { _p->level--; return res_; }

		_p->mark_ = mark_;
		Expression* raw_ = dottedName_raw(_p); // casted to Expression* from void*
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark_ <= resMark) break;

		resMark = _p->mark_;
		res_ = raw_;
	}

	_p->mark_ = resMark;
	_p->level--;
	return res_;
}


// dotted_as_name: dotted_name ["as" NAME]
static Alias* dottedAsName_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Alias* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // dotted_name ["as" NAME]
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = dottedName_rule(_p)) // dotted_name
			and
			(b_ = alif7(_p), !_p->errorIndicator) // ["as" NAME] // _tmp_31_rule same _tmp_28_rule!
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_alias(a_->V.name.name, b_ ? b_->V.name.name : nullptr, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif5_loop0: "," dotted_as_name
static Seq* alif5_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," dotted_as_name
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Alias* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = dottedAsName_rule(_p)) // dotted_as_name
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// alif3_gather: dotted_as_name alif5_loop0
static Seq* alif3_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // dotted_as_name alif5_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Alias* element_{};
		Seq* seq_{};
		if (
			(element_ = dottedAsName_rule(_p)) // dotted_as_name
			and
			(seq_ = alif5_loop0(_p)) // alif5_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// dotted_as_names: ",".dotted_as_name+
static AliasSeq* dottedAsNames_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	AliasSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // ",".dotted_as_name+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AliasSeq* a_{};
		if ((a_ = (AliasSeq*)alif3_gather(_p)))
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif7: "as" NAME
static Expression* alif7(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "as" NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, AS_KW)) // "as"
			and
			(a_ = alifParserEngine_nameToken(_p)) // NAME
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// import_from_as_name: NAME ["as" NAME]
static Alias* importFromAsName_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Alias* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // NAME["as" NAME]
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		Expression* b_{};
		if (
			(a_ = alifParserEngine_nameToken(_p)) // NAME
			and
			(b_ = alif7(_p), !_p->errorIndicator) // ["as" NAME]
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;

			res_ = alifAST_alias(a_->V.name.name, b_ ? b_->V.name.name : nullptr, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif4_loop0: "," import_from_as_name
static Seq* alif4_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," import_from_as_name
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Alias* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = importFromAsName_rule(_p)) // import_from_as_name
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// alif2_gather: import_from_as_name alif4_loop0
static Seq* alif2_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // import_from_as_name alif4_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Alias* element_{};
		Seq* seq_{};

		if (
			(element_ = importFromAsName_rule(_p)) // import_from_as_name
			and
			(seq_ = alif4_loop0(_p)) // alif4_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// import_from_as_names: ",".import_from_as_name+
static AliasSeq* importFromAsNames_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	AliasSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // ",".import_from_as_name+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AliasSeq* a_{};
		if ((a_ = (AliasSeq*)alif2_gather(_p)))
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	import_from_targets:
		> "(" import_from_as_names ","? ")"
		> import_from_as_names !","
		> "*"
*/
static AliasSeq* importFromTargets_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	AliasSeq* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "(" import_from_as_names ","? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		void* optVar{};
		AliasSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = importFromAsNames_rule(_p)) // import_from_as_names
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // import_from_as_names !","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AliasSeq* a_{};
		if (
			(a_ = importFromAsNames_rule(_p)) // import_from_as_names
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, COMMA) // ","?
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "*"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, STAR))) // "*"
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;

			res_ = (AliasSeq*)alifParserEngine_singletonSeq(_p, (Alias*)alifParserEngine_aliasForStar(_p, EXTRA));
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// import_from: "from" dotted_name "import" import_from_targets
static Statement* importFrom_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "from" dotted_name "import" import_from_targets
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		Expression* a_{};
		AliasSeq* b_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, FROM_KW)) // "from"
			and
			(a_ = dottedName_rule(_p)) // dotted_name
			and
			(keyword_1 = alifParserEngine_expectToken(_p, IMPORT_KW)) // "import"
			and
			(b_ = importFromTargets_rule(_p)) // import_from_targets
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_importFrom(a_->V.name.name, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// import_name: "import" dotted_as_names
static Statement* importName_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "import" dotted_as_names
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AliasSeq* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, IMPORT_KW)) // "import"
			and
			(a_ = dottedAsNames_rule(_p)) // dotted_as_names
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_import(a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// import_stmt: > import_name > import_from
static Statement* importStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	{ // import_name
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if ((a_ = importName_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // import_from
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if ((a_ = importFrom_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// yield_stmt: yield_expr
static Statement* yieldStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // yield_expr
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = yieldExpr_rule(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_expr(a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// del_stmt: "del" del_targets &(NEWLINE)
static Statement* delStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "del" del_targets &(NEWLINE)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprSeq* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, DEL_KW)) // "del"
			and
			(a_ = delTargets_rule(_p)) // del_targets
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, NEWLINE) // "NEWLINE" // instade of _tmp_22_rule that return ";" or "NEWLINE"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_delete(a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif3_loop0: "," NAME
static Seq* alif3_loop0(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // "," NAME
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = alifParserEngine_nameToken(_p)) // NAME
			)
		{
			res_ = element_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
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
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
static Seq* alif1_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Seq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // NAME alif3_loop0
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		Expression* element_{};
		Seq* seq_{};
		if (
			(element_ = alifParserEngine_nameToken(_p)) // NAME
			and
			(seq_ = alif3_loop0(_p)) // alif3_loop0
			)
		{
			res_ = alifParserEngine_seqInsertInFront(_p, element_, seq_);
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// global_stmt: "global" ",".NAME+
static Statement* globalStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "global" ",".NAME+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprSeq* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, GLOBAL_KW)) // "global"
			and
			(a_ = (ExprSeq*)alif1_gather(_p)) // ",".NAME+
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_global((IdentifierSeq*)alifParserEngine_mapNamesToIds(_p, a_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}

// nonlocal_stmt: "nonlocal" ",".NAME+
static Statement* nonlocalStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "nonlocal" ",".NAME+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprSeq* a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, NONLOCALE_KW)) // "nonlocal"
			and
			(a_ = (ExprSeq*)alif1_gather(_p)) // ",".NAME+
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_nonlocal((IdentifierSeq*)alifParserEngine_mapNamesToIds(_p, a_), EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}

// return_stmt: "return" star_expressions?
static Statement* returnStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "return" star_expression?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		Expression* a_{}; // casted to Expression*
		if (
			(keyword_ = alifParserEngine_expectToken(_p, RETURN_KW)) // "return"
			and
			(a_ = starExpressions_rule(_p), !_p->errorIndicator) // star_expressions?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_return(a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}

/*
	augassign:
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

	AugOperator* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "+="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, PLUSEQUAL))) // "+="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::Add);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "-="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, MINUSEQUAL))) // "-="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::Sub);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "*="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, STAREQUAL))) // "*="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::Mult);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "/="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, SLASHEQUAL))) // "/="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::Div);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "^="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, CIRCUMFLEXEQUAL))) // "^="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::Pow);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "/*="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, SLASHSTAREQUAL))) // "/*="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::Mod);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "//="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, DOUBLESLASHEQUAL))) // "//="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::FloorDiv);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "&="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, AMPEREQUAL))) // "&="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::BitAnd);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "|="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, VBAREQUAL))) // "|="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::BitOr);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "^^="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, DOUBLECIRCUMFLEXEQUAL))) // "^^="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::BitXor);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "<<="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, LSHIFTEQUAL))) // "<<="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::LShift);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // ">>="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		if ((literal_ = alifParserEngine_expectToken(_p, RSHIFTEQUAL))) // ">>="
		{
			res_ = alifParserEngine_augOperator(_p, Operator::RShift);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif6: star_targets "="
static void* alif6(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // star_targets "="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		Expression* a_{};
		if (
			(a_ = starTargets_rule(_p)) // star_targets
			and
			(literal_ = alifParserEngine_expectToken(_p, EQUAL)) // "="
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}
//	^
//	|
//	|
// alif2_loop1: (star_targets "=")
static Seq* alif2_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifUSizeT capacity_ = 1;
	AlifUSizeT n_ = 0;

	{ // (star_targets "=")
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var_1{};
		while ((var_1 = alif6(_p))) // star_targets "="
		{
			res_ = var_1;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}

	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// alif5: yield_expr > star_expressions
static Expression* alif5(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // yield_expr
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = yieldExpr_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_expressions
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = starExpressions_rule(_p)))
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
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
assignment:
	> (star_targets "=")+ (yield_expr > star_expressions) !"="
	> single_target augassign ~ (yield_expr > star_expressions)
*/
static Statement* assignment_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // (star_targets "=") + (yield_expr > star_expressions) !"="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprSeq* a_{};
		Expression* b_{}; // casted to Expression*
		if (
			(a_ = (ExprSeq*)alif2_loop1(_p)) // (star_targets "=")+ // casted to ExprSeq*
			and
			(b_ = alif5(_p)) // // yield_expr > star_expressions
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, EQUAL) // "="
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_assign(a_, b_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // single_target augassign ~ (yield_expr > star_expressions)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		Expression* a_{};
		AugOperator* b_{};
		Expression* c_{}; // casted to Expression*
		if (
			(a_ = singleTarget_rule(_p)) // single_target
			and
			(b_ = augAssign_rule(_p)) // augassign
			and
			(cutVar = 1)
			and
			(c_ = alif5(_p)) // yield_expr > star_expression // _tmp_16_rule same _tmp_15_rule !?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_augAssign(a_, b_->type, c_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	function_def_raw:
		> "def" NAME "(" params? ")" ":" block
		> "async" "def" NAME "(" params? ")" ":" block
*/
static Statement* functionDef_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "def" NAME "(" params? ")" ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		AlifPToken* literal_2{};
		Expression* a_{};
		Arguments* b_{};
		StmtSeq* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, FUNC_KW)) // "def"
			and
			(a_ = alifParserEngine_nameToken(_p)) // NAME
			and
			(literal_ = alifParserEngine_expectTokenForced(_p, LPAR, L"(")) // "("
			and
			(b_ = parameters_rule(_p)) // parameters?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			(literal_2 = alifParserEngine_expectTokenForced(_p, COLON, L":")) // ":"
			and
			(c_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_functionDef(a_->V.name.name, b_ ? b_ : (Arguments*)alifParserEngine_emptyArguments(_p), c_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "async" "def" NAME "(" params? ")" ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* keyword_1{};
		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		AlifPToken* literal_2{};
		Expression* a_{};
		Arguments* b_{};
		StmtSeq* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, CLASS_KW)) // "async"
			and
			(keyword_1 = alifParserEngine_expectToken(_p, FUNC_KW)) // "def"
			and
			(a_ = alifParserEngine_nameToken(_p)) // NAME
			and
			(literal_ = alifParserEngine_expectTokenForced(_p, LPAR, L"(")) // "("
			and
			(b_ = parameters_rule(_p)) // parameters?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			(literal_2 = alifParserEngine_expectTokenForced(_p, COLON, L":")) // ":"
			and
			(c_ = block_rule(_p)) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_asyncFunctionDef(a_->V.name.name, b_ ? b_ : (Arguments*)alifParserEngine_emptyArguments(_p), c_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif4: "(" arguments? ")"
static Expression* alif4(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Expression* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "(" arguments? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		AlifPToken* literal_1{};
		Expression* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = arguments_rule(_p), !_p->errorIndicator) // arguments?
			and
			(literal_1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
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
	class_def_raw:
		> "class" NAME ["(" arguments? ")"] ":" block
*/
static Statement* classDef_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // "class" NAME ["(" arguments? ")"] ":" block
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		Expression* a_{};
		Expression* b_{};
		StmtSeq* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, CLASS_KW)) // "class"
			and
			(a_ = alifParserEngine_nameToken(_p)) // NAME
			and
			(b_ = alif4(_p), !_p->errorIndicator) // ["(" arguments? ")"]
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(c_ = block_rule(_p), !_p->errorIndicator) // block
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_classDef(a_->V.name.name, b_ ? b_->V.call.args : nullptr, b_ ? b_->V.call.keywords : nullptr, c_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	simple_statement:
		> assignment
		> star_expressions
		> &"return" return_stmt
		> &("import" > "from") import_stmt
		> "pass"
		> &"del" del_stmt
		> &"yield" yield_stmt
		> "brak"
		> "continue"
		> &"global" global_stmt
		> &"nonlocal" nonlocal_stmt
*/
static Statement* simpleStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	if (alifParserEngine_isMemorized(_p, SIMPLE_STMT_TYPE, &res_)) {
		_p->level--;
		return res_;
	}

	AlifIntT mark_ = _p->mark_;
	if (_p->mark_ == _p->fill_
		and
		alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		_p->level--;
		return nullptr;
	}
	AlifIntT startLineNo = _p->tokens[mark_]->lineNo;
	AlifIntT startColOffset = _p->tokens[mark_]->colOffset;

	{ // assignment
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* assignmentVar{};
		if ((assignmentVar = assignment_rule(_p)))
		{
			res_ = assignmentVar;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // star_expressions
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Expression* a_{};
		if ((a_ = starExpressions_rule(_p)))
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_expr(a_, EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"return" return_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RETURN_KW) // "return"
			and
			(a_ = returnStmt_rule(_p)) // return_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &("import" > "from") import_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookahead(1, alif8, _p)
			and
			(a_ = importStmt_rule(_p)) // import_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "pass"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, PASS_KW))) // "pass"
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_pass(EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"del" del_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, DEL_KW) // "del"
			and
			(a_ = delStmt_rule(_p)) // del_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"yield" yield_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, YIELD_KW) // "yield"
			and
			(a_ = yieldStmt_rule(_p)) // yield_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "break"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, BREAK_KW))) // "break"
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_break(EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "continue"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, CONTINUE_KW))) // "continue"
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res_ = alifAST_continue(EXTRA);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"global" global_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, GLOBAL_KW) // "global"
			and
			(a_ = globalStmt_rule(_p)) // global_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"nonlocal" nonlocal_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, NONLOCALE_KW) // "nonlocal"
			and
			(a_ = nonlocalStmt_rule(_p)) // nonlocal_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	alifParserEngine_insertMemo(_p, mark_, SIMPLE_STMT_TYPE, res_);
	_p->level--;
	return res_;
}


// simple_stmts: simple_stmt NEWLINE
static StmtSeq* simpleStmts_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // simple_stmt NEWLINE
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		AlifPToken* newline_{};
		if (
			(a_ = simpleStmt_rule(_p)) // simple_stmt
			and
			(newline_ = alifParserEngine_expectToken(_p, NEWLINE)) // NEWLINE
			)
		{
			res_ = (StmtSeq*)alifParserEngine_singletonSeq(_p, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif3: "def" > "async"
static void* alif3(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "def"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, FUNC_KW))) // "def"
		{
			res_ = keyword_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "async"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW))) // "async"
		{
			res_ = keyword_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}

// alif2: "with" > "async"
static void* alif2(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "with"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, WITH_KW))) // "with"
		{
			res_ = keyword_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "async"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW))) // "async"
		{
			res_ = keyword_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}

// alif1: "for" > "async"
static void* alif1(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // "for"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, FOR_KW))) // "for"
		{
			res_ = keyword_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // "async"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		if ((keyword_ = alifParserEngine_expectToken(_p, ASYNC_KW))) // "async"
		{
			res_ = keyword_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


/*
	compound_stmt:
		> &("def" > "async") function_def
		> &"if" if_stmt
		> &"class" class_def
		> &("with" > "async") with_stmt
		> &("for" > "async") for_stmt
		> &'while' while_stmt
*/
static Statement* compoundStmt_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Statement* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // &("def" > "async") function_def
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookahead(1, alif3, _p)
			and
			(a_ = functionDef_rule(_p)) // function_def
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"if" if_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, IF_KW) // "if"
			and
			(a_ = ifStmt_rule(_p)) // if_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"class" class_def
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, CLASS_KW) // "class"
			and
			(a_ = classDef_rule(_p)) // class_def
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &("with" > "async") with_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookahead(1, alif2, _p)
			and
			(a_ = withStmt_rule(_p)) // with_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &("for" > "async") for_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookahead(1, alif1, _p)
			and
			(a_ = forStmt_rule(_p)) // for_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // &"while" while_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, WHILE_KW) // "while"
			and
			(a_ = whileStmt_rule(_p)) // while_stmt
			)
		{
			res_ = a_;
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// statement: compound_stmt > simple_stmts
static StmtSeq* statement_rule(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{ // compound_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Statement* a_{};
		if ((a_ = compoundStmt_rule(_p)))
		{
			res_ = (StmtSeq*)alifParserEngine_singletonSeq(_p, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}
	{ // simple_stmt
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtSeq* a_{};
		if ((a_ = (StmtSeq*)simpleStmts_rule(_p)))
		{
			res_ = a_;
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


// alif1_loop1: statement
static Seq* alif1_loop1(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res_{};
	AlifIntT mark_ = _p->mark_;
	//AlifVector<void*>* children = (AlifVector<void*>*)alifMem_dataAlloc(sizeof(AlifVector<void*>));
	void** children_ = (void**)alifMem_dataAlloc(sizeof(void*));
	if (!children_) {
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	AlifSizeT capacity_ = 1;
	AlifSizeT n_ = 0;
	{ // statement
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtSeq* statementVar{};
		while ((statementVar = statement_rule(_p))) // statement
		{
			res_ = statementVar;
			if (n_ == capacity_) {
				capacity_ *= 2;
				void** newChildren = (void**)alifMem_dataRealloc(children_, capacity_ * sizeof(void*));
				if (!newChildren) {
					alifMem_dataFree(children_);
					_p->errorIndicator = 1;
					// Memory Error - No Memory alifError_noMemory
					_p->level--;
					return nullptr;
				}
				children_ = newChildren;
			}
			//children->push_back(res_);
			children_[n_++] = res_;
			mark_ = _p->mark_;
		}
		_p->mark_ = mark_;
	}
	//AlifUSizeT size_ = children->get_size();
	//if (size_ == 0 or _p->errorIndicator) {
	if (n_ == 0 or _p->errorIndicator) {
		alifMem_dataFree(children_);
		_p->level--;
		return nullptr;
	}
	//Seq* seq_ = (Seq*)alifNew_genericSeq(size_, _p->astMem);
	Seq* seq_ = (Seq*)alifNew_genericSeq(n_, _p->astMem);
	if (!seq_) {
		alifMem_dataFree(children_);
		_p->errorIndicator = 1;
		// Memory Error - No Memory alifError_noMemory
		_p->level--;
		return nullptr;
	}
	//for (int i = 0; i < size_; i++) SEQ_SETUNTYPED(seq_, i, children->get_element(i));
	for (AlifIntT i = 0; i < n_; i++) SEQ_SETUNTYPED(seq_, i, children_[i]);

	alifMem_dataFree(children_);
	_p->level--;
	return seq_;
}
//	^
//	|
//	|
// statements: statement+
static StmtSeq* statements_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StmtSeq* res_{};
	AlifIntT mark_ = _p->mark_;

	{
		Seq* a_{};
		if (a_ = alif1_loop1(_p)) // statement+
		{
			res_ = (StmtSeq*)alifParserEngine_seqFlatten(_p, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


static Module* interactiveRun_rule(AlifParser* _p) { 
	return nullptr;//
}
static Module* evalRun_rule(AlifParser* _p) {
	return nullptr;//
}
static Module* funcRun_rule(AlifParser* _p) {
	return nullptr;//
}


// file: statements? $
static Module* fileRun_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Module* res_{};
	AlifIntT mark_ = _p->mark_;
	{
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StmtSeq* a_{}; // changed from void* to StmtSeq*
		AlifPToken* endMarkerVar{};
		if (
			(a_ = statements_rule(_p), !_p->errorIndicator)  // statements?
			and
			(endMarkerVar = alifParserEngine_expectToken(_p, ENDMARKER)) // ENDMARKER
			)
		{
			res_ = alifParserEngine_makeModule(_p, a_);
			if (res_ == nullptr
				/* and
				error occurred and stored in ThreadState->currentException */)
			{
				_p->errorIndicator = 1;
				_p->level--;
				return nullptr;
			}
			goto done;
		}
		_p->mark_ = mark_;
	}

	res_ = nullptr;

done:
	_p->level--;
	return res_;
}


void* alifParserEngine_parse(AlifParser* _p) { 
	// تهيئة الكلمات المفتاحية
	_p->keywords = reservedKeywords;
	_p->nKeywordList = nKeywordList;
	_p->softKeyword = softKeywords;

	// بدأ تحليل النص
	void* result{};

	if (_p->startRule == ALIFFILE_INPUT) {
		result = fileRun_rule(_p);
	}
	else if (_p->startRule == ALIFSINGLE_INPUT) {
		result = interactiveRun_rule(_p);
	}
	else if (_p->startRule == ALIFEVAL_INPUT) {
		result = evalRun_rule(_p);
	}
	else if (_p->startRule == ALIFFUNCTYPE_INPUT) {
		result = funcRun_rule(_p);
	}

	return result;
}
