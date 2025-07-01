#include "alif.h"

#include "AlifCore_BytesObject.h"
#include "AlifCore_UStrObject.h"

#include "AlifTokenState.h"
#include "AlifParserEngine.h"
#include "StringParser.h"





static AlifObject* decode_utf8(const char** _sPtr, const char* _end) { // 70
	const char* s_{};
	const char* t_{};
	t_ = s_ = *_sPtr;
	while (s_ < _end and (*s_ & 0x80)) {
		s_++;
	}
	*_sPtr = s_;
	return alifUStr_decodeUTF8(t_, s_ - t_, nullptr);
}



static AlifObject* decode_uStrWithEscapes(AlifParser* _parser,
	const char* _str, AlifUSizeT _len, AlifPToken* _t) { // 83
	AlifObject* v_{};
	AlifObject* u_{};
	char* buf{};
	char* p_{};
	const char* end{};

	/* check for integer overflow */
	if (_len > (AlifUSizeT)ALIF_SIZET_MAX / 6) {
		return nullptr;
	}
	/* "ä" (2 bytes) may become "\U000000E4" (10 bytes), or 1:5
	   "\ä" (3 bytes) may become "\u005c\U000000E4" (16 bytes), or ~1:6 */
	u_ = alifBytes_fromStringAndSize((char*)nullptr, (AlifSizeT)_len * 6);
	if (u_ == nullptr) {
		return nullptr;
	}
	p_ = buf = alifBytes_asString(u_);
	if (p_ == nullptr) {
		return nullptr;
	}
	end = _str + _len;
	while (_str < end) {
		if (*_str == '\\') {
			*p_++ = *_str++;
			if (_str >= end or *_str & 0x80) {
				strcpy(p_, "u005c");
				p_ += 5;
				if (_str >= end) {
					break;
				}
			}
		}
		if (*_str & 0x80) {
			AlifObject* w_{};
			AlifIntT kind{};
			const void* data{};
			AlifSizeT wLen{};
			AlifSizeT i_{};
			w_ = decode_utf8(&_str, end);
			if (w_ == nullptr) {
				ALIF_DECREF(u_);
				return nullptr;
			}
			kind = ALIFUSTR_KIND(w_);
			data = ALIFUSTR_DATA(w_);
			wLen = ALIFUSTR_GET_LENGTH(w_);
			for (i_ = 0; i_ < wLen; i_++) {
				AlifUCS4 chr = ALIFUSTR_READ(kind, data, i_);
				sprintf(p_, "\\U%08x", chr);
				p_ += 10;
			}
			/* Should be impossible to overflow */
			ALIF_DECREF(w_);
		}
		else {
			*p_++ = *_str++;
		}
	}
	_len = (AlifUSizeT)(p_ - buf);
	_str = buf;

	const char* firstInvalidEscape{};
	v_ = alifUStr_decodeUStrEscapeInternal(_str, (AlifSizeT)_len, nullptr, nullptr, &firstInvalidEscape);
	// HACK: later we can simply pass the line no, since we don't preserve the tokens
	// when we are decoding the string but we preserve the line numbers.
	//if (v_ != nullptr && first_invalid_escape != nullptr and _t != nullptr) {
	//	if (warn_invalidEscapeSequence(_parser, first_invalid_escape, _t) < 0) {
	//		/* We have not decref u before because first_invalid_escape points
	//		   inside u. */
	//		ALIF_XDECREF(u_);
	//		ALIF_DECREF(v_);
	//		return nullptr;
	//	}
	//}
	ALIF_XDECREF(u_);
	return v_;
}

static AlifObject* decode_bytesWithEscapes(AlifParser* _p, const char* _str,
	AlifSizeT _len, AlifPToken* _t) { // 166
	const char* first_invalid_escape;
	AlifObject* result = _alifBytes_decodeEscape(_str, _len, nullptr, &first_invalid_escape);
	if (result == nullptr) {
		return nullptr;
	}

	if (first_invalid_escape != nullptr) {
		//if (warn_invalidEscapeSequence(_p, first_invalid_escape, _t) < 0) {
		//	ALIF_DECREF(result);
		//	return nullptr;
		//}
	}
	return result;
}



AlifObject* alifParserEngine_decodeString(AlifParser* _p, AlifIntT _raw,
	const char* _s, AlifUSizeT _len, AlifPToken* _t) { // 184
	if (_raw) {
		return alifUStr_decodeUTF8Stateful(_s, (AlifSizeT)_len, nullptr, nullptr);
	}
	return decode_uStrWithEscapes(_p, _s, _len, _t);
}

AlifObject* alifParserEngine_parseString(AlifParser* _p, AlifPToken* _t) { // 196
	const char* s = alifBytes_asString(_t->bytes);
	if (s == nullptr) return nullptr;

	AlifUSizeT len{};
	AlifIntT quote = ALIF_CHARMASK(*s);
	AlifIntT bytesMode{};
	AlifIntT rawMode{};

	if (ALIF_ISALPHA(quote)) {
		while (!bytesMode or !rawMode) {
			if (quote == 'b' or quote == 'B') {
				quote = (unsigned char)*++s;
				bytesMode = 1;
			}
			else if (quote == 'u' or quote == 'U') {
				quote = (unsigned char)*++s;
			}
			else if (quote == 'r' or quote == 'R') {
				quote = (unsigned char)*++s;
				rawMode = 1;
			}
			else {
				break;
			}
		}
	}

	if (quote != L'\'' and quote != L'\"') {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	// skip the leading quote char
	s++;
	len = strlen(s);

	if (len > INT_MAX) {
		//alifErr_setString(_alifExcOverflowError_, "string to parse is too long");
		return nullptr;
	}
	if (s[--len] != quote) {
		/* Last quote char must match the first. */
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	if (len >= 4 and s[0] == quote and s[1] == quote) {
		s += 2;
		len -= 2;
		if (s[--len] != quote or s[--len] != quote) {
			//ALIFERR_BADINTERNALCALL();
			return nullptr;
		}
	}

	rawMode = rawMode or strchr(s, '\\') == nullptr;
	if (bytesMode) {
		/* Disallow non-ASCII characters. */
		const char* ch{};
		for (ch = s; *ch; ch++) {
			if (ALIF_CHARMASK(*ch) >= 0x80) {
				//RAISE_SYNTAX_ERROR_KNOWN_LOCATION( _t,
				//	"bytes can only contain ASCII "
				//	"literal characters");
				return nullptr;
			}
		}
		if (rawMode) {
			return alifBytes_fromStringAndSize(s, (AlifSizeT)len);
		}
		return decode_bytesWithEscapes(_p, s, (AlifSizeT)len, _t);
	}

	return alifParserEngine_decodeString(_p, rawMode, s, len, _t);
}
