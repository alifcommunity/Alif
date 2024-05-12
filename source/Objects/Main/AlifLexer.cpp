#include "alif.h"

#include "AlifCore_AlifToken.h"
#include "ErrorCode.h"

#include "AlifTokenState.h"
#include "Helpers.h"
#include "AlifCore_Memory.h" // temp

#pragma warning(disable : 4996) // for disable unsafe functions error // temp

// Alternate tab spacing
#define ALTTABSIZE 1

#define IS_IDENTIFIER_START(wc) ((wc >= L'a' and wc <= L'z') \
								or (wc >= L'A' and wc <= L'Z') /* to exclude nums and symbols */ \
								or (wc == L'_') \
								or (wc < L'٠' and wc >= 128) \
								or (wc > L'٩' and wc >= 128)) /* exclude arabic-indic nums */

#define IS_IDENTIFIER_WCHAR(wc) ((wc >= L'a' and wc <= L'z') \
								or (wc >= L'A' and wc <= L'Z') \
								or (wc >= L'0' and wc <= L'9') \
								or (wc == L'_') \
								or (wc >= 128))

#define TOK_GETMODE(tok) (&(_tokInfo->tokModeStack[_tokInfo->tokModeStackIndex]))

#define MAKE_TOKEN(TT) alifLexer_setupToken(_tokInfo, _token, TT, pStart, pEnd)


static void tok_backup(TokenInfo* _tokInfo, int _wc) { // 96
	if (_wc != WEOF) {
		if (--_tokInfo->cur < _tokInfo->buf) {
			// error
		}
		if ((int)(wchar_t)*_tokInfo->cur != ALIF_WCHARMASK(_wc)) {
			// error
		}
		_tokInfo->colOffset--;
	}
}


static int tok_nextChar(TokenInfo* _tokInfo) { // 56
	int rc{};
	for (;;) {
		if (_tokInfo->cur != _tokInfo->inp) {
			if ((unsigned int)_tokInfo->colOffset >= (unsigned int)INT_MAX) {
				//_tokInfo->done = E_COLOMN_OVERFLOW;
				return WEOF;
			}
			_tokInfo->colOffset++;
			return ALIF_WCHARMASK(*_tokInfo->cur++);
		}
		if (_tokInfo->done != E_OK) {
			return WEOF;
		}

		rc = _tokInfo->underflow(_tokInfo);

		if (!rc) {
			_tokInfo->cur = _tokInfo->inp;
			return WEOF;
		}
		_tokInfo->lineStart = _tokInfo->cur;

	}
}


static AlifIntT set_fStringExpr(TokenInfo* _tokInfo, AlifToken* _token, wchar_t _wcs) { // 110

	TokenizerMode* tokMode = TOK_GETMODE(_tokInfo);

	if (_token->data) return 0;

	AlifObject* res = nullptr;

	// Check if there is a # character in the expression
	AlifIntT hashDetected = 0;
	for (AlifUSizeT i = 0; i < tokMode->lastExprSize - tokMode->lastExprEnd; i++) {
		if (tokMode->lastExprBuff[i] == L'#') {
			hashDetected = 1;
			break;
		}
	}

	if (hashDetected) {
		AlifUSizeT inputLength = tokMode->lastExprSize - tokMode->lastExprEnd;
		wchar_t* result = (wchar_t*)alifMem_dataAlloc((inputLength + 1) * sizeof(wchar_t));
		if (!result) return -1;

		AlifUSizeT i = 0;
		AlifUSizeT j = 0;

		for (i = 0, j = 0; i < inputLength; i++) {
			if (tokMode->lastExprBuff[i] == '#') {
				// Skip characters until newline or end of string
				while (tokMode->lastExprBuff[i] != L'\0' && i < inputLength) {
					if (tokMode->lastExprBuff[i] == L'\n') {
						result[j++] = tokMode->lastExprBuff[i];
						break;
					}
					i++;
				}
			}
			else {
				result[j++] = tokMode->lastExprBuff[i];
			}
		}

		result[j] = L'\0';  // Null-terminate the result string
		res = alifUnicode_decodeUTF8(result, j, nullptr);
		alifMem_dataFree(result);
	}
	else {
		res = alifUnicode_decodeUTF8(tokMode->lastExprBuff, tokMode->lastExprSize - tokMode->lastExprEnd, nullptr);
	}

	if (!res) return -1;
	_token->data = res;
	return 0;
}


AlifIntT alifLexer_updateFStringExpr(TokenInfo* _tokInfo, wchar_t _cur) { // 175

	AlifUSizeT size = wcslen(_tokInfo->cur);
	TokenizerMode* tokMode = TOK_GETMODE(_tokInfo);
	wchar_t* newBuffer{};

	switch (_cur) {

	case 0:
		if (!tokMode->lastExprBuff or tokMode->lastExprEnd >= 0) {
			return 1;
		}
		newBuffer = (wchar_t*)alifMem_dataRealloc(tokMode->lastExprBuff, tokMode->lastExprSize + size);

		if (newBuffer == nullptr) {
			alifMem_dataFree(tokMode->lastExprBuff);
			goto error;
		}
		tokMode->lastExprBuff = newBuffer;
		wcsncpy(tokMode->lastExprBuff + tokMode->lastExprSize, _tokInfo->cur, size);
		tokMode->lastExprSize += size;
		break;
	case L'{':
		if (tokMode->lastExprBuff != nullptr) {
			alifMem_dataFree(tokMode->lastExprBuff);
		}
		tokMode->lastExprBuff = (wchar_t*)alifMem_dataAlloc(size);
		if (tokMode->lastExprBuff == nullptr) {
			goto error;
		}
		tokMode->lastExprSize = size;
		tokMode->lastExprEnd = -1;
		wcsncpy(tokMode->lastExprBuff, _tokInfo->cur, size);
		break;
	case L'}':
	case L'!':
	case L':':
		if (tokMode->lastExprEnd == -1) {
			tokMode->lastExprEnd = wcslen(_tokInfo->start);
		}
		break;
	default:
		//ALIF_UNREACHABLE();
		break;
	}

	return 1;

error:
	_tokInfo->done = E_NOMEM;
	return 0;
}


static int tok_decimalTail(TokenInfo* _tokInfo) { // 365
	int wcs{};

	while (1) {
		do {
			wcs = tok_nextChar(_tokInfo);
		} while (ALIF_ISDIGIT(wcs));
		if (wcs != L'_') {
			break;
		}
		wcs = tok_nextChar(_tokInfo);
		if (!ALIF_ISDIGIT(wcs)) {
			tok_backup(_tokInfo, wcs);
			// error
			return 0;
		}
	}
	return wcs;
}

static inline AlifIntT tok_continuationLine(TokenInfo* _tokInfo) { // 387

	AlifIntT wcs = tok_nextChar(_tokInfo);
	if (wcs == L'\r') wcs = tok_nextChar(_tokInfo);
	if (wcs != L'\n') { _tokInfo->done = E_LINECONT; return -1; }

	wcs = tok_nextChar(_tokInfo);
	if (wcs == WEOF) {
		_tokInfo->done = E_WEOF;
		_tokInfo->cur = _tokInfo->inp;
		return -1;
	}
	else {
		tok_backup(_tokInfo, wcs);
	}

	return wcs;
}

static int tokGet_normalMode(TokenInfo* _tokInfo, TokenizerMode* _currentTok, AlifToken* _token) { // 408
	int wcs{};
	int blankLine{};

	const wchar_t* pStart{};
	const wchar_t* pEnd{};

nextline:
	_tokInfo->start = nullptr;
	_tokInfo->startingColOffset = -1;
	blankLine = 0;

	// indentation level
	if (_tokInfo->atBeginOfLine) {
		int col = 0;
		int altCol = 0;
		_tokInfo->atBeginOfLine = 0;
		int contLineCol = 0;

		for (;;) {
			wcs = tok_nextChar(_tokInfo);
			if (wcs == L' ') { col++; altCol++; }
			else if (wcs == L'\t')
			{
				col = (col / _tokInfo->tabSize + 1) * _tokInfo->tabSize;
				altCol = (altCol / ALTTABSIZE + 1) * ALTTABSIZE;
			}
			else if (wcs == L'\\') {
				contLineCol = contLineCol ? contLineCol : col;
				if ((wcs = tok_continuationLine(_tokInfo)) == -1) return MAKE_TOKEN(ERRORTOKEN);
			}
			else break;
		}
		tok_backup(_tokInfo, wcs);
		if (wcs == L'#' or wcs == L'\n' or wcs == L'\r') {
			if (col == 0 and wcs == L'\n' and _tokInfo->prompt != nullptr) blankLine = 0;
			else if (_tokInfo->prompt != nullptr and _tokInfo->lineNo == 1) {
				blankLine = 0;
				col = altCol = 0;
			}
			else { blankLine = 1; }
		}
		if (!blankLine and _tokInfo->level == 0) {
			col = contLineCol ? contLineCol : col;
			altCol = contLineCol ? contLineCol : altCol;
			if (col == _tokInfo->indStack[_tokInfo->indent])
			{
				if (altCol != _tokInfo->alterIndStack[_tokInfo->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokInfo)); // indent error
				}
			}
			else if (col > _tokInfo->indStack[_tokInfo->indent]) {
				// indent - always one
				if (_tokInfo->indent + 1 >= MAXINDENT) {
					_tokInfo->done = E_TOODEEP;
					_tokInfo->cur = _tokInfo->inp;
					return MAKE_TOKEN(ERRORTOKEN);
				}
				if (altCol <= _tokInfo->alterIndStack[_tokInfo->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokInfo)); // indent error
				}
				_tokInfo->pendInd++;
				_tokInfo->indStack[++_tokInfo->indent] = col;
				_tokInfo->alterIndStack[++_tokInfo->indent] = altCol;
			}
			else {
				/* col < tok->indstack[tok->indent] */
				/* Dedent -- any number, must be consistent */

				while (_tokInfo->indent > 0 and col < _tokInfo->indStack[_tokInfo->indent]) {
					_tokInfo->pendInd--;
					_tokInfo->indent--;
				}
				if (col != _tokInfo->indStack[_tokInfo->indent]) {
					_tokInfo->done = E_DEDENT;
					_tokInfo->cur = _tokInfo->inp;
					return MAKE_TOKEN(ERRORTOKEN);
				}
				if (altCol != _tokInfo->alterIndStack[_tokInfo->indent]) {
					//return MAKE_TOKEN(alifTokenizer_indentError(_tokInfo)); // indent error
				}
			}
		}
	}

	_tokInfo->start = _tokInfo->cur;
	_tokInfo->startingColOffset = _tokInfo->colOffset;

	// return pending indents/dedents
	if (_tokInfo->pendInd != 0) {
		if (_tokInfo->pendInd < 0) {
			if (_tokInfo->tokExtraTokens) {
				pStart = _tokInfo->cur;
				pEnd = _tokInfo->cur;
			}
			_tokInfo->pendInd++;
			return MAKE_TOKEN(DEDENT);
		}
		else {
			if (_tokInfo->tokExtraTokens) {
				pStart = _tokInfo->buf;
				pEnd = _tokInfo->cur;
			}
			_tokInfo->pendInd--;
			return MAKE_TOKEN(INDENT);
		}
	}


	wcs = tok_nextChar(_tokInfo);
	tok_backup(_tokInfo, wcs);

again:
	_tokInfo->start = nullptr;

	// skip spaces
	do {
		wcs = tok_nextChar(_tokInfo);
	} while (wcs == L' ' or wcs == L'\t' or wcs == L'\014');

	// Set start of current token
	_tokInfo->start = _tokInfo->cur == nullptr ? nullptr : _tokInfo->cur - 1;
	_tokInfo->startingColOffset = _tokInfo->colOffset - 1;

	// Skip comment
	if (wcs == L'#') {
		const wchar_t* p{};
		AlifIntT currentStartingColOffset{};

		while (wcs != WEOF and wcs != L'\n' and wcs != L'\r') {
			wcs = tok_nextChar(_tokInfo);
		}

		if (_tokInfo->tokExtraTokens) { p = _tokInfo->start; }

		if (_tokInfo->tokExtraTokens) {
			tok_backup(_tokInfo, wcs);
			pStart = p;
			pEnd = _tokInfo->cur;
			_tokInfo->commentNewline = blankLine;
			return MAKE_TOKEN(COMMENT);
		}
	}

	if (_tokInfo->done == E_INTERACT_STOP) {
		return MAKE_TOKEN(ENDMARKER);
	}

	// check for EOF and errors now
	if (wcs == WEOF) {
		if (_tokInfo->level) {
			return MAKE_TOKEN(ERRORTOKEN);
		}
		return MAKE_TOKEN(_tokInfo->done == E_WEOF ? ENDMARKER : ERRORTOKEN);
	}


	/* Identifire */
	if (IS_IDENTIFIER_START(wcs)) {
		AlifIntT b_ = 0, r_ = 0, u_ = 0, f_ = 0;
		while (true) {
			if (!(b_ or u_ or f_) and wcs == L'ب') b_ = 1; // ب = بايت
			else if (!(b_ or u_ or r_) and wcs == L'ت') u_ = 1; // ت = ترميز
			else if (!(r_ or u_) and wcs == L'خ') r_ = 1; // خ = خام
			else if (!(f_ or b_ or u_) and wcs == L'م') f_ = 1; // م = منسق
			else break;

			wcs = tok_nextChar(_tokInfo);
			if (wcs == L'"' or wcs == L'\'') {
				if (f_) goto fStringQuote;
				goto letterQuote;
			}
		}
		while (IS_IDENTIFIER_WCHAR(wcs)) { // يجب مراجعة وظيفة هذا التحقق
			wcs = tok_nextChar(_tokInfo);
		}
		tok_backup(_tokInfo, wcs);

		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;

		return MAKE_TOKEN(NAME);
	}

	if (wcs == L'\r') { wcs = tok_nextChar(_tokInfo); }

	/* Newline */
	if (wcs == L'\n') {
		_tokInfo->atBeginOfLine = 1;
		if (blankLine or _tokInfo->level > 0) {
			if (_tokInfo->tokExtraTokens) {
				if (_tokInfo->commentNewline) {
					_tokInfo->commentNewline = 0;
				}
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
				return MAKE_TOKEN(NL);
			}
			goto nextline;
		}
		if (_tokInfo->commentNewline and _tokInfo->tokExtraTokens) {
			_tokInfo->commentNewline = 0;
			pStart = _tokInfo->start;
			pEnd = _tokInfo->cur;
			return MAKE_TOKEN(NL);
		}
		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur - 1;
		_tokInfo->countLine = 0; // Leave L'\n' out of the string

		return MAKE_TOKEN(NEWLINE);
	}

	/* Period or number starting with period? */
	if (wcs == '.') {

		wcs = tok_nextChar(_tokInfo);
		if (ALIF_ISDIGIT(wcs)) {
			goto fraction;
		}

		tok_backup(_tokInfo, wcs);
		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;

		return MAKE_TOKEN(DOT);
	}


	/* Number */
	if (ALIF_ISDIGIT(wcs)) {
		if (wcs == L'0') {
			/* Hex or Octal or Binary */
			wcs = tok_nextChar(_tokInfo);
			if (wcs == L'ه') {
				wcs = tok_nextChar(_tokInfo);
				do {
					if (wcs == L'_') {
						wcs = tok_nextChar(_tokInfo);
					}
					if (!ALIF_ISXDIGIT(wcs)) {
						tok_backup(_tokInfo, wcs);
						//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ستعشري غير صحيح"));
					}
					do {
						wcs = tok_nextChar(_tokInfo);
					} while (ALIF_ISXDIGIT(wcs));
				} while (wcs == L'_');
				//if (!verify_endOfNumber(_tokInfo, wcs, L"ستعشري")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
			else if (wcs == L'ث') {
				// Octal
				wcs = tok_nextChar(_tokInfo);
				do {
					if (wcs == L'_') { wcs = tok_nextChar(_tokInfo); }
					if (wcs < L'0' or wcs >= L'8') {
						if (ALIF_ISDIGIT(wcs)) {
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثماني غير صحيح '%wcs'", wcs));
						}
						else {
							tok_backup(_tokInfo, wcs);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثماني غير صحيح"));
						}
					}
					do {
						wcs = tok_nextChar(_tokInfo);
					} while (L'0' <= wcs and wcs < L'8');
				} while (wcs == L'-');
				if (ALIF_ISDIGIT(wcs)) {
					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثماني غير صحيح '%wcs'", wcs));
				}
				//if (!verify_endOfNumber(_tokInfo, wcs, L"ثماني")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
			else if (wcs == L'ن') {
				// Binary
				wcs = tok_nextChar(_tokInfo);
				do {
					if (wcs == L'_') { wcs = tok_nextChar(_tokInfo); }

					if (wcs != L'0' and wcs != L'1') {
						if (ALIF_ISDIGIT(wcs)) {
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثنائي غير صحيح '%wcs'", wcs));
						}
						else {
							tok_nextChar(_tokInfo);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثنائي غير صحيح"));
						}
					}
					do {
						wcs = tok_nextChar(_tokInfo);
					} while (wcs == L'0' or wcs == L'1');
				} while (wcs == L'_');

				if (ALIF_ISDIGIT(wcs)) {
					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم ثنائي غير صحيح '%wcs'", wcs));
				}
				//if (!verify_endOfNumber(_tokInfo, wcs, L"ثنائي")) {
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
			wcs = tok_decimalTail(_tokInfo);
			if (wcs == 0) return MAKE_TOKEN(ERRORTOKEN);
			{
				/* float number */
				if (wcs == L'.') {
					wcs = tok_nextChar(_tokInfo);
				fraction:
					// Fraction
					if (ALIF_ISDIGIT(wcs)) {
						wcs = tok_decimalTail(_tokInfo);
						if (wcs == 0) return MAKE_TOKEN(ERRORTOKEN);
					}
				}
				if (wcs == L'س') { /* exponent */
					AlifIntT e{};
				exponent:
					e = wcs;
					wcs = tok_nextChar(_tokInfo);
					if (wcs == L'+' or wcs == L'-') {
						wcs = tok_nextChar(_tokInfo);
						if (!ALIF_ISDIGIT(wcs)) {
							tok_backup(_tokInfo, wcs);
							//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"رقم عشري غير صحيح"));
						}
					}
					else if (!ALIF_ISDIGIT(wcs)) {
						tok_backup(_tokInfo, wcs);
						//if (!verify_endOfNumber(_tokInfo, e, L"عشري")) {
						//	return MAKE_TOKEN(ERRORTOKEN);
						//}
						tok_backup(_tokInfo, e);
						pStart = _tokInfo->start;
						pEnd = _tokInfo->cur;
						return MAKE_TOKEN(NUMBER);
					}
					wcs = tok_decimalTail(_tokInfo);
					if (wcs == 0) return MAKE_TOKEN(ERRORTOKEN);
				}
				if (wcs == L'ت') {
					/* Imaginary part */
				imaginary:
					wcs = tok_nextChar(_tokInfo);
					//if (!verify_endOfNumber(_tokInfo, c, L"تخيلي")) {
					//	return MAKE_TOKEN(ERRORTOKEN);
					//}
				}
				//else if (!verify_endOfNumber(_tokInfo, c, L"عشري")) {
				//	return MAKE_TOKEN(ERRORTOKEN);
				//}
			}
		}

		tok_backup(_tokInfo, wcs);
		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;
		return MAKE_TOKEN(NUMBER);
	}

fStringQuote:
	if ((ALIF_TOLOWER(*_tokInfo->start) == L'م' or ALIF_TOLOWER(*_tokInfo->start) == L'خ')
		and (wcs == L'\'' or wcs == L'"'))
	{
		AlifIntT quote = wcs;
		AlifIntT quoteSize = 1;

		_tokInfo->firstLineNo = _tokInfo->lineNo;
		_tokInfo->multiLineStart = _tokInfo->lineStart;

		AlifIntT afterQuote = tok_nextChar(_tokInfo);
		if (afterQuote == quote) {
			AlifIntT afterAfterQuote = tok_nextChar(_tokInfo);
			if (afterAfterQuote == quote) {
				quoteSize = 3;
			}
			else {
				tok_backup(_tokInfo, afterAfterQuote);
				tok_backup(_tokInfo, afterQuote);
			}
		}
		if (afterQuote != quote) {
			tok_backup(_tokInfo, afterQuote);
		}

		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;
		if (_tokInfo->tokModeStackIndex + 1 >= MAXFSTRING_LEVEL) {
			//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"لقد تجاوزت الحد الاقصى للنص المنسق المتداخل"));
		}
		TokenizerMode* currentTok = TOK_GETMODE(_tokInfo);
		currentTok->type = Token_FStringMode;
		currentTok->fStringQuote = quote;
		currentTok->fStringQuoteSize = quoteSize;
		currentTok->fStringStart = _tokInfo->start;
		currentTok->fStringMultiLineStart = _tokInfo->lineStart;
		currentTok->fStringLineStart = _tokInfo->lineNo;
		currentTok->fStringStartOffset = -1;
		currentTok->fStringMultiLineStartOffset = -1;
		currentTok->lastExprBuff = nullptr;
		currentTok->lastExprSize = 0;
		currentTok->lastExprEnd = -1;

		if (*_tokInfo->start == L'م') {
			currentTok->fStringRaw = ALIF_TOLOWER(*(_tokInfo->start + 1)) == L'خ';
		}
		else if (*_tokInfo->start == L'خ') {
			currentTok->fStringRaw = 1;
		}
		else {
			//ALIF_UNREACHABLE();
		}

		currentTok->curlyBracDepth = 0;
		currentTok->curlyBracExprStartDepth = -1;
		return MAKE_TOKEN(FSTRINGSTART);
	}

letterQuote:
	if (wcs == L'\'' or wcs == L'"') {
		int quote = wcs;
		int quoteSize = 1;
		int endQuoteSize = 0;
		int hasEscapedQuote = 0;

		_tokInfo->firstLineNo = _tokInfo->lineNo;
		_tokInfo->multiLineStart = _tokInfo->lineStart;

		// find the quote size and start of string
		wcs = tok_nextChar(_tokInfo);
		if (wcs == quote) {
			wcs = tok_nextChar(_tokInfo);
			if (wcs == quote) {
				quoteSize = 3;
			}
			else {
				endQuoteSize = 1; // اي انه نص فارغ
			}
		}
		if (wcs != quote) {
			tok_backup(_tokInfo, wcs);
		}

		/* the rest of STRING */
		while (endQuoteSize != quoteSize) {
			wcs = tok_nextChar(_tokInfo);
			if (_tokInfo->done == E_ERROR) {
				return MAKE_TOKEN(ERRORTOKEN);
			}
			if (_tokInfo->done == E_DECODE) break;

			if (wcs == EOF or (quoteSize == 1 and wcs == L'\n')) {
				_tokInfo->cur = (wchar_t*)_tokInfo->start;
				_tokInfo->cur++;
				_tokInfo->lineStart = _tokInfo->multiLineStart;
				AlifIntT start = _tokInfo->lineNo;
				_tokInfo->lineNo = _tokInfo->firstLineNo;

				if (INSIDE_FSTRING(_tokInfo)) {
					TokenizerMode* currentToken = TOK_GETMODE(_tokInfo);
					if (currentToken->fStringQuote == quote and currentToken->fStringQuoteSize == quoteSize) {
						//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"خطأ في النص المنسق '{'", start));
					}
				}

				if (quoteSize == 3) {
					//alifTokenizer_syntaxError(_tokInfo, L"نص متعدد الاسطر لم يتم إنهاؤه" " في السطر %d", start);
					if (wcs != L'\n') {
						_tokInfo->done = E_EOFS;
					}
					return MAKE_TOKEN(ERRORTOKEN);
				}
				else {
					if (hasEscapedQuote) {
						//alifTokenizer_syntaxError(_tokInfo, L"نص لم يتم إنهاؤه" " في السطر %d", start);
					}
					else {
						//alifTokenizer_syntaxError(_tokInfo, L"نص لم يتم إنهاؤه" " في السطر %d", start);
					}
					if (wcs != L'\n') {
						_tokInfo->done = E_EOLS;
					}
					return MAKE_TOKEN(ERRORTOKEN);
				}
			}
			if (wcs == quote) {
				endQuoteSize++;
			}
			else {
				endQuoteSize = 0;
				if (wcs == L'\\') {
					wcs = tok_nextChar(_tokInfo);  // skip escape char
					if (wcs == quote) {	           // record if the escape was a quote
						hasEscapedQuote = 1;
					}
					if (wcs == L'\r') {
						wcs = tok_nextChar(_tokInfo);
					}
				}
			}
		}

		pStart = _tokInfo->start;
		pEnd = _tokInfo->cur;
		return MAKE_TOKEN(STRING);
	}

	/* line continuation */
	if (wcs == L'\\') {
		if ((wcs = tok_continuationLine(_tokInfo)) == -1) {
			return MAKE_TOKEN(ERRORTOKEN);
		}
		_tokInfo->countLine = 1;
		goto again; // read next line
	}


	// Punctuation character
	AlifIntT isPunctuation = (wcs == L':' or wcs == L'}' or wcs == L'!' or wcs == L'}');
	if (isPunctuation and INSIDE_FSTRING(_tokInfo) and INSIDE_FSTRING_EXPR(_currentTok)) {
		AlifIntT cursor = _currentTok->curlyBracDepth - (wcs != L'{');
		if (cursor == 0 and !alifLexer_updateFStringExpr(_tokInfo, wcs)) {
			return MAKE_TOKEN(ENDMARKER);
		}
		if (cursor == 0 and wcs != L'{' and set_fStringExpr(_tokInfo, _token, wcs)) {
			return MAKE_TOKEN(ERRORTOKEN);
		}

		if (wcs == L':' and cursor == _currentTok->curlyBracExprStartDepth) {
			_currentTok->type = Token_FStringMode;
			pStart = _tokInfo->start;
			pEnd = _tokInfo->cur;
			return MAKE_TOKEN(alifToken_oneChar(wcs));
		}
	}


	{ // two_character token
		AlifIntT wcs2 = tok_nextChar(_tokInfo);
		AlifIntT currToken = alifToken_twoChars(wcs, wcs2);
		if (currToken != OP) {
			AlifIntT wcs3 = tok_nextChar(_tokInfo);
			AlifIntT currentTok3 = alifToken_threeChars(wcs, wcs2, wcs3);
			if (currentTok3 != OP) {
				currToken = currentTok3;
			}
			else {
				tok_backup(_tokInfo, wcs3);
			}
			pStart = _tokInfo->start;
			pEnd = _tokInfo->cur;
		}
		tok_backup(_tokInfo, wcs2);
	}

	// keep track of parentheses nesting level
	switch (wcs) {
	case L'(':
	case L'[':
	case L'{':
		if (_tokInfo->level >= MAXLEVEL) {
			//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"تم تجاوز الحد الاقصى لتداخل الاقواس"));
		}
		_tokInfo->parenStack[_tokInfo->level] = wcs;
		_tokInfo->parenLineNoStack[_tokInfo->level] = _tokInfo->lineNo;
		_tokInfo->parenLineNoStack[_tokInfo->level] = (AlifIntT)(_tokInfo->start - _tokInfo->lineStart);
		_tokInfo->level++;
		if (INSIDE_FSTRING(_tokInfo)) {
			_currentTok->curlyBracDepth++;
		}
		break;
	case L')':
	case L']':
	case L'}':
		// code here






		if (INSIDE_FSTRING(_tokInfo)) {
			_currentTok->curlyBracDepth--;
			if (wcs == L'}' and _currentTok->curlyBracDepth == _currentTok->curlyBracExprStartDepth) {
				_currentTok->curlyBracExprStartDepth--;
				_currentTok->type = Token_FStringMode;
			}
		}
		break;
	default:
		break;
	}

	//if (!ALIF_UNICODE_ISPRINTABLE(wcs)) {
	//	return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"حرف غير قابل للطباعة", wcs));
	//}

	// punctuation character
	pStart = _tokInfo->start;
	pEnd = _tokInfo->cur;
	return MAKE_TOKEN(alifToken_oneChar(wcs));
}


static AlifIntT tokGet_fStringMode(TokenInfo* _tokInfo, TokenizerMode* _currentTok, AlifToken* _token) {
	const wchar_t* pStart{};
	const wchar_t* pEnd{};
	AlifIntT endQuoteSize = 0;
	AlifIntT unicodeEscape = 0;

	_tokInfo->start = _tokInfo->cur;
	_tokInfo->firstLineNo = _tokInfo->lineNo;
	_tokInfo->startingColOffset = _tokInfo->colOffset;

	// If we start with a bracket, we defer to the normal mode as there is nothing for us to tokenize
	// before it.
	AlifIntT startChar = tok_nextChar(_tokInfo);
	if (startChar == L'{') {
		AlifIntT peek1 = tok_nextChar(_tokInfo);
		tok_backup(_tokInfo, peek1);
		tok_backup(_tokInfo, startChar);
		if (peek1 != L'{') {
			_currentTok->curlyBracExprStartDepth++;
			if (_currentTok->curlyBracExprStartDepth >= MAX_EXPR_NESTING) {
				//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"نص منسق:التعبير المتداخل للنص المنسق وصل الحد الاقصى لعدد التداخلات"));
			}
			TOK_GETMODE(_tokinfo)->type = Token_RegularMode;
			return tokGet_normalMode(_tokInfo, _currentTok, _token);
		}
	}
	else {
		tok_backup(_tokInfo, startChar);
	}

	// Check if we are at the end of the string
	for (AlifIntT i = 0; i < _currentTok->fStringQuoteSize; i++) {
		int quote = tok_nextChar(_tokInfo);
		if (quote != _currentTok->fStringQuote) {
			tok_backup(_tokInfo, quote);
			goto fStringMiddle;
		}
	}

	if (_currentTok->lastExprBuff != nullptr) {
		alifMem_dataFree(_currentTok->lastExprBuff);
		_currentTok->lastExprBuff = NULL;
		_currentTok->lastExprSize = 0;
		_currentTok->lastExprEnd = -1;
	}

	pStart = _tokInfo->start;
	pEnd = _tokInfo->cur;
	_tokInfo->tokModeStackIndex--;
	return MAKE_TOKEN(FSTRINGEND);

fStringMiddle:

	// TODO: This is a bit of a hack, but it works for now. We need to find a better way to handle
	// this.
	_tokInfo->multiLineStart = _tokInfo->lineStart;
	while (endQuoteSize != _currentTok->fStringQuoteSize) {
		AlifIntT wcs = tok_nextChar(_tokInfo);
		if (_tokInfo->done == E_ERROR) return MAKE_TOKEN(ERRORTOKEN);

		AlifIntT inFormatSpec = (_currentTok->lastExprEnd != -1 and INSIDE_FSTRING_EXPR(_currentTok));

		if (wcs == WEOF or (_currentTok->fStringQuoteSize == 1 and wcs == L'\n')) {

			// If we are in a format spec and we found a newline,
			// it means that the format spec ends here and we should
			// return to the regular mode.
			if (inFormatSpec and wcs == L'\n') {
				tok_backup(_tokInfo, wcs);
				TOK_GETMODE(_tokInfo)->type = Token_RegularMode;
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
				return MAKE_TOKEN(FSTRINGMIDDLE);
			}

			// shift the tok_state's location into
			// the start of string, and report the error
			// from the initial quote character
			_tokInfo->cur = (wchar_t*)_currentTok->fStringStart;
			_tokInfo->cur++;
			_tokInfo->lineStart = _currentTok->fStringMultiLineStart;
			AlifIntT start = _tokInfo->lineNo;

			TokenizerMode* currentTok = TOK_GETMODE(_tokInfo);
			_tokInfo->lineNo = currentTok->fStringLineStart;

			if (currentTok->fStringQuoteSize == 3) {
				//alifTokenizer_syntaxError(_tokInfo, L"نص متعدد الاسطر غير منتهي" L" (السطر %d)", start);
				if (wcs != L'\n') {
					_tokInfo->done = E_EOFS;
				}
				return MAKE_TOKEN(ERRORTOKEN);
			}
			else {
				//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, L"نص منسق غير منتهي" L" السطر %d)", start));
			}
		}

		if (wcs == _currentTok->fStringQuote) {
			endQuoteSize++;
			continue;
		}
		else {
			endQuoteSize = 0;
		}

		if (wcs == L'{') {
			int peek = tok_nextChar(_tokInfo);
			if (peek != L'{' or inFormatSpec) {
				tok_backup(_tokInfo, peek);
				tok_backup(_tokInfo, wcs);
				_currentTok->curlyBracExprStartDepth++;
				if (_currentTok->curlyBracExprStartDepth >= MAX_EXPR_NESTING) {
					//return MAKE_TOKEN(alifTokenizer_syntaxError(_tokInfo, "نص منسق:التعبير المتداخل للنص المنسق وصل الحد الاقصى لعدد التداخلات"));
				}
				TOK_GETMODE(_tokInfo)->type = Token_RegularMode;
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
			}
			else {
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur - 1;
			}
			return MAKE_TOKEN(FSTRINGMIDDLE);
		}
		else if (wcs == L'}') {
			if (unicodeEscape) {
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
				return MAKE_TOKEN(FSTRINGMIDDLE);
			}
			AlifIntT peek = tok_nextChar(_tokInfo);

			// The tokenizer can only be in the format spec if we have already completed the expression
			// scanning (indicated by the end of the expression being set) and we are not at the top level
			// of the bracket stack (-1 is the top level). Since format specifiers can't legally use double
			// brackets, we can bypass it here.
			if (peek == L'}' and !inFormatSpec) {
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur - 1;
			}
			else {
				tok_backup(_tokInfo, peek);
				tok_backup(_tokInfo, wcs);
				TOK_GETMODE(_tokInfo)->type = Token_RegularMode;
				pStart = _tokInfo->start;
				pEnd = _tokInfo->cur;
			}
			return MAKE_TOKEN(FSTRINGMIDDLE);
		}
		else if (wcs == L'\\') {
			int peek = tok_nextChar(_tokInfo);
			if (peek == '\r') {
				peek = tok_nextChar(_tokInfo);
			}
			// Special case when the backslash is right before a curly
			// brace. We have to restore and return the control back
			// to the loop for the next iteration.
			if (peek == L'{' or peek == L'}') {
				if (!_currentTok->fStringRaw) {
					//if (alifTokenizer_warnInvalidEscapeSequence(_tokInfo, peek)) {
					//	return MAKE_TOKEN(ERRORTOKEN);
					//}
				}
				tok_backup(_tokInfo, peek);
				continue;
			}

			if (!_currentTok->fStringRaw) {
				if (peek == L'N') {
					/* Handle named unicode escapes (\N{BULLET}) */
					peek = tok_nextChar(_tokInfo);
					if (peek == L'{') {
						unicodeEscape = 1;
					}
					else {
						tok_backup(_tokInfo, peek);
					}
				}
			} /* else {
				skip the escaped character
			}*/
		}
	}

	// Backup the f-string quotes to emit a final FSTRINGMIDDLE and
	// add the quotes to the FSTRINGEND in the next tokenizer iteration.
	for (AlifIntT i = 0; i < _currentTok->fStringQuoteSize; i++) {
		tok_backup(_tokInfo, _currentTok->fStringQuote);
	}
	pStart = _tokInfo->start;
	pEnd = _tokInfo->cur;
	return MAKE_TOKEN(FSTRINGMIDDLE);
}


static int token_get(TokenInfo* _tokInfo, AlifToken* _token) { // 1460
	TokenizerMode* currentTok = TOK_GETMODE(_tokInfo);
	if (currentTok->type == Token_RegularMode) {
		return tokGet_normalMode(_tokInfo, currentTok, _token);
	}
	else {
		return tokGet_fStringMode(_tokInfo, currentTok, _token);
	}
}

int alifTokenizer_get(TokenInfo* _tokInfo, AlifToken* _token) { // 1471
	int result = token_get(_tokInfo, _token);
	return result;
}
