#include "alif.h"

#include "AlifCore_BytesObject.h"
#include "AlifCore_UStrObject.h"

#include "AlifTokenState.h"
#include "AlifParserEngine.h"
#include "StringParser.h"


AlifObject* alifParserEngine_decodeString(AlifParser* _p, AlifIntT _raw,
	const char* _s, AlifUSizeT _len, AlifPToken* _t) { // 184
	if (_raw) {
		return alifUStr_decodeUTF8Stateful(_s, _len, nullptr, nullptr);
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

	//if (ALIF_ISALPHA(quote)) {
	//	while (!bytesMode or !rawMode) {
	//		if (quote == 'b' or quote == 'B') {
	//			quote = (unsigned char)*++s;
	//			bytesMode = 1;
	//		}
	//		else if (quote == 'u' or quote == 'U') {
	//			quote = (unsigned char)*++s;
	//		}
	//		else if (quote == 'r' or quote == 'R') {
	//			quote = (unsigned char)*++s;
	//			rawMode = 1;
	//		}
	//		else {
	//			break;
	//		}
	//	}
	//}

	if (quote != '\'' and quote != '\"') {
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
			return alifBytes_fromStringAndSize(s, len);
		}
		return decode_bytesWithEscapes(_p, s, len, _t);
	}

	return alifParserEngine_decodeString(_p, rawMode, s, len, _t);
}
