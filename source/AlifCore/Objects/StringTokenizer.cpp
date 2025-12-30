#include "alif.h"
#include "ErrorCode.h"

#include "Helpers.h"
#include "AlifTokenState.h"


static AlifIntT tok_underflowString(TokenState* _tokState) {
	char* end = strchr(_tokState->inp, '\n');
	if (end != nullptr) {
		end++;
	}
	else {
		end = strchr(_tokState->inp, '\0');
		if (end == _tokState->inp) {
			_tokState->done = E_EOF;
			return 0;
		}
	}
	if (_tokState->start == nullptr) {
		_tokState->buf = _tokState->cur;
	}
	_tokState->lineStart = _tokState->cur;
	LINE_ADVANCE();
	_tokState->inp = end;
	return 1;
}


static AlifIntT buf_getc(TokenState* tok) {
	return ALIF_CHARMASK(*tok->string++);
}

/* Unfetch a byte from TOK, using the string buffer. */
static void buf_ungetc(AlifIntT c, TokenState* tok) {
	tok->string--;
}

/* Set the readline function for TOK to ENC. For the string-based
   tokenizer, this means to just record the encoding. */
static AlifIntT buf_setreadl(TokenState* tok, const char* enc) {
	tok->enc = enc;
	return 1;
}


static char* decode_str(const char* input, AlifIntT single,
	TokenState* tok, AlifIntT preserve_crlf) {
	AlifObject* utf8 = nullptr;
	char* str{};
	const char* s{};
	const char* newl[2] = { nullptr, nullptr };
	int lineno = 0;
	tok->input = str = _alifTokenizer_translateNewlines(input, single, preserve_crlf, tok);
	if (str == nullptr)
		return nullptr;
	tok->enc = nullptr;
	tok->string = str;
	if (!_alifTokenizer_checkBom(buf_getc, buf_ungetc, buf_setreadl, tok))
		return _alifTokenizer_errorRet(tok);
	str = tok->string;             /* string after BOM if any */
	if (tok->enc != nullptr) {
		utf8 = _alifTokenizer_translateIntoUTF8(str, tok->enc);
		if (utf8 == nullptr)
			return _alifTokenizer_errorRet(tok);
		str = alifBytes_asString(utf8);
	}
	for (s = str;; s++) {
		if (*s == '\0') break;
		else if (*s == '\n') {
			newl[lineno] = s;
			lineno++;
			if (lineno == 2) break;
		}
	}
	tok->enc = nullptr;
	/* need to check line 1 and 2 separately since check_coding_spec
	   assumes a single line as input */
	if (newl[0]) {
		if (!_alifTokenizer_checkCodingSpec(str, newl[0] - str, tok, buf_setreadl)) {
			return nullptr;
		}
		if (tok->enc == nullptr and tok->decodingState != DecodingState_::State_Normal and newl[1]) {
			if (!_alifTokenizer_checkCodingSpec(newl[0] + 1, newl[1] - newl[0],
				tok, buf_setreadl))
				return nullptr;
		}
	}
	if (tok->enc != nullptr) {
		utf8 = _alifTokenizer_translateIntoUTF8(str, tok->enc);
		if (utf8 == nullptr)
			return _alifTokenizer_errorRet(tok);
		str = ALIFBYTES_AS_STRING(utf8);
	}
	tok->decodingBuffer = utf8; /* CAUTION */
	return str;
}


TokenState* _alifTokenizer_fromString(const char* _str,
	AlifIntT _execInput, AlifIntT _preserveCRIF) {
	TokenState* tok = alifTokenizer_tokNew();
	char* decoded{};

	if (tok == nullptr)
		return nullptr;
	decoded = decode_str(_str, _execInput, tok, _preserveCRIF);
	if (decoded == nullptr) {
		alifTokenizer_free(tok);
		return nullptr;
	}

	tok->buf = tok->cur = tok->inp = decoded;
	tok->end = decoded;
	tok->underflow = &tok_underflowString;
	return tok;
}
