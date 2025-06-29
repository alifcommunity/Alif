#include "alif.h"
#include "ErrorCode.h"

#include "AlifCore_Errors.h"
#include "AlifTokenState.h"
#include "AlifLexer.h"
#include "AlifParserEngine.h"





static inline void raise_unclosedParenthesesError(AlifParser* p) { // 51
	AlifIntT errorLineno = p->tok->parenLineNoStack[p->tok->level - 1];
	AlifIntT errorCol = p->tok->parenColStack[p->tok->level - 1];
	_raiseError_knownLocation(p, _alifExcSyntaxError_,
		errorLineno, errorCol, errorLineno, -1,
		"'%c' لم يتم إغلاق القوس",
		p->tok->parenStack[p->tok->level - 1]);
}


AlifIntT _alifParserEngine_tokenizerError(AlifParser* p) { // 61
	if (alifErr_occurred()) {
		return -1;
	}

	const char* msg = nullptr;
	AlifObject* errtype = _alifExcSyntaxError_;
	AlifSizeT col_offset = -1;
	p->errorIndicator = 1;
	switch (p->tok->done) {
	case E_TOKEN:
		msg = "invalid token";
		break;
	case E_EOF:
		if (p->tok->level) {
			//raiseUnclosedParentheses_error(p);
		}
		else {
			//RAISE_SYNTAX_ERROR("unexpected EOF while parsing");
		}
		return -1;
	case E_DEDENT:
		//RAISE_INDENTATION_ERROR("unindent does not match any outer indentation level");
		return -1;
	case E_INTR:
		if (!alifErr_occurred()) {
			//alifErr_setNone(_alifExcKeyboardInterrupt_);
		}
		return -1;
	case E_NOMEM:
		//alifErr_noMemory();
		return -1;
	case E_TABSPACE:
		//errtype = _alifExcTabError_;
		msg = "inconsistent use of tabs and spaces in indentation";
		break;
	case E_TOODEEP:
		//errtype = _alifExcIndentationError_;
		msg = "too many levels of indentation";
		break;
	case E_LINECONT: {
		col_offset = p->tok->cur - p->tok->buf - 1;
		msg = "unexpected character after line continuation character";
		break;
	}
	case E_COLUMNOVERFLOW:
		//alifErr_setString(_alifExcOverflowError_,
		//	"Parser column offset overflow - source line is too big");
		return -1;
	default:
		msg = "unknown parsing error";
	}

	//RAISE_ERROR_KNOWN_LOCATION(p, errtype, p->tok->lineNo,
	//	col_offset >= 0 ? col_offset : 0,
	//	p->tok->lineNo, -1, msg);
	return -1;
}



static AlifIntT _alifParserEngine_tokenizeFullSourceToCheckForErrors(AlifParser* _p) { // 155
	// Tokenize the whole input to see if there are any tokenization
	// errors such as mismatching parentheses. These will get priority
	// over generic syntax errors only if the line number of the error is
	// before the one that we had for the generic error.

	// We don't want to tokenize to the end for interactive input
	if (_p->tok->prompt != nullptr) {
		return 0;
	}

	AlifObject* type{}, * value{}, * traceback{};
	alifErr_fetch(&type, &value, &traceback);

	AlifPToken* currentToken = _p->knownErrToken != nullptr
		? _p->knownErrToken : _p->tokens[_p->fill - 1];
	AlifSizeT currentErrLine = currentToken->lineNo;

	AlifIntT ret = 0;
	AlifToken new_token{};
	//_alifToken_init(&new_token); ?!

	for (;;) {
		switch (alifTokenizer_get(_p->tok, &new_token)) {
		case ERRORTOKEN:
			if (alifErr_occurred()) {
				ret = -1;
				goto exit;
			}
			if (_p->tok->level != 0) {
				AlifIntT errorLineno = _p->tok->parenLineNoStack[_p->tok->level - 1];
				if (currentErrLine > errorLineno) {
					raise_unclosedParenthesesError(_p);
					ret = -1;
					goto exit;
				}
			}
			break;
		case ENDMARKER:
			break;
		default:
			continue;
		}
		break;
	}


exit:
	_alifToken_free(&new_token);
	if (alifErr_occurred() and _p->tok->tokModeStackIndex <= 0) {
		ALIF_XDECREF(value);
		ALIF_XDECREF(type);
		ALIF_XDECREF(traceback);
	}
	else {
		alifErr_restore(type, value, traceback);
	}
	return ret;
}


void* _alifParserEngine_raiseError(AlifParser* _p, AlifObject* _errtype,
	AlifIntT _useMark, const char* _errmsg, ...) { // 219
	if (_p->errorIndicator and alifErr_occurred()) {
		return nullptr;
	}
	if (_p->fill == 0) {
		va_list va;
		va_start(va, _errmsg);
		_alifParserEngine_raiseErrorKnownLocation(_p, _errtype, 0, 0, 0, -1, _errmsg, va);
		va_end(va);
		return nullptr;
	}
	if (_useMark and _p->mark == _p->fill
		and alifParserEngine_fillToken(_p) < 0)
	{
		_p->errorIndicator = 1;
		return nullptr;
	}
	AlifPToken* t = _p->knownErrToken != nullptr
		? _p->knownErrToken
		: _p->tokens[_useMark ? _p->mark : _p->fill - 1];
	AlifSizeT colOffset;
	AlifSizeT endColOffset = -1;
	if (t->colOffset == -1) {
		if (_p->tok->cur == _p->tok->buf) {
			colOffset = 0;
		}
		else {
			const char* start = _p->tok->buf ? _p->tok->lineStart : _p->tok->buf;
			colOffset = ALIF_SAFE_DOWNCAST(_p->tok->cur - start, intptr_t, int);
		}
	}
	else {
		colOffset = t->colOffset + 1;
	}

	if (t->endColOffset != -1) {
		endColOffset = t->endColOffset + 1;
	}

	va_list va;
	va_start(va, _errmsg);
	_alifParserEngine_raiseErrorKnownLocation(_p, _errtype, t->lineNo,
		colOffset, t->endLineNo, endColOffset, _errmsg, va);
	va_end(va);

	return nullptr;
}


static AlifObject* getErrorLine_fromTokenizerBuffers(AlifParser* _p, AlifSizeT _lineno) { // 265
	/* If the file descriptor is interactive, the source lines of the current
	 * (multi-line) statement are stored in p->tok->interactive_src_start.
	 * If not, we're parsing from a string, which means that the whole source
	 * is stored in p->tok->str. */
	char* curLine = _p->tok->interactive ? _p->tok->interactiveSrcStart : _p->tok->string;
	if (curLine == nullptr) {
		return alif_getConstant(ALIF_CONSTANT_EMPTY_STR);
	}

	AlifSizeT relativeLineno = _p->startingLineNo ? _lineno - _p->startingLineNo + 1 : _lineno;
	const char* bufEnd = _p->tok->interactive ? _p->tok->interactiveSrcEnd : _p->tok->inp;

	if (bufEnd < curLine) {
		bufEnd = curLine + strlen(curLine);
	}

	for (int i = 0; i < relativeLineno - 1; i++) {
		char* new_line = strchr(curLine, '\n');
		if (new_line == nullptr or new_line + 1 > bufEnd) {
			break;
		}
		curLine = new_line + 1;
	}

	char* next_newline;
	if ((next_newline = strchr(curLine, '\n')) == nullptr) { // This is the last line
		next_newline = curLine + strlen(curLine);
	}
	return alifUStr_decodeUTF8(curLine, next_newline - curLine, "replace");
}


void* _alifParserEngine_raiseErrorKnownLocation(AlifParser* _p, AlifObject* _errtype,
	AlifSizeT _lineno, AlifSizeT _colOffset,
	AlifSizeT _endLineno, AlifSizeT _endColOffset,
	const char* _errmsg, va_list _va) { // 308
	// Bail out if we already have an error set.
	if (_p->errorIndicator and alifErr_occurred()) {
		return nullptr;
	}
	AlifObject* value = nullptr;
	AlifObject* errstr = nullptr;
	AlifObject* errorLine = nullptr;
	AlifObject* tmp = nullptr;
	_p->errorIndicator = 1;

	if (_endLineno == CURRENT_POS) {
		_endLineno = _p->tok->lineNo;
	}
	if (_endColOffset == CURRENT_POS) {
		_endColOffset = _p->tok->cur - _p->tok->lineStart;
	}

	errstr = alifUStr_fromFormatV(_errmsg, _va);
	if (!errstr) {
		goto error;
	}

	if (_p->tok->interactive and _p->tok->interactiveSrcStart != nullptr) {
		errorLine = getErrorLine_fromTokenizerBuffers(_p, _lineno);
	}
	else if (_p->startRule == ALIF_FILE_INPUT) {
		errorLine = _alifErr_programDecodedTextObject(_p->tok->fn,
			(AlifIntT)_lineno, _p->tok->encoding);
	}

	if (!errorLine) {
		if (_p->tok->lineNo <= _lineno and _p->tok->inp > _p->tok->buf) {
			AlifSizeT size = _p->tok->inp - _p->tok->buf;
			errorLine = alifUStr_decodeUTF8(_p->tok->buf, size, "replace");
		}
		else if (_p->tok->fp == nullptr or _p->tok->fp == stdin) {
			errorLine = getErrorLine_fromTokenizerBuffers(_p, _lineno);
		}
		else {
			errorLine = alif_getConstant(ALIF_CONSTANT_EMPTY_STR);
		}
		if (!errorLine) {
			goto error;
		}
	}

	AlifSizeT colNumber;
	colNumber = _colOffset;
	AlifSizeT endColNumber;
	endColNumber = _endColOffset;

	colNumber = _alifParserEngine_byteOffsetToCharacterOffset(errorLine, _colOffset);
	if (colNumber < 0) {
		goto error;
	}

	if (_endColOffset > 0) {
		endColNumber = _alifParserEngine_byteOffsetToCharacterOffset(errorLine, _endColOffset);
		if (endColNumber < 0) {
			goto error;
		}
	}

	tmp = alif_buildValue("(OnnNnn)", _p->tok->fn, _lineno, colNumber, errorLine, _endLineno, endColNumber);
	if (!tmp) {
		goto error;
	}
	value = alifTuple_pack(2, errstr, tmp);
	ALIF_DECREF(tmp);
	if (!value) {
		goto error;
	}
	alifErr_setObject(_errtype, value);

	ALIF_DECREF(errstr);
	ALIF_DECREF(value);
	return nullptr;

error:
	ALIF_XDECREF(errstr);
	ALIF_XDECREF(errorLine);
	return nullptr;
}



void _alifParserEngine_setSyntaxError(AlifParser* _p, AlifPToken* _lastToken) { // 405
	// Existing syntax error
	if (alifErr_occurred()) {
		AlifIntT is_tok_ok = (_p->tok->done == E_DONE or _p->tok->done == E_OK);
		if (is_tok_ok and alifErr_exceptionMatches(_alifExcSyntaxError_)) {
			_alifParserEngine_tokenizeFullSourceToCheckForErrors(_p);
		}
		// Propagate the existing syntax error.
		return;
	}
	// Initialization error
	if (_p->fill == 0) {
		RAISE_SYNTAX_ERROR("حدث خطأ قبل البدأ بعملية المطابقة");
	}

	if (_lastToken->type == ERRORTOKEN and _p->tok->done == E_EOF) {
		if (_p->tok->level) {
			raise_unclosedParenthesesError(_p);
		}
		else {
			RAISE_SYNTAX_ERROR("نهاية الملف غير صحيحة اثناء عملية المطابقة");
		}
		return;
	}

	if (_lastToken->type == INDENT or _lastToken->type == DEDENT) {
		RAISE_INDENTATION_ERROR(_lastToken->type == INDENT ? "مسافة_طويلة غير صحيحة" : "مسافة_راجعة غير صحيحة");
		return;
	}
	// Unknown error (generic case)

	RAISE_SYNTAX_ERROR_KNOWN_LOCATION(_lastToken, "خطأ في النسق");

	_alifParserEngine_tokenizeFullSourceToCheckForErrors(_p);
}




void alifParserEngineError_stackOverflow(AlifParser* _p) { // 448
	_p->errorIndicator = 1;
	//alifErr_setString(_alifExcMemoryError_,
	//	"Parser stack overflowed - Alif source too complex to parse");
}
