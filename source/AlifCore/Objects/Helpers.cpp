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
	errmsg = alifUStr_fromFormatVFroError(_format, _vargs); //* alif //* todo
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



AlifIntT alifTokenizer_syntaxError(TokenState* tok, const char* format, ...) { // 63
	va_list vargs{};
	va_start(vargs, format);
	AlifIntT ret = _syntaxError_range(tok, format, -1, -1, vargs);
	va_end(vargs);
	return ret;
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
