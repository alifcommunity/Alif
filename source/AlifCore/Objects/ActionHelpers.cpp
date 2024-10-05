#include "alif.h"


#include "AlifParserEngine.h"
#include "StringParser.h"
#include "AlifCore_State.h"


void* alifParserEngine_dummyName(AlifParser* p, ...) { // 8
	return &_alifDureRun_.parser.dummyName;
}

ASDLSeq* alifParserEngine_singletonSeq(AlifParser* _p, void* _a) { // 15
	ASDLSeq* seq = (ASDLSeq*)alifNew_genericSeq(1, _p->astMem);
	if (!seq) return nullptr;
	ASDL_SEQ_SETUNTYPED(seq, 0, _a);
	return seq;
}

ASDLSeq* alifParserEngine_seqInsertInFront(AlifParser* _p,
	void* _a, ASDLSeq* _seq) { // 28

	if (!_seq) {
		return alifParserEngine_singletonSeq(_p, _a);
	}

	ASDLSeq* newSeq = (ASDLSeq*)alifNew_genericSeq(ASDL_SEQ_LEN(_seq) + 1, _p->astMem);
	if (!newSeq) return nullptr;

	ASDL_SEQ_SETUNTYPED(newSeq, 0, _a);
	for (AlifUSizeT i = 1, l = ASDL_SEQ_LEN(newSeq); i < l; i++) {
		ASDL_SEQ_SETUNTYPED(newSeq, i, ASDL_SEQ_GETUNTYPED(_seq, i - 1));
	}

	return newSeq;
}

//static AlifSizeT getFlattenedSeq_size(ASDLSeq* _seqs) {
//	AlifSizeT size = 0;
//	for (AlifSizeT i = 0, l = SEQ_LEN(_seqs); i < l; i++) {
//		ASDLSeq* innerSeq = (ASDLSeq*)SEQ_GETUNTYPED(_seqs, i);
//		size += SEQ_LEN(innerSeq);
//	}
//	return size;
//}

ASDLSeq* alifParserEngine_seqFlatten(AlifParser* _p, ASDLSeq* _seqs) { // 81
	AlifSizeT flatSeqSize = getFlattenedSeq_size(_seqs);

	ASDLSeq* flattenedSeq = (ASDLSeq*)alifNew_genericSeq(flatSeqSize, _p->astMem);
	if (!flattenedSeq) return nullptr;

	AlifIntT flatSeqIndex = 0;
	for (AlifSizeT i = 0, l = ASDL_SEQ_LEN(_seqs); i < l; i++) {
		ASDLSeq* innerSeq = (ASDLSeq*)ASDL_SEQ_GETUNTYPED(_seqs, i);
		for (AlifSizeT j = 0, li = ASDL_SEQ_LEN(innerSeq); j < li; j++) {
			ASDL_SEQ_SETUNTYPED(flattenedSeq, flatSeqIndex++, ASDL_SEQ_GETUNTYPED(innerSeq, j));
		}
	}

	return flattenedSeq;
}

// Creates a new name like first_name.second_name
ExprTy alifParserEngine_joinNamesWithDot(AlifParser* _p,
	ExprTy _firstName, ExprTy _secondName) { // 118
	AlifObject* str_ = alifUStr_fromFormat("%U.%U", _firstName->V.name.name, _secondName->V.name.name);
	if (!str_) return nullptr;

	AlifInterpreter* interp = _alifInterpreter_get();
	alifUStr_internImmortal(interp, &str_);
	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
		ALIF_DECREF(str_);
		return nullptr;
	}

	return alifAST_name(str_, ExprContext_::Load, EXTRA_EXPR(_firstName, _secondName));
}

AliasTy alifParserEngine_aliasForStar(AlifParser* _p,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 160

	AlifObject* str_ = alifUStr_internFromString("*");
	if (!str_) return nullptr;

	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
		ALIF_DECREF(str_);
		return nullptr;
	}
	return alifAST_alias(str_, nullptr, _lineNo, _colOffset, _endLineNo, _endColOffset, _astMem);
}

ASDLIdentifierSeq* alifParserEngine_mapNamesToIds(AlifParser* _p,
	ASDLExprSeq* _seq) { // 175

	AlifUSizeT len_ = ASDL_SEQ_LEN(_seq);
	ASDLIdentifierSeq* newSeq = alifNew_identifierSeq(len_, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifUSizeT i = 0; i < len_; i++) {
		ExprTy a_ = ASDL_SEQ_GET(_seq, i);
		ASDL_SEQ_SET(newSeq, i, a_->V.name.name);
	}

	return newSeq;
}

CompExprPair* alifParserEngine_compExprPair(AlifParser* _p, CmpOp_ _cmpOp, ExprTy _expr) { // 193

	CompExprPair* a_ = (CompExprPair*)alifASTMem_malloc(_p->astMem, sizeof(CompExprPair));
	if (!a_) return nullptr;

	a_->cmpOp = _cmpOp;
	a_->expr = _expr;

	return a_;
}

ASDLIntSeq* alifParserEngine_getCmpOps(AlifParser* _p, ASDLSeq* _seq) { // 206
	AlifSizeT len_ = ASDL_SEQ_LEN(_seq);

	ASDLIntSeq* newSeq = alifNew_intSeq(len_, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifSizeT i = 0; i < len_; i++) {
		CompExprPair* pair = (CompExprPair*)ASDL_SEQ_GETUNTYPED(_seq, i);
		ASDL_SEQ_SET(newSeq, i, pair->cmpOp);
	}

	return newSeq;
}

ASDLExprSeq* alifParserEngine_getExprs(AlifParser* _p, ASDLSeq* _seq) { // 223
	AlifSizeT len_ = ASDL_SEQ_LEN(_seq);

	ASDLExprSeq* newSeq = alifNew_exprSeq(len_, _p->astMem);
	if (!newSeq) return nullptr;

	for (AlifSizeT i = 0; i < len_; i++) {
		CompExprPair* pair_ = (CompExprPair*)ASDL_SEQ_GETUNTYPED(_seq, i);
		ASDL_SEQ_SET(newSeq, i, pair_->expr);
	}

	return newSeq;
}

//static ASDLExprSeq* setContext_seq(AlifParser* _p, ASDLExprSeq* _seq, ExprCTX _ctx) {
//	AlifUSizeT len = SEQ_LEN(_seq);
//	if (len == 0) return nullptr;
//
//	ASDLExprSeq* newSeq = alifNew_exprSeq(len, _p->astMem);
//	if (!newSeq) return nullptr;
//
//	for (AlifUSizeT i = 0; i < len; i++) {
//		Expression* e = SEQ_GET(_seq, i);
//		SEQ_SET(newSeq, i, alifParserEngine_setExprContext(_p, e, _ctx));
//	}
//	return newSeq;
//}
//
//static Expression* setContext_name(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
//	return alifAST_name(_expr->V.name.name, _ctx, EXTRA_EXPR(_expr, _expr));
//}
//
//static Expression* setContext_tuple(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
//	return alifAST_tuple(setContext_seq(_p, _expr->V.tuple.elts, _ctx), _ctx, EXTRA_EXPR(_expr, _expr));
//}
//
//static Expression* setContext_list(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
//	return alifAST_list(setContext_seq(_p, _expr->V.list.elts, _ctx), _ctx, EXTRA_EXPR(_expr, _expr));
//}
//
//static Expression* setContext_subScript(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
//	return alifAST_subScript(_expr->V.subScript.val, _expr->V.subScript.slice, _ctx, EXTRA_EXPR(_expr, _expr));
//}
//
//static Expression* setContext_attribute(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
//	return alifAST_attribute(_expr->V.attribute.val, _expr->V.attribute.attr, _ctx, EXTRA_EXPR(_expr, _expr));
//}
//
//static Expression* setContext_star(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
//	return alifAST_star(alifParserEngine_setExprContext(_p, _expr->V.star.val, _ctx), _ctx, EXTRA_EXPR(_expr, _expr));
//}
//
//Expression* alifParserEngine_setExprContext(AlifParser* _p, Expression* _expr, ExprCTX _ctx) {
//	Expression* new_{};
//	switch (_expr->type) {
//	case NameK:
//		new_ = setContext_name(_p, _expr, _ctx);
//		break;
//	case TupleK:
//		new_ = setContext_tuple(_p, _expr, _ctx);
//		break;
//	case ListK:
//		new_ = setContext_list(_p, _expr, _ctx);
//		break;
//	case SubScriptK:
//		new_ = setContext_subScript(_p, _expr, _ctx);
//		break;
//	case AttributeK:
//		new_ = setContext_attribute(_p, _expr, _ctx);
//		break;
//	case StarK:
//		new_ = setContext_star(_p, _expr, _ctx);
//		break;
//	default:
//		new_ = _expr;
//	}
//
//	return new_;
//}
//
//KeyValuePair* alifParserEngine_keyValuePair(AlifParser* _p, Expression* _key, Expression* _val) {
//
//	KeyValuePair* a_ = (KeyValuePair*)alifASTMem_malloc(_p->astMem, sizeof(KeyValuePair));
//	if (!a_) return nullptr;
//
//	a_->key_ = _key;
//	a_->val_ = _val;
//
//	return a_;
//}
//
//ASDLExprSeq* alifParserEngine_getKeys(AlifParser* _p, ASDLSeq* _seq) {
//
//	AlifUSizeT len_ = SEQ_LEN(_seq);
//	ASDLExprSeq* newSeq = alifNew_exprSeq(len_, _p->astMem);
//	if (!newSeq) return nullptr;
//
//	for (AlifUSizeT i = 0; i < len_; i++) {
//		KeyValuePair* pair_ = (KeyValuePair*)SEQ_GETUNTYPED(_seq, i);
//		SEQ_SET(newSeq, i, pair_->key_);
//	}
//
//	return newSeq;
//}
//
//ASDLExprSeq* alifParserEngine_getValues(AlifParser* _p, ASDLSeq* _seq) {
//
//	AlifUSizeT len_ = SEQ_LEN(_seq);
//	ASDLExprSeq* newSeq = alifNew_exprSeq(len_, _p->astMem);
//	if (!newSeq) return nullptr;
//
//	for (AlifUSizeT i = 0; i < len_; i++) {
//		KeyValuePair* pair_ = (KeyValuePair*)SEQ_GETUNTYPED(_seq, i);
//		SEQ_SET(newSeq, i, pair_->val_);
//	}
//
//	return newSeq;
//}

NameDefaultPair* alifParserEngine_nameDefaultPair(AlifParser* _p,
	ArgTy _arg, ExprTy _value) { // 428

	NameDefaultPair* a_ = (NameDefaultPair*)alifASTMem_malloc(_p->astMem, sizeof(NameDefaultPair));
	if (!a_) return nullptr;

	a_->arg = alifAST_arg(_arg->arg, _arg->lineNo, _arg->colOffset, _arg->endLineNo, _arg->endColOffset, _p->astMem);
	a_->value = _value;

	return a_;
}

StarEtc* alifParserEngine_starEtc(AlifParser* _p, ArgTy _varArg,
	ASDLSeq* _kwOnlyArgs, ArgTy _kwArg) { // 454
	StarEtc* a_ = (StarEtc*)alifASTMem_malloc(_p->astMem, sizeof(StarEtc));
	if (!a_) return nullptr;

	a_->varArg = _varArg;
	a_->kwOnlyArgs = _kwOnlyArgs;
	a_->kwArg = _kwArg;

	return a_;
}

//ASDLSeq* alifParserEngine_joinSequences(AlifParser* _p, ASDLSeq* _a, ASDLSeq* _b) {
//	AlifUSizeT firstLen = SEQ_LEN(_a);
//	AlifUSizeT secondLen = SEQ_LEN(_b);
//	ASDLSeq* newSeq = (ASDLSeq*)alifNew_genericSeq(firstLen + secondLen, _p->astMem);
//	if (!newSeq) return nullptr;
//
//	AlifIntT k_ = 0;
//	for (AlifUSizeT i = 0; i < firstLen; i++) {
//		SEQ_SETUNTYPED(newSeq, k_++, SEQ_GETUNTYPED(_a, i));
//	}
//	for (AlifUSizeT i = 0; i < secondLen; i++) {
//		SEQ_SETUNTYPED(newSeq, k_++, SEQ_GETUNTYPED(_b, i));
//	}
//
//	return newSeq;
//}
//
//static ASDLArgSeq* get_names(AlifParser* _p, ASDLSeq* _namesWithDefaults) {
//	AlifUSizeT len_ = SEQ_LEN(_namesWithDefaults);
//	ASDLArgSeq* seq_ = alifNew_argSeq(len_, _p->astMem);
//	if (!seq_) return nullptr;
//
//	for (AlifUSizeT i = 0; i < len_; i++) {
//		NameDefaultPair* pair_ = (NameDefaultPair*)SEQ_GETUNTYPED(_namesWithDefaults, i);
//		SEQ_SET(seq_, i, pair_->arg_);
//	}
//	return seq_;
//}
//
//static ASDLExprSeq* get_defaults(AlifParser* _p, ASDLSeq* _namesWithDefaults) {
//
//	AlifUSizeT len_ = SEQ_LEN(_namesWithDefaults);
//	ASDLExprSeq* seq_ = alifNew_exprSeq(len_, _p->astMem);
//	if (!seq_) return nullptr;
//
//	for (AlifUSizeT i = 0; i < len_; i++) {
//		NameDefaultPair* pair_ = (NameDefaultPair*)SEQ_GETUNTYPED(_namesWithDefaults, i);
//		SEQ_SET(seq_, i, pair_->value_);
//	}
//
//	return seq_;
//}
//
//static AlifIntT make_posOnlyArgs(AlifParser* _p, ASDLArgSeq* _slashWithoutDefault,
//	SlashWithDefault* slashWithDefault, ASDLArgSeq** _posOnlyArgs) {
//
//	if (_slashWithoutDefault != nullptr) {
//		// code here
//	}
//	else if (slashWithDefault != nullptr) {
//		// code here
//	}
//	else
//	{
//		*_posOnlyArgs = alifNew_argSeq(0, _p->astMem);
//	}
//
//	return *_posOnlyArgs == nullptr ? -1 : 0;
//}
//
//static AlifIntT make_posArgs(AlifParser* _p, ASDLArgSeq* _plainNames,
//	ASDLSeq* _namesWithDefault, ASDLArgSeq** _posArgs) {
//
//	if (_plainNames != nullptr and _namesWithDefault != nullptr) {
//		ASDLArgSeq* namesWithDefaultNames = get_names(_p, _namesWithDefault);
//		if (!namesWithDefaultNames) return -1;
//		*_posArgs = (ASDLArgSeq*)alifParserEngine_joinSequences(_p, (ASDLSeq*)_plainNames, (ASDLSeq*)namesWithDefaultNames);
//	}
//	else if (_plainNames == nullptr and _namesWithDefault != nullptr) {
//		*_posArgs = get_names(_p, _namesWithDefault);
//	}
//	else if (_plainNames != nullptr and _namesWithDefault == nullptr) {
//		*_posArgs = _plainNames;
//	}
//	else {
//		*_posArgs = alifNew_argSeq(0, _p->astMem);
//	}
//	return *_posArgs == nullptr ? -1 : 0;
//}
//
//static AlifIntT make_posDefaults(AlifParser* _p, SlashWithDefault* _slashWithDefault,
//	ASDLSeq* _namesWithDefault, ASDLExprSeq** _posDefaults) {
//
//	if (_slashWithDefault != nullptr and _namesWithDefault != nullptr) {
//		ASDLExprSeq* slashWithDefaultValues = get_defaults(_p, _slashWithDefault->namesWithDefaults);
//		if (!slashWithDefaultValues) return -1;
//
//		ASDLExprSeq* namesWithDefaultValues = get_defaults(_p, _namesWithDefault);
//		if (!namesWithDefaultValues) return -1;
//
//		*_posDefaults = (ASDLExprSeq*)alifParserEngine_joinSequences(_p, (ASDLSeq*)slashWithDefaultValues, (ASDLSeq*)namesWithDefaultValues);
//	}
//	else if (_slashWithDefault == nullptr and _namesWithDefault != nullptr) {
//		*_posDefaults = get_defaults(_p, _namesWithDefault);
//	}
//	else if (_slashWithDefault != nullptr and _namesWithDefault == nullptr) {
//		*_posDefaults = get_defaults(_p, _slashWithDefault->namesWithDefaults);
//	}
//	else {
//		*_posDefaults = alifNew_exprSeq(0, _p->astMem);
//	}
//	return *_posDefaults == nullptr ? -1 : 0;
//}
//
//static AlifIntT make_kwArgs(AlifParser* _p, StarEtc* _starEtc, ASDLArgSeq** _kwOnlyArgs, ASDLExprSeq** _kwDefaults) {
//	if (_starEtc != nullptr and _starEtc->kwOnlyArgs != nullptr) {
//		*_kwOnlyArgs = get_names(_p, _starEtc->kwOnlyArgs);
//	}
//	else {
//		*_kwOnlyArgs = alifNew_argSeq(0, _p->astMem);
//	}
//
//	if (*_kwOnlyArgs == nullptr) return -1;
//
//	if (_starEtc != nullptr and _starEtc->kwOnlyArgs != nullptr) {
//		*_kwDefaults = get_defaults(_p, _starEtc->kwOnlyArgs);
//	}
//	else {
//		*_kwDefaults = alifNew_exprSeq(0, _p->astMem);
//	}
//
//	if (*_kwDefaults == nullptr) return -1;
//
//	return 0;
//}

// Constructs an arguments_ty object out of all the parsed constructs in the parameters rule
ArgumentsTy alifParserEngine_makeArguments(AlifParser* _p,
	ASDLArgSeq* _slashWithoutDefault, SlashWithDefault* _slashWithDefault,
	ASDLArgSeq* _plainNames, ASDLSeq* _namesWithDefault, StarEtc* _starEtc) { // 638

	ASDLArgSeq* posOnlyArgs{};
	if (make_posOnlyArgs(_p, _slashWithoutDefault, _slashWithDefault, &posOnlyArgs) == -1) return nullptr;

	ASDLArgSeq* posArgs{};
	if (make_posArgs(_p, _plainNames, _namesWithDefault, &posArgs) == -1) return nullptr;

	ASDLExprSeq* posDefaults{};
	if (make_posDefaults(_p, _slashWithDefault, _namesWithDefault, &posDefaults) == -1) return nullptr;

	ArgTy varArg{};
	if (_starEtc != nullptr and _starEtc->varArg != nullptr) {
		varArg = _starEtc->varArg;
	}

	ASDLArgSeq* kwOnlyArgs{};
	ASDLExprSeq* kwDefaults{};
	if (make_kwArgs(_p, _starEtc, &kwOnlyArgs, &kwDefaults) == -1) {
		return nullptr;
	}

	ArgTy kwArg{};
	if (_starEtc != nullptr and _starEtc->kwArg != nullptr) {
		kwArg = _starEtc->kwArg;
	}

	return alifAST_arguments(posOnlyArgs, posArgs, varArg, kwOnlyArgs, kwDefaults, kwArg, posDefaults, _p->astMem);
}

ArgumentsTy alifParserEngine_emptyArguments(AlifParser* _p) { // 681

	ASDLArgSeq* posOnlyArgs = alifNew_argSeq(0, _p->astMem);
	if (!posOnlyArgs) return nullptr;

	ASDLArgSeq* posArgs = alifNew_argSeq(0, _p->astMem);
	if (!posArgs) return nullptr;

	ASDLExprSeq* posDefaults = alifNew_exprSeq(0, _p->astMem);
	if (!posDefaults) return nullptr;

	ASDLArgSeq* kwOnlyArgs = alifNew_argSeq(0, _p->astMem);
	if (!kwOnlyArgs) return nullptr;

	ASDLExprSeq* kwDefaults = alifNew_exprSeq(0, _p->astMem);
	if (!kwDefaults) return nullptr;


	return alifAST_arguments(posOnlyArgs, posArgs, nullptr, kwOnlyArgs,
		kwDefaults, nullptr, posDefaults, _p->astMem);
}

AugOperator* alifParserEngine_augOperator(AlifParser* _p, Operator_ _type) { // 710

	AugOperator* a_ = (AugOperator*)alifASTMem_malloc(_p->astMem, sizeof(AugOperator));
	if (!a_) return nullptr;

	a_->type = _type;
	return a_;
}

//static AlifIntT seqNumber_ofStarExprs(ASDLSeq* _seq) {
//	AlifIntT n_ = 0;
//	for (AlifUSizeT i = 0, l = SEQ_LEN(_seq); i < l; i++) {
//		KeywordOrStar* k_ = (KeywordOrStar*)SEQ_GETUNTYPED(_seq, i);
//		if (!k_->isKeyword) {
//			n_++;
//		}
//	}
//	return n_;
//}
//
//KeywordOrStar* alifParserEngine_keywordOrStarred(AlifParser* _p, void* _element, AlifIntT _isKeyword) { // 764
//
//	KeywordOrStar* a_ = (KeywordOrStar*)alifASTMem_malloc(_p->astMem, sizeof(KeywordOrStar));
//	if (!a_) return nullptr;
//
//	a_->element = _element;
//	a_->isKeyword = _isKeyword;
//
//	return a_;
//}
//
//ASDLExprSeq* alifParserEngine_seqExtractStarExprs(AlifParser* _p, ASDLSeq* _kwArgs) { // 791
//	AlifIntT newLen = seqNumber_ofStarExprs(_kwArgs);
//	if (newLen == 0) return nullptr;
//
//	ASDLExprSeq* newSeq = alifNew_exprSeq(newLen, _p->astMem);
//	if (!newSeq) return nullptr;
//
//	AlifIntT index_ = 0;
//	for (AlifUSizeT i = 0, len_ = SEQ_LEN(_kwArgs); i < len_; i++) {
//		KeywordOrStar* k_ = (KeywordOrStar*)SEQ_GETUNTYPED(_kwArgs, i);
//		if (!k_->isKeyword) {
//			SEQ_SET(newSeq, index_++, (Expression*)k_->element);
//		}
//	}
//
//	return newSeq;
//}
//
//KeywordSeq* alifParserEngine_seqDeleteStarExprs(AlifParser* _p, ASDLSeq* _kwArgs) { // 814
//
//	AlifUSizeT len_ = SEQ_LEN(_kwArgs);
//	AlifIntT newLen = seqNumber_ofStarExprs(_kwArgs);
//	if (newLen == 0) return nullptr;
//
//	KeywordSeq* newSeq = alifNew_keywordSeq(newLen, _p->astMem);
//	if (!newSeq) return nullptr;
//
//	AlifIntT index_ = 0;
//	for (AlifUSizeT i = 0; i < len_; i++) {
//		KeywordOrStar* k_ = (KeywordOrStar*)SEQ_GETUNTYPED(_kwArgs, i);
//		if (!k_->isKeyword) {
//			SEQ_SET(newSeq, index_++, (Keyword*)k_->element);
//		}
//	}
//
//	return newSeq;
//}


ModuleTy alifParserEngine_makeModule(AlifParser* _p, ASDLStmtSeq* _a) {
	//ASDLTypeIgnoreSeq* typeIgnores = nullptr;
	//AlifSizeT num = _p->typeIgnoreComments.numItems;
	//if (num > 0) {
	//	typeIgnores = alifASDL_typeIgnoreSeqNew(num, _p->astMem);
	//	if (typeIgnores == nullptr) {
	//		return nullptr;
	//	}
	//	for (AlifSizeT i = 0; i < num; i++) {
	//		AlifObject* tag = alifParserEngine_newTypeComment(_p, _p->typeIgnoreComments.items[i].comment);
	//		if (tag == nullptr) {
	//			return nullptr;
	//		}
	//		TypeIgnoreTy ti = alifAST_typeIgnore(_p->typeIgnoreComments.items[i].lineno,
	//			tag, _p->astMem);
	//		if (ti == nullptr) {
	//			return nullptr;
	//		}
	//		ASDL_SEQ_SET(typeIgnores, i, ti);
	//	}
	//}
	return alifAST_module(_a, /*typeIgnores,*/ _p->astMem);
}


//static ResultTokenWithMetadata* resultToken_withMetadata(AlifParser* _p, void* _result, AlifObject* _metadata) {
//
//	ResultTokenWithMetadata* res_ = (ResultTokenWithMetadata*)alifASTMem_malloc(_p->astMem, sizeof(ResultTokenWithMetadata));
//	if (res_ == nullptr) return nullptr;
//
//	res_->metadata = _metadata;
//	res_->result = _result;
//	return res_;
//}
//
//ResultTokenWithMetadata* alifParserEngine_checkFStringConversion(AlifParser* _p, AlifPToken* _convTok, Expression* _conv) {
//
//	if (_convTok->lineNo != _conv->lineNo or _convTok->endColOffset != _conv->colOffset) {
//		// error
//		return nullptr; //
//	}
//
//	return resultToken_withMetadata(_p, _conv, _convTok->data);
//}
//
//ResultTokenWithMetadata* alifParserEngine_setupFullFormatSpec(AlifParser* _p, AlifPToken* _lit, ASDLExprSeq* _spec,
//	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
//
//	if (!_spec) return nullptr;
//
//	/*
//	.
//	.
//	.
//	*/
//
//	Expression* res_ = alifAST_joinedStr(_spec, _lineNo, _colOffset, _endLineNo, _endColOffset, _p->astMem);
//	if (!res_) return nullptr;
//
//	return resultToken_withMetadata(_p, res_, _lit->data);
//}
//
//Expression* alifParserEngine_collectCallSeqs(AlifParser* _p, ASDLExprSeq* _a, ASDLSeq* _b,
//	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
//
//	AlifUSizeT argsLen = SEQ_LEN(_a);
//	AlifUSizeT totalLen = argsLen;
//
//	if (_b == nullptr)
//	{
//		return alifAST_call(alifParserEngine_dummyName(), _a, nullptr,
//			_lineNo, _colOffset, _endLineNo, _endColOffset, _astMem);
//	}
//
//	ASDLExprSeq* stars_ = alifParserEngine_seqExtractStarExprs(_p, _b);
//	KeywordSeq* keywords_ = alifParserEngine_seqDeleteStarExprs(_p, _b);
//
//	if (stars_) totalLen += SEQ_LEN(stars_);
//
//	ASDLExprSeq* args_ = alifNew_exprSeq(totalLen, _astMem);
//
//	AlifUSizeT i = 0;
//	for (i = 0; i < argsLen; i++) {
//		SEQ_SET(args_, i, SEQ_GET(_a, i));
//	}
//	for (; i < totalLen; i++) {
//		SEQ_SET(args_, i, SEQ_GET(stars_, i - argsLen));
//	}
//
//	return alifAST_call(alifParserEngine_dummyName(), args_, keywords_,
//		_lineNo, _colOffset, _endLineNo, _endColOffset, _astMem);
//}
//
//static Expression* alifParserEngine_decodeFStringPart(AlifParser* _p, AlifIntT _isRaw, Expression* _const, AlifPToken* _token) {
//
//	const wchar_t* bStr = alifUStr_asUTF8(_const->V.constant.val);
//	if (bStr == nullptr) return nullptr;
//
//	AlifUSizeT len_;
//	if (wcscmp(bStr, L"{{") == 0 or wcscmp(bStr, L"}}") == 0) {
//		len_ = 1;
//	}
//	else {
//		len_ = wcslen(bStr);
//	}
//
//	_isRaw = _isRaw or wcschr(bStr, L'\\') == nullptr;
//	AlifObject* str_ = alifParserEngine_decodeString(_p, _isRaw, bStr, len_, _token);
//	if (str_ == nullptr) {
//		//alifParserEngine_raiseDecodeError(_p);
//		return nullptr;
//	}
//	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
//		ALIF_DECREF(str_);
//		return nullptr;
//	}
//	return alifAST_constant(str_, nullptr, _const->lineNo, _const->colOffset,
//		_const->endLineNo, _const->endColOffset, _p->astMem);
//}
//
//static ASDLExprSeq* unpackTopLevel_joinedStrs(AlifParser* _p, ASDLExprSeq* _rawExpr) {
//	// هذه الدالة غير ضرورية ويجب مراجعة عملها
//	/* The parser might put multiple f-string values into an individual
//	 * JoinedStr node at the top level due to stuff like f-string debugging
//	 * expressions. This function flattens those and promotes them to the
//	 * upper level. Only simplifies AST, but the compiler already takes care
//	 * of the regular output, so this is not necessary if you are not going
//	 * to expose the output AST to Alif level. */
//
//	AlifSizeT i, reqSize, rawSize;
//
//	reqSize = rawSize = SEQ_LEN(_rawExpr);
//	Expression* expr_;
//	for (i = 0; i < rawSize; i++) {
//		expr_ = SEQ_GET(_rawExpr, i);
//		if (expr_->type == JoinStrK) {
//			reqSize += SEQ_LEN(expr_->V.joinStr.vals) - 1;
//		}
//	}
//
//	ASDLExprSeq* expressions = alifNew_exprSeq(reqSize, _p->astMem);
//
//	AlifSizeT raw_index, req_index = 0;
//	for (raw_index = 0; raw_index < rawSize; raw_index++) {
//		expr_ = SEQ_GET(_rawExpr, raw_index);
//		if (expr_->type == JoinStrK) {
//			ASDLExprSeq* values = expr_->V.joinStr.vals;
//			for (AlifSizeT n = 0; n < SEQ_LEN(values); n++) {
//				SEQ_SET(expressions, req_index, SEQ_GET(values, n));
//				req_index++;
//			}
//		}
//		else {
//			SEQ_SET(expressions, req_index, expr_);
//			req_index++;
//		}
//	}
//	return expressions;
//}
//
//Expression* alifParserEngine_joinedStr(AlifParser* _p, AlifPToken* _a, ASDLExprSeq* _rawExprs, AlifPToken* _b) {
//	ASDLExprSeq* expr_ = unpackTopLevel_joinedStrs(_p, _rawExprs);
//	AlifUSizeT nItems = SEQ_LEN(expr_);
//
//	const wchar_t* quoteStr = _alifWBytes_asString(_a->bytes);
//	if (quoteStr == nullptr) return nullptr;
//
//	AlifIntT isRaw = wcspbrk(quoteStr, L"خ") != nullptr;
//
//	ASDLExprSeq* seq_ = alifNew_exprSeq(nItems, _p->astMem);
//	if (seq_ == nullptr) return nullptr;
//
//
//	AlifUSizeT index_ = 0;
//	for (AlifUSizeT i = 0; i < nItems; i++) {
//		Expression* item_ = SEQ_GET(expr_, i);
//		if (item_->type == ConstantK) {
//			item_ = alifParserEngine_decodeFStringPart(_p, isRaw, item_, _b);
//			if (item_ == nullptr) return nullptr;
//
//			/* Tokenizer emits string parts even when the underlying string
//			might become an empty value (e.g. FSTRING_MIDDLE with the value \\n)
//			so we need to check for them and simplify it here. */
//			if (ALIFUSTR_CHECKEXACT(item_->V.constant.val) and
//				ALIFUSTR_GET_LENGTH(item_->V.constant.val) == 0)
//				continue;
//		}
//		SEQ_SET(seq_, index_++, item_);
//	}
//
//	ASDLExprSeq* resizedExprs{};
//	if (index_ != nItems) {
//		resizedExprs = alifNew_exprSeq(index_, _p->astMem);
//		if (resizedExprs == nullptr) return nullptr;
//
//		for (AlifUSizeT i = 0; i < index_; i++) {
//			SEQ_SET(resizedExprs, i, SEQ_GET(seq_, i));
//		}
//	}
//	else {
//		resizedExprs = seq_;
//	}
//
//	return alifAST_joinedStr(resizedExprs, _a->lineNo, _a->colOffset, _b->endLineNo, _b->endColOffset, _p->astMem);
//}
//
//Expression* alifParserEngine_decodeConstantFromToken(AlifParser* _p, AlifPToken* _t) {
//
//	AlifSizeT bSize{};
//	wchar_t* bStr{};
//
//	if (alifWBytes_asStringAndSize(_t->bytes, &bStr, &bSize) == -1) return nullptr;
//
//	AlifObject* str_ = alifParserEngine_decodeString(_p, 0, bStr, bSize, _t);
//	if (str_ == nullptr) return nullptr;
//
//	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
//		ALIF_DECREF(str_);
//		return nullptr;
//	}
//
//	return alifAST_constant(str_, nullptr, _t->lineNo, _t->colOffset, _t->endLineNo, _t->endColOffset, _p->astMem);
//}
//
//Expression* alifParserEngine_constantFromToken(AlifParser* _p, AlifPToken* _t) {
//	wchar_t* bStr = (wchar_t*)_alifWBytes_asString(_t->bytes);
//	if (bStr == nullptr) return nullptr;
//
//	AlifObject* str_ = alifUStr_fromString(bStr);
//	if (str_ == nullptr) return nullptr;
//
//	if (alifASTMem_listAddAlifObj(_p->astMem, str_) < 0) {
//		ALIF_DECREF(str_);
//		return nullptr;
//	}
//
//	return alifAST_constant(str_, nullptr, _t->lineNo, _t->colOffset, _t->endLineNo, _t->endColOffset, _p->astMem);
//}
//
//Expression* alifParserEngine_constantFromString(AlifParser* _p, AlifPToken* _tok) {
//	wchar_t* str = _alifWBytes_asString(_tok->bytes);
//	if (str == nullptr) return nullptr;
//
//	AlifObject* s = alifParserEngine_parseString(_p, _tok);
//	if (s == nullptr) return nullptr;
//	if (alifASTMem_listAddAlifObj(_p->astMem, s) < 0) {
//		ALIF_DECREF(s);
//		return nullptr;
//	}
//	AlifObject* type{};
//	if (str and str[0] == L'م') {
//		type = alifParserEngine_newIdentifier(_p, L"م");
//		if (type == nullptr) return nullptr;
//	}
//	return alifAST_constant(s, type, _tok->lineNo, _tok->colOffset, _tok->endLineNo, _tok->endColOffset, _p->astMem);
//}
//
//Expression* alifParserEngine_formattedValue(AlifParser* _p, Expression* _expr, AlifPToken* _d,
//	ResultTokenWithMetadata* _c, ResultTokenWithMetadata* _f, AlifPToken* _closingBrace,
//	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
//
//	AlifIntT conversionVal = -1;
//	if (_c != nullptr) {
//		Expression* conversionExpr = (Expression*)_c->result;
//		AlifUCS4 first = ALIFUSTR_READ_WCHAR(conversionExpr->V.name.name, 0);
//
//		/*
//		.
//		.
//		*/
//	}
//	else if (_d and _f) {
//		conversionVal = (AlifIntT)L'ر';
//	}
//
//	Expression* formattedValue = alifAST_formattedValue(_expr, conversionVal,
//		_f ? (Expression*)_f->result : nullptr,
//		_lineNo, _colOffset, _endLineNo, _endColOffset, _astMem);
//
//	if (_d) {
//		/*
//		.
//		.
//		*/
//	}
//	else {
//		return formattedValue;
//	}
//
//	return nullptr;
//}
//
//Expression* alifParserEngine_combineStrings(AlifParser* _p, ASDLExprSeq* _strings,
//	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
//
//	AlifUSizeT len_ = SEQ_LEN(_strings);
//
//	AlifIntT fStringFound{};
//	AlifIntT unicodeStringFound{};
//	AlifIntT bytesFound{};
//
//	AlifUSizeT i = 0;
//	AlifUSizeT nFlattenedElems = 0;
//	for (i = 0; i < len_; i++) {
//		Expression* elem = SEQ_GET(_strings, i);
//		if (elem->type == ConstantK) {
//			if (ALIFBYTES_CHECK(elem->V.constant.val)) {
//				bytesFound = 1;
//			}
//			else {
//				unicodeStringFound = 1;
//			}
//			nFlattenedElems++;
//		}
//		else {
//			nFlattenedElems += SEQ_LEN(elem->V.joinStr.vals);
//			fStringFound = 1;
//		}
//	}
//
//	if ((unicodeStringFound or fStringFound) and bytesFound) {
//		// error
//		return nullptr;
//	}
//
//	if (bytesFound) {
//		AlifObject* res_ = alifBytes_fromString(L"");
//
//		/* Bytes literals never get a type, but just for consistency
//	   since they are represented as Constant nodes, we'll mirror
//	   the same behavior as unicode strings for determining the type. */
//		AlifObject* type_ = SEQ_GET(_strings, 0)->V.constant.type;
//		for (i = 0; i < len_; i++) {
//			Expression* element_ = SEQ_GET(_strings, i);
//			alifBytes_concat(&res_, element_->V.constant.val);
//		}
//		if (!res_ or alifASTMem_listAddAlifObj(_astMem, res_) < 0) {
//			ALIF_XDECREF(res_);
//			return nullptr;
//		}
//		return alifAST_constant(res_, type_, _lineNo, _colOffset, _endLineNo, _endColOffset, _p->astMem);
//	}
//
//	if (!fStringFound and len_ == 1) {
//		return SEQ_GET(_strings, 0);
//	}
//
//	ASDLExprSeq* flattened_ = alifNew_exprSeq(nFlattenedElems, _p->astMem);
//	if (flattened_ == nullptr) return nullptr;
//
//	/* build flattened list */
//	AlifUSizeT currentPos = 0;
//	AlifUSizeT j = 0;
//	for (i = 0; i < len_; i++) {
//		Expression* element_ = SEQ_GET(_strings, i);
//		if (element_->type == ConstantK) {
//			SEQ_SET(flattened_, currentPos++, element_);
//		}
//		else {
//			for (j = 0; j < SEQ_LEN(element_->V.joinStr.vals); j++) {
//				Expression* subVal = SEQ_GET(element_->V.joinStr.vals, j);
//				if (subVal == nullptr) return nullptr;
//
//				SEQ_SET(flattened_, currentPos++, subVal);
//			}
//		}
//	}
//
//	/* calculate folded element count */
//	AlifUSizeT nElements = 0;
//	AlifIntT prevIsConstant = 0;
//	for (i = 0; i < nFlattenedElems; i++) {
//		Expression* element_ = SEQ_GET(flattened_, i);
//
//		if (fStringFound and element_->type == ConstantK and
//			ALIFUSTR_CHECKEXACT(element_->V.constant.val) and
//			ALIFUSTR_GET_LENGTH(element_->V.constant.val) == 0)
//			continue;
//
//		if (!prevIsConstant or element_->type != ConstantK) nElements++;
//
//		prevIsConstant = element_->type == ConstantK;
//	}
//
//	ASDLExprSeq* values_ = alifNew_exprSeq(nElements, _p->astMem);
//	if (values_ == nullptr) return nullptr;
//
//	/* build folded list */
//	// code here
//
//	if (fStringFound) {
//		Expression* element_ = SEQ_GET(values_, 0);
//		return element_;
//	}
//
//	return alifAST_joinedStr(values_, _lineNo, _colOffset, _endLineNo, _endColOffset, _p->astMem);
//}
