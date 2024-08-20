#include "alif.h"

#include "AlifCore_UStrObject.h"

























#include "StringLib/UCS2Lib.h" // 4954
#include "StringLib/Codecs.h"
#include "StringLib/Undef.h"

#include "StringLib/UCS4Lib.h" // 4958
#include "StringLib/Codecs.h"
#include "StringLib/Undef.h"




AlifIntT alif_decodeUTF8Ex(const char* s, AlifSizeT size, wchar_t** wstr, AlifUSizeT* wlen,
	const char** reason, AlifErrorHandler_ errors) { // 5267
	const char* origs = s;
	const char* e_;
	wchar_t* unicode;
	AlifSizeT outPos;

	int surrogateescape = 0;
	int surrogatepass = 0;
	switch (errors)
	{
	case AlifErrorHandler_::Alif_Error_Strict:
		break;
	case AlifErrorHandler_::Alif_Error_SurrogateEscape:
		surrogateescape = 1;
		break;
	case AlifErrorHandler_::Alif_Error_SurrogatePass:
		surrogatepass = 1;
		break;
	default:
		return -3;
	}

	/* Note: size will always be longer than the resulting Unicode
	   character count */
	if (ALIF_SIZET_MAX / (AlifSizeT)sizeof(wchar_t) - 1 < size) {
		return -1;
	}

	unicode = (wchar_t*)alifMem_dataAlloc((size + 1) * sizeof(wchar_t));
	if (!unicode) {
		return -1;
	}

	/* Unpack UTF-8 encoded data */
	e_ = s + size;
	outPos = 0;
	while (s < e_) {
		AlifUCS4 ch_;
#if SIZEOF_WCHAR_T == 4
		ch_ = ucs4Lib_utf8Decode(&s, e_, (AlifUCS4*)unicode, &outPos);
#else
		ch_ = ucs2Lib_utf8Decode(&s, e_, (AlifUCS2*)unicode, &outPos);
#endif
		if (ch_ > 0xFF) {
#if SIZEOF_WCHAR_T == 4
			ALIF_UNREACHABLE();
#else
			/* write a surrogate pair */
			unicode[outPos++] = (wchar_t)alifUnicode_highSurrogate(ch_);
			unicode[outPos++] = (wchar_t)alifUnicode_lowSurrogate(ch_);
#endif
		}
		else {
			if (!ch_ && s == e_) {
				break;
			}

			if (surrogateescape) {
				unicode[outPos++] = 0xDC00 + (unsigned char)*s++;
			}
			else {
				/* Is it a valid three-byte code? */
				if (surrogatepass
					&& (e_ - s) >= 3
					&& (s[0] & 0xf0) == 0xe0
					&& (s[1] & 0xc0) == 0x80
					&& (s[2] & 0xc0) == 0x80)
				{
					ch_ = ((s[0] & 0x0f) << 12) + ((s[1] & 0x3f) << 6) + (s[2] & 0x3f);
					s += 3;
					unicode[outPos++] = ch_;
				}
				else {
					alifMem_dataFree(unicode);
					if (reason != nullptr) {
						switch (ch_) {
						case 0:
							*reason = "unexpected end of data";
							break;
						case 1:
							*reason = "invalid start byte";
							break;
							/* 2, 3, 4 */
						default:
							*reason = "invalid continuation byte";
							break;
						}
					}
					if (wlen != nullptr) {
						*wlen = s - origs;
					}
					return -2;
				}
			}
		}
	}
	unicode[outPos] = L'\0';
	if (wlen) {
		*wlen = outPos;
	}
	*wstr = unicode;
	return 0;
}
