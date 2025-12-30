#include "alif.h"

#include "ErrorCode.h"
#include "AlifCore_Token.h"

#include "AlifTokenState.h"


/* ----------------- Errors ----------------- */


static AlifIntT _syntaxError_range(TokenState* _tok, const char* _format,
	AlifIntT _colOffset, AlifIntT endColOffset, va_list _vargs) { // 10

	AlifSizeT lineLen{};

	if (_tok->done == E_ERROR) {
		return ERRORTOKEN;
	}
	AlifObject* errmsg{}, * errtext{}, * args{};
	errmsg = alifUStr_fromFormatV(_format, _vargs);
	if (!errmsg) {
		goto error;
	}

	errtext = alifUStr_decodeUTF8(_tok->lineStart,
		_tok->cur - _tok->lineStart, "replace");
	if (!errtext) {
		goto error;
	}

	if (_colOffset == -1) {
		_colOffset = (AlifIntT)ALIFUSTR_GET_LENGTH(errtext);
	}
	if (endColOffset == -1) {
		endColOffset = _colOffset;
	}

	lineLen = strcspn(_tok->lineStart, "\n");
	if (lineLen != _tok->cur - _tok->lineStart) {
		ALIF_DECREF(errtext);
		errtext = alifUStr_decodeUTF8(_tok->lineStart, lineLen,
			"replace");
	}
	if (!errtext) {
		goto error;
	}

	args = alif_buildValue("(O(OiiNii))", errmsg, _tok->fn, _tok->lineNo,
		_colOffset, errtext, _tok->lineNo, endColOffset);
	if (args) {
		alifErr_setObject(_alifExcSyntaxError_, args);
		ALIF_DECREF(args);
	}

error:
	ALIF_XDECREF(errmsg);
	_tok->done = E_ERROR;
	return ERRORTOKEN;
}



AlifIntT alifTokenizer_syntaxError(TokenState* _tok, const char* format, ...) { // 63
	va_list vargs{};
	va_start(vargs, format);
	AlifIntT ret = _syntaxError_range(_tok, format, -1, -1, vargs);
	va_end(vargs);
	return ret;
}






AlifIntT _alifTokenizer_indentError(TokenState* _tok) { // 86
	_tok->done = E_TABSPACE;
	_tok->cur = _tok->inp;
	return ERRORTOKEN;
}


char* _alifTokenizer_errorRet(TokenState* _tok) /* XXX */
{
	_tok->decodingErred = 1;
	if ((_tok->fp != nullptr or _tok->readline != nullptr)
		and _tok->buf != nullptr) {/* see _alifTokenizer_free */
		alifMem_dataFree(_tok->buf);
	}
	_tok->buf = _tok->cur = _tok->inp = nullptr;
	_tok->start = nullptr;
	_tok->end = nullptr;
	_tok->done = E_DECODE;
	return nullptr;                /* as if it were EOF */
}





/* ----------------- String Manipulation ----------------- */

char* _alifTokenizer_newString(const char* _s, AlifSizeT _len, TokenState* _tok) { // 180
	char* result = (char*)alifMem_dataAlloc(_len + 1);
	if (!result) {
		_tok->done = E_NOMEM;
		return nullptr;
	}
	memcpy(result, _s, _len);
	result[_len] = '\0';
	return result;
}





AlifObject* _alifTokenizer_translateIntoUTF8(const char* str, const char* enc) { // 193
	AlifObject* utf8{};
	AlifObject* buf = alifUStr_decode(str, strlen(str), enc, nullptr);
	if (buf == nullptr)
		return nullptr;
	utf8 = alifUStr_asUTF8String(buf);
	ALIF_DECREF(buf);
	return utf8;
}








char* _alifTokenizer_translateNewlines(const char* _s, AlifIntT _execInput,
	AlifIntT _preserveCRLF, class TokenState* _tok) { // 204
	AlifIntT skipNextLF = 0;
	AlifUSizeT neededLength = strlen(_s) + 2, finalLength;
	char* buf{}, * current{};
	char c = '\0';
	buf = (char*)alifMem_dataAlloc(neededLength);
	if (buf == nullptr) {
		_tok->done = E_NOMEM;
		return nullptr;
	}
	for (current = buf; *_s; _s++, current++) {
		c = *_s;
		if (skipNextLF) {
			skipNextLF = 0;
			if (c == '\n') {
				c = *++_s;
				if (!c)
					break;
			}
		}
		if (!_preserveCRLF and c == '\r') {
			skipNextLF = 1;
			c = '\n';
		}
		*current = c;
	}
	/* If this is exec input, add a newline to the end of the string if
	   there isn't one already. */
	if (_execInput and c != '\n' and c != '\0') {
		*current = '\n';
		current++;
	}
	*current = '\0';
	finalLength = current - buf + 1;
	if (finalLength < neededLength and finalLength) {
		/* should never fail */
		char* result = (char*)alifMem_dataRealloc(buf, finalLength);
		if (result == nullptr) {
			alifMem_dataFree(buf);
		}
		buf = result;
	}
	return buf;
}







AlifIntT _alifTokenizer_checkBom(AlifIntT get_char(TokenState*),
	void unget_char(AlifIntT, TokenState*),
	AlifIntT set_readline(TokenState*, const char*),
	TokenState* tok) { // 257
	AlifIntT ch1, ch2, ch3;
	ch1 = get_char(tok);
	tok->decodingState = DecodingState_::State_Seek_Coding;
	if (ch1 == EOF) {
		return 1;
	}
	else if (ch1 == 0xEF) {
		ch2 = get_char(tok);
		if (ch2 != 0xBB) {
			unget_char(ch2, tok);
			unget_char(ch1, tok);
			return 1;
		}
		ch3 = get_char(tok);
		if (ch3 != 0xBF) {
			unget_char(ch3, tok);
			unget_char(ch2, tok);
			unget_char(ch1, tok);
			return 1;
		}
	}
	else {
		unget_char(ch1, tok);
		return 1;
	}
	if (tok->encoding != nullptr)
		alifMem_dataFree(tok->encoding);
	tok->encoding = _alifTokenizer_newString("utf-8", 5, tok);
	if (!tok->encoding)
		return 0;
	/* No need to set_readline: input is already utf-8 */
	return 1;
}

static const char* get_normalName(const char* s)  /* for utf-8 and latin-1 */
{ // 295
	char buf[13];
	AlifIntT i{};
	for (i = 0; i < 12; i++) {
		AlifIntT c = s[i];
		if (c == '\0')
			break;
		else if (c == '_')
			buf[i] = '-';
		else
			buf[i] = ALIF_TOLOWER(c);
	}
	buf[i] = '\0';
	if (strcmp(buf, "utf-8") == 0 or
		strncmp(buf, "utf-8-", 6) == 0)
		return "utf-8";
	else if (strcmp(buf, "latin-1") == 0 or
		strcmp(buf, "iso-8859-1") == 0 or
		strcmp(buf, "iso-latin-1") == 0 or
		strncmp(buf, "latin-1-", 8) == 0 or
		strncmp(buf, "iso-8859-1-", 11) == 0 or
		strncmp(buf, "iso-latin-1-", 12) == 0)
		return "iso-8859-1";
	else
		return s;
}

static AlifIntT get_codingSpec(const char* s, char** spec,
	AlifSizeT size, TokenState* tok) { // 325
	AlifSizeT i{};
	*spec = nullptr;
	/* Coding spec must be in a comment, and that comment must be
	 * the only statement on the source code line. */
	for (i = 0; i < size - 6; i++) {
		if (s[i] == '#')
			break;
		if (s[i] != ' ' and s[i] != '\t' and s[i] != '\014')
			return 1;
	}
	for (; i < size - 6; i++) { /* XXX inefficient search */
		const char* t = s + i;
		if (memcmp(t, "coding", 6) == 0) {
			const char* begin = nullptr;
			t += 6;
			if (t[0] != ':' and t[0] != '=')
				continue;
			do {
				t++;
			}
			while (t[0] == ' ' or t[0] == '\t');

			begin = t;
			while (ALIF_ISALNUM(t[0]) or
				t[0] == '-' or t[0] == '_' or t[0] == '.')
				t++;

			if (begin < t) {
				char* r = _alifTokenizer_newString(begin, t - begin, tok);
				const char* q;
				if (!r)
					return 0;
				q = get_normalName(r);
				if (r != q) {
					alifMem_dataFree(r);
					r = _alifTokenizer_newString(q, strlen(q), tok);
					if (!r)
						return 0;
				}
				*spec = r;
				break;
			}
		}
	}
	return 1;
}


AlifIntT _alifTokenizer_checkCodingSpec(const char* line, AlifSizeT size, TokenState* tok,
	AlifIntT set_readline(TokenState*, const char*)) { // 378
	char* cs{};
	if (tok->contLine) {
		/* It's a continuation line, so it can't be a coding spec. */
		tok->decodingState = DecodingState_::State_Normal;
		return 1;
	}
	if (!get_codingSpec(line, &cs, size, tok)) {
		return 0;
	}
	if (!cs) {
		AlifSizeT i{};
		for (i = 0; i < size; i++) {
			if (line[i] == '#' or line[i] == '\n' or line[i] == '\r')
				break;
			if (line[i] != ' ' and line[i] != '\t' and line[i] != '\014') {
				/* Stop checking coding spec after a line containing
				 * anything except a comment. */
				tok->decodingState = DecodingState_::State_Normal;
				break;
			}
		}
		return 1;
	}
	tok->decodingState = DecodingState_::State_Normal;
	if (tok->encoding == nullptr) {
		if (strcmp(cs, "utf-8") != 0 and !set_readline(tok, cs)) {
			_alifTokenizer_errorRet(tok);
			alifErr_format(_alifExcSyntaxError_, "encoding problem: %s", cs);
			alifMem_dataFree(cs);
			return 0;
		}
		tok->encoding = cs;
	}
	else {                /* then, compare cs with BOM */
		if (strcmp(tok->encoding, cs) != 0) {
			_alifTokenizer_errorRet(tok);
			alifErr_format(_alifExcSyntaxError_,
				"encoding problem: %s with BOM", cs);
			alifMem_dataFree(cs);
			return 0;
		}
		alifMem_dataFree(cs);
	}
	return 1;
}
