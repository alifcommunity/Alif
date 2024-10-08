#include "alif.h"

#include "AlifCore_Token.h"
#include "ErrorCode.h"

#include "AlifTokenState.h"
#include "Helpers.h"
#include "AlifCore_Memory.h"


// Alternate tab spacing
#define ALTTABSIZE 1

#define IS_IDENTIFIER_START(_c) ((_c >= 'a' and _c <= 'z') \
								or (_c >= 'A' and _c <= 'Z') /* to exclude nums and symbols */ \
								or (_c == '_') \
								or (_c < '٠' and _c >= 128) \
								or (_c > '٩' and _c >= 128)) /* exclude arabic-indic nums */

#define IS_IDENTIFIER_CHAR(_c) ((_c >= 'a' and _c <= 'z') \
								or (_c >= 'A' and _c <= 'Z') \
								or (_c >= '0' and _c <= '9') \
								or (_c == '_') \
								or (_c >= 128))

#define TOK_GET_MODE(_tokState) (&(_tokState->tokModeStack[_tokState->tokModeStackIndex]))
#define TOK_NEXT_MODE(_tokState) (&(_tokState->tokModeStack[++_tokState->tokModeStackIndex]))

#define MAKE_TOKEN(TT) alifLexer_setupToken(_tokState, _token, TT, pStart, pEnd)


static inline AlifIntT contains_nullBytes(const char* _str, AlifUSizeT _size) { // 49
	return memchr(_str, 0, _size) != nullptr;
}

static AlifIntT tok_nextChar(TokenState* _tokState) { // 56
	AlifIntT rc{};
	for (;;) {
		if (_tokState->cur != _tokState->inp) {
			if ((AlifUIntT)_tokState->colOffset >= (AlifUIntT)INT_MAX) {
				_tokState->done = E_COLUMNOVERFLOW;
				return EOF;
			}
			_tokState->colOffset++;
			return ALIF_CHARMASK(*_tokState->cur++);
		}
		if (_tokState->done != E_OK) {
			return EOF;
		}

		rc = _tokState->underflow(_tokState);

		if (!rc) {
			_tokState->cur = _tokState->inp;
			return EOF;
		}
		_tokState->lineStart = _tokState->cur;
		if (contains_nullBytes(_tokState->lineStart, _tokState->inp - _tokState->lineStart)) {
			//alifTokenizer_syntaxError(_tokState, "source code cannot contain null bytes");
			_tokState->cur = _tokState->inp;
			return EOF;
		}
	}
	ALIF_UNREACHABLE();
}

static void tok_backup(TokenState* _tokState, AlifIntT _c) { // 97
	if (_c != EOF) {
		if (--_tokState->cur < _tokState->buf) {
			//ALIF_FATALERROR("tokenizer beginning of buffer");
		}
		if ((AlifIntT)(unsigned char)*_tokState->cur != ALIF_CHARMASK(_c)) {
			//ALIF_FATALERROR("tok_backup: wrong character");
		}
		_tokState->colOffset--;
	}
}

//static AlifIntT set_fStringExpr(TokenState* _tokState, AlifToken* _token, wchar_t _wcs) {
//
//	TokenizerMode* tokMode = TOK_GETMODE(_tokState);
//
//	if (_token->data) return 0;
//
//	AlifObject* res = nullptr;
//
//	// Check if there is a # character in the expression
//	AlifIntT hashDetected = 0;
//	for (AlifUSizeT i = 0; i < tokMode->lastExprSize - tokMode->lastExprEnd; i++) {
//		if (tokMode->lastExprBuff[i] == '#') {
//			hashDetected = 1;
//			break;
//		}
//	}
//
//	if (hashDetected) {
//		AlifUSizeT inputLength = tokMode->lastExprSize - tokMode->lastExprEnd;
//		wchar_t* result = (wchar_t*)alifMem_dataAlloc((inputLength + 1) * sizeof(wchar_t));
//		if (!result) return -1;
//
//		AlifUSizeT i = 0;
//		AlifUSizeT j = 0;
//
//		for (i = 0, j = 0; i < inputLength; i++) {
//			if (tokMode->lastExprBuff[i] == '#') {
//				// Skip characters until newline or end of string
//				while (tokMode->lastExprBuff[i] != '\0' && i < inputLength) {
//					if (tokMode->lastExprBuff[i] == '\n') {
//						result[j++] = tokMode->lastExprBuff[i];
//						break;
//					}
//					i++;
//				}
//			}
//			else {
//				result[j++] = tokMode->lastExprBuff[i];
//			}
//		}
//
//		result[j] = '\0';  // Null-terminate the result string
//		res = alifUStr_decodeUTF8(result, j, nullptr);
//		alifMem_dataFree(result);
//	}
//	else {
//		res = alifUStr_decodeUTF8(tokMode->lastExprBuff, tokMode->lastExprSize - tokMode->lastExprEnd, nullptr);
//	}
//
//	if (!res) return -1;
//	_token->data = res;
//	return 0;
//}
//
//
//AlifIntT alifLexer_updateFStringExpr(TokenState* _tokState, wchar_t _cur) {
//
//	AlifUSizeT size = wcslen(_tokState->cur);
//	TokenizerMode* tokMode = TOK_GETMODE(_tokState);
//	wchar_t* newBuffer{};
//
//	switch (_cur) {
//
//	case 0:
//		if (!tokMode->lastExprBuff or tokMode->lastExprEnd >= 0) {
//			return 1;
//		}
//		newBuffer = (wchar_t*)alifMem_dataRealloc(tokMode->lastExprBuff, tokMode->lastExprSize + size);
//
//		if (newBuffer == nullptr) {
//			alifMem_dataFree(tokMode->lastExprBuff);
//			goto error;
//		}
//		tokMode->lastExprBuff = newBuffer;
//		wcsncpy(tokMode->lastExprBuff + tokMode->lastExprSize, _tokState->cur, size);
//		tokMode->lastExprSize += size;
//		break;
//	case '{':
//		if (tokMode->lastExprBuff != nullptr) {
//			alifMem_dataFree(tokMode->lastExprBuff);
//		}
//		tokMode->lastExprBuff = (wchar_t*)alifMem_dataAlloc(size);
//		if (tokMode->lastExprBuff == nullptr) {
//			goto error;
//		}
//		tokMode->lastExprSize = size;
//		tokMode->lastExprEnd = -1;
//		wcsncpy(tokMode->lastExprBuff, _tokState->cur, size);
//		break;
//	case '}':
//	case '!':
//	case ':':
//		if (tokMode->lastExprEnd == -1) {
//			tokMode->lastExprEnd = wcslen(_tokState->start);
//		}
//		break;
//	default:
//		//ALIF_UNREACHABLE();
//		break;
//	}
//
//	return 1;
//
//error:
//	_tokState->done = E_NOMEM;
//	return 0;
//}
//
//
//static AlifIntT tok_decimalTail(TokenState* _tokState) {
//	AlifIntT wcs{};
//
//	while (1) {
//		do {
//			wcs = tok_nextChar(_tokState);
//		} while (ALIF_ISDIGIT(wcs));
//		if (wcs != '_') {
//			break;
//		}
//		wcs = tok_nextChar(_tokState);
//		if (!ALIF_ISDIGIT(wcs)) {
//			tok_backup(_tokState, wcs);
//			// error
//			return 0;
//		}
//	}
//	return wcs;
//}

static inline AlifIntT tok_continuationLine(TokenState* _tokState) { // 387

	AlifIntT c_ = tok_nextChar(_tokState);
	if (c_ == '\r') c_ = tok_nextChar(_tokState);
	if (c_ != '\n') { _tokState->done = E_LINECONT; return -1; }

	c_ = tok_nextChar(_tokState);
	if (c_ == EOF) {
		_tokState->done = E_EOF;
		_tokState->cur = _tokState->inp;
		return -1;
	}
	else {
		tok_backup(_tokState, c_);
	}

	return c_;
}

static AlifIntT tokGet_normalMode(TokenState* _tokState,
	TokenizerMode* _currentTok, AlifToken* _token) { // 408
	AlifIntT c_{};
	AlifIntT blankLine{}, nonASCII{};

	const char* pStart{};
	const char* pEnd{};

nextline:
	_tokState->start = nullptr;
	_tokState->startingColOffset = -1;
	blankLine = 0;

	// indentation level
	if (_tokState->atBeginOfLine) {
		AlifIntT col = 0;
		AlifIntT altCol = 0;
		_tokState->atBeginOfLine = 0;
		AlifIntT contLineCol = 0;

		for (;;) {
			c_ = tok_nextChar(_tokState);
			if (c_ == ' ') { col++; altCol++; }
			else if (c_ == '\t')
			{
				col = (col / _tokState->tabSize + 1) * _tokState->tabSize;
				altCol = (altCol / ALTTABSIZE + 1) * ALTTABSIZE;
			}
			else if (c_ == '\014') {/* Control-L (formfeed) */
				col = altCol = 0; /* For Emacs users */
			}
			else if (c_ == '\\') {
				contLineCol = contLineCol ? contLineCol : col;
				if ((c_ = tok_continuationLine(_tokState)) == -1)
					return MAKE_TOKEN(ERRORTOKEN);
			}
			else break;
		}
		tok_backup(_tokState, c_);
		if (c_ == '#' or c_ == '\n' or c_ == '\r') {
			if (col == 0 and c_ == '\n' and _tokState->prompt != nullptr) {
				blankLine = 0;
			}
			else if (_tokState->prompt != nullptr and _tokState->lineNo == 1) {
				blankLine = 0;
				col = altCol = 0;
			}
			else { blankLine = 1; }
		}
		if (!blankLine and _tokState->level == 0) {
			col = contLineCol ? contLineCol : col;
			altCol = contLineCol ? contLineCol : altCol;
			if (col == _tokState->indStack[_tokState->indent])
			{
				if (altCol != _tokState->alterIndStack[_tokState->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokState)); // indent error
				}
			}
			else if (col > _tokState->indStack[_tokState->indent]) {
				// indent - always one
				if (_tokState->indent + 1 >= MAXINDENT) {
					_tokState->done = E_TOODEEP;
					_tokState->cur = _tokState->inp;
					return MAKE_TOKEN(ERRORTOKEN);
				}
				if (altCol <= _tokState->alterIndStack[_tokState->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokState)); // indent error
				}
				_tokState->pendInd++;
				_tokState->indStack[++_tokState->indent] = col;
				_tokState->alterIndStack[++_tokState->indent] = altCol;
			}
			else {
				/* col < tok->indstack[tok->indent] */
				/* Dedent -- any number, must be consistent */

				while (_tokState->indent > 0
					and col < _tokState->indStack[_tokState->indent]) {
					_tokState->pendInd--;
					_tokState->indent--;
				}
				if (col != _tokState->indStack[_tokState->indent]) {
					_tokState->done = E_DEDENT;
					_tokState->cur = _tokState->inp;
					return MAKE_TOKEN(ERRORTOKEN);
				}
				if (altCol != _tokState->alterIndStack[_tokState->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokState)); // indent error
				}
			}
		}
	}

	_tokState->start = _tokState->cur;
	_tokState->startingColOffset = _tokState->colOffset;

	// return pending indents/dedents
	if (_tokState->pendInd != 0) {
		if (_tokState->pendInd < 0) {
			if (_tokState->tokExtraTokens) {
				pStart = _tokState->cur;
				pEnd = _tokState->cur;
			}
			_tokState->pendInd++;
			return MAKE_TOKEN(DEDENT);
		}
		else {
			if (_tokState->tokExtraTokens) {
				pStart = _tokState->buf;
				pEnd = _tokState->cur;
			}
			_tokState->pendInd--;
			return MAKE_TOKEN(INDENT);
		}
	}


	c_ = tok_nextChar(_tokState);
	tok_backup(_tokState, c_);

again:
	_tokState->start = nullptr;

	// skip spaces
	do {
		c_ = tok_nextChar(_tokState);
	} while (c_ == ' ' or c_ == '\t' or c_ == '\014');

	// Set start of current token
	_tokState->start = _tokState->cur == nullptr ? nullptr : _tokState->cur - 1;
	_tokState->startingColOffset = _tokState->colOffset - 1;

	// Skip comment
	if (c_ == '#') {
		const char* p{};
		const char* prefix{}, * typeStart;
		AlifIntT currentStartingColOffset{};

		while (c_ != EOF and c_ != '\n' and c_ != '\r') {
			c_ = tok_nextChar(_tokState);
		}

		if (_tokState->tokExtraTokens) { p = _tokState->start; }

		//if (_tokState->typeComments) {
		//	p = _tokState->start;
		//	currentStartingColOffset = _tokState->startingColOffset;
		//	prefix = typeCommentPrefix;
		//	while (*prefix and p < _tokState->cur) {
		//		if (*prefix == ' ') {
		//			while (*p == ' ' or *p == '\t') {
		//				p++;
		//				currentStartingColOffset++;
		//			}
		//		}
		//		else if (*prefix == *p) {
		//			p++;
		//			currentStartingColOffset++;
		//		}
		//		else {
		//			break;
		//		}

		//		prefix++;
		//	}

		//	/* This is a type comment if we matched all of type_comment_prefix. */
		//	if (!*prefix) {
		//		AlifIntT isTypeIgnore = 1;
		//		// +6 in order to skip the word 'ignore'
		//		const char* ignore_end = p + 6;
		//		const int ignore_end_col_offset = currentStartingColOffset + 6;
		//		tok_backup(_tokState, c_);  /* don't eat the newline or EOF */

		//		typeStart = p;

		//		/* A TYPE_IGNORE is "type: ignore" followed by the end of the token
		//		 * or anything ASCII and non-alphanumeric. */
		//		isTypeIgnore = (
		//			_tokState->cur >= ignore_end && memcmp(p, "ignore", 6) == 0
		//			and !(_tokState->cur > ignore_end
		//				and ((unsigned char)ignore_end[0] >= 128 or ALIF_ISALNUM(ignore_end[0]))));

		//		if (isTypeIgnore) {
		//			pStart = ignore_end;
		//			pEnd = _tokState->cur;

		//			/* If this type ignore is the only thing on the line, consume the newline also. */
		//			if (blankLine) {
		//				tok_nextChar(_tokState);
		//				_tokState->atBeginOfLine = 1;
		//			}
		//			return MAKE_TYPE_COMMENT_TOKEN(TYPE_IGNORE, ignore_end_col_offset, _tokState->col_offset);
		//		}
		//		else {
		//			pStart = typeStart;
		//			pEnd = _tokState->cur;
		//			return MAKE_TYPE_COMMENT_TOKEN(TYPE_COMMENT, currentStartingColOffset, _tokState->col_offset);
		//		}
		//	}
		//}

		if (_tokState->tokExtraTokens) {
			tok_backup(_tokState, c_);
			pStart = p;
			pEnd = _tokState->cur;
			_tokState->commentNewline = blankLine;
			return MAKE_TOKEN(COMMENT);
		}
	}

	if (_tokState->done == E_INTERACT_STOP) {
		return MAKE_TOKEN(ENDMARKER);
	}

	// check for EOF and errors now
	if (c_ == EOF) {
		if (_tokState->level) {
			return MAKE_TOKEN(ERRORTOKEN);
		}
		return MAKE_TOKEN(_tokState->done == E_EOF ? ENDMARKER : ERRORTOKEN);
	}


	/* Identifire */
	nonASCII = 0;
	if (IS_IDENTIFIER_START(c_)) {
		AlifIntT b_ = 0, r_ = 0, u_ = 0, f_ = 0;
		while (true) {
			if (!(b_ or u_ or f_) and c_ == 'ب') b_ = 1; // ب = بايت
			else if (!(b_ or u_ or r_) and c_ == 'ت') u_ = 1; // ت = ترميز
			else if (!(r_ or u_) and c_ == 'خ') r_ = 1; // خ = خام
			else if (!(f_ or b_ or u_) and c_ == 'م') f_ = 1; // م = منسق
			else break;

			c_ = tok_nextChar(_tokState);
			if (c_ == '"' or c_ == '\'') {
				if (f_) goto fStringQuote;
				goto letterQuote;
			}
		}
		while (IS_IDENTIFIER_CHAR(c_)) { // يجب مراجعة وظيفة هذا التحقق
			if (c_ >= 128) {
				nonASCII = 1;
			}
			c_ = tok_nextChar(_tokState);
		}
		tok_backup(_tokState, c_);
		//if (nonASCII and !verify_identifier(_tokState)) {
		//	return MAKE_TOKEN(ERRORTOKEN);
		//}

		pStart = _tokState->start;
		pEnd = _tokState->cur;

		return MAKE_TOKEN(NAME);
	}

	if (c_ == '\r') { c_ = tok_nextChar(_tokState); }

	/* Newline */
	if (c_ == '\n') {
		_tokState->atBeginOfLine = 1;
		if (blankLine or _tokState->level > 0) {
			if (_tokState->tokExtraTokens) {
				if (_tokState->commentNewline) {
					_tokState->commentNewline = 0;
				}
				pStart = _tokState->start;
				pEnd = _tokState->cur;
				return MAKE_TOKEN(NL);
			}
			goto nextline;
		}
		if (_tokState->commentNewline and _tokState->tokExtraTokens) {
			_tokState->commentNewline = 0;
			pStart = _tokState->start;
			pEnd = _tokState->cur;
			return MAKE_TOKEN(NL);
		}
		pStart = _tokState->start;
		pEnd = _tokState->cur - 1;
		_tokState->contLine = 0; // Leave '\n' out of the string

		return MAKE_TOKEN(NEWLINE);
	}

	/* Period or number starting with period? */
	if (c_ == '.') {

		c_ = tok_nextChar(_tokState);
		if (ALIF_ISDIGIT(c_)) {
			goto fraction;
		}
		else if (c_ == '.') {
			c_ = tok_nextChar(_tokState);
			if (c_ == '.') {
				pStart = _tokState->start;
				pEnd = _tokState->cur;
				return MAKE_TOKEN(ELLIPSIS);
			}
			else {
				tok_backup(_tokState, c_);
			}
			tok_backup(_tokState, '.');
		}
		else {
			tok_backup(_tokState, c_);
		}
		pStart = _tokState->start;
		pEnd = _tokState->cur;
		return MAKE_TOKEN(DOT);
	}


	/* Number */
	if (ALIF_ISDIGIT(c_)) {
		if (c_ == '0') {
			/* Hex or Octal or Binary */
			c_ = tok_nextChar(_tokState);
			if (c_ == 'ه') {
				c_ = tok_nextChar(_tokState);
				do {
					if (c_ == '_') {
						c_ = tok_nextChar(_tokState);
					}
					if (!ALIF_ISXDIGIT(c_)) {
						tok_backup(_tokState, c_);
						//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"رقم ستعشري غير صحيح"));
					}
					do {
						c_ = tok_nextChar(_tokState);
					} while (ALIF_ISXDIGIT(c_));
				} while (c_ == '_');
				//if (!verify_endOfNumber(_tokState, c_, L"ستعشري")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
			else if (c_ == 'ث') {
				// Octal
				c_ = tok_nextChar(_tokState);
				do {
					if (c_ == '_') { c_ = tok_nextChar(_tokState); }
					if (c_ < '0' or c_ >= '8') {
						if (ALIF_ISDIGIT(c_)) {
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"رقم ثماني غير صحيح '%wcs'", wcs));
						}
						else {
							tok_backup(_tokState, c_);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"رقم ثماني غير صحيح"));
						}
					}
					do {
						c_ = tok_nextChar(_tokState);
					} while ('0' <= c_ and c_ < '8');
				} while (c_ == '-');
				if (ALIF_ISDIGIT(c_)) {
					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"رقم ثماني غير صحيح '%wcs'", wcs));
				}
				//if (!verify_endOfNumber(_tokState, wcs, L"ثماني")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
			else if (c_ == 'ن') {
				// Binary
				c_ = tok_nextChar(_tokState);
				do {
					if (c_ == '_') { c_ = tok_nextChar(_tokState); }

					if (c_ != '0' and c_ != '1') {
						if (ALIF_ISDIGIT(c_)) {
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"رقم ثنائي غير صحيح '%wcs'", wcs));
						}
						else {
							tok_nextChar(_tokState);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"رقم ثنائي غير صحيح"));
						}
					}
					do {
						c_ = tok_nextChar(_tokState);
					} while (c_ == '0' or c_ == '1');
				} while (c_ == '_');

				if (ALIF_ISDIGIT(c_)) {
					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"رقم ثنائي غير صحيح '%wcs'", wcs));
				}
				//if (!verify_endOfNumber(_tokState, wcs, L"ثنائي")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
			else {
				/*
					يجب التأكد من هذه الجزئية وطريقة عملها
				*/


			}
		}
		else {
			/* Decimal */
			c_ = tok_decimalTail(_tokState);
			if (c_ == 0) return MAKE_TOKEN(ERRORTOKEN);
			{
				/* float number */
				if (c_ == '.') {
					c_ = tok_nextChar(_tokState);
				fraction:
					// Fraction
					if (ALIF_ISDIGIT(c_)) {
						c_ = tok_decimalTail(_tokState);
						if (c_ == 0) return MAKE_TOKEN(ERRORTOKEN);
					}
				}
				if (c_ == 'س') { /* exponent */
					AlifIntT e{};
				exponent:
					e = c_;
					c_ = tok_nextChar(_tokState);
					if (c_ == '+' or c_ == '-') {
						c_ = tok_nextChar(_tokState);
						if (!ALIF_ISDIGIT(c_)) {
							tok_backup(_tokState, c_);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"رقم عشري غير صحيح"));
						}
					}
					else if (!ALIF_ISDIGIT(c_)) {
						tok_backup(_tokState, c_);
						//if (!verify_endOfNumber(_tokState, e, L"عشري")) {
						//	return MAKE_TOKEN(ERRORTOKEN);
						//}
						tok_backup(_tokState, e);
						pStart = _tokState->start;
						pEnd = _tokState->cur;
						return MAKE_TOKEN(NUMBER);
					}
					c_ = tok_decimalTail(_tokState);
					if (c_ == 0) return MAKE_TOKEN(ERRORTOKEN);
				}
				if (c_ == 'ت') {
					/* Imaginary part */
				imaginary:
					c_ = tok_nextChar(_tokState);
					//if (!verify_endOfNumber(_tokState, c, L"تخيلي")) {
					//	return MAKE_TOKEN(ERRORTOKEN);
					//}
				}
				//else if (!verify_endOfNumber(_tokState, c, L"عشري")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
		}

		tok_backup(_tokState, c_);
		pStart = _tokState->start;
		pEnd = _tokState->cur;
		return MAKE_TOKEN(NUMBER);
	}

fStringQuote:
	if ((ALIF_TOLOWER(*_tokState->start) == 'م' or ALIF_TOLOWER(*_tokState->start) == 'خ')
		and (c_ == '\'' or c_ == '"'))
	{
		AlifIntT quote = c_;
		AlifIntT quoteSize = 1;

		_tokState->firstLineNo = _tokState->lineNo;
		_tokState->multiLineStart = _tokState->lineStart;

		AlifIntT afterQuote = tok_nextChar(_tokState);
		if (afterQuote == quote) {
			AlifIntT afterAfterQuote = tok_nextChar(_tokState);
			if (afterAfterQuote == quote) {
				quoteSize = 3;
			}
			else {
				tok_backup(_tokState, afterAfterQuote);
				tok_backup(_tokState, afterQuote);
			}
		}
		if (afterQuote != quote) {
			tok_backup(_tokState, afterQuote);
		}

		pStart = _tokState->start;
		pEnd = _tokState->cur;
		if (_tokState->tokModeStackIndex + 1 >= MAXFSTRING_LEVEL) {
			//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"لقد تجاوزت الحد الاقصى للنص المنسق المتداخل"));
		}
		TokenizerMode* theCurrentTok = TOK_NEXT_MODE(_tokState);
		theCurrentTok->type = TokenizerModeType_::Token_FStringMode;
		theCurrentTok->fStringQuote = quote;
		theCurrentTok->fStringQuoteSize = quoteSize;
		theCurrentTok->fStringStart = _tokState->start;
		theCurrentTok->fStringMultiLineStart = _tokState->lineStart;
		theCurrentTok->fStringLineStart = _tokState->lineNo;
		theCurrentTok->fStringStartOffset = -1;
		theCurrentTok->fStringMultiLineStartOffset = -1;
		theCurrentTok->lastExprBuff = nullptr;
		theCurrentTok->lastExprSize = 0;
		theCurrentTok->lastExprEnd = -1;
		theCurrentTok->inFormatSpec = 0;
		theCurrentTok->fStringDebug = 0;

		if (*_tokState->start == 'م') {
			theCurrentTok->fStringRaw = ALIF_TOLOWER(*(_tokState->start + 1)) == 'خ';
		}
		else if (*_tokState->start == 'خ') {
			theCurrentTok->fStringRaw = 1;
		}
		else {
			ALIF_UNREACHABLE();
		}

		theCurrentTok->curlyBracDepth = 0;
		theCurrentTok->curlyBracExprStartDepth = -1;
		return MAKE_TOKEN(FSTRINGSTART);
	}

letterQuote:
	if (c_ == '\'' or c_ == '"') {
		AlifIntT quote = c_;
		AlifIntT quoteSize = 1;
		AlifIntT endQuoteSize = 0;
		AlifIntT hasEscapedQuote = 0;

		_tokState->firstLineNo = _tokState->lineNo;
		_tokState->multiLineStart = _tokState->lineStart;

		// find the quote size and start of string
		c_ = tok_nextChar(_tokState);
		if (c_ == quote) {
			c_ = tok_nextChar(_tokState);
			if (c_ == quote) {
				quoteSize = 3;
			}
			else {
				endQuoteSize = 1; // اي انه نص فارغ
			}
		}
		if (c_ != quote) {
			tok_backup(_tokState, c_);
		}

		/* the rest of STRING */
		while (endQuoteSize != quoteSize) {
			c_ = tok_nextChar(_tokState);
			if (_tokState->done == E_ERROR) {
				return MAKE_TOKEN(ERRORTOKEN);
			}
			if (_tokState->done == E_DECODE) break;

			if (c_ == EOF or (quoteSize == 1 and c_ == '\n')) {
				_tokState->cur = (char*)_tokState->start;
				_tokState->cur++;
				_tokState->lineStart = _tokState->multiLineStart;
				AlifIntT start = _tokState->lineNo;
				_tokState->lineNo = _tokState->firstLineNo;

				if (INSIDE_FSTRING(_tokState)) {
					TokenizerMode* currentToken = TOK_GET_MODE(_tokState);
					if (currentToken->fStringQuote == quote
						and currentToken->fStringQuoteSize == quoteSize) {
						//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"خطأ في النص المنسق '{'", start));
					}
				}

				if (quoteSize == 3) {
					//alifTokenizer_syntaxError(_tokState, "نص متعدد الاسطر لم يتم إنهاؤه" " في السطر %d", start);
					if (c_ != '\n') {
						_tokState->done = E_EOFS;
					}
					return MAKE_TOKEN(ERRORTOKEN);
				}
				else {
					if (hasEscapedQuote) {
						//alifTokenizer_syntaxError(_tokState, "نص لم يتم إنهاؤه" " في السطر %d", start);
					}
					else {
						//alifTokenizer_syntaxError(_tokState, "نص لم يتم إنهاؤه" " في السطر %d", start);
					}
					if (c_ != '\n') {
						_tokState->done = E_EOLS;
					}
					return MAKE_TOKEN(ERRORTOKEN);
				}
			}
			if (c_ == quote) {
				endQuoteSize++;
			}
			else {
				endQuoteSize = 0;
				if (c_ == '\\') {
					c_ = tok_nextChar(_tokState);  // skip escape char
					if (c_ == quote) {	           // record if the escape was a quote
						hasEscapedQuote = 1;
					}
					if (c_ == '\r') {
						c_ = tok_nextChar(_tokState);
					}
				}
			}
		}

		pStart = _tokState->start;
		pEnd = _tokState->cur;
		return MAKE_TOKEN(STRING);
	}

	/* line continuation */
	if (c_ == '\\') {
		if ((c_ = tok_continuationLine(_tokState)) == -1) {
			return MAKE_TOKEN(ERRORTOKEN);
		}
		_tokState->contLine = 1;
		goto again; // read next line
	}


	// Punctuation character
	AlifIntT isPunctuation = (c_ == ':' or c_ == '}' or c_ == '!' or c_ == '}');
	if (isPunctuation and INSIDE_FSTRING(_tokState) and INSIDE_FSTRING_EXPR(_currentTok)) {
		AlifIntT cursor = _currentTok->curlyBracDepth - (c_ != '{');
		AlifIntT inFormatSpec = _currentTok->inFormatSpec;
		AlifIntT cursorInFormatWithDebug =
			cursor == 1 and (_currentTok->fStringDebug or inFormatSpec);
		AlifIntT cursorValid = cursor == 0 or cursorInFormatWithDebug;
		if (cursor == 0 and !alifLexer_updateFStringExpr(_tokState, c_)) {
			return MAKE_TOKEN(ENDMARKER);
		}
		if (cursor == 0 and c_ != '{' and set_fStringExpr(_tokState, _token, c_)) {
			return MAKE_TOKEN(ERRORTOKEN);
		}

		if (c_ == ':' and cursor == _currentTok->curlyBracExprStartDepth) {
			_currentTok->type = TokenizerModeType_::Token_FStringMode;
			pStart = _tokState->start;
			pEnd = _tokState->cur;
			return MAKE_TOKEN(alifToken_oneChar(c_));
		}
	}


	{ // two_character token
		AlifIntT c_2 = tok_nextChar(_tokState);
		AlifIntT currentToken = alifToken_twoChars(c_, c_2);
		if (currentToken != OP) {
			AlifIntT c_3 = tok_nextChar(_tokState);
			AlifIntT currentTok3 = alifToken_threeChars(c_, c_2, c_3);
			if (currentTok3 != OP) {
				currentToken = currentTok3;
			}
			else {
				tok_backup(_tokState, c_3);
			}
			pStart = _tokState->start;
			pEnd = _tokState->cur;
			return MAKE_TOKEN(currentToken);
		}
		tok_backup(_tokState, c_2);
	}

	// keep track of parentheses nesting level
	switch (c_) {
	case '(':
	case '[':
	case '{':
		if (_tokState->level >= MAXLEVEL) {
			//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"تم تجاوز الحد الاقصى لتداخل الاقواس"));
		}
		_tokState->parenStack[_tokState->level] = c_;
		_tokState->parenLineNoStack[_tokState->level] = _tokState->lineNo;
		_tokState->parenLineNoStack[_tokState->level] = (AlifIntT)(_tokState->start - _tokState->lineStart);
		_tokState->level++;
		if (INSIDE_FSTRING(_tokState)) {
			_currentTok->curlyBracDepth++;
		}
		break;
	case ')':
	case ']':
	case '}':
		//if (INSIDE_FSTRING(tok) and !_currentTok->curlyBracDepth and c_ == '}') {
		//	return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, "f-string: single '}' is not allowed"));
		//}
		//if (!tok->tokExtraTokens and !tok->level) {
		//	return MAKE_TOKEN(alifTokenizer_syntaxError(tok, "unmatched '%c'", c));
		//}
		if (_tokState->level > 0) {
			_tokState->level--;
			AlifIntT opening = _tokState->parenStack[_tokState->level];
			if (!_tokState->tokExtraTokens
				and
				((opening == '(' and c_ == ')')
				or
				(opening == '[' and c_ == ']')
				or
				(opening == '{' and c_ == '}')))
			{
				//if (INSIDE_FSTRING(_tokState) and opening == '{') {
				//	AlifIntT previous_bracket = _currentTok->curlyBracDepth - 1;
				//	if (previous_bracket == _currentTok->curlyBracExprStartDepth) {
				//		return MAKE_TOKEN(alifTokenizer_syntaxError(tok, "f-string: unmatched '%c'", c));
				//	}
				//}
				//if (_tokState->parenLineNoStack[_tokState->level] != _tokState->lineNo) {
				//	return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState,
				//		"closing parenthesis '%c' does not match "
				//		"opening parenthesis '%c' on line %d",
				//		c_, opening, tok->parenlinenostack[_tokState->level]));
				//}
				//else {
				//	return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState,
				//		"closing parenthesis '%c' does not match "
				//		"opening parenthesis '%c'",
				//		c_, opening));
				//}
			}
		}

		if (INSIDE_FSTRING(_tokState)) {
			_currentTok->curlyBracDepth--;
			if (_currentTok->curlyBracDepth < 0) {
				//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, "f-string: unmatched '%c'", c_));
			}
			if (c_ == '}' and _currentTok->curlyBracDepth == _currentTok->curlyBracExprStartDepth) {
				_currentTok->curlyBracExprStartDepth--;
				_currentTok->type = TokenizerModeType_::Token_FStringMode;
				_currentTok->inFormatSpec = 0;
				_currentTok->fStringDebug = 0;
			}
		}
		break;
	default:
		break;
	}

	//if (!ALIF_UNICODE_ISPRINTABLE(wcs)) {
	//	return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"حرف غير قابل للطباعة", wcs));
	//}

	if (c_ == '=' and INSIDE_FSTRING_EXPR(_currentTok)) {
		_currentTok->fStringDebug = 1;
	}

	// punctuation character
	pStart = _tokState->start;
	pEnd = _tokState->cur;
	return MAKE_TOKEN(alifToken_oneChar(c_));
}


//static AlifIntT tokGet_fStringMode(TokenState* _tokState, TokenizerMode* _currentTok, AlifToken* _token) {
//	const wchar_t* pStart{};
//	const wchar_t* pEnd{};
//	AlifIntT endQuoteSize = 0;
//	AlifIntT unicodeEscape = 0;
//
//	_tokState->start = _tokState->cur;
//	_tokState->firstLineNo = _tokState->lineNo;
//	_tokState->startingColOffset = _tokState->colOffset;
//
//	// If we start with a bracket, we defer to the normal mode as there is nothing for us to tokenize
//	// before it.
//	AlifIntT startChar = tok_nextChar(_tokState);
//	if (startChar == '{') {
//		AlifIntT peek1 = tok_nextChar(_tokState);
//		tok_backup(_tokState, peek1);
//		tok_backup(_tokState, startChar);
//		if (peek1 != '{') {
//			_currentTok->curlyBracExprStartDepth++;
//			if (_currentTok->curlyBracExprStartDepth >= MAX_EXPR_NESTING) {
//				//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"نص منسق:التعبير المتداخل للنص المنسق وصل الحد الاقصى لعدد التداخلات"));
//			}
//			TOK_GETMODE(_tokinfo)->type = Token_RegularMode;
//			return tokGet_normalMode(_tokState, _currentTok, _token);
//		}
//	}
//	else {
//		tok_backup(_tokState, startChar);
//	}
//
//	// Check if we are at the end of the string
//	for (AlifIntT i = 0; i < _currentTok->fStringQuoteSize; i++) {
//		AlifIntT quote = tok_nextChar(_tokState);
//		if (quote != _currentTok->fStringQuote) {
//			tok_backup(_tokState, quote);
//			goto fStringMiddle;
//		}
//	}
//
//	if (_currentTok->lastExprBuff != nullptr) {
//		alifMem_dataFree(_currentTok->lastExprBuff);
//		_currentTok->lastExprBuff = nullptr;
//		_currentTok->lastExprSize = 0;
//		_currentTok->lastExprEnd = -1;
//	}
//
//	pStart = _tokState->start;
//	pEnd = _tokState->cur;
//	_tokState->tokModeStackIndex--;
//	return MAKE_TOKEN(FSTRINGEND);
//
//fStringMiddle:
//
//	// TODO: This is a bit of a hack, but it works for now. We need to find a better way to handle
//	// this.
//	_tokState->multiLineStart = _tokState->lineStart;
//	while (endQuoteSize != _currentTok->fStringQuoteSize) {
//		AlifIntT wcs = tok_nextChar(_tokState);
//		if (_tokState->done == E_ERROR) return MAKE_TOKEN(ERRORTOKEN);
//
//		AlifIntT inFormatSpec = (_currentTok->lastExprEnd != -1 and INSIDE_FSTRING_EXPR(_currentTok));
//
//		if (wcs == EOF or (_currentTok->fStringQuoteSize == 1 and wcs == '\n')) {
//
//			// If we are in a format spec and we found a newline,
//			// it means that the format spec ends here and we should
//			// return to the regular mode.
//			if (inFormatSpec and wcs == '\n') {
//				tok_backup(_tokState, wcs);
//				TOK_GETMODE(_tokState)->type = Token_RegularMode;
//				pStart = _tokState->start;
//				pEnd = _tokState->cur;
//				return MAKE_TOKEN(FSTRINGMIDDLE);
//			}
//
//			// shift the tok_state's location into
//			// the start of string, and report the error
//			// from the initial quote character
//			_tokState->cur = (wchar_t*)_currentTok->fStringStart;
//			_tokState->cur++;
//			_tokState->lineStart = _currentTok->fStringMultiLineStart;
//			AlifIntT start = _tokState->lineNo;
//
//			TokenizerMode* currentTok = TOK_GETMODE(_tokState);
//			_tokState->lineNo = currentTok->fStringLineStart;
//
//			if (currentTok->fStringQuoteSize == 3) {
//				//alifTokenizer_syntaxError(_tokState, L"نص متعدد الاسطر غير منتهي" L" (السطر %d)", start);
//				if (wcs != '\n') {
//					_tokState->done = E_EOFS;
//				}
//				return MAKE_TOKEN(ERRORTOKEN);
//			}
//			else {
//				//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, L"نص منسق غير منتهي" L" السطر %d)", start));
//			}
//		}
//
//		if (wcs == _currentTok->fStringQuote) {
//			endQuoteSize++;
//			continue;
//		}
//		else {
//			endQuoteSize = 0;
//		}
//
//		if (wcs == '{') {
//			AlifIntT peek = tok_nextChar(_tokState);
//			if (peek != '{' or inFormatSpec) {
//				tok_backup(_tokState, peek);
//				tok_backup(_tokState, wcs);
//				_currentTok->curlyBracExprStartDepth++;
//				if (_currentTok->curlyBracExprStartDepth >= MAX_EXPR_NESTING) {
//					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokState, "نص منسق:التعبير المتداخل للنص المنسق وصل الحد الاقصى لعدد التداخلات"));
//				}
//				TOK_GETMODE(_tokState)->type = Token_RegularMode;
//				pStart = _tokState->start;
//				pEnd = _tokState->cur;
//			}
//			else {
//				pStart = _tokState->start;
//				pEnd = _tokState->cur - 1;
//			}
//			return MAKE_TOKEN(FSTRINGMIDDLE);
//		}
//		else if (wcs == '}') {
//			if (unicodeEscape) {
//				pStart = _tokState->start;
//				pEnd = _tokState->cur;
//				return MAKE_TOKEN(FSTRINGMIDDLE);
//			}
//			AlifIntT peek = tok_nextChar(_tokState);
//
//			// The tokenizer can only be in the format spec if we have already completed the expression
//			// scanning (indicated by the end of the expression being set) and we are not at the top level
//			// of the bracket stack (-1 is the top level). Since format specifiers can't legally use double
//			// brackets, we can bypass it here.
//			if (peek == '}' and !inFormatSpec) {
//				pStart = _tokState->start;
//				pEnd = _tokState->cur - 1;
//			}
//			else {
//				tok_backup(_tokState, peek);
//				tok_backup(_tokState, wcs);
//				TOK_GETMODE(_tokState)->type = Token_RegularMode;
//				pStart = _tokState->start;
//				pEnd = _tokState->cur;
//			}
//			return MAKE_TOKEN(FSTRINGMIDDLE);
//		}
//		else if (wcs == '\\') {
//			AlifIntT peek = tok_nextChar(_tokState);
//			if (peek == '\r') {
//				peek = tok_nextChar(_tokState);
//			}
//			// Special case when the backslash is right before a curly
//			// brace. We have to restore and return the control back
//			// to the loop for the next iteration.
//			if (peek == '{' or peek == '}') {
//				if (!_currentTok->fStringRaw) {
//					//if (alifTokenizer_warnInvalidEscapeSequence(_tokState, peek)) {
//					//	return MAKE_TOKEN(ERRORTOKEN);
//					//}
//				}
//				tok_backup(_tokState, peek);
//				continue;
//			}
//
//			if (!_currentTok->fStringRaw) {
//				if (peek == 'N') {
//					/* Handle named unicode escapes (\N{BULLET}) */
//					peek = tok_nextChar(_tokState);
//					if (peek == '{') {
//						unicodeEscape = 1;
//					}
//					else {
//						tok_backup(_tokState, peek);
//					}
//				}
//			} /* else {
//				skip the escaped character
//			}*/
//		}
//	}
//
//	// Backup the f-string quotes to emit a final FSTRINGMIDDLE and
//	// add the quotes to the FSTRINGEND in the next tokenizer iteration.
//	for (AlifIntT i = 0; i < _currentTok->fStringQuoteSize; i++) {
//		tok_backup(_tokState, _currentTok->fStringQuote);
//	}
//	pStart = _tokState->start;
//	pEnd = _tokState->cur;
//	return MAKE_TOKEN(FSTRINGMIDDLE);
//}


static AlifIntT token_get(TokenState* _tokState, AlifToken* _token) { // 1481
	TokenizerMode* currentTok = TOK_GET_MODE(_tokState);
	if (currentTok->type == TokenizerModeType_::Token_RegularMode) {
		return tokGet_normalMode(_tokState, currentTok, _token);
	}
	else {
		return tokGet_fStringMode(_tokState, currentTok, _token);
	}
}

AlifIntT alifTokenizer_get(TokenState* _tokState, AlifToken* _token) { // 1492
	AlifIntT result = token_get(_tokState, _token);
	if (_tokState->decodingErred) {
		result = ERRORTOKEN;
		_tokState->done = E_DECODE;
	}
	return result;
}
