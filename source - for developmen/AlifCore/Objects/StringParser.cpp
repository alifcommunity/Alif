#include "alif.h"

#include "AlifCore_BytesObject.h"
//#include "AlifCore_UStrObject.h"

#include "AlifTokenState.h"
#include "AlifParserEngine.h"
#include "StringParser.h"


AlifObject* alifParserEngine_decodeString(AlifParser* _p, int _raw, const wchar_t* _s, size_t _len, AlifPToken* _t) {
	if (_raw) {
		return alifUStr_decodeUTF8Stateful(_s, _len, nullptr, nullptr);
	}
	//return decode_UStrWithEscape(_p, _s, _len, _t);
	return nullptr; // temp
}

AlifObject* alifParserEngine_parseString(AlifParser* _p, AlifPToken* _t) {
	const wchar_t* s = _alifWBytes_asString(_t->bytes);
	if (s == nullptr) return nullptr;

	AlifUSizeT len{};
	int quote = ALIF_WCHARMASK(*s);
	int bytesMode{};
	int rawMode{};

	//if (ALIF_ISALPHA(quote)) {
	//	while (!bytesMode or !rawMode) {
	//		// if L'b' or L'B'
	//	}
	//}

	if (quote != L'\'' and quote != L'\"') {
		// error
		return nullptr;
	}
	// skip the leading quote wchar_t
	s++;
	len = wcslen(s);
	// error
	// error

	if (len >= 4 and s[0] == quote and s[1] == quote) {
		s += 2;
		len -= 2;
		// error
	}

	rawMode ? rawMode : rawMode = (wcschr(s, L'\\') == nullptr); // يجب مراجعتها لانه تم التعديل عليها
	/*
	.
	.
	*/

	return alifParserEngine_decodeString(_p, rawMode, s, len, _t);
}
