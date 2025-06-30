#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_State.h"
#include "ErrorCode.h"

#include "AlifLexer.h"
#include "Tokenizer.h"
#include "AlifParserEngine.h"
#include "AlifCore_Memory.h"


 //* alif
/*
	هذا التعريف يستخدم فقط لعد الاحرف والارقام للاسماء
*/
#define IS_IDENTIFIER_CHAR(_c) ((_c >= 161 and _c <= 191)	\
								or (_c >= 128 and _c <= 138) /* الاحرف العربية - البايت الثاني منها */	\
								or (_c >= '0' and _c <= '9') \
								or (_c == '_')	\
								or (_c == 216 or _c == 217))

#define IS_2BYTE_IDENTIFIER(_c) (_c == 216 or _c == 217)


AlifSizeT _alifParserEngine_byteOffsetToCharacterOffsetRaw(const char* _str,
	AlifSizeT _colOffset) { // 47
	AlifSizeT len = (AlifSizeT)strlen(_str);
	if (_colOffset > len + 1) {
		_colOffset = len + 1;
	}
	AlifObject* text = alifUStr_decodeUTF8(_str, _colOffset, "replace");
	if (!text) {
		return -1;
	}
	AlifSizeT size = ALIFUSTR_GET_LENGTH(text);
	ALIF_DECREF(text);
	return size;
}

AlifSizeT _alifParserEngine_byteOffsetToCharacterOffset(AlifObject* _line,
	AlifSizeT _colOffset) { // 64
	const char* str = alifUStr_asUTF8(_line);
	if (!str) {
		return -1;
	}
	return _alifParserEngine_byteOffsetToCharacterOffsetRaw(str, _colOffset);
}

AlifIntT alifParserEngine_insertMemo(AlifParser* _p,
	AlifIntT _mark, AlifIntT _type, void* _node) { // 76
	Memo* m = (Memo*)alifASTMem_malloc(_p->astMem, sizeof(Memo));
	if (m == nullptr) return -1;
	m->type = _type;
	m->node = _node;
	m->mark = _p->mark;
	m->next = _p->tokens[_mark]->memo;
	_p->tokens[_mark]->memo = m;
	return 0;
}

AlifIntT alifParserEngine_updateMemo(AlifParser* _p,
	AlifIntT _mark, AlifIntT _type, void* _node) { // 93
	for (Memo* m = _p->tokens[_mark]->memo; m != nullptr; m = m->next) {
		if (m->type == _type) {
			m->node = _node;
			m->mark = _p->mark;
			return 0;
		}
	}
	return alifParserEngine_insertMemo(_p, _mark, _type, _node);
}


static AlifIntT str_lettersCount(const char* _str) { //* alif
	AlifIntT len = 0;
	AlifIntT ch = 0;

	while (IS_IDENTIFIER_CHAR((unsigned char)_str[len])) {
		if (IS_2BYTE_IDENTIFIER((unsigned char)_str[len])) {
			len++;
		}
		ch++;
		len++;
	}
	return ch;
}

static AlifIntT get_keywordOrName(AlifParser* _p, AlifToken* _token) { // 158
	AlifIntT bytesCount = _token->endColOffset - _token->colOffset;
	AlifIntT lettersCount = str_lettersCount(_token->start); //* alif

	if (lettersCount >= _p->nKeywordList
		or _p->keywords[lettersCount] == nullptr
		or _p->keywords[lettersCount]->type == -1) return NAME;

	for (KeywordToken* k = _p->keywords[lettersCount]; k != nullptr and k->type != -1; k++) {
		if (strncmp(k->str, _token->start, bytesCount) == 0) {
			return k->type;
		}
	}

	return NAME;
}


static AlifIntT initialize_token(AlifParser* _p,
	AlifPToken* _pToken, AlifToken* _token, AlifIntT _tokType) { // 177
	_pToken->type = (_tokType == NAME) ? get_keywordOrName(_p, _token) : _tokType;
	_pToken->bytes = alifBytes_fromStringAndSize(_token->start, _token->end - _token->start);

	if (_pToken->bytes == nullptr) {
		return -1;
	}
	if (alifASTMem_listAddAlifObj(_p->astMem, _pToken->bytes) < 0) {
		ALIF_DECREF(_pToken->bytes);
		return -1;
	}

	_pToken->data = nullptr;
	if (_token->data != nullptr) {
		if (alifASTMem_listAddAlifObj(_p->astMem, _token->data) < 0) {
			ALIF_DECREF(_pToken->data);
			return -1;
		}
	}

	_pToken->level = _token->level;
	_pToken->lineNo = _token->lineNo;
	_pToken->colOffset = _p->tok->lineNo == _p->startingLineNo ? _p->startingColOffset + _token->colOffset : _token->colOffset;

	_pToken->endLineNo = _token->endLineNo;
	_pToken->endColOffset = _p->tok->lineNo == _p->startingLineNo ? _p->startingColOffset + _token->endColOffset : _token->endColOffset;

	_p->fill += 1;

	if (_tokType == ERRORTOKEN and _p->tok->done == E_DECODE) {
		//return alifParserEngine_raiseDecodeError(_p);
		return -1; // temp
	}

	return (_tokType == ERRORTOKEN ? _alifParserEngine_tokenizerError(_p) : 0);
}

static AlifIntT resize_tokensArr(AlifParser* _p) { // 218 
	AlifIntT newSize = _p->size * 2;
	AlifPToken** newTokens = (AlifPToken**)alifMem_dataRealloc(_p->tokens, (AlifUSizeT)newSize * sizeof(AlifPToken*));
	if (newTokens == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	_p->tokens = newTokens;

	for (AlifIntT i = _p->size; i < newSize; i++) {
		_p->tokens[i] = (AlifPToken*)alifMem_dataAlloc(sizeof(AlifPToken));
		if (_p->tokens[i] == nullptr) {
			_p->size = i; // Needed, in order to cleanup correctly after parser fails
			//alifErr_noMemory();
			return -1;
		}
	}

	_p->size = newSize;
	return 0;
}

AlifIntT alifParserEngine_fillToken(AlifParser* _p) { // 240 
	AlifToken newToken{};
	AlifPToken* pT{};

	AlifIntT type = alifTokenizer_get(_p->tok, &newToken);

	//while (type == TYPEIGNORE) {
	//	AlifSizeT len = newToken.endColOffset - newToken.colOffset;
	//	char* tag = (char*)alifMem_dataAlloc(len + 1);
	//	strncpy(tag, newToken.start, (AlifUSizeT)len);
	//	tag[len] = L'0';
	//	if (!growableComment_arrayAdd(&_p->typeIgnoreComments, _p->tok->lineNo, tag)) {
	//		//alifErr_noMemory();
	//		goto error;
	//	}
	//	type = alifTokenizer_get(_p->tok, &newToken);
	//}

	if (_p->startRule == ALIF_SINGLE_INPUT and type == ENDMARKER and _p->parsingStarted) {
		type = NEWLINE;
		_p->parsingStarted = 0;
		if (_p->tok->indent and !(_p->flags & ALIFPARSE_DONT_IMPLY_DEDENT)) {
			_p->tok->pendInd = -_p->tok->indent;
			_p->tok->indent = 0;
		}
	}
	else {
		_p->parsingStarted = 1;
	}

	if ((_p->fill == _p->size) and (resize_tokensArr(_p)) != 0) {
		goto error;
	}

	pT = _p->tokens[_p->fill];
	return initialize_token(_p, pT, &newToken, type);

error:
	_alifToken_free(&newToken);
	return -1;
}

AlifIntT alifParserEngine_isMemorized(AlifParser* _p, AlifIntT _type, void* _pres) { // 345
	if (_p->mark == _p->fill) {
		if (alifParserEngine_fillToken(_p) < 0) {
			_p->errorIndicator = 1;
			return -1;
		}
	}

	AlifPToken* t_ = _p->tokens[_p->mark];

	for (Memo* m_ = t_->memo; m_ != nullptr; m_ = m_->next) {
		if (m_->type == _type) {
			_p->mark = m_->mark;
			*(void**)_pres = m_->node;
			return 1;
		}
	}
	return 0;
}

AlifIntT alifParserEngine_lookaheadWithInt(AlifIntT _positive,
	AlifPToken* (_func)(AlifParser*, AlifIntT), AlifParser* _p, AlifIntT _arg) { // 397
	AlifIntT mark_ = _p->mark;
	void* res = _func(_p, _arg);
	_p->mark = mark_;
	return (res != nullptr) == _positive;
}

AlifIntT alifParserEngine_lookahead(AlifIntT _positive,
	void* (_func)(AlifParser*), AlifParser* _p) { // 406
	AlifIntT mark_ = _p->mark;
	void* res = _func(_p);
	_p->mark = mark_;
	return (res != nullptr) == _positive;
}

AlifPToken* alifParserEngine_expectToken(AlifParser* _p, AlifIntT _type) { // 415
	/*
		إذا وصل المؤشر mark
		الى مؤشر الملء fill
		هذا يعني أنه لم يعد هنالك رموز المأخوذة tokens
		قابلة للإستخدام وبالتالي يجب جلب رمز جديد
		-------------------------------------------
		الفرق بين المؤشر الحالي ومؤشر الملء يدل على عدد الرموز المأخوذة tokens
	*/
	if (_p->mark == _p->fill) {
		if (alifParserEngine_fillToken(_p) < 0) {
			_p->errorIndicator = 1;
			return nullptr;
		}
	}
	AlifPToken* t = _p->tokens[_p->mark];
	if (t->type != _type) {
		return nullptr;
	}
	_p->mark += 1;
	return t;
}

AlifPToken* alifParserEngine_expectTokenForced(AlifParser* _p,
	AlifIntT _type, const char* _expected) { // 445 

	if (_p->errorIndicator == 1) return nullptr;
	if (_p->mark == _p->fill) {
		if (alifParserEngine_fillToken(_p) < 0) {
			_p->errorIndicator = 1;
			return nullptr;
		}
	}

	AlifPToken* t_ = _p->tokens[_p->mark];
	if (t_->type != _type) {
		//RAISE_SYNTAX_ERROR_KNOWN_LOCATION(t_, "expected '%s'", _expected);
		return nullptr;
	}
	_p->mark += 1;
	return t_;
}

AlifPToken* alifParserEngine_getLastNonWhitespaceToken(AlifParser* _p) { // 491
	AlifPToken* token = nullptr;
	for (AlifIntT i = _p->mark - 1; i >= 0; i--) {
		token = _p->tokens[i];
		if (token->type != ENDMARKER and (token->type < NEWLINE or token->type > DEDENT)) {
			break;
		}
	}

	return token;
}

AlifObject* alifParserEngine_newIdentifier(AlifParser* _p, const char* _s) { // 505
	AlifInterpreter* interp{};
	AlifObject* id = alifUStr_decodeUTF8(_s, (AlifSizeT)strlen(_s), nullptr);
	if (!id) {
		goto error;
	}
	if (!ALIFUSTR_IS_ASCII(id))
	{
		//if (!init_normalization(_p))
		//{
		//	ALIF_DECREF(id);
		//	goto error;
		//}
		//AlifObject* form = alifUStr_internFromString("NFKC");
		//if (form == nullptr) {
		//	ALIF_DECREF(id);
		//	goto error;
		//}
		//AlifObject* args[2] = { form, id };
		//AlifObject* id2 = alifObject_vectorCall(_p->normalize, args, 2, nullptr);
		//ALIF_DECREF(id);
		//ALIF_DECREF(form);
		//if (!id2) {
		//	goto error;
		//}

		//if (!ALIFUSTR_CHECK(id2))
		//{
		//	alifErr_format(_alifExcTypeError_,
		//		"unicodedata.normalize() must return a string, not "
		//		"%.200s",
		//		alifType_name(ALIF_TYPE(id2)));
		//	ALIF_DECREF(id2);
		//	goto error;
		//}
		//id = id2;
	}
	interp = _alifInterpreter_get();
	alifUStr_internImmortal(interp, &id);
	if (alifASTMem_listAddAlifObj(_p->astMem, id) < 0)
	{
		ALIF_DECREF(id);
		goto error;
	}
	return id;
error:
	_p->errorIndicator = 1;
	return nullptr;
}

static ExprTy alifParserEngine_nameFromToken(AlifParser* _p, AlifPToken* _t) { // 562
	if (_t == nullptr) return nullptr;

	const char* s = alifBytes_asString(_t->bytes);

	if (!s) {
		_p->errorIndicator = 1;
		return nullptr;
	}
	AlifObject* id = alifParserEngine_newIdentifier(_p, s);

	return alifAST_name(id, ExprContext_::Load, _t->lineNo, _t->colOffset, _t->endLineNo, _t->endColOffset, _p->astMem);
}

ExprTy alifParserEngine_nameToken(AlifParser* _p) { // 582
	AlifPToken* tok = alifParserEngine_expectToken(_p, NAME);
	return alifParserEngine_nameFromToken(_p, tok);
}

void* alifParserEngine_stringToken(AlifParser* _p) { // 589
	return alifParserEngine_expectToken(_p, STRING);
}

static AlifObject* parseNumber_raw(const char* _s) { // 611
	const char* end{};
	long x_{};
	double dx{};
	//AlifComplex compl{};
	AlifIntT imFlag{};

	errno = 0;
	end = _s + strlen(_s) - 1;
	imFlag = *end == L'j' or *end == L'J';
	if (_s[0] == L'0') {
		x_ = (long)alifOS_strToULong(_s, (char**)&end, 0);
		if (x_ < 0 and errno == 0) {
			return alifLong_fromString(_s, (char**)0, 0);
		}
	}
	else {
		x_ = alifOS_strToLong(_s, (char**)&end, 0);
	}
	if (*end == L'\0') {
		if (errno != 0) {
			return alifLong_fromString(_s, (char**)0, 0);
		}
		return alifLong_fromLong(x_);
	}

	//if (imFlag) {
	//	compl.real = 0.;
	//	compl.imag = alifOS_stringToDouble(_s, (char**)&end, nullptr);
	//	if (compl.imag == -1.0 and alifErr_occurred()) {
	//		return nullptr;
	//	}
	//	return alifComplex_fromCComplex(compl);
	//}
	dx = alifOS_stringToDouble(_s, nullptr, nullptr);
	if (dx == -1.0 /*and alifErr_occurred()*/) {
		return nullptr;
	}
	return alifFloat_fromDouble(dx);
}

static AlifObject* parse_number(const char* _s) { // 655
	char* dup{};
	char* end{};
	AlifObject* res{};

	if (strchr(_s, L'_') == nullptr) {
		return parseNumber_raw(_s);
	}

	dup = (char*)alifMem_dataAlloc(strlen(_s) + 1);
	if (dup == nullptr) {
		//return alifErr_noMemory();
		return nullptr; // temp
	}
	end = dup;
	for (; *_s; _s++) {
		if (*_s != L'_') {
			*end++ = *_s;
		}
	}
	*end = L'\0';
	res = parseNumber_raw(dup);
	alifMem_dataFree(dup);
	return res;
}

ExprTy alifParserEngine_numberToken(AlifParser* _p) { // 684
	AlifPToken* tok = alifParserEngine_expectToken(_p, NUMBER);
	if (tok == nullptr) return nullptr;

	const char* rawNum = alifBytes_asString(tok->bytes);
	if (rawNum == nullptr) {
		_p->errorIndicator = 1;
		return nullptr;
	}

	if (_p->featureVersion < 6 and strchr(rawNum, L'_') != nullptr) {
		_p->errorIndicator = 1;
		//return RAISE_SYNTAX_ERROR("Underscores in numeric literals are only supported "
		//	"in Alif5"); //* todo
		return nullptr; // temp
	}

	AlifObject* num = parse_number(rawNum);
	if (num == nullptr) {
		_p->errorIndicator = 1;
		//AlifThread* tstate = _alifThread_get();
		//if (tstate->currentException != nullptr and
		//	ALIF_TYPE(tstate->currentException) == (AlifTypeObject*)_alifExcValueError_) {
		//	AlifObject* exc = alifErr_getRaisedException();
		//	RAISE_ERROR_KNOWN_LOCATION(
		//		_p, _alifExcSyntaxError_,
		//		tok->lineNo, -1 /* col_offset */,
		//		tok->endLineNo, -1 /* end_col_offset */,
		//		"%S - Consider hexadecimal for huge integer literals "
		//		"to avoid decimal conversion limits.",
		//		exc);
			//ALIF_DECREF(exc);
		//}
		return nullptr;
	}


	if (alifASTMem_listAddAlifObj(_p->astMem, num) < 0) {
		ALIF_DECREF(num);
		_p->errorIndicator = 1;
		return nullptr;
	}

	return alifAST_constant(num, nullptr, tok->lineNo, tok->colOffset, tok->endLineNo, tok->endColOffset, _p->astMem); // type should be NUMBER not nullptr!
}


static AlifIntT compute_parserFlags(AlifCompilerFlags* flags) { // 769
	AlifIntT parser_flags = 0;
	if (!flags) {
		return 0;
	}
	if (flags->flags & ALIFCF_DONT_IMPLY_DEDENT) {
		parser_flags |= ALIFPARSE_DONT_IMPLY_DEDENT;
	}
	if (flags->flags & ALIFCF_IGNORE_COOKIE) {
		parser_flags |= ALIFPARSE_IGNORE_COOKIE;
	}
	if (flags->flags & CO_FUTURE_BARRY_AS_BDFL) {
		parser_flags |= ALIFPARSE_BARRY_AS_BDFL;
	}
	if (flags->flags & ALIFCF_TYPE_COMMENTS) {
		parser_flags |= ALIFPARSE_TYPE_COMMENTS;
	}
	if (flags->flags & ALIFCF_ALLOW_INCOMPLETE_INPUT) {
		parser_flags |= ALIFPARSE_ALLOW_INCOMPLETE_INPUT;
	}
	return parser_flags;
}


AlifParser* alifParserEngine_parserNew(TokenState* _tokState,
	AlifIntT _startRule, AlifIntT _flags, AlifIntT _featureVersion,
	AlifIntT* _error, AlifASTMem* _astMem) { // 796

	AlifParser* p_ = (AlifParser*)alifMem_dataAlloc(sizeof(AlifParser));
	if (p_ == nullptr) {
		//return (AlifParser*)alifErr_noMemory();
		return nullptr;
	}
	//_tokState->typeComments = (_flags & ALIFPARSE_TYPE_COMMENTS) > 0;

	p_->tok = _tokState;
	p_->keywords = nullptr;
	p_->nKeywordList = -1;
	p_->softKeyword = nullptr;
	p_->tokens = (AlifPToken**)alifMem_dataAlloc(sizeof(AlifPToken*));
	if (!p_->tokens) {
		alifMem_dataFree(p_);
		//return (AlifParser*)alifErr_noMemory();
		return nullptr; // temp
	}
	p_->tokens[0] = (AlifPToken*)alifMem_dataAlloc(sizeof(AlifPToken));
	if (!p_->tokens[0]) {
		alifMem_dataFree(p_->tokens);
		alifMem_dataFree(p_);
		//return (AlifParser*)alifErr_noMemory();
		return nullptr; // temp
	}
	//if (!growableComment_arrayInit(&p_->typeIgnoreComments, 10)) {
	//	alifMem_dataFree(p_->tokens[0]);
	//	alifMem_dataFree(p_->tokens);
	//	alifMem_dataFree(p_);
	//	return (AlifParser*)alifErr_noMemory();
	//}

	p_->mark = 0;
	p_->fill = 0;
	p_->size = 1;

	p_->errorCode = _error;
	p_->astMem = _astMem;
	p_->startRule = _startRule;
	p_->parsingStarted = 0;
	p_->normalize = nullptr;
	p_->errorIndicator = 0;
	p_->startingLineNo = 0;
	p_->startingColOffset = 0;
	p_->flags = _flags;
	p_->featureVersion = _featureVersion;
	p_->knownErrToken = nullptr;
	p_->level = 0;
	p_->callInvalidRules = 0;

	return p_;
}

void alifParserEngine_parserFree(AlifParser* _p) { // 852
	ALIF_XDECREF(_p->normalize);
	for (int i = 0; i < _p->size; i++) {
		alifMem_dataFree(_p->tokens[i]);
	}
	alifMem_dataFree(_p->tokens);
	//growableComment_arrayDeallocate(&_p->typeIgnoreComments);
	alifMem_dataFree(_p);
}

static void resetParserState_forErrorPass(AlifParser* _p) { // 864
	for (AlifIntT i = 0; i < _p->fill; i++) {
		_p->tokens[i]->memo = nullptr;
	}
	_p->mark = 0;
	_p->callInvalidRules = 1;
	// Don't try to get extra tokens in interactive mode when trying to
	// raise specialized errors in the second pass.
	_p->tok->interactiveUnderflow = InteractiveUnderflow_::IUnderflow_Stop;
}

void* alifParserEngine_runParser(AlifParser* _p) { // 883

	void* res = alifParserEngine_parse(_p);
	if (res == nullptr) {
		//if ((_p->flags & ALIFPARSE_ALLOW_INCOMPLETE_INPUT) and is_endOfSource(_p)) {
		//	alifErr_clear();
		//	return alifParserEngine_raiseError(_p, ALIFEXC_INCOMPLETEINPUTERROR, 0, "incomplete input");
		//}
		if (alifErr_occurred() and !alifErr_exceptionMatches(_alifExcSyntaxError_)) {
			return nullptr;
		}
		AlifPToken* lastToken = _p->tokens[_p->fill - 1];
		resetParserState_forErrorPass(_p);
		alifParserEngine_parse(_p);

		_alifParserEngine_setSyntaxError(_p, lastToken);
		return nullptr;
	}

	//if (_p->startRule == ALIF_SINGLE_INPUT and bad_singleStatement(_p)) {
	//	_p->tok->done = E_BADSINGLE;
	//	return RAISE_SYNTAX_ERROR("multiple statements found while compiling a single statement");
	//}

	return res;
}

ModuleTy alifParser_astFromFile(FILE* _fp, AlifIntT _startRule,
	AlifObject* _fn, const char* _enc, const char* _ps1, const char* _ps2,
	AlifCompilerFlags* _flags, AlifIntT* _error, AlifObject** _interactiveSrc,
	AlifASTMem* _astMem) { // 928 // _alifPegen_run_parser_from_file_pointer() 

	TokenState* tokState = alifTokenizerInfo_fromFile(_fp, _enc, _ps1, _ps2);
	if (tokState == nullptr) {
		if (alifErr_occurred()) {
			//alifParserEngine_raiseTokenizerInitError(_fn);
			return nullptr;
		}
		return nullptr;
	}
	if (!tokState->fp or _ps1 != nullptr or _ps2 != nullptr
		or alifUStr_compareWithASCIIString(_fn, "<stdin>") == 0) {
		tokState->interactive = 1;
	}

	tokState->fn = ALIF_NEWREF(_fn);

	ModuleTy result{};
	AlifIntT parserFlags = compute_parserFlags(_flags);
	AlifParser* p_ = alifParserEngine_parserNew(tokState, _startRule,
		parserFlags, ALIF_MINOR_VERSION, _error, _astMem);
	if (p_ == nullptr) goto error;

	result = (ModuleTy)alifParserEngine_runParser(p_);
	alifParserEngine_parserFree(p_);

	if (tokState->interactive and tokState->interactiveSrcStart and result and _interactiveSrc != nullptr) {
		*_interactiveSrc = alifUStr_fromString(tokState->interactiveSrcStart);
		if (!_interactiveSrc or alifASTMem_listAddAlifObj(_astMem, *_interactiveSrc) < 0) {
			ALIF_XDECREF(_interactiveSrc);
			result = nullptr;
			goto error;
		}
	}
error:
	alifTokenizer_free(tokState);
	return result;
}
