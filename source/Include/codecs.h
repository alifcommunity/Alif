#pragma once





















//STRINGLIB(utf8_decode)(const char** inptr, const char* end,
//	STRINGLIB_CHAR* dest,
//	AlifSizeT* outpos)
//{
//	AlifUCS4 ch;
//	const char* s = *inptr;
//	STRINGLIB_CHAR* p = dest + *outpos;
//
//	while (s < end) {
//		ch = (unsigned char)*s;
//
//		if (ch < 0x80) {
//			if (ALIF_IS_ALIGNED(s, ALIGNOF_SIZE_T)) {
//				const char* _s = s;
//				STRINGLIB_CHAR* _p = p;
//				while (_s + SIZEOF_SIZE_T <= end) {
//					size_t value = *(const size_t*)_s;
//					if (value & ASCII_CHAR_MASK)
//						break;
//#if ALIF_LITTLE_ENDIAN
//					_p[0] = (STRINGLIB_CHAR)(value & 0xFFu);
//					_p[1] = (STRINGLIB_CHAR)((value >> 8) & 0xFFu);
//					_p[2] = (STRINGLIB_CHAR)((value >> 16) & 0xFFu);
//					_p[3] = (STRINGLIB_CHAR)((value >> 24) & 0xFFu);
//# if SIZEOF_SIZE_T == 8
//					_p[4] = (STRINGLIB_CHAR)((value >> 32) & 0xFFu);
//					_p[5] = (STRINGLIB_CHAR)((value >> 40) & 0xFFu);
//					_p[6] = (STRINGLIB_CHAR)((value >> 48) & 0xFFu);
//					_p[7] = (STRINGLIB_CHAR)((value >> 56) & 0xFFu);
//# endif
//#else
//# if SIZEOF_SIZE_T == 8
//					_p[0] = (STRINGLIB_CHAR)((value >> 56) & 0xFFu);
//					_p[1] = (STRINGLIB_CHAR)((value >> 48) & 0xFFu);
//					_p[2] = (STRINGLIB_CHAR)((value >> 40) & 0xFFu);
//					_p[3] = (STRINGLIB_CHAR)((value >> 32) & 0xFFu);
//					_p[4] = (STRINGLIB_CHAR)((value >> 24) & 0xFFu);
//					_p[5] = (STRINGLIB_CHAR)((value >> 16) & 0xFFu);
//					_p[6] = (STRINGLIB_CHAR)((value >> 8) & 0xFFu);
//					_p[7] = (STRINGLIB_CHAR)(value & 0xFFu);
//# else
//					_p[0] = (STRINGLIB_CHAR)((value >> 24) & 0xFFu);
//					_p[1] = (STRINGLIB_CHAR)((value >> 16) & 0xFFu);
//					_p[2] = (STRINGLIB_CHAR)((value >> 8) & 0xFFu);
//					_p[3] = (STRINGLIB_CHAR)(value & 0xFFu);
//# endif
//#endif
//					_s += SIZEOF_SIZE_T;
//					_p += SIZEOF_SIZE_T;
//				}
//				s = _s;
//				p = _p;
//				if (s == end)
//					break;
//				ch = (unsigned char)*s;
//			}
//			if (ch < 0x80) {
//				s++;
//				*p++ = ch;
//				continue;
//			}
//		}
//
//		if (ch < 0xE0) {
//			/* \xC2\x80-\xDF\xBF -- 0080-07FF */
//			AlifUCS4 ch2;
//			if (ch < 0xC2) {
//				/* invalid sequence
//				\x80-\xBF -- continuation byte
//				\xC0-\xC1 -- fake 0000-007F */
//				goto InvalidStart;
//			}
//			if (end - s < 2) {
//				break;
//			}
//			ch2 = (unsigned char)s[1];
//			if (!IS_CONTINUATION_BYTE(ch2))
//				/* invalid continuation byte */
//				goto InvalidContinuation1;
//			ch = (ch << 6) + ch2 -
//				((0xC0 << 6) + 0x80);
//			assert((ch > 0x007F) && (ch <= 0x07FF));
//			s += 2;
//			if (STRINGLIB_MAX_CHAR <= 0x007F ||
//				(STRINGLIB_MAX_CHAR < 0x07FF && ch > STRINGLIB_MAX_CHAR))
//				/* Out-of-range */
//				goto Return;
//			*p++ = ch;
//			continue;
//		}
//
//		if (ch < 0xF0) {
//			/* \xE0\xA0\x80-\xEF\xBF\xBF -- 0800-FFFF */
//			AlifUCS4 ch2, ch3;
//			if (end - s < 3) {
//				if (end - s < 2)
//					break;
//				ch2 = (unsigned char)s[1];
//				if (!IS_CONTINUATION_BYTE(ch2) ||
//					(ch2 < 0xA0 ? ch == 0xE0 : ch == 0xED))
//					goto InvalidContinuation1;
//				break;
//			}
//			ch2 = (unsigned char)s[1];
//			ch3 = (unsigned char)s[2];
//			if (!IS_CONTINUATION_BYTE(ch2)) {
//				goto InvalidContinuation1;
//			}
//			if (ch == 0xE0) {
//				if (ch2 < 0xA0)
//					/* invalid sequence
//					   \xE0\x80\x80-\xE0\x9F\xBF -- fake 0000-0800 */
//					goto InvalidContinuation1;
//			}
//			else if (ch == 0xED && ch2 >= 0xA0) {
//				goto InvalidContinuation1;
//			}
//			if (!IS_CONTINUATION_BYTE(ch3)) {
//				goto InvalidContinuation2;
//			}
//			ch = (ch << 12) + (ch2 << 6) + ch3 -
//				((0xE0 << 12) + (0x80 << 6) + 0x80);
//			//assert((ch > 0x07FF) && (ch <= 0xFFFF));
//			s += 3;
//			if (STRINGLIB_MAX_CHAR <= 0x07FF ||
//				(STRINGLIB_MAX_CHAR < 0xFFFF && ch > STRINGLIB_MAX_CHAR))
//				goto Return;
//			*p++ = ch;
//			continue;
//		}
//
//		if (ch < 0xF5) {
//			/* \xF0\x90\x80\x80-\xF4\x8F\xBF\xBF -- 10000-10FFFF */
//			AlifUCS4 ch2, ch3, ch4;
//			if (end - s < 4) {
//				if (end - s < 2)
//					break;
//				ch2 = (unsigned char)s[1];
//				if (!IS_CONTINUATION_BYTE(ch2) ||
//					(ch2 < 0x90 ? ch == 0xF0 : ch == 0xF4))
//					goto InvalidContinuation1;
//				if (end - s < 3)
//					break;
//				ch3 = (unsigned char)s[2];
//				if (!IS_CONTINUATION_BYTE(ch3))
//					goto InvalidContinuation2;
//				break;
//			}
//			ch2 = (unsigned char)s[1];
//			ch3 = (unsigned char)s[2];
//			ch4 = (unsigned char)s[3];
//			if (!IS_CONTINUATION_BYTE(ch2)) {
//				goto InvalidContinuation1;
//			}
//			if (ch == 0xF0) {
//				if (ch2 < 0x90)
//					/* invalid sequence
//					   \xF0\x80\x80\x80-\xF0\x8F\xBF\xBF -- fake 0000-FFFF */
//					goto InvalidContinuation1;
//			}
//			else if (ch == 0xF4 && ch2 >= 0x90) {
//				/* invalid sequence
//				   \xF4\x90\x80\x80- -- 110000- overflow */
//				goto InvalidContinuation1;
//			}
//			if (!IS_CONTINUATION_BYTE(ch3)) {
//				goto InvalidContinuation2;
//			}
//			if (!IS_CONTINUATION_BYTE(ch4)) {
//				goto InvalidContinuation3;
//			}
//			ch = (ch << 18) + (ch2 << 12) + (ch3 << 6) + ch4 -
//				((0xF0 << 18) + (0x80 << 12) + (0x80 << 6) + 0x80);
//			assert((ch > 0xFFFF) && (ch <= 0x10FFFF));
//			s += 4;
//			if (STRINGLIB_MAX_CHAR <= 0xFFFF ||
//				(STRINGLIB_MAX_CHAR < 0x10FFFF && ch > STRINGLIB_MAX_CHAR))
//				goto Return;
//			*p++ = ch;
//			continue;
//		}
//		goto InvalidStart;
//	}
//	ch = 0;
//Return:
//	*inptr = s;
//	*outpos = p - dest;
//	return ch;
//InvalidStart:
//	ch = 1;
//	goto Return;
//InvalidContinuation1:
//	ch = 2;
//	goto Return;
//InvalidContinuation2:
//	ch = 3;
//	goto Return;
//InvalidContinuation3:
//	ch = 4;
//	goto Return;
//}
