
#include "AlifCore_BitUtils.h"


#if (SIZEOF_SIZE_T == 8)
# define ASCII_CHAR_MASK 0x8080808080808080ULL
#elif (SIZEOF_SIZE_T == 4)
# define ASCII_CHAR_MASK 0x80808080U
#else
# error Cpp 'AlifUSizeT' size should be either 4 or 8!
#endif

/* 10xxxxxx */
#define IS_CONTINUATION_BYTE(_ch) ((_ch) >= 0x80 and (_ch) < 0xC0) // 20

ALIF_LOCAL_INLINE(AlifUCS4)
STRINGLIB(utf8Decode)(const char** _inPtr, const char* _end,
	STRINGLIB_CHAR* _dest, AlifSizeT* _outPos) { // 22
	AlifUCS4 ch_;
	const char* s_ = *_inPtr;
	STRINGLIB_CHAR* p_ = _dest + *_outPos;

	while (s_ < _end) {
		ch_ = (unsigned char)*s_;

		if (ch_ < 0x80) {
			/* Fast path for runs of ASCII characters. Given that common UTF-8
			   input will consist of an overwhelming majority of ASCII
			   characters, we try to optimize for this case by checking
			   as many characters as a Cpp 'AlifUSizeT' can contain.
			   First, check if we can do an aligned read, as most CPUs have
			   a penalty for unaligned reads.
			*/
			if (ALIF_IS_ALIGNED(s_, ALIGNOF_SIZE_T)) {
				/* Help register allocation */
				const char* _s = s_;
				STRINGLIB_CHAR* _p = p_;
				while (_s + SIZEOF_SIZE_T <= _end) {
					/* Read a whole AlifUSizeT at a time (either 4 or 8 bytes),
					   and do a fast unrolled copy if it only contains ASCII
					   characters. */
					AlifUSizeT value = *(const AlifUSizeT*)_s;
					if (value & ASCII_CHAR_MASK)
						break;
#if ALIF_LITTLE_ENDIAN
					_p[0] = (STRINGLIB_CHAR)(value & 0xFFu);
					_p[1] = (STRINGLIB_CHAR)((value >> 8) & 0xFFu);
					_p[2] = (STRINGLIB_CHAR)((value >> 16) & 0xFFu);
					_p[3] = (STRINGLIB_CHAR)((value >> 24) & 0xFFu);
# if SIZEOF_SIZE_T == 8
					_p[4] = (STRINGLIB_CHAR)((value >> 32) & 0xFFu);
					_p[5] = (STRINGLIB_CHAR)((value >> 40) & 0xFFu);
					_p[6] = (STRINGLIB_CHAR)((value >> 48) & 0xFFu);
					_p[7] = (STRINGLIB_CHAR)((value >> 56) & 0xFFu);
# endif
#else
# if SIZEOF_SIZE_T == 8
					_p[0] = (STRINGLIB_CHAR)((value >> 56) & 0xFFu);
					_p[1] = (STRINGLIB_CHAR)((value >> 48) & 0xFFu);
					_p[2] = (STRINGLIB_CHAR)((value >> 40) & 0xFFu);
					_p[3] = (STRINGLIB_CHAR)((value >> 32) & 0xFFu);
					_p[4] = (STRINGLIB_CHAR)((value >> 24) & 0xFFu);
					_p[5] = (STRINGLIB_CHAR)((value >> 16) & 0xFFu);
					_p[6] = (STRINGLIB_CHAR)((value >> 8) & 0xFFu);
					_p[7] = (STRINGLIB_CHAR)(value & 0xFFu);
# else
					_p[0] = (STRINGLIB_CHAR)((value >> 24) & 0xFFu);
					_p[1] = (STRINGLIB_CHAR)((value >> 16) & 0xFFu);
					_p[2] = (STRINGLIB_CHAR)((value >> 8) & 0xFFu);
					_p[3] = (STRINGLIB_CHAR)(value & 0xFFu);
# endif
#endif
					_s += SIZEOF_SIZE_T;
					_p += SIZEOF_SIZE_T;
				}
				s_ = _s;
				p_ = _p;
				if (s_ == _end)
					break;
				ch_ = (unsigned char)*s_;
			}
			if (ch_ < 0x80) {
				s_++;
				*p_++ = ch_;
				continue;
			}
		}

		if (ch_ < 0xE0) {
			/* \xC2\x80-\xDF\xBF -- 0080-07FF */
			AlifUCS4 ch2;
			if (ch_ < 0xC2) {
				/* invalid sequence
				\x80-\xBF -- continuation byte
				\xC0-\xC1 -- fake 0000-007F */
				goto invalidStart;
			}
			if (_end - s_ < 2) {
				/* unexpected end of data: the caller will decide whether
				   it's an error or not */
				break;
			}
			ch2 = (unsigned char)s_[1];
			if (!IS_CONTINUATION_BYTE(ch2))
				/* invalid continuation byte */
				goto invalidContinuation1;
			ch_ = (ch_ << 6) + ch2 -
				((0xC0 << 6) + 0x80);
			s_ += 2;
			if (STRINGLIB_MAX_CHAR <= 0x007F or
				(STRINGLIB_MAX_CHAR < 0x07FF and ch_ > STRINGLIB_MAX_CHAR))
				/* Out-of-range */
				goto returnRes;
			*p_++ = ch_;
			continue;
		}

		if (ch_ < 0xF0) {
			/* \xE0\xA0\x80-\xEF\xBF\xBF -- 0800-FFFF */
			AlifUCS4 ch2, ch3;
			if (_end - s_ < 3) {
				/* unexpected end of data: the caller will decide whether
				   it's an error or not */
				if (_end - s_ < 2)
					break;
				ch2 = (unsigned char)s_[1];
				if (!IS_CONTINUATION_BYTE(ch2) or
					(ch2 < 0xA0 ? ch_ == 0xE0 : ch_ == 0xED))
					/* for clarification see comments below */
					goto invalidContinuation1;
				break;
			}
			ch2 = (unsigned char)s_[1];
			ch3 = (unsigned char)s_[2];
			if (!IS_CONTINUATION_BYTE(ch2)) {
				/* invalid continuation byte */
				goto invalidContinuation1;
			}
			if (ch_ == 0xE0) {
				if (ch2 < 0xA0)
					/* invalid sequence
					   \xE0\x80\x80-\xE0\x9F\xBF -- fake 0000-0800 */
					goto invalidContinuation1;
			}
			else if (ch_ == 0xED && ch2 >= 0xA0) {
				goto invalidContinuation1;
			}
			if (!IS_CONTINUATION_BYTE(ch3)) {
				/* invalid continuation byte */
				goto invalidContinuation2;
			}
			ch_ = (ch_ << 12) + (ch2 << 6) + ch3 -
				((0xE0 << 12) + (0x80 << 6) + 0x80);
			s_ += 3;
			if (STRINGLIB_MAX_CHAR <= 0x07FF or
				(STRINGLIB_MAX_CHAR < 0xFFFF and ch_ > STRINGLIB_MAX_CHAR))
				/* Out-of-range */
				goto returnRes;
			*p_++ = ch_;
			continue;
		}

		if (ch_ < 0xF5) {
			/* \xF0\x90\x80\x80-\xF4\x8F\xBF\xBF -- 10000-10FFFF */
			AlifUCS4 ch2, ch3, ch4;
			if (_end - s_ < 4) {
				/* unexpected end of data: the caller will decide whether
				   it's an error or not */
				if (_end - s_ < 2)
					break;
				ch2 = (unsigned char)s_[1];
				if (!IS_CONTINUATION_BYTE(ch2) or
					(ch2 < 0x90 ? ch_ == 0xF0 : ch_ == 0xF4))
					/* for clarification see comments below */
					goto invalidContinuation1;
				if (_end - s_ < 3)
					break;
				ch3 = (unsigned char)s_[2];
				if (!IS_CONTINUATION_BYTE(ch3))
					goto invalidContinuation2;
				break;
			}
			ch2 = (unsigned char)s_[1];
			ch3 = (unsigned char)s_[2];
			ch4 = (unsigned char)s_[3];
			if (!IS_CONTINUATION_BYTE(ch2)) {
				/* invalid continuation byte */
				goto invalidContinuation1;
			}
			if (ch_ == 0xF0) {
				if (ch2 < 0x90)
					/* invalid sequence
					   \xF0\x80\x80\x80-\xF0\x8F\xBF\xBF -- fake 0000-FFFF */
					goto invalidContinuation1;
			}
			else if (ch_ == 0xF4 and ch2 >= 0x90) {
				/* invalid sequence
				   \xF4\x90\x80\x80- -- 110000- overflow */
				goto invalidContinuation1;
			}
			if (!IS_CONTINUATION_BYTE(ch3)) {
				/* invalid continuation byte */
				goto invalidContinuation2;
			}
			if (!IS_CONTINUATION_BYTE(ch4)) {
				/* invalid continuation byte */
				goto invalidContinuation3;
			}
			ch_ = (ch_ << 18) + (ch2 << 12) + (ch3 << 6) + ch4 -
				((0xF0 << 18) + (0x80 << 12) + (0x80 << 6) + 0x80);
			s_ += 4;
			if (STRINGLIB_MAX_CHAR <= 0xFFFF or
				(STRINGLIB_MAX_CHAR < 0x10FFFF and ch_ > STRINGLIB_MAX_CHAR))
				/* Out-of-range */
				goto returnRes;
			*p_++ = ch_;
			continue;
		}
		goto invalidStart;
	}
	ch_ = 0;
returnRes:
	*_inPtr = s_;
	*_outPos = p_ - _dest;
	return ch_;
invalidStart:
	ch_ = 1;
	goto returnRes;
invalidContinuation1:
	ch_ = 2;
	goto returnRes;
invalidContinuation2:
	ch_ = 3;
	goto returnRes;
invalidContinuation3:
	ch_ = 4;
	goto returnRes;
}




#undef ASCII_CHAR_MASK // 254



ALIF_LOCAL_INLINE(char*)
STRINGLIB(utf8Encoder)(AlifBytesWriter* _writer, AlifObject* _uStr,
	const STRINGLIB_CHAR* _data, AlifSizeT _size,
	AlifErrorHandler_ _errorHandler, const char* _errors) { // 260
	AlifSizeT i_{};                /* index into data of next input character */
	char* p_{};                     /* next free byte in output buffer */
#if STRINGLIB_SIZEOF_CHAR > 1
	AlifObject* errorHandlerObj = nullptr;
	AlifObject* exc = nullptr;
	AlifObject* rep = nullptr;
#endif
#if STRINGLIB_SIZEOF_CHAR == 1
	const AlifSizeT maxCharSize = 2;
#elif STRINGLIB_SIZEOF_CHAR == 2
	const AlifSizeT maxCharSize = 3;
#else /*  STRINGLIB_SIZEOF_CHAR == 4 */
	const AlifSizeT maxCharSize = 4;
#endif

	if (_size > ALIF_SIZET_MAX / maxCharSize) {
		/* integer overflow */
		//alifErr_noMemory();
		return nullptr;
	}

	alifBytesWriter_init(_writer);
	p_ = (char*)alifBytesWriter_alloc(_writer, _size * maxCharSize);
	if (p_ == nullptr)
		return nullptr;

	for (i_ = 0; i_ < _size;) {
		AlifUCS4 ch = _data[i_++];

		if (ch < 0x80) {
			/* Encode ASCII */
			*p_++ = (char)ch;

		}
		else
#if STRINGLIB_SIZEOF_CHAR > 1
			if (ch < 0x0800)
#endif
			{
				/* Encode Latin-1 */
				*p_++ = (char)(0xc0 | (ch >> 6));
				*p_++ = (char)(0x80 | (ch & 0x3f));
			}
#if STRINGLIB_SIZEOF_CHAR > 1
			else if (alifUnicode_isSurrogate(ch)) {
				AlifSizeT startpos{}, endpos{}, newpos{};
				AlifSizeT k{};
				//if (_errorHandler == AlifErrorHandler_::Alif_Error_Unknown) {
				//	_errorHandler = alif_getErrorHandler(_errors);
				//}

				startpos = i_ - 1;
				endpos = startpos + 1;

				while ((endpos < _size) and alifUnicode_isSurrogate(_data[endpos]))
					endpos++;

				/* Only overallocate the buffer if it's not the last write */
				_writer->overAllocate = (endpos < _size);

				switch (_errorHandler)
				{
				case AlifErrorHandler_::Alif_Error_Replace:
					memset(p_, '?', endpos - startpos);
					p_ += (endpos - startpos);
				case AlifErrorHandler_::Alif_Error_Ignore:
					i_ += (endpos - startpos - 1);
					break;

				case AlifErrorHandler_::Alif_Error_SurrogatePass:
					for (k = startpos; k < endpos; k++) {
						ch = _data[k];
						*p_++ = (char)(0xe0 | (ch >> 12));
						*p_++ = (char)(0x80 | ((ch >> 6) & 0x3f));
						*p_++ = (char)(0x80 | (ch & 0x3f));
					}
					i_ += (endpos - startpos - 1);
					break;

				case AlifErrorHandler_::Alif_Error_BackSlashReplace:
					/* subtract preallocated bytes */
					_writer->minSize -= maxCharSize * (endpos - startpos);
					p_ = backSlash_replace(_writer, p_,
						_uStr, startpos, endpos);
					if (p_ == nullptr)
						goto error;
					i_ += (endpos - startpos - 1);
					break;

				case AlifErrorHandler_::Alif_Error_XMLCharRefReplace:
					/* subtract preallocated bytes */
					_writer->minSize -= maxCharSize * (endpos - startpos);
					p_ = xmlCharRef_replace(_writer, p_,
						_uStr, startpos, endpos);
					if (p_ == nullptr)
						goto error;
					i_ += (endpos - startpos - 1);
					break;

				case AlifErrorHandler_::Alif_Error_SurrogateEscape:
					for (k = startpos; k < endpos; k++) {
						ch = _data[k];
						if (!(0xDC80 <= ch && ch <= 0xDCFF))
							break;
						*p_++ = (char)(ch & 0xff);
					}
					if (k >= endpos) {
						i_ += (endpos - startpos - 1);
						break;
					}
					startpos = k;
				default:
					//rep = unicodeEncode_callErrorHandler(
					//	_errors, &errorHandlerObj, "utf-8", "surrogates not allowed",
					//	_uStr, &exc, startpos, endpos, &newpos);
					//if (!rep)
					//	goto error;

					if (newpos < startpos) {
						_writer->overAllocate = 1;
						p_ = (char*)alifBytesWriter_prepare(_writer, p_,
							maxCharSize * (startpos - newpos));
						if (p_ == nullptr)
							goto error;
					}
					else {
						/* subtract preallocated bytes */
						_writer->minSize -= maxCharSize * (newpos - startpos);
						/* Only overallocate the buffer if it's not the last write */
						_writer->overAllocate = (newpos < _size);
					}

					//if (ALIFBYTES_CHECK(rep)) {
					//	p_ = alifBytesWriter_writeBytes(_writer, p_,
					//		ALIFBYTES_AS_STRING(rep),
					//		ALIFBYTES_GET_SIZE(rep));
					//}
					//else {
					//	/* rep is unicode */
					//	if (!ALIFUSTR_IS_ASCII(rep)) {
					//		raise_encodeException(&exc, "utf-8", _uStr,
					//			startpos, endpos, "surrogates not allowed");
					//		goto error;
					//	}

					//	p_ = alifBytesWriter_writeBytes(_writer, p_,
					//		ALIFUSTR_DATA(rep),
					//		ALIFUSTR_GET_LENGTH(rep));
					//}

					//if (p_ == nullptr)
					//	goto error;
					//ALIF_CLEAR(rep);

					i_ = newpos;
				}
			}
			else
#if STRINGLIB_SIZEOF_CHAR > 2
				if (ch < 0x10000)
#endif
				{
					*p_++ = (char)(0xe0 | (ch >> 12));
					*p_++ = (char)(0x80 | ((ch >> 6) & 0x3f));
					*p_++ = (char)(0x80 | (ch & 0x3f));
				}
#if STRINGLIB_SIZEOF_CHAR > 2
				else /* ch >= 0x10000 */
				{
					/* Encode UCS4 Unicode ordinals */
					*p_++ = (char)(0xf0 | (ch >> 18));
					*p_++ = (char)(0x80 | ((ch >> 12) & 0x3f));
					*p_++ = (char)(0x80 | ((ch >> 6) & 0x3f));
					*p_++ = (char)(0x80 | (ch & 0x3f));
				}
#endif /* STRINGLIB_SIZEOF_CHAR > 2 */
#endif /* STRINGLIB_SIZEOF_CHAR > 1 */
	}

#if STRINGLIB_SIZEOF_CHAR > 1
	ALIF_XDECREF(errorHandlerObj);
	ALIF_XDECREF(exc);
#endif
	return p_;

#if STRINGLIB_SIZEOF_CHAR > 1
error:
	ALIF_XDECREF(rep);
	ALIF_XDECREF(errorHandlerObj);
	ALIF_XDECREF(exc);
	return nullptr;
#endif
}

 // 473
#if SIZEOF_LONG == 8
# define UCS2_REPEAT_MASK 0x0001000100010001ul
#elif SIZEOF_LONG == 4
# define UCS2_REPEAT_MASK 0x00010001ul
#else
# error C 'long' size should be either 4 or 8!
#endif

/* The mask for fast checking. */
#if STRINGLIB_SIZEOF_CHAR == 1
# define FAST_CHAR_MASK         (UCS2_REPEAT_MASK * (0xFFFFu & ~STRINGLIB_MAX_CHAR))
#else

# define FAST_CHAR_MASK         (UCS2_REPEAT_MASK * 0x8000u)
#endif
#define STRIPPED_MASK           (UCS2_REPEAT_MASK * 0x00FFu)
/* Swap bytes. */
#define SWAB(value)             ((((value) >> 8) & STRIPPED_MASK) | \
                                 (((value) & STRIPPED_MASK) << 8))


ALIF_LOCAL_INLINE(AlifUCS4)
STRINGLIB(utf16Decode)(const unsigned char** inptr, const unsigned char* e,
	STRINGLIB_CHAR* dest, AlifSizeT* outpos,
	AlifIntT native_ordering) { // 500
	AlifUCS4 ch{};
	const unsigned char* q = *inptr;
	STRINGLIB_CHAR* p = dest + *outpos;
	/* Offsets from q for retrieving byte pairs in the right order. */
#if ALIF_LITTLE_ENDIAN
	AlifIntT ihi = !!native_ordering, ilo = !native_ordering;
#else
	AlifIntT ihi = !native_ordering, ilo = !!native_ordering;
#endif
	--e;

	while (q < e) {
		AlifUCS4 ch2;
		if (ALIF_IS_ALIGNED(q, ALIGNOF_LONG)) {
			/* Fast path for runs of in-range non-surrogate chars. */
			const unsigned char* _q = q;
			while (_q + SIZEOF_LONG <= e) {
				unsigned long block = *(const unsigned long*)_q;
				if (native_ordering) {
					if (block & FAST_CHAR_MASK)
						break;
				}
				else {
					if (block & SWAB(FAST_CHAR_MASK))
						break;
#if STRINGLIB_SIZEOF_CHAR == 1
					block >>= 8;
#else
					block = SWAB(block);
#endif
				}
#if ALIF_LITTLE_ENDIAN
# if SIZEOF_LONG == 4
				p[0] = (STRINGLIB_CHAR)(block & 0xFFFFu);
				p[1] = (STRINGLIB_CHAR)(block >> 16);
# elif SIZEOF_LONG == 8
				p[0] = (STRINGLIB_CHAR)(block & 0xFFFFu);
				p[1] = (STRINGLIB_CHAR)((block >> 16) & 0xFFFFu);
				p[2] = (STRINGLIB_CHAR)((block >> 32) & 0xFFFFu);
				p[3] = (STRINGLIB_CHAR)(block >> 48);
# endif
#else
# if SIZEOF_LONG == 4
				p[0] = (STRINGLIB_CHAR)(block >> 16);
				p[1] = (STRINGLIB_CHAR)(block & 0xFFFFu);
# elif SIZEOF_LONG == 8
				p[0] = (STRINGLIB_CHAR)(block >> 48);
				p[1] = (STRINGLIB_CHAR)((block >> 32) & 0xFFFFu);
				p[2] = (STRINGLIB_CHAR)((block >> 16) & 0xFFFFu);
				p[3] = (STRINGLIB_CHAR)(block & 0xFFFFu);
# endif
#endif
				_q += SIZEOF_LONG;
				p += SIZEOF_LONG / 2;
			}
			q = _q;
			if (q >= e)
				break;
		}

		ch = (q[ihi] << 8) | q[ilo];
		q += 2;
		if (!alifUnicode_isSurrogate(ch)) {
#if STRINGLIB_SIZEOF_CHAR < 2
			if (ch > STRINGLIB_MAX_CHAR)
				/* Out-of-range */
				goto Return;
#endif
			* p++ = (STRINGLIB_CHAR)ch;
			continue;
		}

		/* UTF-16 code pair: */
		if (!alifUnicode_isHighSurrogate(ch))
			goto IllegalEncoding;
		if (q >= e)
			goto UnexpectedEnd;
		ch2 = (q[ihi] << 8) | q[ilo];
		q += 2;
		if (!alifUnicode_isLowSurrogate(ch2))
			goto IllegalSurrogate;
		ch = alifUnicode_joinSurrogates(ch, ch2);
#if STRINGLIB_SIZEOF_CHAR < 4
		/* Out-of-range */
		goto Return;
#else
		* p++ = (STRINGLIB_CHAR)ch;
#endif
	}
	ch = 0;
Return:
	*inptr = q;
	*outpos = p - dest;
	return ch;
UnexpectedEnd:
	ch = 1;
	goto Return;
IllegalEncoding:
	ch = 2;
	goto Return;
IllegalSurrogate:
	ch = 3;
	goto Return;
}
#undef UCS2_REPEAT_MASK
#undef FAST_CHAR_MASK
#undef STRIPPED_MASK
#undef SWAB











#if STRINGLIB_MAX_CHAR >= 0x80
ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(utf16Encode)(const STRINGLIB_CHAR* in, AlifSizeT len,
	unsigned short** outptr, AlifIntT native_ordering) { // 619
	unsigned short* out = *outptr;
	const STRINGLIB_CHAR* end = in + len;
#if STRINGLIB_SIZEOF_CHAR == 1
	if (native_ordering) {
		const STRINGLIB_CHAR* unrolled_end = in + ALIF_SIZE_ROUND_DOWN(len, 4);
		while (in < unrolled_end) {
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
			out[3] = in[3];
			in += 4; out += 4;
		}
		while (in < end) {
			*out++ = *in++;
		}
	}
	else {
# define SWAB2(CH)  ((CH) << 8) /* high byte is zero */
		const STRINGLIB_CHAR* unrolled_end = in + ALIF_SIZE_ROUND_DOWN(len, 4);
		while (in < unrolled_end) {
			out[0] = SWAB2(in[0]);
			out[1] = SWAB2(in[1]);
			out[2] = SWAB2(in[2]);
			out[3] = SWAB2(in[3]);
			in += 4; out += 4;
		}
		while (in < end) {
			AlifUCS4 ch = *in++;
			*out++ = SWAB2((AlifUCS2)ch);
		}
#undef SWAB2
	}
	*outptr = out;
	return len;
#else
	if (native_ordering) {
#if STRINGLIB_MAX_CHAR < 0x10000
		const STRINGLIB_CHAR* unrolled_end = in + ALIF_SIZE_ROUND_DOWN(len, 4);
		while (in < unrolled_end) {
			/* check if any character is a surrogate character */
			if (((in[0] ^ 0xd800) &
				(in[1] ^ 0xd800) &
				(in[2] ^ 0xd800) &
				(in[3] ^ 0xd800) & 0xf800) == 0)
				break;
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
			out[3] = in[3];
			in += 4; out += 4;
		}
#endif
		while (in < end) {
			AlifUCS4 ch;
			ch = *in++;
			if (ch < 0xd800)
				*out++ = ch;
			else if (ch < 0xe000)
				/* reject surrogate characters (U+D800-U+DFFF) */
				goto fail;
#if STRINGLIB_MAX_CHAR >= 0x10000
			else if (ch >= 0x10000) {
				out[0] = alifUnicode_highSurrogate(ch);
				out[1] = alifUnicode_lowSurrogate(ch);
				out += 2;
			}
#endif
			else
				*out++ = ch;
		}
	}
	else {
#define SWAB2(CH)  (((CH) << 8) | ((CH) >> 8))
#if STRINGLIB_MAX_CHAR < 0x10000
		const STRINGLIB_CHAR* unrolled_end = in + ALIF_SIZE_ROUND_DOWN(len, 4);
		while (in < unrolled_end) {
			/* check if any character is a surrogate character */
			if (((in[0] ^ 0xd800) &
				(in[1] ^ 0xd800) &
				(in[2] ^ 0xd800) &
				(in[3] ^ 0xd800) & 0xf800) == 0)
				break;
			out[0] = SWAB2(in[0]);
			out[1] = SWAB2(in[1]);
			out[2] = SWAB2(in[2]);
			out[3] = SWAB2(in[3]);
			in += 4; out += 4;
		}
#endif
		while (in < end) {
			AlifUCS4 ch = *in++;
			if (ch < 0xd800)
				*out++ = SWAB2((AlifUCS2)ch);
			else if (ch < 0xe000)
				/* reject surrogate characters (U+D800-U+DFFF) */
				goto fail;
#if STRINGLIB_MAX_CHAR >= 0x10000
			else if (ch >= 0x10000) {
				AlifUCS2 ch1 = alifUnicode_highSurrogate(ch);
				AlifUCS2 ch2 = alifUnicode_lowSurrogate(ch);
				out[0] = SWAB2(ch1);
				out[1] = SWAB2(ch2);
				out += 2;
			}
#endif
			else
				*out++ = SWAB2((AlifUCS2)ch);
		}
#undef SWAB2
	}
	*outptr = out;
	return len;
fail:
	*outptr = out;
	return len - (end - in + 1);
#endif
}



static inline uint32_t
STRINGLIB(SWAB4)(STRINGLIB_CHAR ch) { // 742
	uint32_t word = ch;
#if STRINGLIB_SIZEOF_CHAR == 1
	/* high bytes are zero */
	return (word << 24);
#elif STRINGLIB_SIZEOF_CHAR == 2
	/* high bytes are zero */
	return ((word & 0x00FFu) << 24) | ((word & 0xFF00u) << 8);
#else
	return _alif_bswap32(word);
#endif
}

ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(utf32Encode)(const STRINGLIB_CHAR* in,
	AlifSizeT len, uint32_t** outptr, AlifIntT native_ordering) { // 757
	uint32_t* out = *outptr;
	const STRINGLIB_CHAR* end = in + len;
	if (native_ordering) {
		const STRINGLIB_CHAR* unrolled_end = in + ALIF_SIZE_ROUND_DOWN(len, 4);
		while (in < unrolled_end) {
#if STRINGLIB_SIZEOF_CHAR > 1
			/* check if any character is a surrogate character */
			if (((in[0] ^ 0xd800) &
				(in[1] ^ 0xd800) &
				(in[2] ^ 0xd800) &
				(in[3] ^ 0xd800) & 0xf800) == 0)
				break;
#endif
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
			out[3] = in[3];
			in += 4; out += 4;
		}
		while (in < end) {
			AlifUCS4 ch;
			ch = *in++;
#if STRINGLIB_SIZEOF_CHAR > 1
			if (alifUnicode_isSurrogate(ch)) {
				/* reject surrogate characters (U+D800-U+DFFF) */
				goto fail;
			}
#endif
			* out++ = ch;
		}
	}
	else {
		const STRINGLIB_CHAR* unrolled_end = in + ALIF_SIZE_ROUND_DOWN(len, 4);
		while (in < unrolled_end) {
#if STRINGLIB_SIZEOF_CHAR > 1
			/* check if any character is a surrogate character */
			if (((in[0] ^ 0xd800) &
				(in[1] ^ 0xd800) &
				(in[2] ^ 0xd800) &
				(in[3] ^ 0xd800) & 0xf800) == 0)
				break;
#endif
			out[0] = STRINGLIB(SWAB4)(in[0]);
			out[1] = STRINGLIB(SWAB4)(in[1]);
			out[2] = STRINGLIB(SWAB4)(in[2]);
			out[3] = STRINGLIB(SWAB4)(in[3]);
			in += 4; out += 4;
		}
		while (in < end) {
			AlifUCS4 ch = *in++;
#if STRINGLIB_SIZEOF_CHAR > 1
			if (alifUnicode_isSurrogate(ch)) {
				/* reject surrogate characters (U+D800-U+DFFF) */
				goto fail;
			}
#endif
			* out++ = STRINGLIB(SWAB4)(ch);
		}
	}
	*outptr = out;
	return len;
#if STRINGLIB_SIZEOF_CHAR > 1
	fail:
	*outptr = out;
	return len - (end - in + 1);
#endif
}


#endif // 830
