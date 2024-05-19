#include "alif.h"


//#include "AlifParserEngine.h"
#include "StringParser.h"




////////////////////////////////////////////////////
/*
	هذه الدالة مؤقتة
	حيث تقوم بإرجاع اسم فارغ
	يتم إستخدامه مؤقتاً حتى يتوفر الأسم الحقيقي
	--> مثال: تستخدم مع استدعاء الدوال
*/
static AlifObject* a = new AlifObject{};

static Expression* dummyName = new Expression{};
#define INIT_DUMMY_NAME { \
	dummyName->type = NameK; \
	dummyName->V.name.name = a; \
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
////////////////////////////////////////////////////


Seq* alifParserEngine_singletonSeq(AlifParser* _p, void* _a) {
	Seq* seq = (Seq*)alifNew_genericSeq(1, _p->astMem);
	if (!seq) return nullptr;
	SEQ_SETUNTYPED(seq, 0, _a);
	return seq;
}

Seq* alifParserEngine_seqInsertInFront(AlifParser* _p, void* _a, Seq* _seq) {

	if (!_seq) {
		return alifParserEngine_singletonSeq(_p, _a);
	}

	Seq* newSeq = (Seq*)alifNew_genericSeq(SEQ_LEN(_seq) + 1, _p->astMem);
	if (!newSeq) return nullptr;

	SEQ_SETUNTYPED(newSeq, 0, _a);
	for (AlifUSizeT i = 1, l = SEQ_LEN(newSeq); i < l; i++) {
		SEQ_SETUNTYPED(newSeq, i, SEQ_GETUNTYPED(_seq, i - 1));
	}

	return newSeq;
}

static AlifSizeT getFlattenedSeq_size(Seq* _seqs) {
	AlifSizeT size = 0;
	for (AlifSizeT i = 0, l = SEQ_LEN(_seqs); i < l; i++) {
		Seq* innerSeq = (Seq*)SEQ_GETUNTYPED(_seqs, i);
		size += SEQ_LEN(innerSeq);
	}
	return size;
}

Seq* alifParserEngine_seqFlatten(AlifParser* _p, Seq* _seqs) {
	AlifSizeT flatSeqSize = getFlattenedSeq_size(_seqs);

	Seq* flattenedSeq = (Seq*)alifNew_genericSeq(flatSeqSize, _p->astMem);
	if (!flattenedSeq) {
		return nullptr;
	}
	int flatSeqIndex = 0;
	for (AlifSizeT i = 0, l = SEQ_LEN(_seqs); i < l; i++) {
		Seq* innerSeq = (Seq*)SEQ_GETUNTYPED(_seqs, i);
		for (AlifSizeT j = 0, li = SEQ_LEN(innerSeq); j < li; j++) {
			SEQ_SETUNTYPED(flattenedSeq, flatSeqIndex++, SEQ_GETUNTYPED(innerSeq, j));
		}
	}

	return flattenedSeq;
}

// Creates a new name like first_name.second_name
Expression* alifParserEngine_joinNamesWithDot(AlifParser* _p, Expression* _firstName, Expression* _secondName) {
	//AlifObject* str_ = alifUStr_fromFormat("%U.%U", _firstName->V.name.name, _secondName->V.name.name);
	//if (!str_) return nullptr;

	//alifUStr_InternInPlace(&str_);
	//if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
	//	ALIF_DECREF(str_);
	//	return nullptr;
	//}

	//return alifAST_name(str_, ExprCTX::Load, EXTRA_EXPR(_firstName, _secondName));
	return nullptr; // temp
}

Alias* alifParserEngine_aliasForStar(AlifParser* _p,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	AlifObject* str_ = alifUnicode_internFromString(L"*");
	if (!str_) return nullptr;

	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
		//ALIF_DECREF(str_);
		return nullptr;
	}
	return alifAST_alias(str_, nullptr, _lineNo, _colOffset, _endLineNo, _endColOffset, _astMem);
}

IdentifierSeq* alifParserEngine_mapNamesToIds(AlifParser* _p, ExprSeq* _seq) {
	AlifUSizeT len_ = SEQ_LEN(_seq);

	IdentifierSeq* newSeq = alifNew_identifierSeq(len_, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifUSizeT i = 0; i < len_; i++) {
		Expression* a_ = SEQ_GET(_seq, i);
		SEQ_SET(newSeq, i, a_->V.name.name);
	}

	return newSeq;
}

CompExprPair* alifParserEngine_compExprPair(AlifParser* _p, CmpOp _cmpOp, Expression* _expr) {

	CompExprPair* a_ = (CompExprPair*)alifASTMem_malloc(_p->astMem, sizeof(CompExprPair));
	if (!a_) return nullptr;

	a_->cmpOp = _cmpOp;
	a_->expr_ = _expr;

	return a_;
}

IntSeq* alifParserEngine_getCmpOps(AlifParser* _p, Seq* _seq) {
	AlifUSizeT len_ = SEQ_LEN(_seq);

	IntSeq* newSeq = alifNew_intSeq(len_, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifUSizeT i = 0; i < len_; i++) {
		CompExprPair* pair_ = (CompExprPair*)SEQ_GETUNTYPED(_seq, i);
		SEQ_SET(newSeq, i, pair_->cmpOp);
	}

	return newSeq;
}

ExprSeq* alifParserEngine_getExprs(AlifParser* _p, Seq* _seq) {
	AlifUSizeT len_ = SEQ_LEN(_seq);

	ExprSeq* newSeq = alifNew_exprSeq(len_, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifUSizeT i = 0; i < len_; i++) {
		CompExprPair* pair_ = (CompExprPair*)SEQ_GETUNTYPED(_seq, i);
		SEQ_SET(newSeq, i, pair_->expr_);
	}

	return newSeq;
}

static ExprSeq* setContext_seq(AlifParser* _p, ExprSeq* _seq, ExprCTX _ctx) {
	AlifUSizeT len = SEQ_LEN(_seq);
	if (len == 0) return nullptr;

	ExprSeq* newSeq = alifNew_exprSeq(len, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifUSizeT i = 0; i < len; i++) {
		Expression* e = SEQ_GET(_seq, i);
		SEQ_SET(newSeq, i, alifParserEngine_setExprContext(_p, e, _ctx));
	}
	return newSeq;
}

static Expression* setContext_name(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
	return alifAST_name(_expr->V.name.name, _ctx, EXTRA_EXPR(_expr, _expr));
}

static Expression* setContext_tuple(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
	return alifAST_tuple(setContext_seq(_p, _expr->V.tuple.elts, _ctx), _ctx, EXTRA_EXPR(_expr, _expr));
}

static Expression* setContext_list(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
	return alifAST_list(setContext_seq(_p, _expr->V.list.elts, _ctx), _ctx, EXTRA_EXPR(_expr, _expr));
}

static Expression* setContext_subScript(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
	return alifAST_subScript(_expr->V.subScript.val, _expr->V.subScript.slice, _ctx, EXTRA_EXPR(_expr, _expr));
}

static Expression* setContext_attribute(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
	return alifAST_attribute(_expr->V.attribute.val, _expr->V.attribute.attr, _ctx, EXTRA_EXPR(_expr, _expr));
}

static Expression* setContext_star(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
	return alifAST_star(alifParserEngine_setExprContext(_p, _expr->V.star.val, _ctx), _ctx, EXTRA_EXPR(_expr, _expr));
}

Expression* alifParserEngine_setExprContext(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
	Expression* new_{};
	switch (_expr->type) {
	case NameK:
		new_ = setContext_name(_p, _expr, _ctx);
		break;
	case TupleK:
		new_ = setContext_tuple(_p, _expr, _ctx);
		break;
	case ListK:
		new_ = setContext_list(_p, _expr, _ctx);
		break;
	case SubScriptK:
		new_ = setContext_subScript(_p, _expr, _ctx);
		break;
	case AttributeK:
		new_ = setContext_attribute(_p, _expr, _ctx);
		break;
	case StarK:
		new_ = setContext_star(_p, _expr, _ctx);
		break;
	default:
		new_ = _expr;
	}

	return new_;
}

KeyValuePair* alifParserEngine_keyValuePair(AlifParser* _p, Expression* _key, Expression* _val) {

	KeyValuePair* a_ = (KeyValuePair*)alifASTMem_malloc(_p->astMem, sizeof(KeyValuePair));
	if (!a_) return nullptr;

	a_->key_ = _key;
	a_->val_ = _val;

	return a_;
}

ExprSeq* alifParserEngine_getKeys(AlifParser* _p, Seq* _seq) {

	AlifUSizeT len_ = SEQ_LEN(_seq);
	ExprSeq* newSeq = alifNew_exprSeq(len_, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifUSizeT i = 0; i < len_; i++) {
		KeyValuePair* pair_ = (KeyValuePair*)SEQ_GETUNTYPED(_seq, i);
		SEQ_SET(newSeq, i, pair_->key_);
	}

	return newSeq;
}

ExprSeq* alifParserEngine_getValues(AlifParser* _p, Seq* _seq) {

	AlifUSizeT len_ = SEQ_LEN(_seq);
	ExprSeq* newSeq = alifNew_exprSeq(len_, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifUSizeT i = 0; i < len_; i++) {
		KeyValuePair* pair_ = (KeyValuePair*)SEQ_GETUNTYPED(_seq, i);
		SEQ_SET(newSeq, i, pair_->val_);
	}

	return newSeq;
}

NameDefaultPair* alifParserEngine_nameDefaultPair(AlifParser* _p, Arg* _arg, Expression* _value) {

	NameDefaultPair* a_ = (NameDefaultPair*)alifASTMem_malloc(_p->astMem, sizeof(NameDefaultPair));
	if (!a_) return nullptr;

	a_->arg_ = alifAST_arg(_arg->arg, _arg->lineNo, _arg->colOffset, _arg->endLineNo, _arg->endColOffset, _p->astMem);
	a_->value_ = _value;

	return a_;
}

StarEtc* alifParserEngine_starEtc(AlifParser* _p, Arg* _varArg, Seq* _kwOnlyArgs, Arg* _kwArg) {
	StarEtc* a_ = (StarEtc*)alifASTMem_malloc(_p->astMem, sizeof(StarEtc));
	if (!a_) return nullptr;

	a_->varArg = _varArg;
	a_->kwOnlyArgs = _kwOnlyArgs;
	a_->kwArg = _kwArg;

	return a_;
}

Seq* alifParserEngine_joinSequences(AlifParser* _p, Seq* _a, Seq* _b) {
	AlifUSizeT firstLen = SEQ_LEN(_a);
	AlifUSizeT secondLen = SEQ_LEN(_b);
	Seq* newSeq = (Seq*)alifNew_genericSeq(firstLen + secondLen, _p->astMem);
	if (!newSeq) return nullptr;

	AlifIntT k_ = 0;
	for (AlifUSizeT i = 0; i < firstLen; i++) {
		SEQ_SETUNTYPED(newSeq, k_++, SEQ_GETUNTYPED(_a, i));
	}
	for (AlifUSizeT i = 0; i < secondLen; i++) {
		SEQ_SETUNTYPED(newSeq, k_++, SEQ_GETUNTYPED(_b, i));
	}

	return newSeq;
}

static ArgSeq* get_names(AlifParser* _p, Seq* _namesWithDefaults) {
	AlifUSizeT len_ = SEQ_LEN(_namesWithDefaults);
	ArgSeq* seq_ = alifNew_argSeq(len_, _p->astMem);
	if (!seq_) return nullptr;

	for (AlifUSizeT i = 0; i < len_; i++) {
		NameDefaultPair* pair_ = (NameDefaultPair*)SEQ_GETUNTYPED(_namesWithDefaults, i);
		SEQ_SET(seq_, i, pair_->arg_);
	}
	return seq_;
}

static ExprSeq* get_defaults(AlifParser* _p, Seq* _namesWithDefaults) {

	AlifUSizeT len_ = SEQ_LEN(_namesWithDefaults);
	ExprSeq* seq_ = alifNew_exprSeq(len_, _p->astMem);
	if (!seq_) return nullptr;

	for (AlifUSizeT i = 0; i < len_; i++) {
		NameDefaultPair* pair_ = (NameDefaultPair*)SEQ_GETUNTYPED(_namesWithDefaults, i);
		SEQ_SET(seq_, i, pair_->value_);
	}

	return seq_;
}

static AlifIntT make_posOnlyArgs(AlifParser* _p, ArgSeq* _slashWithoutDefault,
	SlashWithDefault* slashWithDefault, ArgSeq** _posOnlyArgs) {

	if (_slashWithoutDefault != nullptr) {
		// code here
	}
	else if (slashWithDefault != nullptr) {
		// code here
	}
	else
	{
		*_posOnlyArgs = alifNew_argSeq(0, _p->astMem);
	}

	return *_posOnlyArgs == nullptr ? -1 : 0;
}

static AlifIntT make_posArgs(AlifParser* _p, ArgSeq* _plainNames,
	Seq* _namesWithDefault, ArgSeq** _posArgs) {

	if (_plainNames != nullptr and _namesWithDefault != nullptr) {
		ArgSeq* namesWithDefaultNames = get_names(_p, _namesWithDefault);
		if (!namesWithDefaultNames) return -1;
		*_posArgs = (ArgSeq*)alifParserEngine_joinSequences(_p, (Seq*)_plainNames, (Seq*)namesWithDefaultNames);
	}
	else if (_plainNames == nullptr and _namesWithDefault != nullptr) {
		*_posArgs = get_names(_p, _namesWithDefault);
	}
	else if (_plainNames != nullptr and _namesWithDefault == nullptr) {
		*_posArgs = _plainNames;
	}
	else {
		*_posArgs = alifNew_argSeq(0, _p->astMem);
	}
	return *_posArgs == nullptr ? -1 : 0;
}

static AlifIntT make_posDefaults(AlifParser* _p, SlashWithDefault* _slashWithDefault,
	Seq* _namesWithDefault, ExprSeq** _posDefaults) {

	if (_slashWithDefault != nullptr and _namesWithDefault != nullptr) {
		ExprSeq* slashWithDefaultValues = get_defaults(_p, _slashWithDefault->namesWithDefaults);
		if (!slashWithDefaultValues) return -1;

		ExprSeq* namesWithDefaultValues = get_defaults(_p, _namesWithDefault);
		if (!namesWithDefaultValues) return -1;

		*_posDefaults = (ExprSeq*)alifParserEngine_joinSequences(_p, (Seq*)slashWithDefaultValues, (Seq*)namesWithDefaultValues);
	}
	else if (_slashWithDefault == nullptr and _namesWithDefault != nullptr) {
		*_posDefaults = get_defaults(_p, _namesWithDefault);
	}
	else if (_slashWithDefault != nullptr and _namesWithDefault == nullptr) {
		*_posDefaults = get_defaults(_p, _slashWithDefault->namesWithDefaults);
	}
	else {
		*_posDefaults = alifNew_exprSeq(0, _p->astMem);
	}
	return *_posDefaults == nullptr ? -1 : 0;
}

static AlifIntT make_kwArgs(AlifParser* _p, StarEtc* _starEtc, ArgSeq** _kwOnlyArgs, ExprSeq** _kwDefaults) {
	if (_starEtc != nullptr and _starEtc->kwOnlyArgs != nullptr) {
		*_kwOnlyArgs = get_names(_p, _starEtc->kwOnlyArgs);
	}
	else {
		*_kwOnlyArgs = alifNew_argSeq(0, _p->astMem);
	}

	if (*_kwOnlyArgs == nullptr) return -1;

	if (_starEtc != nullptr and _starEtc->kwOnlyArgs != nullptr) {
		*_kwDefaults = get_defaults(_p, _starEtc->kwOnlyArgs);
	}
	else {
		*_kwDefaults = alifNew_exprSeq(0, _p->astMem);
	}

	if (*_kwDefaults == nullptr) return -1;

	return 0;
}

// Constructs an arguments_ty object out of all the parsed constructs in the parameters rule
Arguments* alifParserEngine_makeArguments(AlifParser* _p, ArgSeq* _slashWithoutDefault, SlashWithDefault* _slashWithDefault,
	ArgSeq* _plainNames, Seq* _namesWithDefault, StarEtc* _starEtc) {

	ArgSeq* posOnlyArgs{};
	if (make_posOnlyArgs(_p, _slashWithoutDefault, _slashWithDefault, &posOnlyArgs) == -1) return nullptr;

	ArgSeq* posArgs{};
	if (make_posArgs(_p, _plainNames, _namesWithDefault, &posArgs) == -1) return nullptr;

	ExprSeq* posDefaults{};
	if (make_posDefaults(_p, _slashWithDefault, _namesWithDefault, &posDefaults) == -1) return nullptr;

	Arg* varArg{};
	if (_starEtc != nullptr and _starEtc->varArg != nullptr) {
		varArg = _starEtc->varArg;
	}

	ArgSeq* kwOnlyArgs{};
	ExprSeq* kwDefaults{};
	if (make_kwArgs(_p, _starEtc, &kwOnlyArgs, &kwDefaults) == -1) {
		return nullptr;
	}

	Arg* kwArg{};
	if (_starEtc != nullptr and _starEtc->kwArg != nullptr) {
		kwArg = _starEtc->kwArg;
	}

	return alifAST_arguments(posOnlyArgs, posArgs, varArg, kwOnlyArgs, kwDefaults, kwArg, posDefaults, _p->astMem);
}

Arguments* alifParserEngine_emptyArguments(AlifParser* _p) {

	ArgSeq* posOnlyArgs = alifNew_argSeq(0, _p->astMem);
	if (!posOnlyArgs) return nullptr;

	ArgSeq* posArgs = alifNew_argSeq(0, _p->astMem);
	if (!posArgs) return nullptr;

	ExprSeq* posDefaults = alifNew_exprSeq(0, _p->astMem);
	if (!posDefaults) return nullptr;

	ArgSeq* kwOnlyArgs = alifNew_argSeq(0, _p->astMem);
	if (!kwOnlyArgs) return nullptr;

	ExprSeq* kwDefaults = alifNew_exprSeq(0, _p->astMem);
	if (!kwDefaults) return nullptr;


	return alifAST_arguments(posOnlyArgs, posArgs, nullptr, kwOnlyArgs,
		kwDefaults, nullptr, posDefaults, _p->astMem);
}

AugOperator* alifParserEngine_augOperator(AlifParser* _p, Operator _type) {

	AugOperator* a_ = (AugOperator*)alifASTMem_malloc(_p->astMem, sizeof(AugOperator));
	if (!a_) return nullptr;

	a_->type = _type;
	return a_;
}

static AlifIntT seqNumber_ofStarExprs(Seq* _seq) {
	AlifIntT n_ = 0;
	for (AlifUSizeT i = 0, l = SEQ_LEN(_seq); i < l; i++) {
		KeywordOrStar* k_ = (KeywordOrStar*)SEQ_GETUNTYPED(_seq, i);
		if (!k_->isKeyword) {
			n_++;
		}
	}
	return n_;
}

KeywordOrStar* alifParserEngine_keywordOrStarred(AlifParser* _p, void* _element, AlifIntT _isKeyword) { 

	KeywordOrStar* a_ = (KeywordOrStar*)alifASTMem_malloc(_p->astMem, sizeof(KeywordOrStar));
	if (!a_) return nullptr;

	a_->element = _element;
	a_->isKeyword = _isKeyword;

	return a_;
}

ExprSeq* alifParserEngine_seqExtractStarExprs(AlifParser* _p, Seq* _kwArgs) { 
	AlifIntT newLen = seqNumber_ofStarExprs(_kwArgs);
	if (newLen == 0) return nullptr;

	ExprSeq* newSeq = alifNew_exprSeq(newLen, _p->astMem);
	if (!newSeq) return nullptr;

	AlifIntT index_ = 0;
	for (AlifUSizeT i = 0, len_ = SEQ_LEN(_kwArgs); i < len_; i++) {
		KeywordOrStar* k_ = (KeywordOrStar*)SEQ_GETUNTYPED(_kwArgs, i);
		if (!k_->isKeyword) {
			SEQ_SET(newSeq, index_++, (Expression*)k_->element);
		}
	}

	return newSeq;
}

KeywordSeq* alifParserEngine_seqDeleteStarExprs(AlifParser* _p, Seq* _kwArgs) {

	AlifUSizeT len_ = SEQ_LEN(_kwArgs);
	AlifIntT newLen = seqNumber_ofStarExprs(_kwArgs);
	if (newLen == 0) return nullptr;

	KeywordSeq* newSeq = alifNew_keywordSeq(newLen, _p->astMem);
	if (!newSeq) return nullptr;

	AlifIntT index_ = 0;
	for (AlifUSizeT i = 0; i < len_; i++) {
		KeywordOrStar* k_ = (KeywordOrStar*)SEQ_GETUNTYPED(_kwArgs, i);
		if (!k_->isKeyword) {
			SEQ_SET(newSeq, index_++, (Keyword*)k_->element);
		}
	}

	return newSeq;
}

Module* alifParserEngine_makeModule(AlifParser* _p, StmtSeq* _a) {
	AlifSizeT num = _p->typeIgnoreComments.numItems;
	if (num > 0) {
		/*
		..
		..
		..
		*/
	}

	return alifAST_module(_a, _p->astMem);
}

static ResultTokenWithMetadata* resultToken_withMetadata(AlifParser* _p, void* _result, AlifObject* _metadata) {

	ResultTokenWithMetadata* res_ = (ResultTokenWithMetadata*)alifASTMem_malloc(_p->astMem, sizeof(ResultTokenWithMetadata));
	if (res_ == nullptr) return nullptr;

	res_->metadata = _metadata;
	res_->result = _result;
	return res_;
}

ResultTokenWithMetadata* alifParserEngine_checkFStringConversion(AlifParser* _p, AlifPToken* _convTok, Expression* _conv) {

	if (_convTok->lineNo != _conv->lineNo or _convTok->endColOffset != _conv->colOffset) {
		// error
		return nullptr; //
	}

	return resultToken_withMetadata(_p, _conv, _convTok->data);
}

ResultTokenWithMetadata* alifParserEngine_setupFullFormatSpec(AlifParser* _p, AlifPToken* _lit, ExprSeq* _spec,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	if (!_spec) return nullptr;

	/*
	.
	.
	.
	*/

	Expression* res_ = alifAST_joinedStr(_spec, _lineNo, _colOffset, _endLineNo, _endColOffset, _p->astMem);
	if (!res_) return nullptr;

	return resultToken_withMetadata(_p, res_, _lit->data);
}

Expression* alifParserEngine_collectCallSeqs(AlifParser* _p, ExprSeq* _a, Seq* _b,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	AlifUSizeT argsLen = SEQ_LEN(_a);
	AlifUSizeT totalLen = argsLen;

	if (_b == nullptr)
	{
		return alifAST_call(alifParserEngine_dummyName(), _a, nullptr,
			_lineNo, _colOffset, _endLineNo, _endColOffset, _astMem);
	}

	ExprSeq* stars_ = alifParserEngine_seqExtractStarExprs(_p, _b);
	KeywordSeq* keywords_ = alifParserEngine_seqDeleteStarExprs(_p, _b);

	if (stars_) totalLen += SEQ_LEN(stars_);

	ExprSeq* args_ = alifNew_exprSeq(totalLen, _astMem);

	AlifUSizeT i = 0;
	for (i = 0; i < argsLen; i++) {
		SEQ_SET(args_, i, SEQ_GET(_a, i));
	}
	for (; i < totalLen; i++) {
		SEQ_SET(args_, i, SEQ_GET(stars_, i - argsLen));
	}

	return alifAST_call(alifParserEngine_dummyName(), args_, keywords_,
		_lineNo, _colOffset, _endLineNo, _endColOffset, _astMem);
}

static Expression* alifParserEngine_decodeFStringPart(AlifParser* _p, AlifIntT _isRaw, Expression* _const, AlifPToken* _token) {

	const wchar_t* bStr = alifUnicode_asUTF8(_const->V.constant.val);
	if (bStr == nullptr) return nullptr;

	AlifUSizeT len_;
	if (wcscmp(bStr, L"{{") == 0 or wcscmp(bStr, L"}}") == 0) {
		len_ = 1;
	}
	else {
		len_ = wcslen(bStr);
	}

	_isRaw = _isRaw or wcschr(bStr, L'\\') == nullptr;
	AlifObject* str_ = alifParserEngine_decodeString(_p, _isRaw, bStr, len_, _token);
	if (str_ == nullptr) {
		//alifParserEngine_raiseDecodeError(_p);
		return nullptr;
	}
	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
		ALIF_DECREF(str_);
		return nullptr;
	}
	return alifAST_constant(str_, nullptr, _const->lineNo, _const->colOffset,
		_const->endLineNo, _const->endColOffset, _p->astMem);
}

static ExprSeq* unpackTopLevel_joinedStrs(AlifParser* _p, ExprSeq* _rawExpr) {
	// هذه الدالة غير ضرورية ويجب مراجعة عملها
	/* The parser might put multiple f-string values into an individual
	 * JoinedStr node at the top level due to stuff like f-string debugging
	 * expressions. This function flattens those and promotes them to the
	 * upper level. Only simplifies AST, but the compiler already takes care
	 * of the regular output, so this is not necessary if you are not going
	 * to expose the output AST to Alif level. */

	AlifSizeT i, reqSize, rawSize;

	reqSize = rawSize = SEQ_LEN(_rawExpr);
	Expression* expr_;
	for (i = 0; i < rawSize; i++) {
		expr_ = SEQ_GET(_rawExpr, i);
		if (expr_->type == JoinStrK) {
			reqSize += SEQ_LEN(expr_->V.joinStr.vals) - 1;
		}
	}

	ExprSeq* expressions = alifNew_exprSeq(reqSize, _p->astMem);

	AlifSizeT raw_index, req_index = 0;
	for (raw_index = 0; raw_index < rawSize; raw_index++) {
		expr_ = SEQ_GET(_rawExpr, raw_index);
		if (expr_->type == JoinStrK) {
			ExprSeq* values = expr_->V.joinStr.vals;
			for (AlifSizeT n = 0; n < SEQ_LEN(values); n++) {
				SEQ_SET(expressions, req_index, SEQ_GET(values, n));
				req_index++;
			}
		}
		else {
			SEQ_SET(expressions, req_index, expr_);
			req_index++;
		}
	}
	return expressions;
}

Expression* alifParserEngine_joinedStr(AlifParser* _p, AlifPToken* _a, ExprSeq* _rawExprs, AlifPToken* _b) {
	ExprSeq* expr_ = unpackTopLevel_joinedStrs(_p, _rawExprs);
	AlifUSizeT nItems = SEQ_LEN(expr_);

	const wchar_t* quoteStr = alifWBytes_asString(_a->bytes);
	if (quoteStr == nullptr) return nullptr;

	AlifIntT isRaw = wcspbrk(quoteStr, L"خ") != nullptr;

	ExprSeq* seq_ = alifNew_exprSeq(nItems, _p->astMem);
	if (seq_ == nullptr) return nullptr;


	AlifUSizeT index_ = 0;
	for (AlifUSizeT i = 0; i < nItems; i++) {
		Expression* item_ = SEQ_GET(expr_, i);
		if (item_->type == ConstantK) {
			item_ = alifParserEngine_decodeFStringPart(_p, isRaw, item_, _b);
			if (item_ == nullptr) return nullptr;

			/* Tokenizer emits string parts even when the underlying string
			might become an empty value (e.g. FSTRING_MIDDLE with the value \\n)
			so we need to check for them and simplify it here. */
			if (ALIFUNICODE_CHECK_TYPE(item_->V.constant.val) and
				ALIFUNICODE_GET_LENGTH(item_->V.constant.val) == 0)
				continue;
		}
		SEQ_SET(seq_, index_++, item_);
	}

	ExprSeq* resizedExprs{};
	if (index_ != nItems) {
		resizedExprs = alifNew_exprSeq(index_, _p->astMem);
		if (resizedExprs == nullptr) return nullptr;

		for (AlifUSizeT i = 0; i < index_; i++) {
			SEQ_SET(resizedExprs, i, SEQ_GET(seq_, i));
		}
	}
	else {
		resizedExprs = seq_;
	}

	return alifAST_joinedStr(resizedExprs, _a->lineNo, _a->colOffset, _b->endLineNo, _b->endColOffset, _p->astMem);
}

Expression* alifParserEngine_decodeConstantFromToken(AlifParser* _p, AlifPToken* _t) {

	AlifSizeT bSize{};
	wchar_t* bStr{};

	if (alifWBytes_asStringAndSize(_t->bytes, &bStr, &bSize) == -1) return nullptr;

	AlifObject* str_ = alifParserEngine_decodeString(_p, 0, bStr, bSize, _t);
	if (str_ == nullptr) return nullptr;

	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
		ALIF_DECREF(str_);
		return nullptr;
	}

	return alifAST_constant(str_, nullptr, _t->lineNo, _t->colOffset, _t->endLineNo, _t->endColOffset, _p->astMem);
}

Expression* alifParserEngine_constantFromToken(AlifParser* _p, AlifPToken* _t) {
	wchar_t* bStr = (wchar_t*)alifWBytes_asString(_t->bytes);
	if (bStr == nullptr) return nullptr;

	AlifObject* str_ = alifUStr_fromString(bStr);
	if (str_ == nullptr) return nullptr;

	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
		ALIF_DECREF(str_);
		return nullptr;
	}

	return alifAST_constant(str_, nullptr, _t->lineNo, _t->colOffset, _t->endLineNo, _t->endColOffset, _p->astMem);
}

Expression* alifParserEngine_constantFromString(AlifParser* _p, AlifPToken* _tok) {
	wchar_t* str = alifWBytes_asString(_tok->bytes);
	if (str == nullptr) return nullptr;

	AlifObject* s = alifParserEngine_parseString(_p, _tok);
	if (s == nullptr) return nullptr;
	if (alifASTMem_listAddAlifObj(_p->astMem, s) < 0) {
		ALIF_DECREF(s);
		return nullptr;
	}
	AlifObject* type{};
	if (str and str[0] == L'م') {
		type = alifParserEngine_newIdentifier(_p, L"م");
		if (type == nullptr) return nullptr;
	}
	return alifAST_constant(s, type, _tok->lineNo, _tok->colOffset, _tok->endLineNo, _tok->endColOffset, _p->astMem);
}

Expression* alifParserEngine_formattedValue(AlifParser* _p, Expression* _expr, AlifPToken* _d,
	ResultTokenWithMetadata* _c, ResultTokenWithMetadata* _f, AlifPToken* _closingBrace,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	AlifIntT conversionVal = -1;
	if (_c != nullptr) {
		Expression* conversionExpr = (Expression*)_c->result;
		AlifUCS4 first = ALIFUNICODE_READ_WCHAR(conversionExpr->V.name.name, 0);

		/*
		.
		.
		*/
	}
	else if (_d and _f) {
		conversionVal = (AlifIntT)L'ر';
	}

	Expression* formattedValue = alifAST_formattedValue(_expr, conversionVal,
		_f ? (Expression*)_f->result : nullptr,
		_lineNo, _colOffset, _endLineNo, _endColOffset, _astMem);

	if (_d) {
		/*
		.
		.
		*/
	}
	else {
		return formattedValue;
	}
}

Expression* alifParserEngine_combineStrings(AlifParser* _p, ExprSeq* _strings,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	AlifUSizeT len_ = SEQ_LEN(_strings);

	AlifIntT fStringFound{};
	AlifIntT unicodeStringFound{};
	AlifIntT bytesFound{};

	AlifUSizeT i = 0;
	AlifUSizeT nFlattenedElems = 0;
	for (i = 0; i < len_; i++) {
		Expression* elem = SEQ_GET(_strings, i);
		if (elem->type == ConstantK) {
			if (ALIFBYTES_CHECK(elem->V.constant.val)) {
				bytesFound = 1;
			}
			else {
				unicodeStringFound = 1;
			}
			nFlattenedElems++;
		}
		else {
			nFlattenedElems += SEQ_LEN(elem->V.joinStr.vals);
			fStringFound = 1;
		}
	}

	if ((unicodeStringFound or fStringFound) and bytesFound) {
		// error
		return nullptr;
	}

	if (bytesFound) {
		AlifObject* res_ = alifBytes_fromString(L"");

		/* Bytes literals never get a type, but just for consistency
	   since they are represented as Constant nodes, we'll mirror
	   the same behavior as unicode strings for determining the type. */
		AlifObject* type_ = SEQ_GET(_strings, 0)->V.constant.type;
		for (i = 0; i < len_; i++) {
			Expression* element_ = SEQ_GET(_strings, i);
			alifBytes_concat(&res_, element_->V.constant.val);
		}
		if (!res_ or alifASTMem_listAddAlifObj(_astMem, res_) < 0) {
			ALIF_XDECREF(res_);
			return nullptr;
		}
		return alifAST_constant(res_, type_, _lineNo, _colOffset, _endLineNo, _endColOffset, _p->astMem);
	}

	if (!fStringFound and len_ == 1) {
		return SEQ_GET(_strings, 0);
	}

	ExprSeq* flattened_ = alifNew_exprSeq(nFlattenedElems, _p->astMem);
	if (flattened_ == nullptr) return nullptr;

	/* build flattened list */
	AlifUSizeT currentPos = 0;
	AlifUSizeT j = 0;
	for (i = 0; i < len_; i++) {
		Expression* element_ = SEQ_GET(_strings, i);
		if (element_->type == ConstantK) {
			SEQ_SET(flattened_, currentPos++, element_);
		}
		else {
			for (j = 0; j < SEQ_LEN(element_->V.joinStr.vals); j++) {
				Expression* subVal = SEQ_GET(element_->V.joinStr.vals, j);
				if (subVal == nullptr) return nullptr;

				SEQ_SET(flattened_, currentPos++, subVal);
			}
		}
	}

	/* calculate folded element count */
	AlifUSizeT nElements = 0;
	AlifIntT prevIsConstant = 0;
	for (i = 0; i < nFlattenedElems; i++) {
		Expression* element_ = SEQ_GET(flattened_, i);

		if (fStringFound and element_->type == ConstantK and
			ALIFUNICODE_CHECK_TYPE(element_->V.constant.val) and
			ALIFUNICODE_GET_LENGTH(element_->V.constant.val) == 0)
			continue;

		if (!prevIsConstant or element_->type != ConstantK) nElements++;

		prevIsConstant = element_->type == ConstantK;
	}

	ExprSeq* values_ = alifNew_exprSeq(nElements, _p->astMem);
	if (values_ == nullptr) return nullptr;

	/* build folded list */
	// code here

	if (fStringFound) {
		Expression* element_ = SEQ_GET(values_, 0);
		return element_;
	}

	return alifAST_joinedStr(values_, _lineNo, _colOffset, _endLineNo, _endColOffset, _p->astMem);
}
