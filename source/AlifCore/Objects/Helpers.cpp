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








/* ----------------- String Manipulation ----------------- */

char* alifTokenizer_newString(const char* _s, AlifSizeT _len, TokenState* _tok) { // 180
	char* result = (char*)alifMem_dataAlloc(_len + 1);
	if (!result) {
		_tok->done = E_NOMEM;
		return nullptr;
	}
	memcpy(result, _s, _len);
	result[_len] = '\0';
	return result;
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
