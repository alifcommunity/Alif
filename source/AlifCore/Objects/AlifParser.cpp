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


static const AlifIntT nKeywordList = 7;
static KeywordToken* reservedKeywords[7] = {
	new (KeywordToken[1])  { {nullptr, -1} },  // 0 char
	new (KeywordToken[3])  { {"ك", 501}, {"و", 502}, {nullptr, -1} },  // 1 char
	new (KeywordToken[8])  { {"في", 511}, {"او", 512}, {"أو", 512}, {"من", 513},  {"مع", 514}, {"صح", 515}, {"هل", 516}, {nullptr, -1} },  // 2 chars
	new (KeywordToken[11]) { {"اذا", 521}, {"إذا", 521}, {"ليس", 522}, {"مرر", 523}, {"عدم", 524}, {"ولد", 525},  {"صنف", 526}, {"خطا", 527}, {"خطأ", 527},  {"عام", 528}, {nullptr, -1} },  // 3 chars
	new (KeywordToken[10]) { {"احذف", 531}, {"دالة", 532}, {"لاجل", 533}, {"لأجل", 533},  {"والا", 534}, {"وإلا", 534}, {"توقف", 535}, {"نطاق", 536}, {"ارجع", 537}, {nullptr, -1}},  // 4 chars
	new (KeywordToken[6])  { {"اواذا", 541}, {"أوإذا", 541}, {"بينما", 542},  {"انتظر", 543}, {"استمر", 544}, {nullptr, -1}},  // 5 chars
	new (KeywordToken[3])  { {"مزامنة", 551}, {"استورد", 552}, {nullptr, -1}}  // 6 chars
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











// حذف_هدف_جزء: *اسم* > "(" حذف_هدف ")" > "(" حذف_اهداف? ")" > "[" حذف_اهداف? "]"
static ExprTy delTAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			res = alifParserEngine_setExprContext(_p, a_, ExprContext_::Del);
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
	{ // "(" حذف_هدف ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = delTarget_rule(_p)) // حذف_هدف
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res = alifParserEngine_setExprContext(_p, a_, ExprContext_::Del);
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
	{ // "(" حذف_هدف? ")"
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
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_tuple(a_, ExprContext_::Del, EXTRA);
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
	{ // "(" حذف_هدف? ")"
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
			res = alifAST_list(a_, ExprContext_::Del, EXTRA);
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
	حذف_هدف:
		> هدف_اولي "." *اسم* !هدف_تالي
		> هدف_اولي "[" قواطع "]" !هدف_تالي
		> حذف_هدف_جزء
*/
static ExprTy delTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, DEL_TARGET_TYPE, &res)) {
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

	{ // هدف_اولي "." *اسم* !هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(literal = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !هدف_تالي
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_attribute(a_, b_->V.name.name, ExprContext_::Del, EXTRA);
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
	{ // هدف_اولي "[" قواطع "]" !هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // قواطع
			and
			(literal = alifParserEngine_expectToken(_p, RSQR)) // "]"
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !هدف_تالي
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_subScript(a_, b_, ExprContext_::Del, EXTRA);
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
	{ // حذف_هدف_جزء
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = delTAtom_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, DEL_TARGET_TYPE, res);
	_p->level--;
	return res;
}


// ألف31_حلقة0: "," حذف_هدف
static ASDLSeq* alif31_loop0(AlifParser* _p) {

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

	{ // "," حذف_هدف
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy element{};
		while (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element = delTarget_rule(_p)) // حذف_هدف
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف12_تجميع: حذف_هدف ألف31_حلقة0
static ASDLSeq* alif12_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // حذف_هدف ألف31_حلقة0
		ExprTy element_{};
		ASDLSeq* seq_{};
		if (
			(element_ = delTarget_rule(_p)) // حذف_هدف
			and
			(seq_ = alif31_loop0(_p)) // ألف31_حلقة0
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
// ^
// |
// |
// حذف_اهداف: ",".حذف_هدف+ ","?
static ASDLExprSeq* delTargets_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res{};
	AlifIntT mark = _p->mark;

	{ // ",".حذف_هدف+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal{};
		ASDLExprSeq* a_{};
		if (
			(a_ = (ASDLExprSeq*)alif12_gather(_p)) // ",".حذف_هدف+
			and
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","?
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


// هدف_تالي: "(" > "[" > "."
static void* tLookahead_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "("
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, LPAR))) // "("
		{
			res = literal;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "["
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, LSQR))) // "["
		{
			res = literal;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "."
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, DOT))) // "."
		{
			res = literal;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


static ExprTy tPrimary_raw(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // هدف_اولي "." *اسم* &هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(literal = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p)
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_attribute(a_, b_->V.name.name, ExprContext_::Load, EXTRA);
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
	{ // هدف_اولي "[" قواطع "]" &هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // قواطع
			and
			(literal1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p)
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_subScript(a_, b_, ExprContext_::Load, EXTRA);
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
	{ // هدف_اولي "(" الوسيطات? ")" &هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(b_ = arguments_rule(_p), !_p->errorIndicator) // الوسيطات?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p)
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_call(a_, b_ ? b_->V.call.args : nullptr, b_ ? b_->V.call.keywords : nullptr, EXTRA);
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
	{ // جزء &هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = atom_rule(_p)) // جزء
			and
			alifParserEngine_lookahead(1, tLookahead_rule, _p) // &هدف_تالي
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
/*
	Left-recursive
	هدف_اولي:
		> هدف_اولي "." *اسم* &هدف_تالي
		> هدف_اولي "[" قواطع "]" &هدف_تالي
		> هدف_اولي "(" الوسيطات? ")" &هدف_تالي
		> جزء &هدف_تالي
*/
static ExprTy tPrimary_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, T_PRIMARY_TYPE, &res)) {
		_p->level--;
		return res;
	}
	AlifIntT mark = _p->mark;
	AlifIntT resMark = _p->mark;

	while (true) {
		AlifIntT var = alifParserEngine_updateMemo(_p, mark, T_PRIMARY_TYPE, res);
		if (var) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw = tPrimary_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMark)  break;

		resMark = _p->mark;
		res = raw;
	}

	_p->mark = resMark;
	_p->level--;

	return res;
}


/*
	هدف_صفة:
		> هدف_اولي "." *اسم* !هدف_تالي
		> هدف_اولي "[" قواطع "]" !هدف_تالي
*/
static ExprTy singleSubScriptAttributeTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // هدف_اولي "." *اسم* !هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(literal = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !هدف_تالي
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_attribute(a_, b_->V.name.name, ExprContext_::Store, EXTRA);
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
	{ // هدف_اولي "[" قواطع "]" !هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // قواطع
			and
			(literal = alifParserEngine_expectToken(_p, RSQR)) // "]"
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !هدف_تالي
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_subScript(a_, b_, ExprContext_::Store, EXTRA);
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


// هدف: هدف_صفة > *اسم* > "(" هدف ")"
static ExprTy singleTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // هدف_صفة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = singleSubScriptAttributeTarget_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			res = alifParserEngine_setExprContext(_p, a_, ExprContext_::Store);
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
	{ // "(" هدف ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = singleTarget_rule(_p)) // هدف
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


// ألف27: !"*" هدف_نجمة
static ExprTy alif27(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // !"*" هدف_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, STAR) // "*"
			and
			(a_ = starTarget_rule(_p)) // هدف_نجمة
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
// ^
// |
// |
// هدف_نجمة: "*" (!"*" هدف_نجمة) > هدف_مع_جزء_نجمة
static ExprTy starTarget_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, STAR_TARGET_TYPE, &res)) {
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

	{ // "*" (!"*" هدف_نجمة)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = alif27(_p)) // !"*" هدف_نجمة
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_star((ExprTy)alifParserEngine_setExprContext(_p, a_, ExprContext_::Store), ExprContext_::Store, EXTRA);
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
	{ // هدف_مع_جزء_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = targetWithStarAtom_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, STAR_TARGET_TYPE, res);
	_p->level--;
	return res;
}


// ألف30_حلقة0: "," هدف_نجمة
static ASDLSeq* alif30_loop0(AlifParser* _p) {

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

	{ // "," هدف_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy element{};
		while (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element = starTarget_rule(_p)) // هدف_نجمة
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف11_تجميع: هدف_نجمة ألف30_حلقة0
static ASDLSeq* alif11_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // هدف_نجمة ألف30_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy element{};
		ASDLSeq* seq{};
		if (
			(element = starTarget_rule(_p)) // هدف_نجمة
			and
			(seq = alif30_loop0(_p)) // ألف30_حلقة0
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
// ^
// |
// |
// تتالي_اهداف_مصفوفة_نجمة: ",".هدف_نجمة+ ","?
static ASDLExprSeq* starTargetsListSeq_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res{};
	AlifIntT mark = _p->mark;

	{ // ",".هدف_نجمة+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal{};
		ASDLExprSeq* a_{};
		if (
			(a_ = (ASDLExprSeq*)alif11_gather(_p)) // ",".هدف_نجمة+
			and
			(literal = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
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


// ألف26: "," هدف_نجمة
static void* alif26(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "," هدف_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starTarget_rule(_p)) // هدف_نجمة
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
// ^
// |
// |
// ألف29_حلقة1: ("," هدف_نجمة)
static ASDLSeq* alif29_loop1(AlifParser* _p) {

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

	{ // ("," هدف_نجمة)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var{};
		while (var = alif26(_p))
		{
			res = var;

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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// تتالي_اهداف_مترابطة_نجمة: هدف_نجمة ("," هدف_نجمة)+ ","? > هدف_نجمة ","
static ASDLExprSeq* starTargetsTupleSeq_rule(AlifParser* _p) {
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res{};
	AlifIntT mark = _p->mark;

	{ // هدف_نجمة ("," هدف_نجمة)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal{};
		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = starTarget_rule(_p)) // هدف_نجمة
			and
			(b_ = alif29_loop1(_p)) // ("," هدف_نجمة)+
			and
			(literal = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			res = (ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_);
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
	{ // هدف_نجمة ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(a_ = starTarget_rule(_p)) // هدف_نجمة
			and
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			res = (ASDLExprSeq*)alifParserEngine_singletonSeq(_p, a_);
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
	جزء_نجمة:
		> *اسم*
		> "(" هدف_مع_جزء_نجمة ")"
		> "(" تتالي_اهداف_مترابطة_نجمة? ")"
		> "[" تتالي_اهداف_مصفوفة_نجمة? "]"
*/
static ExprTy starAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p))) // *اسم*
		{
			res = alifParserEngine_setExprContext(_p, a_, ExprContext_::Store);
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
	{ // "(" هدف_مع_جزء_نجمة ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = targetWithStarAtom_rule(_p)) // هدف_مع_جزء_نجمة
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			res = alifParserEngine_setExprContext(_p, a_, ExprContext_::Store);
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
	{ // "(" تتالي_اهداف_مترابطة_نجمة? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ASDLExprSeq* a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = starTargetsTupleSeq_rule(_p), !_p->errorIndicator) // تتالي_اهداف_مترابطة_نجمة?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_tuple(a_, ExprContext_::Store, EXTRA);
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
	{ // "[" تتالي_اهداف_مصفوفة_نجمة? "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ASDLExprSeq* a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(a_ = starTargetsListSeq_rule(_p), !_p->errorIndicator) // تتالي_اهداف_مصفوفة_نجمة?
			and
			(literal1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_list(a_, ExprContext_::Store, EXTRA);
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
	هدف_مع_جزء_نجمة:
		> هدف_اولي "." *اسم* !هدف_تالي
		> هدف_اولي "[" قواطع "]" !هدف_تالي
		> جزء_نجمة
*/
static ExprTy targetWithStarAtom_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, TARGETWITH_STARATOM_TYPE, &res)) {
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

	{ // هدف_اولي "." *اسم* !هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !هدف_تالي
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_attribute(a_, b_->V.name.name, ExprContext_::Store, EXTRA);
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
	{ // هدف_اولي "[" قواطع "]" !هدف_تالي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = tPrimary_rule(_p)) // هدف_اولي
			and
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // قواطع
			and
			(literal1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			and
			alifParserEngine_lookahead(0, tLookahead_rule, _p) // !هدف_تالي
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_subScript(a_, b_, ExprContext_::Store, EXTRA);
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
	{ // جزء_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy starAtomVar{};
		if ((starAtomVar = starAtom_rule(_p))) // جزء_نجمة
		{
			res = starAtomVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, TARGETWITH_STARATOM_TYPE, res);
	_p->level--;
	return res;
}


// ألف25: "," هدف_نجمة
static void* alif25(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "," هدف_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starTarget_rule(_p)) // هدف_نجمة
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
// ^
// |
// |
// ألف28_حلقة0: ("," هدف_نجمة)
static ASDLSeq* alif28_loop0(AlifParser* _p) {

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

	{ // ("," هدف_نجمة)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var{};
		while ((var = alif25(_p)))
		{
			res = var;

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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// أهداف_نجمة: هدف_نجمة !"," > هدف_نجمة ("," هدف_نجمة)* ","?
static ExprTy starTargets_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // هدف_نجمة !","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = starTarget_rule(_p)) // هدف_نجمة
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, COMMA) // !","
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
	{ // هدف_نجمة ("," هدف_نجمة)* ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal{};
		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = starTarget_rule(_p)) // هدف_نجمة
			and
			(b_ = alif28_loop0(_p)) // ("," هدف_نجمة)*
			and
			(literal = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) {
				_p->level--;
				return nullptr;
			}
			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_tuple((ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), ExprContext_::Store, EXTRA);
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


// وسيط_مفتاحي_او_نجمة_مضاعفة: *اسم* "=" تعبير > "**" تعبير
static KeywordOrStar* kwArgOrDoubleStar_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeywordOrStar* res{};
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

	{ // *اسم* "=" تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(literal = alifParserEngine_expectToken(_p, EQUAL)) // "="
			and
			(b_ = expression_rule(_p)) // تعبير
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifParserEngine_keywordOrStarred(_p, (Keyword*)alifAST_keyword(a_->V.name.name, b_, EXTRA), 1);
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
	{ // "**" تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, DOUBLESTAR)) // "**"
			and
			(a_ = expression_rule(_p)) // تعبير
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifParserEngine_keywordOrStarred(_p, (Keyword*)alifAST_keyword(nullptr, a_, EXTRA), 1);
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


// وسيط_مفتاحي_او_نجمة: *اسم* "=" تعبير > تعبير_نجمة
static KeywordOrStar* kwArgOrStar_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeywordOrStar* res{};
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

	{ // *اسم* "=" تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			and
			(literal = alifParserEngine_expectToken(_p, EQUAL)) // "="
			and
			(b_ = expression_rule(_p)) // تعبير
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifParserEngine_keywordOrStarred(_p, (Keyword*)alifAST_keyword(a_->V.name.name, b_, EXTRA), 1);
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
	{ // تعبير_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = expression_rule(_p))) // تعبير
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifParserEngine_keywordOrStarred(_p, alifAST_star(a_, ExprContext_::Load, EXTRA), 0);
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


// ألف27_حلقة0: "," وسيط_مفتاحي_او_نجمة_مضاعفة
static ASDLSeq* alif27_loop0(AlifParser* _p) {

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

	{ // "," وسيط_مفتاحي_او_نجمة_مضاعفة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		KeywordOrStar* element{};
		while (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element = kwArgOrDoubleStar_rule(_p)) // وسيط_مفتاحي_او_نجمة_مضاعفة
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف10_تجميع: وسيط_مفتاحي_او_نجمة_مضاعفة alif27_loop0
static ASDLSeq* alif10_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // وسيط_مفتاحي_او_نجمة_مضاعفة ألف27_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeywordOrStar* element{};
		ASDLSeq* seq{};
		if (
			(element = kwArgOrDoubleStar_rule(_p)) // وسيط_مفتاحي_او_نجمة_مضاعفة
			and
			(seq = alif27_loop0(_p)) // ألف27_حلقة0
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
// ^
// |
// |
// ألف26_حلقة0: "," وسيط_مفتاحي_او_نجمة
static ASDLSeq* alif26_loop0(AlifParser* _p) {

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

	{ // "," وسيط_مفتاحي_او_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		KeywordOrStar* element{};
		while (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element = kwArgOrStar_rule(_p)) // وسيط_مفتاحي_او_نجمة
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف9_تجميع: وسيط_مفتاحي_او_نجمة ألف26_حلقة0
static ASDLSeq* alif9_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // وسيط_مفتاحي_او_نجمة ألف26_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeywordOrStar* element{};
		ASDLSeq* seq{};
		if (
			(element = kwArgOrStar_rule(_p)) // وسيط_مفتاحي_او_نجمة
			and
			(seq = alif26_loop0(_p)) // ألف26_حلقة0
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
// ^
// |
// |
/*
	وسيطات_مفتاحية:
		> ",".وسيط_مفتاحي_او_نجمة+ "," ",".وسيط_مفتاحي_او_نجمة_مضاعفة+
		> ",".وسيط_مفتاحي_او_نجمة+
		> ",".وسيط_مفتاحي_او_نجمة_مضاعفة+
*/
static ASDLSeq* kwArgs_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // ",".وسيط_مفتاحي_او_نجمة+ "," ",".وسيط_مفتاحي_او_نجمة_مضاعفة+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ASDLSeq* a_{};
		ASDLSeq* b_{};
		if (
			(a_ = alif9_gather(_p)) // ",".وسيط_مفتاحي_او_نجمة+
			and
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(b_ = alif10_gather(_p)) // ",".وسيط_مفتاحي_او_نجمة_مضاعفة+
			)
		{
			res = alifParserEngine_joinSequences(_p, a_, b_);
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
	{ // ",".وسيط_مفتاحي_او_نجمة+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLSeq* a_{};
		if ((a_ = alif9_gather(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // ",".وسيط_مفتاحي_او_نجمة_مضاعفة+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLSeq* a_{};
		if ((a_ = alif10_gather(_p)))
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


// ألف24: "," وسيطات_مفتاحية
static void* alif24(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "," وسيطات_مفتاحية
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ASDLSeq* a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = kwArgs_rule(_p)) // وسيطات_مفتاحية
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
// ^
// |
// |
// ألف25_حلقة0: "," (تعبير_نجمة > تعبير !"=")
static ASDLSeq* alif25_loop0(AlifParser* _p) {

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

	{ // "," (تعبير_نجمة > تعبير !"=")
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		void* element{};
		while (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element = alif23(_p)) // (تعبير_نجمة > تعبير !"=")
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف23: تعبير_نجمة > تعبير !"="
static void* alif23(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // تعبير_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = starExpression_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // تعبير !"="
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = expression_rule(_p)) // تعبير
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, NOTEQUAL) // !"="
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
// ^
// |
// |
// ألف8_تجميع: (تعبير_نجمة > تعبير !"=") ألف25_حلقة0
static ASDLSeq* alif8_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // (تعبير_نجمة > تعبير !"=") ألف25_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* element{};
		ASDLSeq* seq{};
		if (
			(element = alif23(_p)) // (تعبير_نجمة > تعبير !"=")
			and
			(seq = alif25_loop0(_p)) // ألف25_حلقة0
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
// ^
// |
// |
/*
	وسيطات:
		> ",".(تعبير_نجمة > تعبير !"=")+ ["," وسيطات_مفتاحية]
		> وسيطات_مفتاحية
*/
static ExprTy args_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // ",".(تعبير_نجمة > تعبير !"=")+ ["," وسيطات_مفتاحية]
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLExprSeq* a_{};
		void* b_{};
		if (
			(a_ = (ASDLExprSeq*)alif8_gather(_p)) // ",".(تعبير_نجمة > تعبير !"=")+
			and
			(b_ = alif24(_p), !_p->errorIndicator) // ["," وسيطات_مفتاحية]
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifParserEngine_collectCallSeqs(_p, a_, (ASDLSeq*)b_, EXTRA);
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
	{ // وسيطات_مفتاحية
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLSeq* a_{};
		if ((a_ = kwArgs_rule(_p)))
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_call((ExprTy)alifParserEngine_dummyName(_p), (ASDLExprSeq*)alifParserEngine_seqExtractStarExprs(_p, a_), (ASDLKeywordSeq*)alifParserEngine_seqDeleteStarExprs(_p, a_), EXTRA);
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


// الوسيطات: وسيطات ","? &")"
static ExprTy arguments_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, ARGUMENTS_TYPE, &res)) {
		_p->level--;
		return res;
	}
	AlifIntT mark = _p->mark;

	{ // وسيطات ","? &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ExprTy a_{};
		if (
			(a_ = args_rule(_p)) // وسيطات
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RPAR) // ")"
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
	alifParserEngine_insertMemo(_p, mark, ARGUMENTS_TYPE, res);
	_p->level--;
	return res;
}


// alif22: "اذا" انفصال
static void* alif22(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "اذا" انفصال
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, IF_KW)) // "اذا"
			and
			(a_ = disjunction_rule(_p)) // انفصال
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
// ^
// |
// |
// ألف24_حلقة0: ("اذا" انفصال)
static ASDLSeq* alif24_loop0(AlifParser* _p) {

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

	{ // ("اذا" انفصال)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var{};
		while ((var = alif22(_p)))
		{
			res = var;

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

	AlifSizeT size = children.size;
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
// ^
// |
// |
/*
	لاجل_اذا_بند:
		> "مزامنة" "لاجل" اهداف_نجمة "في" ~ انفصال ("اذا" انفصال)*
		> "لاجل" اهداف_نجمة "في" ~ انفصال ("اذا" انفصال)*
		> "مزامنة"? "لاجل" (وحدة_او ("," وحدة_او)* ","?) !"في"
*/
static Comprehension* forIfClause_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	Comprehension* res{};
	AlifIntT mark = _p->mark;

	{ // "مزامنة" "لاجل" اهداف_نجمة "في" ~ انفصال ("اذا" انفصال)*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		AlifPToken* keyword2{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLExprSeq* c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, ASYNC_KW)) // "مزامنة"
			and
			(keyword1 = alifParserEngine_expectToken(_p, FOR_KW)) // "لاجل"
			and
			(a_ = starTargets_rule(_p)) // اهداف_نجمة
			and
			(keyword2 = alifParserEngine_expectToken(_p, IN_KW)) // "في"
			and
			(cutVar = 1)
			and
			(b_ = disjunction_rule(_p)) // انفصال
			and
			(c_ = (ASDLExprSeq*)alif24_loop0(_p)) // ("اذا" انفصال)*
			)
		{
			//res_ = CHECK_VERSION(Comprehension*, 6, L"المزامنة الضمنية", alifAST_comprehension(a_, b_, c, 1, _p->astMem));
			res = alifAST_comprehension(a_, b_, c_, 1, _p->astMem);
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
		if (cutVar) { _p->level--; return nullptr; }
	}
	{ // "لاجل" اهداف_نجمة "في" ~ انفصال ("اذا" انفصال)*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLExprSeq* c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, FOR_KW)) // "لاجل"
			and
			(a_ = starTargets_rule(_p)) // اهداف_نجمة
			and
			(keyword1 = alifParserEngine_expectToken(_p, IN_KW)) // "في"
			and
			(cutVar = 1)
			and
			(b_ = disjunction_rule(_p)) // انفصال
			and
			(c_ = (ASDLExprSeq*)alif24_loop0(_p)) // ("اذا" انفصال)*
			)
		{
			//res_ = CHECK_VERSION(Comprehension*, 6, L"المزامنة الضمنية", alifAST_comprehension(a_, b_, c, 1, _p->astMem));
			res = alifAST_comprehension(a_, b_, c_, 1, _p->astMem);
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
		if (cutVar) { _p->level--; return nullptr; }
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// ألف23_حلقة1: لاجل_اذا_بند
static ASDLSeq* alif23_loop1(AlifParser* _p) {

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

	{ // لاجل_اذا_بند
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		Comprehension* a_{};
		while ((a_ = forIfClause_rule(_p)))
		{
			res = a_;

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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// لاجل_اذا_بنود: لاجل_اذا_بند+
static ASDLComprehensionSeq* forIfClauses_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLComprehensionSeq* res{};
	AlifIntT mark = _p->mark;

	{ // لاجل_اذا_بند+
		ASDLComprehensionSeq* a_{};
		if ((a_ = (ASDLComprehensionSeq*)alif23_loop1(_p)))
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


// فهرس_ضمني: "{" زوج لاجل_اذا_بنود "}"
static ExprTy dictComp_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "{" زوج لاجل_اذا_بنود "}"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		KeyValuePair* a_{};
		ASDLComprehensionSeq* b_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LBRACE)) // "{"
			and
			(a_ = kvPair_rule(_p)) // زوج
			and
			(b_ = forIfClauses_rule(_p)) // لاجل_اذا_بنود
			and
			(literal1 = alifParserEngine_expectToken(_p, RBRACE)) // "}"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_dictComp(a_->key, a_->val, b_, EXTRA);
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


// مصفوفة_ضمنية: "[" تعبير لاجل_اذا_بنود "]"
static ExprTy listComp_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "[" تعبير لاجل_اذا_بنود "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		ASDLComprehensionSeq* b_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(a_ = expression_rule(_p)) // تعبير
			and
			(b_ = forIfClauses_rule(_p)) // لاجل_اذا_بنود
			and
			(literal1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_listComp(a_, b_, EXTRA);
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


// زوج: تعبير ":" تعبير
static KeyValuePair* kvPair_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeyValuePair* res{};
	AlifIntT mark = _p->mark;

	{ // تعبير ":" تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = expression_rule(_p)) // تعبير
			and
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = expression_rule(_p)) // تعبير
			)
		{
			res = alifParserEngine_keyValuePair(_p, a_, b_);
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


// زوج_نجمة_مضاعفة: "**" وحدة_او > زوج
static KeyValuePair* doubleStarKVPair_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	KeyValuePair* res{};
	AlifIntT mark = _p->mark;

	{ // "**" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};

		if (
			(literal = alifParserEngine_expectToken(_p, DOUBLESTAR)) // "**"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_keyValuePair(_p, nullptr, a_);
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
	{ // زوج
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeyValuePair* a_{};
		if ((a_ = kvPair_rule(_p)))
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


// ألف22_حلقة0: "," زوج_نجمة_مضاعفة
static ASDLSeq* alif22_loop0(AlifParser* _p) {

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

	{ // "," زوج_نجمة_مضاعفة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		KeyValuePair* element{};
		while (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element = doubleStarKVPair_rule(_p)) // زوج_نجمة_مضاعفة
			)
		{
			res = element;
			if (!res) {
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف7_تجميع: زوج_نجمة_مضاعفة ألف22_حلقة0
static ASDLSeq* alif7_gather(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // زوج_نجمة_مضاعفة ألف22_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		KeyValuePair* element{};
		ASDLSeq* seq{};
		if (
			(element = doubleStarKVPair_rule(_p)) // زوج_نجمة_مضاعفة
			and
			(seq = alif22_loop0(_p)) // ألف22_حلقة0
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
// ^
// |
// |
// زوج_نجمة_مضاعفة: ",".ازواج_نجمة_مضاعفة+ ","?
static ASDLSeq* doubleStarKVPairs_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // ",".زوج_نجمة_مضاعفة+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* literal{};
		ASDLSeq* a_{};
		if (
			(a_ = alif7_gather(_p)) // ",".زوج_نجمة_مضاعفة+
			and
			(literal = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
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


// فهرس: "{" ازواج_نجمة_مضاعفة? "}"
static ExprTy dict_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "{" ازواج_نجمة_مضاعفة? "}"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ASDLSeq* a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LBRACE)) // "{"
			and
			(a_ = doubleStarKVPairs_rule(_p), !_p->errorIndicator) // ازواج_نجمة_مضاعفة?
			and
			(literal = alifParserEngine_expectToken(_p, RBRACE)) // "}"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_dict((ASDLExprSeq*)alifParserEngine_getKeys(_p, a_), (ASDLExprSeq*)alifParserEngine_getValues(_p, a_), EXTRA);
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


// مصفوفة: "[" تعبيرات_فرعية_نجمة? "]"
static ExprTy list_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "[" تعبيرات_فرعية_نجمة? "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ASDLExprSeq* a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(a_ = starSubExpressions_rule(_p), !_p->errorIndicator) // تعبيرات_فرعية_نجمة
			and
			(literal1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_list(a_, ExprContext_::Load, EXTRA);
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


// نص_تنفيذي_تنسيق: *نص_تنفيذي_وسط* > نص_تنفيذي_مجال  
static ExprTy fStringFormatSpec_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // *نص_تنفيذي_وسط*
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, FSTRINGMIDDLE)))
		{
			res = alifParserEngine_decodeConstantFromToken(_p, a_);
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
	{ // نص_تنفيذي_مجال  
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = fStringReplacementField_rule(_p)))
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


// ألف21حلقة0: نص_تنفيذي_تنسيق
static ASDLSeq* alif21_loop0(AlifParser* _p) {

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

	{ // نص_تنفيذي_تنسيق
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy var1{};
		while ((var1 = fStringFormatSpec_rule(_p)))
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// نص_تنفيذي_تنسيق_كامل: ":" نص_تنفيذي_تنسيق*
static ResultTokenWithMetadata* fStringFullFormatSpec_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ResultTokenWithMetadata* res{};
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

	{ // ":" نص_تنفيذي_تنسيق*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ASDLSeq* a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(a_ = alif21_loop0(_p)) // // نص_تنفيذي_تنسيق*
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifParserEngine_setupFullFormatSpec(_p, literal, (ASDLExprSeq*)a_, EXTRA);
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


// نص_تنفيذي_تحويل: "!" *اسم*
static ResultTokenWithMetadata* fStringConversion_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ResultTokenWithMetadata* res{};
	AlifIntT mark = _p->mark;

	{ // "!" *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, EXCLAMATION)) // "!"
			and
			(a_ = alifParserEngine_nameToken(_p)) // *اسم*
			)
		{
			res = alifParserEngine_checkFStringConversion(_p, literal, a_);
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


// ألف20: تعبير_ولد > تعبيرات_نجمة
static ExprTy alif20(AlifParser* _p) {

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
// ^
// |
// |
// نص_تنفيذي_مجال  : "{" (تعبير_ولد > تعبيرات_نجمة) "="? نص_تنفيذي_تحويل? نص_تنفيذي_تنسيق_كامل? "}"
static ExprTy fStringReplacementField_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "{" (تعبير_ولد > تعبيرات_نجمة) "="? نص_تنفيذي_تحويل? نص_تنفيذي_تنسيق_كامل? "}"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		ResultTokenWithMetadata* b_{};
		ResultTokenWithMetadata* c_{};
		AlifPToken* d_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LBRACE)) // "{"
			and
			(a_ = alif20(_p)) // تعبير_ولد > تعبير_نجمة
			and
			(d_ = alifParserEngine_expectToken(_p, EQUAL), !_p->errorIndicator) // "="?
			and
			(c_ = fStringConversion_rule(_p), !_p->errorIndicator) // نص_تنفيذي_تحويل?
			and
			(b_ = fStringFullFormatSpec_rule(_p), !_p->errorIndicator) // نص_تنفيذي_تنسيق_كامل?
			and
			(literal1 = alifParserEngine_expectToken(_p, RBRACE)) // "}"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifParserEngine_formattedValue(_p, a_, d_, c_, b_, literal1, EXTRA);
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


// نص_تنفيذي_وسط: نص_تنفيذي_مجال   > *نص_تنفيذي_وسط*
static ExprTy fStringMiddle_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // نص_تنفيذي_مجال  
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = fStringReplacementField_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // *نص_تنفيذي_وسط*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = alifParserEngine_expectToken(_p, FSTRINGMIDDLE)))
		{
			res = alifParserEngine_constantFromToken(_p, a_);
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


// ألف20_حلقة0: نص_تنفيذي_وسط
static ASDLSeq* alif20_loop0(AlifParser* _p) {

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

	{ // (نص_تنفيذي > نص)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy var1{};
		while ((var1 = fStringMiddle_rule(_p)))
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// نص_تنفيذي: *نص_تنفيذي_بداية* نص_تنفيذي_وسط* *نص_تنفيذي_نهاية
static ExprTy fString_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // *نص_تنفيذي_بداية* نص_تنفيذي_وسط* *نص_تنفيذي_نهاية*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		ASDLSeq* b_{};
		AlifPToken* c_{};
		if (
			(a_ = alifParserEngine_expectToken(_p, FSTRINGSTART)) // "*نص_تنفيذي_بداية*"
			and
			(b_ = alif20_loop0(_p)) // نص_تنفيذي_وسط*
			and
			(c_ = alifParserEngine_expectToken(_p, FSTRINGEND)) // "*نص_تنفيذي_نهاية*"
			)
		{
			res = alifParserEngine_joinedStr(_p, a_, (ASDLExprSeq*)b_, c_);
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


// نص: *نص*
static ExprTy string_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // *نص*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* a_{};
		if ((a_ = (AlifPToken*)alifParserEngine_stringToken(_p)))
		{
			res = alifParserEngine_constantFromString(_p, a_);
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


// ألف19: نص_تنفيذي > نص
static void* alif19(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // نص_تنفيذي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = fString_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // نص
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = string_rule(_p)))
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
// ^
// |
// |
// ألف19_حلقة1: (نص_تنفيذي > نص)
static ASDLSeq* alif19_loop1(AlifParser* _p) { 

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

	{ // (نص_تنفيذي > نص)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var1{};
		while ((var1 = alif19(_p)))
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// نصوص: (نص_تنفيذي > نص)+
static ExprTy strings_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, STRINGS_TYPE, &res)) {
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

	{ // (نص_تنفيذي > نص)+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLExprSeq* a_{};
		if ((a_ = (ASDLExprSeq*)alif19_loop1(_p)))
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifParserEngine_concatenateStrings(_p, a_, EXTRA);
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
	alifParserEngine_insertMemo(_p, mark, STRINGS_TYPE, res);
	_p->level--;
	return res;

}


// ألف21: تعبير_فرعي_نجمة "," تعبيرات_فرعية_نجمة?
static ASDLExprSeq* alif21(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res{};
	AlifIntT mark = _p->mark;

	{ // تعبيرات_فرعية_نجمة "," تعبيرات_فرعية_نجمة?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ASDLExprSeq* b_{};
		if (
			(a_ = starSubExpression_rule(_p)) // تعبير_فرعي_نجمة
			and
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(b_ = starSubExpressions_rule(_p), !_p->errorIndicator) // تعبيرات_فرعية_نجمة?
			)
		{
			res = (ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, (ASDLSeq*)b_);
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
// ^
// |
// |
// مترابطة: "(" [تعبير_فرعي_نجمة "," تعبيرات_فرعية_نجمة?] ")"
static ExprTy tuple_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "(" [تعبير_فرعي_نجمة "," تعبير_فرعي_نجمة?] ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ASDLExprSeq* a_{}; // casted to ASDLExprSeq*
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = alif21(_p), !_p->errorIndicator) // [تعبير_فرعي_نجمة ',' تعبيرات_فرعية_نجمة?]
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_tuple(a_, ExprContext_::Load, EXTRA);
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

// ألف29: تعبير_ولد > تعبير
static ExprTy alif29(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // تعبير_ولد
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = yieldExpr_rule(_p)) // تعبير_ولد
			)
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = expression_rule(_p)) // تعبير
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
// ^
// |
// |
// مجموعة: "(" تعبير_ولد > تعبير ")"
static ExprTy group_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;
	{ // "(" تعبير_ولد > تعبير ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = alif29(_p)) // تعبير_ولد > تعبير
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
// ^
// |
// |
// ألف28: مترابطة > مجموعة
static ExprTy alif28(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;
	{ // مترابطة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy tupleVar{};
		if ((tupleVar = tuple_rule(_p)))
		{
			res = tupleVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // مجموعة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy groupVar{};
		if ((groupVar = group_rule(_p)))
		{
			res = groupVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
// ^
// |
// |
// ألف18: فهرس > فهرس_ضمني
static ExprTy alif18(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;
	{ // فهرس
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy dictVar{};
		if ((dictVar = dict_rule(_p)))
		{
			res = dictVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // فهرس_ضمني
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy dictCompVar{};
		if ((dictCompVar = dictComp_rule(_p)))
		{
			res = dictCompVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
// ^
// |
// |
// ألف17: مصفوفة > مصفوفة_ضمنية
static ExprTy alif17(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;
	{ // مصفوفة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy listVar{};
		if ((listVar = list_rule(_p)))
		{
			res = listVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // مصفوفة_ضمنية
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy listCompVar{};
		if ((listCompVar = listComp_rule(_p)))
		{
			res = listCompVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
// ^
// |
// |
// ألف16 : *نص* > *نص_تنفيذي_بداية*
static void* alif16(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // *نص*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* stringVar{};
		if (stringVar = alifParserEngine_stringToken(_p))
		{
			res = stringVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // *نص_تنفيذي_بداية*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* fStringStart{};
		if ((fStringStart = alifParserEngine_expectToken(_p, FSTRINGSTART)))
		{
			res = fStringStart;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
// ^
// |
// |
/*
	جزء:
		> *اسم*
		> "صح"
		> "خطأ"
		> "عدم"
		> &(*نصوص* > *نص_تنفيذي_بداية*) نص
		> *عدد*
		> &"(" مترابطة
		> &"[" (مصفوفة > مصفوفة_ضمنية)
		> &"{" (فهرس > فهرس_ضمني)
*/
static ExprTy atom_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy nameVar{};
		if ((nameVar = alifParserEngine_nameToken(_p)))
		{
			res = nameVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // "صح"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, TRUE_KW)))
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_constant(ALIF_TRUE, nullptr, EXTRA);
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
	{ // "خطأ"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, FALSE_KW)))
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_constant(ALIF_FALSE, nullptr, EXTRA);
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
	{ // "عدم"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		if ((keyword = alifParserEngine_expectToken(_p, NONE_KW)))
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_constant(ALIF_NONE, nullptr, EXTRA);
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
	{ // &(*نصوص* > *نص_تنفيذي_بداية*) نص
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy stringsVar{};
		if (
			alifParserEngine_lookahead(1, alif16, _p)
			and
			(stringsVar = strings_rule(_p)) // نصوص
			)
		{
			res = stringsVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // *عدد*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy numberVar{};
		if ((numberVar = alifParserEngine_numberToken(_p)))
		{
			res = numberVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"(" (مترابطة > مجموعة)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy val{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LPAR) // "("
			and
			(val = alif28(_p)) // مترابطة > مجموعة
			)
		{
			res = val;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"[" (مصفوفة > مصفوفة_ضمنية)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy listVar{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LSQR) // "["
			and
			(listVar = alif17(_p)) // مصفوفة > مصفوفة_ضمنية
			)
		{
			res = listVar;
			goto done;
		}
		_p->mark = mark;
	}
	{ // &"{" (فهرس > فهرس_ضمني)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy dictVar{};
		if (
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, LBRACE) // "{"
			and
			(dictVar = alif18(_p)) // فهرس > فهرس_ضمني
			)
		{
			res = dictVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;

}


// ألف15: ":" تعبير?
static ExprTy alif15(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;
	{ // ":" تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(a_ = expression_rule(_p), !_p->errorIndicator) // تعبير?
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
// ^
// |
// |
// قاطع: تعبير? ":" تعبير? [":" تعبير?] > تعبير
static ExprTy slice_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		ExprTy c_{};
		if (
			(a_ = expression_rule(_p), !_p->errorIndicator) // تعبير?
			and
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = expression_rule(_p), !_p->errorIndicator) // تعبير?
			and
			(c_ = alif15(_p), !_p->errorIndicator) // [":" تعبير?]
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_slice(a_, b_, c_, EXTRA);
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
	{ // تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = expression_rule(_p)))
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


// ألف14: قاطع > تعبير_نجمة
static void* alif14(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // قاطع
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = slice_rule(_p)))
		{
			res = a_;
			goto done;
		}
		_p->mark = mark;
	}
	{ // تعبير_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = starExpression_rule(_p)))
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
// ^
// |
// |
// ألف18_حلقة0: "," (قاطع > تعبير_نجمة)
static ASDLSeq* alif18_loop0(AlifParser* _p) { 

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

	{ // "," (قاطع > تعبير_نجمة)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		void* element{};
		while (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element = alif14(_p)) // قاطع > تعبير_نجمة
			)
		{
			res = element;
			if (res == nullptr) {
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف6_تجميع: (قاطع > تعبير_نجمة) ألف18_حلقة0
static ASDLSeq* alif6_gather(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // (قاطع > تعبير_نجمة) ألف18_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* element{};
		ASDLSeq* seq{};
		if (
			(element = alif14(_p)) // قاطع > تعبير_نجمة
			and
			(seq = alif18_loop0(_p)) // ألف18_حلقة0
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
// ^
// |
// |
// قواطع: قاطع !"," > ",".(قاطع > تعبير_نجمة)+ ","?
static ExprTy slices_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // قاطع !","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if (
			(a_ = slice_rule(_p)) // قاطع
			and
			alifParserEngine_lookaheadWithInt(0, alifParserEngine_expectToken, _p, COMMA) // ","
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
	{ // ",".(قاطع > تعبير_نجمة)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ASDLExprSeq* a_{};
		if (
			(a_ = (ASDLExprSeq*)alif6_gather(_p)) // ",".(قاطع > تعبير_نجمة)+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_tuple(a_, ExprContext_::Load, EXTRA);
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


static ExprTy primary_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // اولي "." *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = primary_rule(_p)) // اولي
			and
			(literal = alifParserEngine_expectToken(_p, DOT)) // "."
			and
			(b_ = alifParserEngine_nameToken(_p)) // *اسم*
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_attribute(a_, b_->V.name.name, ExprContext_::Load, EXTRA);
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
	{ // اولي "(" وسيطات? ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		void* b_{};
		if (
			(a_ = primary_rule(_p)) // اولي
			and
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(b_ = arguments_rule(_p), !_p->errorIndicator) // وسيطات?
			and
			(literal1 = alifParserEngine_expectToken(_p, RPAR)) // ")"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_call(a_, b_ ? ((ExprTy)b_)->V.call.args : nullptr,
				b_ ? ((ExprTy)b_)->V.call.keywords : nullptr, EXTRA);
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
	{ // اولي "[" قواطع "]"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = primary_rule(_p)) // اولي
			and
			(literal = alifParserEngine_expectToken(_p, LSQR)) // "["
			and
			(b_ = slices_rule(_p)) // قواطع
			and
			(literal1 = alifParserEngine_expectToken(_p, RSQR)) // "]"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_subScript(a_, b_, ExprContext_::Load, EXTRA);
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
	{ // جزء
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy atom{};
		if (atom = atom_rule(_p)) {
			res = atom;
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
	Left-recursive
	اولي:
		> اولي "." *اسم*
		> اولي "(" وسيطات? ")"
		> اولي "[" قواطع "]"
		> جزء
*/
static ExprTy primary_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, PRIMARY_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
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
		AlifIntT var1 = alifParserEngine_updateMemo(_p, mark, PRIMARY_TYPE, res);
		if (var1) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw = primary_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res = raw;
	}

	_p->mark = resMark;
	_p->level--;

	return res;
}


// اولي_انتظر: "انتظر" اولي > اولي
static ExprTy awaitPrimary_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, AWAIT_PRIMARY_TYPE, &res)) {
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

	{ // "انتظر" اولي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ExprTy a_{};

		if (
			(keyword = alifParserEngine_expectToken(_p, AWAIT_KW)) // "انتظر"
			and
			(a_ = primary_rule(_p)) // اولي
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			//res = CHECK_VERSION(ExprTy, 5, L"تعبير إنتظر", alifAST_await(a_, EXTRA)); // تمت إضافته فقط للإستفادة منه في المستقبل في حال الحاجة
			res = alifAST_await(a_, EXTRA);
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
	{ // اولي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy primaryVar{};
		if (primaryVar = primary_rule(_p)) {
			res = primaryVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, AWAIT_PRIMARY_TYPE, res);
	_p->level--;
	return res;
}


// جذر: "/^" جذر > اولي_انتظر
static ExprTy sqrt_rule(AlifParser* _p) {

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "/^" جذر
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, SLASHCIRCUMFLEX)) // "/^"
			and
			(a_ = sqrt_rule(_p)) // جذر
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_unaryOp(UnaryOp_::Sqrt, a_, EXTRA);
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
	{ // اولي_انتظر
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy awaitPrimary{};
		if (awaitPrimary = awaitPrimary_rule(_p)) {
			res = awaitPrimary;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// أس: جذر "^" معامل > جذر
static ExprTy power_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // جذر "^" معامل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = sqrt_rule(_p)) // جذر
			and
			(literal = alifParserEngine_expectToken(_p, CIRCUMFLEX)) // "^"
			and
			(b_ = factor_rule(_p)) // معامل
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::Pow, b_, EXTRA);
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
	{ // جذر
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy sqrtVar{};
		if (sqrtVar = sqrt_rule(_p)) {
			res = sqrtVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// معامل: "+" معامل > "-" معامل > أس
static ExprTy factor_rule(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, FACTOR_TYPE, &res)) {
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

	{ // "+" معامل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, PLUS)) // "+"
			and
			(a_ = factor_rule(_p)) // معامل
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_unaryOp(UnaryOp_::UAdd, a_, EXTRA);
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
	{ // "-" معامل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, MINUS)) // "-"
			and
			(a_ = factor_rule(_p)) // معامل
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_unaryOp(UnaryOp_::USub, a_, EXTRA);
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
	{ // أس
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy power{};
		if (power = power_rule(_p)) {
			res = power;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, FACTOR_TYPE, res);
	_p->level--;
	return res;
}


static ExprTy term_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // حد "*" معامل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = term_rule(_p)) // حد
			and
			(literal = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(b_ = factor_rule(_p)) // معامل
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::Mult, b_, EXTRA);
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
	{ // حد "/" معامل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = term_rule(_p)) // حد
			and
			(literal = alifParserEngine_expectToken(_p, SLASH)) // "/"
			and
			(b_ = factor_rule(_p)) // معامل
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::Div, b_, EXTRA);
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
	{ // حد "/*" معامل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = term_rule(_p)) // حد
			and
			(literal = alifParserEngine_expectToken(_p, SLASHSTAR)) // "/*"
			and
			(b_ = factor_rule(_p)) // معامل
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::FloorDiv, b_, EXTRA);
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
	{ // حد "//" معامل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = term_rule(_p)) // حد
			and
			(literal = alifParserEngine_expectToken(_p, DOUBLESLASH)) // "//"
			and
			(b_ = factor_rule(_p)) // معامل
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::Mod, b_, EXTRA);
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
	{ // معامل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy factorVar{};
		if (factorVar = factor_rule(_p)) {
			res = factorVar;
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
	Left-recursive
	حد:
		> حد "*" معامل
		> حد "/" معامل
		> حد "/*" معامل
		> حد "//" معامل
		> معامل
*/
static ExprTy term_rule(AlifParser* _p) { 
	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, TERM_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
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
		AlifIntT var1 = alifParserEngine_updateMemo(_p, mark, TERM_TYPE, res);
		if (var1) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw = term_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res = raw;
	}

	_p->mark = resMark;
	_p->level--;

	return res;
}


static ExprTy sum_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // جمع "+" حد
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = sum_rule(_p)) // جمع
			and
			(literal = alifParserEngine_expectToken(_p, PLUS))  // "+"
			and
			(b_ = term_rule(_p)) // حد
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::Add, b_, EXTRA);
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
	{ // جمع "-" حد
		if (_p->errorIndicator) {
			_p->level--;
			return nullptr;
		}
		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = sum_rule(_p)) // جمع
			and
			(literal = alifParserEngine_expectToken(_p, MINUS))  // "-"
			and
			(b_ = term_rule(_p)) // حد
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::Sub, b_, EXTRA);
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
	{ // حد
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy termVar{};
		if (termVar = term_rule(_p))
		{
			res = termVar;
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
// جمع: جمع "+" حد > جمع "-" حد > حد
static ExprTy sum_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, SUM_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
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
		AlifIntT var1 = alifParserEngine_updateMemo(_p, mark, SUM_TYPE, res);
		if (var1) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw = sum_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res = raw;
	}

	_p->mark = resMark;
	_p->level--;

	return res;
}


static ExprTy shiftExpr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // تعبير_ازاحة "<<" جمع
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = shiftExpr_rule(_p)) // تعبير_ازاحة
			and
			(literal = alifParserEngine_expectToken(_p, LSHIFT))  // "<<"
			and
			(b_ = sum_rule(_p)) // جمع
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::LShift, b_, EXTRA);
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
	{ // تعبير_ازاحة ">>" جمع
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = shiftExpr_rule(_p)) // تعبير_ازاحة
			and
			(literal = alifParserEngine_expectToken(_p, RSHIFT))  // ">>"
			and
			(b_ = sum_rule(_p)) // جمع
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::RShift, b_, EXTRA);
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
	{ // جمع
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy sumVar{};
		if (sumVar = sum_rule(_p))
		{
			res = sumVar;
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
// تعبير_ازاحة: تعبير_ازاحة "<<" جمع > تعبير_ازاحة ">>" جمع > جمع
static ExprTy shiftExpr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, SHIFT_EXPR_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
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
		AlifIntT var1 = alifParserEngine_updateMemo(_p, mark, SHIFT_EXPR_TYPE, res);
		if (var1) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw = shiftExpr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res = raw;
	}

	_p->mark = resMark;
	_p->level--;

	return res;
}


static ExprTy bitwiseAnd_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // وحدة_و "&" تعبير_ازاحة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = shiftExpr_rule(_p)) // وحدة_و
			and
			(literal = alifParserEngine_expectToken(_p, AMPER))  // "&"
			and
			(b_ = shiftExpr_rule(_p)) // تعبير_ازاحة
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::BitAnd, b_, EXTRA);
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
	{ // تعبير_ازاحة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy shiftExpr{};
		if (shiftExpr = shiftExpr_rule(_p))
		{
			res = shiftExpr;
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
// وحدة_و: وحدة_و "&" تعبير_ازاحة > تعبير_ازاحة
static ExprTy bitwiseAnd_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, BITWISE_AND_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
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
		AlifIntT var1 = alifParserEngine_updateMemo(_p, mark, BITWISE_AND_TYPE, res);
		if (var1) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw = bitwiseAnd_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res = raw;
	}

	_p->mark = resMark;
	_p->level--;

	return res;
}


static ExprTy bitwiseXOr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // وحدة_او_فقط "*|" وحدة_و
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = bitwiseXOr_rule(_p)) // وحدة_او_فقط
			and
			(literal = alifParserEngine_expectToken(_p, STARVBAR))  // "*|"
			and
			(b_ = bitwiseAnd_rule(_p)) // وحدة_و
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::BitXor, b_, EXTRA);
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
	{ // وحدة_و
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy bitwiseAnd{};
		if (bitwiseAnd = bitwiseAnd_rule(_p))
		{
			res = bitwiseAnd;
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
// وحدة_او_فقط: وحدة_او_فقط "*|" وحدة_و > وحدة_و
static ExprTy bitwiseXOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, BITWISE_XOR_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
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
		AlifIntT var1 = alifParserEngine_updateMemo(_p, mark, BITWISE_XOR_TYPE, res);
		if (var1) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw_ = bitwiseXOr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw_ == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res = raw_;
	}

	_p->mark = resMark;
	_p->level--;

	return res;
}


static ExprTy bitwiseOr_raw(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // وحدة_او "|" وحدة_او_فقط
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			and
			(literal = alifParserEngine_expectToken(_p, VBAR))  // "|"
			and
			(b_ = bitwiseXOr_rule(_p)) // وحدة_او_فقط
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_binOp(a_, Operator_::BitOr, b_, EXTRA);
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
	{ // وحدة_او_فقط
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy bitwiseXOr{};
		if (bitwiseXOr = bitwiseXOr_rule(_p)) {
			res = bitwiseXOr;
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
// وحدة_او: وحدة_او "|" وحدة_او_فقط > وحدة_او_فقط
static ExprTy bitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, BITWISE_OR_TYPE, &res)) {
		_p->level--;
		return res;
	}

	AlifIntT mark = _p->mark;
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
		AlifIntT var1 = alifParserEngine_updateMemo(_p, mark, BITWISE_OR_TYPE, res);
		if (var1) { _p->level--; return res; }

		_p->mark = mark;
		ExprTy raw = bitwiseOr_raw(_p);

		if (_p->errorIndicator) { _p->level--; return nullptr; }
		if (raw == nullptr or _p->mark <= resMark) break;

		resMark = _p->mark;
		res = raw;
	}

	_p->mark = resMark;
	_p->level--;

	return res;
}


// وحدة_او_هل: "هل" وحدة_او
static CompExprPair* isBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // "هل" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, IS_KW))  // "هل"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::Is, a_);
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


// وحدة_او_هل_ليس: "هل" "ليس" وحدة_او
static CompExprPair* isNotBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // "هل" "ليس" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, IS_KW))  // "هل"
			and
			(keyword1 = alifParserEngine_expectToken(_p, NOT_KW))  // "ليس"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::IsNot, a_);
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


// وحدة_او_في: "في" وحدة_او
static CompExprPair* inBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // "في" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, IN_KW))  // "في"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::In, a_);
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


// وحدة_او_ليس_في: "ليس" "في" وحدة_او
static CompExprPair* notInBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // "ليس" "في" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, NOT_KW))  // "ليس"
			and
			(keyword1 = alifParserEngine_expectToken(_p, IN_KW))  // "في"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::NotIn, a_);
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


// وحدة_او_اكبر_من: ">" وحدة_او
static CompExprPair* greaterThanBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // ">" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LESSTHAN))  // ">"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::GreaterThan, a_);
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


// وحدة_او_اكبر_من_يساوي: ">=" وحدة_او
static CompExprPair* greaterThanEqBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // ">=" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(literal_ = alifParserEngine_expectToken(_p, GREATEREQUAL))  // ">="
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::GreaterThanEq, a_);
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


// وحدة_او_اصغر_من: "<" وحدة_او
static CompExprPair* lessThanBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // "<" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, GREATERTHAN))  // "<"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::LessThan, a_);
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


// وحدة_او_اصغر_من_يساوي: "<=" وحدة_او
static CompExprPair* lessThanEqBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // "<=" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, LESSEQUAL))  // "<="
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::LessThanEq, a_);
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


// وحدة_او_لا_يساوي: "!=" وحدة_او
static CompExprPair* notEqBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // "!=" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, NOTEQUAL))  // "!=" // from _tmp_89_rule but it's not nessesary
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::NotEq, a_);
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


// وحدة_او_يساوي: "==" وحدة_او
static CompExprPair* eqBitwiseOr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
	AlifIntT mark = _p->mark;

	{ // "==" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, EQUALEQUAL))  // "=="
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			res = alifParserEngine_compExprPair(_p, CmpOp_::Equal, a_);
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
	عملية_مقارنة_وحدة:
		> وحدة_او_مساوي
		> وحدة_او_لا_يساوي
		> وحدة_او_اصغر_من_يساوي
		> وحدة_او_اصغر_من
		> وحدة_او_اكبر_من_يساوي
		> وحدة_او_اكبر_من
		> وحدة_او_ليس_في
		> وحدة_او_في
		> وحدة_او_هل_ليس
		> وحدة_او_هل
*/
static CompExprPair* compareOpBitwiseOrPair_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	CompExprPair* res{};
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

	{ // وحدة_او_مساوي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* eqBitwise{};
		if ((eqBitwise = eqBitwiseOr_rule(_p)))
		{
			res = eqBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_لا_يساوي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* notEqBitwise{};
		if ((notEqBitwise = notEqBitwiseOr_rule(_p)))
		{
			res = notEqBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_اصغر_من_يساوي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* lteBitwise{};
		if ((lteBitwise = lessThanEqBitwiseOr_rule(_p)))
		{
			res = lteBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_اصغر_من
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* ltBitwise{};
		if ((ltBitwise = lessThanBitwiseOr_rule(_p)))
		{
			res = ltBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_اكبر_من_يساوي
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* gteBitwise{};
		if ((gteBitwise = greaterThanEqBitwiseOr_rule(_p)))
		{
			res = gteBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_اكبر_من
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* gtBitwise{};
		if ((gtBitwise = greaterThanBitwiseOr_rule(_p)))
		{
			res = gtBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_ليس_في
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* notInBitwise{};
		if ((notInBitwise = notInBitwiseOr_rule(_p)))
		{
			res = notInBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_في
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* inBitwise{};
		if ((inBitwise = inBitwiseOr_rule(_p)))
		{
			res = inBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_هل_ليس
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* isNotBitwise{};
		if ((isNotBitwise = isNotBitwiseOr_rule(_p)))
		{
			res = isNotBitwise;
			goto done;
		}
		_p->mark = mark;
	}
	{ // وحدة_او_هل
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* isBitwise{};
		if ((isBitwise = isBitwiseOr_rule(_p)))
		{
			res = isBitwise;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// ألف17_حلقة1: عملية_مقارنة_وحدة
static ASDLSeq* alif17_loop1(AlifParser* _p) { 

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

	{ // عملية_مقارنة_وحدة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		CompExprPair* coboPair{};
		while ((coboPair = compareOpBitwiseOrPair_rule(_p)))
		{
			res = coboPair;

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

	AlifSizeT size = children.size;
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
// مقارنة: وحدة_او عملية_مقارنة_وحدة+ > وحدة_او
static ExprTy comparison_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // وحدة_او عملية_مقارنة_وحدة+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			and
			(b_ = alif17_loop1(_p)) // وحدة_او عملية_مقارنة_وحدة+
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_compare(a_, (ASDLIntSeq*)alifParserEngine_getCmpOps(_p, b_),
				alifParserEngine_getExprs(_p, b_), EXTRA);
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
	{ // وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy bitwiseOr{};
		if ((bitwiseOr = bitwiseOr_rule(_p)))
		{
			res = bitwiseOr;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// عكس: "ليس" عكس > مقارنة
static ExprTy inversion_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, INVERSION_TYPE, &res)) {
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

	{ // "ليس" عكس
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, NOT_KW)) // "ليس"
			and
			(a_ = inversion_rule(_p)) // عكس
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_unaryOp(UnaryOp_::Not, a_, EXTRA);
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
	{ // مقارنة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy comparisonVar{};
		if (comparisonVar = comparison_rule(_p))
		{
			res = comparisonVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, INVERSION_TYPE, res);
	_p->level--;
	return res;
}


// ألف13: "," عكس
static void* alif13(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark_ = _p->mark;

	{ // "و" عكس
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, AND_KW)) // "و"
			and
			(a_ = inversion_rule(_p)) // عكس
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
		_p->mark = mark_;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}
//	^
//	|
//	|
// ألف16_حلقة1: ("و" عكس)
static ASDLSeq* alif16_loop1(AlifParser* _p) { 

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

	{ // ("و" عكس)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var1{};
		while ((var1 = alif13(_p)))
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

	AlifSizeT size = children.size;
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
// اتصال: عكس ("و" عكس)+ > عكس
static ExprTy conjuction_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, CONJUCTION_TYPE, &res)) {
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

	{ // عكس ("و" عكس)+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = inversion_rule(_p)) // عكس
			and
			(b_ = alif16_loop1(_p)) // ("و" عكس)+
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_boolOp(BoolOp_::And, (ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), EXTRA);
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
	{ // عكس
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy inversionVar{};
		if (inversionVar = inversion_rule(_p))
		{
			res = inversionVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, CONJUCTION_TYPE, res);
	_p->level--;
	return res;
}


// ألف12: "او" اتصال
static void* alif12(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "او" اتصال
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		ExprTy a_{};
		if (
			(keyword_ = alifParserEngine_expectToken(_p, OR_KW)) // "او"
			and
			(a_ = conjuction_rule(_p)) // اتصال
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
// ألف15_حلقة1: ("او" اتصال)
static ASDLSeq* alif15_loop1(AlifParser* _p) { 

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

	{ // ("او" اتصال)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var1{};
		while ((var1 = alif12(_p)))
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

	AlifSizeT size = children.size;
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
// انفصال: اتصال ("او" اتصال)+ > اتصال
static ExprTy disjunction_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, DISJUCTION_TYPE, &res)) {
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

	{ // اتصال ("او" اتصال)+
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = conjuction_rule(_p)) // اتصال
			and
			(b_ = alif15_loop1(_p)) // ("او" اتصال)+
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_boolOp(BoolOp_::Or, (ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), EXTRA);
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
	{ // اتصال
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy conjuctionVar{};
		if (conjuctionVar = conjuction_rule(_p))
		{
			res = conjuctionVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, DISJUCTION_TYPE, res);
	_p->level--;
	return res;
}


/*
	تعبير:
		> انفصال "اذا" انفصال "والا" تعبير
		> انفصال
*/
static ExprTy expression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, EXPRESSION_TYPE, &res)) {
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

	{ // انفصال "اذا" انفصال "والا" تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		ExprTy a_{};
		ExprTy b_{};
		ExprTy c_{};
		if (
			(a_ = disjunction_rule(_p)) // انفصال
			and
			(keyword = alifParserEngine_expectToken(_p, IF_KW)) // "اذا"
			and
			(b_ = disjunction_rule(_p)) // انفصال
			and
			(keyword1 = alifParserEngine_expectToken(_p, ELSE_KW)) // "والا"
			and
			(c_ = expression_rule(_p)) // تعبير
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_ifExpr(b_, a_, c_, EXTRA);
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
	{ // انفصال
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy disjunctionVar{};
		if (disjunctionVar = disjunction_rule(_p)) {
			res = disjunctionVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, EXPRESSION_TYPE, res);
	_p->level--;
	return res;
}


// ألف11: "," تعبير
static void* alif11(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "," تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};

		if (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = expression_rule(_p)) // تعبير
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
// ألف14_حلقة1: ("," تعبير)
static ASDLSeq* alif14_loop1(AlifParser* _p) { 

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

	{ // ("," تعبير)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var1{};
		while ((var1 = alif11(_p)))
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

	AlifSizeT size = children.size;
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
// تعبيرات: تعبير ("," تعبير)+ ","? > تعبير "," > تعبير
static ExprTy expressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // تعبير("," تعبير)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = expression_rule(_p)) // تعبير
			and
			(b_ = alif14_loop1(_p)) // ("," تعبير)+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_tuple((ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), Load, EXTRA);
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
	{ // تعبير ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy a_{};
		if (
			(a_ = expression_rule(_p)) // تعبير
			and
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_tuple((ASDLExprSeq*)alifParserEngine_singletonSeq(_p, a_), ExprContext_::Load, EXTRA);
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
	{ // تعبير
		if (_p->errorIndicator) {
			_p->level--;
			return nullptr;
		}
		ExprTy expressionVar{};
		if ((expressionVar = expression_rule(_p))) // تعبير
		{
			res = expressionVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// تعبير_فرعي_نجمة: "*" وحدة_او > تعبير
static ExprTy starSubExpression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "*" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_star(a_, ExprContext_::Load, EXTRA);
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
	{ // تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy exprVar{};
		if ((exprVar = expression_rule(_p)))
		{
			res = exprVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// ألف13_حلقة0: "," تعبير_فرعي_نجمة
static ASDLSeq* alif13_loop0(AlifParser* _p) { 

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

	{ // "," تعبير_فرعي_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		ExprTy element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = starSubExpression_rule(_p)) // تعبير_فرعي_نجمة
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

	AlifSizeT size = children.size;
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
// ألف5_تجميع: تعبير_فرعي_نجمة ألف13_حلقة0
static ASDLSeq* alif5_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // تعبير_فرعي_نجمة ألف13_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy element{};
		ASDLSeq* seq{};
		if (
			(element = starSubExpression_rule(_p)) // تعبير_فرعي_نجمة
			and
			(seq = alif13_loop0(_p)) // ألف13_حلقة0
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
// تعبيرات_فرعية_نجمة: ",".تعبير_فرعي_نجمة+ ","?
static ASDLExprSeq* starSubExpressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLExprSeq* res{};
	AlifIntT mark = _p->mark;

	{ // ",".تعبير_فرعي_نجمة+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ASDLExprSeq* a_{};
		if (
			(a_ = (ASDLExprSeq*)alif5_gather(_p)) // ",".تعبير_فرعي_نجمة+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
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


// تعبير_نجمة: "*" وحدة_او > تعبير
static ExprTy starExpression_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	if (alifParserEngine_isMemorized(_p, STAR_EXPRESSION_TYPE, &res)) {
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

	{ // "*" وحدة_او
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = bitwiseOr_rule(_p)) // وحدة_او
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_star(a_, ExprContext_::Load, EXTRA);
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
	{ // تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy expressionVar{};
		if (expressionVar = expression_rule(_p))
		{
			res = expressionVar;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	alifParserEngine_insertMemo(_p, mark, STAR_EXPRESSION_TYPE, res);
	_p->level--;
	return res;
}


// ألف10: "," تعبير_نجمة
static void* alif10(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	void* res{};
	AlifIntT mark = _p->mark;

	{ // "," تعبير_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};

		if (
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = starExpression_rule(_p)) // تعبير_نجمة
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
// ألف12_حلقة1: ("," تعبير_نجمة)
static ASDLSeq* alif12_loop1(AlifParser* _p) { 

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

	{ // ("," تعبير_نجمة)
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* var1{};
		while ((var1 = alif10(_p)))
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

	AlifSizeT size = children.size;
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
/*
	تعبيرات_نجمة:
		> تعبير_نجمة ("," تعبير_نجمة)+ ","?
		> تعبير_نجمة ","
		> تعبير_نجمة
*/
static ExprTy starExpressions_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // تعبير_نجمة ("," تعبير_نجمة)+ ","?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		void* optVar{};
		ExprTy a_{};
		ASDLSeq* b_{};
		if (
			(a_ = starExpression_rule(_p)) // تعبير_نجمة
			and
			(b_ = alif12_loop1(_p)) // ("," تعبير_نجمة)+
			and
			(optVar = alifParserEngine_expectToken(_p, COMMA), !_p->errorIndicator) // ","?
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_tuple((ASDLExprSeq*)alifParserEngine_seqInsertInFront(_p, a_, b_), ExprContext_::Load, EXTRA);
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
	{ // تعبير_نجمة ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(a_ = starExpression_rule(_p)) // تعبير_نجمة
			and
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_tuple((ASDLExprSeq*)alifParserEngine_singletonSeq(_p, a_), ExprContext_::Load, EXTRA);
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
	{ // تعبير_نجمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy starExpression{};
		if (starExpression = starExpression_rule(_p))
		{
			res = starExpression;
			goto done;
		}
		_p->mark = mark;
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// تعبير_ولد: "ولد" "من" تعبير > "ولد" تعبيرات_نجمة?
static ExprTy yieldExpr_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
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

	{ // "ولد" "من" تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, YIELD_KW)) // "ولد"
			and
			(keyword = alifParserEngine_expectToken(_p, FROM_KW)) // "من"
			and
			(a_ = expression_rule(_p)) // تعبير
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_yieldFrom(a_, EXTRA);
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
	{ // "ولد" تعبيرات_نجمة?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ExprTy a_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, YIELD_KW)) // "ولد"
			and
			(a_ = starExpressions_rule(_p), !_p->errorIndicator) // تعبيرات_نجمة?
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_yield(a_, EXTRA);
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


// ألف9: "," > ")" > ":"
static void* alif9(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	AlifPToken* res{};
	AlifIntT mark = _p->mark;

	{ // ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, COMMA)))
		{
			res = literal;
			goto done;
		}
		_p->mark = mark;
	}
	{ // ")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, RPAR)))
		{
			res = literal;
			goto done;
		}
		_p->mark = mark;
	}
	{ // ":"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		if ((literal = alifParserEngine_expectToken(_p, COLON)))
		{
			res = literal;
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
	عند_عنصر:
		> تعبير "ك" هدف_نجمة &("," > ")" > ":")
		> تعبير
*/
static WithItemTy withItem_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	WithItemTy res_{};
	AlifIntT mark_ = _p->mark;

	{ // تعبير "ك" هدف_نجمة &("," > ")" > ":")
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		ExprTy a_{};
		ExprTy b_{};
		if (
			(a_ = expression_rule(_p)) // تعبير
			and
			(keyword = alifParserEngine_expectToken(_p, AS_KW)) // "ك"
			and
			(b_ = starTarget_rule(_p)) // هدف_نجمة
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
	{ // تعبير
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


// ألف11_حلقة0: "," عند_عنصر
static ASDLSeq* alif11_loop0(AlifParser* _p) { 

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

	{ // "," عند_عنصر
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal_{};
		WithItemTy element_{};
		while (
			(literal_ = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(element_ = withItem_rule(_p)) // عند_عنصر
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

	AlifSizeT size = children.size;
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
// ألف4_تجميع: عند_عنصر ألف11_حلقة0
static ASDLSeq* alif4_gather(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLSeq* res{};
	AlifIntT mark = _p->mark;

	{ // عند_عنصر ألف11_حلقة0
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		WithItemTy element{};
		ASDLSeq* seq{};
		if (
			(element = withItem_rule(_p)) // عند_عنصر
			and
			(seq = alif11_loop0(_p)) // ألف11_حلقة0
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
/*
	حالة_عند:
		> "عند" "(" ",".عند_عنصر+ ","? ")" ":" كتلة
		> "عند" ",".عند_عنصر+ ":" كتلة
		> "مزامنة" "عند" "(" ",".عند_عنصر+ ","? ")" ":" كتلة
		> "مزامنة" "عند" ",".عند_عنصر+ ":" كتلة
*/
static StmtTy withStmt_rule(AlifParser* _p) { 

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

	{ // "عند" "(" ",".عند_عنصر+ ","? ")" ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword{};
		AlifPToken* literal{};
		AlifPToken* literal1{};
		AlifPToken* literal2{};
		void* optVar;
		ASDLWithItemSeq* a_{};
		ASDLStmtSeq* b_{};

		if (
			(keyword = alifParserEngine_expectToken(_p, WITH_KW)) // "عند"
			and
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = (ASDLWithItemSeq*)alif4_gather(_p)) // ",".عند_عنصر+
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
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_with(a_, b_, EXTRA);
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
	{ // "عند" ",".عند_عنصر+ ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword{};
		AlifPToken* literal{};
		ASDLWithItemSeq* a_{};
		ASDLStmtSeq* b_{};

		if (
			(keyword = alifParserEngine_expectToken(_p, WITH_KW)) // "عند"
			and
			(a_ = (ASDLWithItemSeq*)alif4_gather(_p)) // ",".عند_عنصر+
			and
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_with(a_, b_, EXTRA);
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
	{ // "مزامنة" "عند" "(" ",".عند_عنصر+ ","? ")" ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		AlifPToken* literal{};
		AlifPToken* literal1{};
		AlifPToken* literal2{};
		void* optVar;
		ASDLWithItemSeq* a_{};
		ASDLStmtSeq* b_{};

		if (
			(keyword = alifParserEngine_expectToken(_p, ASYNC_KW)) // "مزامنة"
			and
			(keyword1 = alifParserEngine_expectToken(_p, WITH_KW)) // "عند"
			and
			(literal = alifParserEngine_expectToken(_p, LPAR)) // "("
			and
			(a_ = (ASDLWithItemSeq*)alif4_gather(_p)) // ",".عند_عنصر+
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
			res = alifAST_asyncWith(a_, b_, EXTRA);
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
	{ // "مزامنة" "عند" ",".عند_عنصر+ ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }
		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		AlifPToken* literal{};
		ASDLWithItemSeq* a_{};
		ASDLStmtSeq* b_{};

		if (
			(keyword = alifParserEngine_expectToken(_p, ASYNC_KW)) // "مزامنة"
			and
			(keyword1 = alifParserEngine_expectToken(_p, WITH_KW)) // "عند"
			and
			(a_ = (ASDLWithItemSeq*)alif4_gather(_p)) // ",".عند_عنصر+
			and
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_asyncWith(a_, b_, EXTRA);
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
	حالة_لاجل:
		> "لاجل" أهداف_نجمة "في" ~ تعبيرات_نجمة ":" كتلة
		> "مزامنة" "لاجل" أهداف_نجمة "في" ~ تعبيرات_نجمة ":" كتلة
*/
static StmtTy forStmt_rule(AlifParser* _p) { 

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

	{ // "لاجل" أهداف_نجمة "في" ~ تعبيرات_نجمة ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		AlifPToken* literal{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLStmtSeq* c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, FOR_KW)) // "لاجل"
			and
			(a_ = starTargets_rule(_p)) // أهداف_نجمة
			and
			(keyword1 = alifParserEngine_expectToken(_p, IN_KW)) // "في"
			and
			(cutVar = 1)
			and
			(b_ = starExpressions_rule(_p)) // تعبيرات_نجمة
			and
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(c_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_for(a_, b_, c_, EXTRA);
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
		if (cutVar) { _p->level--; return nullptr; }
	}
	{ // "مزامنة" "لاجل" أهداف_نجمة "في" ~ تعبيرات_نجمة ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifIntT cutVar = 0;
		AlifPToken* keyword{};
		AlifPToken* keyword1{};
		AlifPToken* keyword2{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ExprTy b_{};
		ASDLStmtSeq* c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, ASYNC_KW)) // "مزامنة"
			and
			(keyword1 = alifParserEngine_expectToken(_p, FOR_KW)) // "لاجل"
			and
			(a_ = starTargets_rule(_p)) // أهداف_نجمة
			and
			(keyword2 = alifParserEngine_expectToken(_p, IN_KW)) // "في"
			and
			(cutVar = 1)
			and
			(b_ = starExpressions_rule(_p)) // تعبيرات_نجمة
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(c_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_asyncFor(a_, b_, c_, EXTRA);
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
		if (cutVar) { _p->level--; return nullptr; }
	}

	res = nullptr;
done:
	_p->level--;
	return res;
}


// حالة_بينما: "بينما" تعبير ":" كتلة
static StmtTy whileStmt_rule(AlifParser* _p) { 

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

	{ // "بينما" تعبير ":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* literal{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, WHILE_KW)) // "بينما"
			and
			(a_ = expression_rule(_p)) // تعبير
			and
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_while(a_, b_, EXTRA);
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


// حالة_والا: "والا" &":" كتلة
static ASDLStmtSeq* elseBlock_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ASDLStmtSeq* res{};
	AlifIntT mark = _p->mark;

	{ // "والا" &":" كتلة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* literal{};
		ASDLStmtSeq* b_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, ELSE_KW)) // "والا"
			and
			(literal = alifParserEngine_expectTokenForced(_p, COLON, ":")) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			)
		{
			res = b_;
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
	حالة_اواذا:
		> "اواذا" تعبير ":" كتلة حالة_اواذا
		> "اواذا" تعبير ":" كتلة حالة_والا?
*/
static StmtTy elifStmt_rule(AlifParser* _p) { 

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

	{ // "اواذا" تعبير ":" كتلة حالة_اواذا
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* literal{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		StmtTy c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, ELIF_KW)) // "اواذا"
			and
			(a_ = expression_rule(_p)) // تعبير
			and
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			and
			(c_ = elifStmt_rule(_p)) // حالة_اواذا
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			alifAST_if(a_, b_, (ASDLStmtSeq*)alifParserEngine_singletonSeq(_p, c_), EXTRA);
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
	{ // "اواذا" تعبير ":" كتلة حالة_والا?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword{};
		AlifPToken* literal{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		ASDLStmtSeq* c_{};
		if (
			(keyword = alifParserEngine_expectToken(_p, ELIF_KW)) // "اواذا"
			and
			(a_ = expression_rule(_p)) // تعبير
			and
			(literal = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			and
			(c_ = elseBlock_rule(_p), !_p->errorIndicator) // حالة_والا?
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			alifAST_if(a_, b_, c_, EXTRA);
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
	حالة_اذا:
		> "اذا" تعبير ":" كتلة حالة_اواذا
		> "اذا" تعبير ":" كتلة حالة_والا?
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

	{ // "اذا" تعبير ":" كتلة حالة_اواذا
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		StmtTy c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "اذا"
			and
			(a_ = expression_rule(_p)) // تعبير
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			and
			(c_ = elifStmt_rule(_p)) // حالة_اواذا
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
	{ // "اذا" تعبير ":" كتلة حالة_والا?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* keyword_{};
		AlifPToken* literal_{};
		ExprTy a_{};
		ASDLStmtSeq* b_{};
		ASDLStmtSeq* c_{};

		if (
			(keyword_ = alifParserEngine_expectToken(_p, IF_KW)) // "اذا"
			and
			(a_ = expression_rule(_p)) // تعبير
			and
			(literal_ = alifParserEngine_expectToken(_p, COLON)) // ":"
			and
			(b_ = block_rule(_p)) // كتلة
			and
			(c_ = elseBlock_rule(_p), !_p->errorIndicator) // حالة_والا?
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


// قيمة_افتراضية: "=" تعبير
static ExprTy default_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ExprTy res{};
	AlifIntT mark = _p->mark;

	{ // "=" تعبير
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ExprTy a_{};
		if (
			(literal = alifParserEngine_expectToken(_p, EQUAL)) // "="
			and
			(a_ = expression_rule(_p)) // تعبير
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


// معامل_وسيط: *اسم*
static ArgTy param_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ArgTy res{};
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

	{ // *اسم*
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ExprTy a_{};
		if ((a_ = alifParserEngine_nameToken(_p)))
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_arg(a_->V.name.name, EXTRA);
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
	معامل_ربما_قيمة:
		> معامل_وسيط قيمة_افتراضية? ","
		> معامل_وسيط قيمة_افتراضية? &")"
*/
static NameDefaultPair* paramMaybeDefault_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	NameDefaultPair* res{};
	AlifIntT mark = _p->mark;

	{ // معامل_وسيط قيمة_افتراضية? ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ArgTy a_{};
		ExprTy b_{};
		if (
			(a_ = param_rule(_p)) // معامل_وسيط
			and
			(b_ = default_rule(_p), !_p->errorIndicator) // قيمة_افتراضية?
			and
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			res = alifParserEngine_nameDefaultPair(_p, a_, b_);
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
	{ // معامل_وسيط قيمة_افتراضية? &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy a_{};
		ExprTy b_{};
		if (
			(a_ = param_rule(_p)) // معامل_وسيط
			and
			(b_ = default_rule(_p), !_p->errorIndicator) // قيمة_افتراضية?
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RPAR) // ")"
			)
		{
			res = alifParserEngine_nameDefaultPair(_p, a_, b_);
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


// معاملات_مع_قيمة: معامل_وسيط قيمة_افتراضية "," > معامل_وسيط قيمة_افتراضية &")"
static NameDefaultPair* paramWithDefault_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	NameDefaultPair* res{};
	AlifIntT mark = _p->mark;

	{ // معامل_وسيط قيمة_افتراضية ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ArgTy a_{};
		ExprTy b_{};
		if (
			(a_ = param_rule(_p)) // معامل_وسيط
			and
			(b_ = default_rule(_p)) // قيمة_افتراضية
			and
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			res = alifParserEngine_nameDefaultPair(_p, a_, b_);
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
	{ // معامل_وسيط قيمة_افتراضية &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy a_{};
		ExprTy b_{};
		if (
			(a_ = param_rule(_p)) // معامل_وسيط
			and
			(b_ = default_rule(_p)) // قيمة_افتراضية
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RPAR) // ")"
			)
		{
			res = alifParserEngine_nameDefaultPair(_p, a_, b_);
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


// معاملات_بدون_قيمة: معامل_وسيط "," > معامل_وسيط &")"
static ArgTy paramNoDefault_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ArgTy res{};
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

	{ // معامل_وسيط ","
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ArgTy a_{};
		if (
			(a_ = param_rule(_p)) // معامل_وسيط
			and
			(literal = alifParserEngine_expectToken(_p, COMMA)) // ","
			)
		{
			AlifPToken* token_ = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token_ == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token_->endLineNo;
			AlifIntT endColOffset = token_->endColOffset;
			res = alifAST_arg(a_->arg, EXTRA);
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
	{ // معامل_وسيط &")"
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy a_{};
		if (
			(a_ = param_rule(_p)) // معامل_وسيط
			and
			alifParserEngine_lookaheadWithInt(1, alifParserEngine_expectToken, _p, RPAR) // ")"
			)
		{
			AlifPToken* token = alifParserEngine_getLastNonWhitespaceToken(_p);
			if (token == nullptr) { _p->level--; return nullptr; }

			AlifIntT endLineNo = token->endLineNo;
			AlifIntT endColOffset = token->endColOffset;
			res = alifAST_arg(a_->arg, EXTRA);
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


// معامل_كلمات_مفتاحية: "**" معاملات_بدون_قيمة
static ArgTy kwds_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ArgTy res{};
	AlifIntT mark = _p->mark;

	{ // "**" معاملات_بدون_قيمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ArgTy a_{};

		if (
			(literal = alifParserEngine_expectToken(_p, DOUBLESTAR)) // "**"
			and
			(a_ = paramNoDefault_rule(_p)) // معاملات_بدون_قيمة
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


// ألف10_حلقة1: معامل_ربما_قيمة
static ASDLSeq* alif10_loop1(AlifParser* _p) { 

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

	{ // معاملات_مع_قيمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		NameDefaultPair* var1{};
		while ((var1 = paramWithDefault_rule(_p)))
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف9_حلقة0: معامل_ربما_قيمة
static ASDLSeq* alif9_loop0(AlifParser* _p) { 

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

	{ // معامل_ربما_قيمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		NameDefaultPair* var1{};
		while ((var1 = paramMaybeDefault_rule(_p)))
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
/*
	الباقي_نجمة:
		> "*" معاملات_بدون_قيمة معامل_ربما_قيمة* معامل_كلمات_مفتاحية?
		> "*" "," معامل_ربما_قيمة+ معامل_كلمات_مفتاحية?
		> معامل_كلمات_مفتاحية
*/
static StarEtc* starEtc_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	StarEtc* res{};
	AlifIntT mark = _p->mark;

	{ // "*" معاملات_بدون_قيمة معامل_ربما_قيمة* معامل_كلمات_مفتاحية?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		ArgTy a_{};
		ASDLSeq* b_{};
		ArgTy c_{};
		if (
			(literal = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(a_ = paramNoDefault_rule(_p)) // معاملات_بدون_قيمة
			and
			(b_ = alif9_loop0(_p)) // معامل_ربما_قيمة*
			and
			(c_ = kwds_rule(_p), !_p->errorIndicator) // معامل_كلمات_مفتاحية?
			)
		{
			res = alifParserEngine_starEtc(_p, a_, b_, c_);
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
	{ // "*" معامل_ربما_قيمة+ معامل_كلمات_مفتاحية?
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		AlifPToken* literal{};
		AlifPToken* literal1{};
		ASDLSeq* a_{};
		ArgTy b_{};
		if (
			(literal = alifParserEngine_expectToken(_p, STAR)) // "*"
			and
			(literal1 = alifParserEngine_expectToken(_p, COMMA)) // ","
			and
			(a_ = alif10_loop1(_p)) // معامل_ربما_قيمة+
			and
			(b_ = kwds_rule(_p), !_p->errorIndicator) // معامل_كلمات_مفتاحية?
			)
		{
			res = alifParserEngine_starEtc(_p, nullptr, a_, b_);
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
	{ // معامل_كلمات_مفتاحية
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy a_{};
		if ((a_ = kwds_rule(_p)))
		{
			res = alifParserEngine_starEtc(_p, nullptr, nullptr, a_);
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


// ألف8_حلقة1: معاملات_مع_قيمة
static ASDLSeq* alif8_loop1(AlifParser* _p) { 

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

	{ // معاملات_مع_قيمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		NameDefaultPair* var1{};
		while ((var1 = paramWithDefault_rule(_p)))
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف7_حلقة0: معاملات_مع_قيمة
static ASDLSeq* alif7_loop0(AlifParser* _p) { 

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

	{ // معاملات_مع_قيمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		NameDefaultPair* var_1{};
		while ((var_1 = paramWithDefault_rule(_p)))
		{
			res = var_1;
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
// ألف6_حلقة1: معاملات_بدون_قيمة
static ASDLSeq* alif6_loop1(AlifParser* _p) { 

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

	{ // معاملات_بدون_قيمة
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ArgTy var1{};
		while ((var1 = paramNoDefault_rule(_p)))
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

	AlifSizeT size = children.size;
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
// ^
// |
// |
/*
معاملات:
	> معاملات_بدون_قيمة+ معاملات_مع_قيمة* الباقي_نجمة؟
	> معاملات_مع_قيمة+ الباقي_نجمة؟
	> الباقي_نجمة
*/
static ArgumentsTy parameters_rule(AlifParser* _p) { 

	if (_p->level++ == MAXSTACK) alifParserEngineError_stackOverflow(_p);
	if (_p->errorIndicator) { _p->level--; return nullptr; }

	ArgumentsTy res{};
	AlifIntT mark = _p->mark;

	{ // معاملات_بدون_قيمة+ معاملات_مع_قيمة* الباقي_نجمة؟
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLArgSeq* a_{};
		ASDLSeq* b_{};
		StarEtc* c_{};
		if (
			(a_ = (ASDLArgSeq*)alif6_loop1(_p)) // معاملات_بدون_قيمة+
			and
			(b_ = alif7_loop0(_p)) // معاملات_مع_قيمة*
			and
			(c_ = starEtc_rule(_p), !_p->errorIndicator) // الباقي_نجمة؟ 
			)
		{
			res = alifParserEngine_makeArguments(_p, nullptr, nullptr, a_, b_, c_);
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
	{ // معاملات_مع_قيمة+ الباقي_نجمة؟
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		ASDLSeq* a_{};
		StarEtc* b_{};
		if (
			(a_ = alif8_loop1(_p)) // معاملات_مع_قيمة+
			and
			(b_ = starEtc_rule(_p), !_p->errorIndicator) // الباقي_نجمة؟ 
			)
		{
			res = alifParserEngine_makeArguments(_p, nullptr, nullptr, nullptr, a_, b_);
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
	{ // الباقي_نجمة؟
		if (_p->errorIndicator) { _p->level--; return nullptr; }

		StarEtc* a_{};
		if ((a_ = starEtc_rule(_p), !_p->errorIndicator))
		{
			res = alifParserEngine_makeArguments(_p, nullptr, nullptr, nullptr, nullptr, a_);
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

	AlifSizeT size = children.size;
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

	AlifSizeT size = children.size;
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

	AlifSizeT size = children.size;
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

	AlifSizeT size = children.size;
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

	AlifSizeT size = children.size;
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

void parser_test(ModuleTy _p); // alif // للحذف
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

	parser_test((ModuleTy)result); // alif // للحذف
	return result;
}



/* ------------------------------------------------------------------------------------ */

// للحذف بعد الانتهاء من الاختبارات
int spaces = 0;


#define VISIT(name, val) visit_ ## name(val) 
const char* operators[] = { 0, "جمع : +", "طرح : -", "ضرب : *", "قسمة : /", "باقي : */", "أس : ^",
	"إزاحة_يسار", "إزاحة_يمين", "وحدة_او", "وحدة_او_فقط", "وحدة_و",
	"بدون_باقي"};
const char* unaryop[] = { 0, "عكس", "ليس", "_جمع : +", "_طرح : -", "جذر : /^"};

void print_space(int _space) {
	for (int i = 0; i < _space; i++) {
		printf(" ");
	}
}

void visit_constant(Constant v) {
	print_space(spaces);
	if (ALIFUSTR_CHECK(v)) {
		if (alifUStr_isASCII(v)) {
			printf("%s : %s \n", v->type->name, (const char*)ALIFUSTR_DATA(v));
			return;
		}
		printf("%s : %s \n", v->type->name, alifUStr_asUTF8(v));
	}
	if (ALIFLONG_CHECK(v)) {
		printf("%s : %lld \n", v->type->name, alifLong_asSizeT(v));
		return;
	}
	if (ALIFFLOAT_CHECK(v)) {
		printf("%s : %f \n", v->type->name, ALIFFLOAT_AS_DOUBLE(v));
		return;
	}
}


void visit_binop(ExprTy v) {
	if (v->type == ExprK_::ConstantK) {
		VISIT(constant, v->V.constant.val);
	}
	else if (v->type == ExprK_::UnaryOpK) {
		print_space(spaces);
		printf("%s\n", unaryop[v->V.unaryOp.op]);
		VISIT(binop, v->V.unaryOp.operand);
		spaces += 4;
	}
	else if (v->type == ExprK_::BinOpK) {
		VISIT(binop, v->V.binOp.left);
		print_space(spaces);
		printf("%s\n", operators[v->V.binOp.op]);
		VISIT(binop, v->V.binOp.right);
		spaces += 4;
	}
}

void visit_expr(ExprTy v) {
	if (v->type == ExprK_::ConstantK) {
		VISIT(constant, v->V.constant.val);
		spaces -= 4;
	}
	else if (v->type == ExprK_::NameK) {
		print_space(spaces);
		printf("%s : %s \n", "اسم", alifUStr_asUTF8(v->V.name.name));
		spaces -= 4;
		return;
	}
	else if (v->type == ExprK_::BinOpK) {
		VISIT(binop, v->V.binOp.left);
		print_space(spaces);
		printf("%s\n", operators[v->V.binOp.op]);
		VISIT(binop, v->V.binOp.right);
		spaces -= 4;
	}
	else if (v->type == ExprK_::JoinStrK) {
		print_space(spaces);
		printf("%s\n", "نص تنفيذي: ");
		spaces += 4;
		ExprTy js{};
		for (int i = 0; i < v->V.joinStr.vals->size; i++) {
			js = v->V.joinStr.vals->typedElements[i];
			VISIT(expr, js);
			spaces += 4;
		}
		spaces -= 4;
	}
	else if (v->type == ExprK_::FormattedValK) {
		VISIT(expr, v->V.formattedValue.val);
	}
	else if (v->type == ExprK_::TupleK) {
		print_space(spaces);
		printf("%s\n", "مترابطة: ");
		spaces += 4;
		ExprTy tp{};
		for (int i = 0; i < v->V.tuple.elts->size; i++) {
			tp = v->V.tuple.elts->typedElements[i];
			VISIT(expr, tp);
			spaces += 4;
		}
		spaces -= 4;
	}
	else if (v->type == ExprK_::ListK) {
		print_space(spaces);
		printf("%s\n", "مصفوفة: ");
		spaces += 4;
		ExprTy ls{};
		for (int i = 0; i < v->V.list.elts->size; i++) {
			ls = v->V.list.elts->typedElements[i];
			VISIT(expr, ls);
			spaces += 4;
		}
		spaces -= 4;
	}
	else if (v->type == ExprK_::DictK) {
		print_space(spaces);
		printf("%s\n", "قاموس: ");
		ExprTy key{};
		ExprTy val{};
		for (int i = 0; i < v->V.dict.keys->size; i++) {
			if (i % 2 == 1 or i == 0)
				printf(":-----------------------------\n");
			key = v->V.dict.keys->typedElements[i];
			spaces += 4;
			VISIT(expr, key);
			val = v->V.dict.vals->typedElements[i];
			spaces += 4;
			VISIT(expr, val);
			if (i%2 == 1 or i == v->V.dict.keys->size - 1)
				printf(":-----------------------------\n");
		}
		spaces -= 4;
	}
}

void parser_test(ModuleTy _p) {
	ASDLStmtSeq* m = _p->V.module.body;
	AlifSizeT size = _p->V.module.body->size;

	for (AlifSizeT i = 0; i < size; i++) {
		printf("الحالة %lld : \n", i);
		if (m->typedElements[i]->type == StmtK_::ExprK) {
			spaces += 4;
			VISIT(expr, m->typedElements[i]->V.expression.val);
			spaces = 0;
		}
	}

}
