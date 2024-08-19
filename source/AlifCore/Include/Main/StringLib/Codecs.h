



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
#if PY_LITTLE_ENDIAN
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
			if (STRINGLIB_MAX_CHAR <= 0x007F ||
				(STRINGLIB_MAX_CHAR < 0x07FF && ch_ > STRINGLIB_MAX_CHAR))
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
				if (!IS_CONTINUATION_BYTE(ch2) ||
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
				/* Decoding UTF-8 sequences in range \xED\xA0\x80-\xED\xBF\xBF
				   will result in surrogates in range D800-DFFF. Surrogates are
				   not valid UTF-8 so they are rejected.
				   See https://www.unicode.org/versions/Unicode5.2.0/ch03.pdf
				   (table 3-7) and http://www.rfc-editor.org/rfc/rfc3629.txt */
				goto invalidContinuation1;
			}
			if (!IS_CONTINUATION_BYTE(ch3)) {
				/* invalid continuation byte */
				goto invalidContinuation2;
			}
			ch_ = (ch_ << 12) + (ch2 << 6) + ch3 -
				((0xE0 << 12) + (0x80 << 6) + 0x80);
			s_ += 3;
			if (STRINGLIB_MAX_CHAR <= 0x07FF ||
				(STRINGLIB_MAX_CHAR < 0xFFFF && ch_ > STRINGLIB_MAX_CHAR))
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
				if (!IS_CONTINUATION_BYTE(ch2) ||
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
			else if (ch_ == 0xF4 && ch2 >= 0x90) {
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
			if (STRINGLIB_MAX_CHAR <= 0xFFFF ||
				(STRINGLIB_MAX_CHAR < 0x10FFFF && ch_ > STRINGLIB_MAX_CHAR))
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
