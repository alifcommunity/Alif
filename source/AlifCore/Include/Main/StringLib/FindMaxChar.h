

// 10
#if (SIZEOF_SIZE_T == 8)
# define UCS1_ASCII_CHAR_MASK 0x8080808080808080ULL
#elif (SIZEOF_SIZE_T == 4)
# define UCS1_ASCII_CHAR_MASK 0x80808080U
#else
# error CPP 'size_t' size should be either 4 or 8!
#endif



#if STRINGLIB_SIZEOF_CHAR == 1


ALIF_LOCAL_INLINE(AlifUCS4)
STRINGLIB(findMaxChar)(const STRINGLIB_CHAR* _begin, const STRINGLIB_CHAR* _end) { // 20
	const unsigned char* p_ = (const unsigned char*)_begin;
	const unsigned char* end_ = (const unsigned char*)_end;

	while ((STRINGLIB_CHAR*)p_ < _end) {
		if (ALIF_IS_ALIGNED(p_, ALIGNOF_SIZE_T)) {
			const unsigned char* _p = p_;
			while ((STRINGLIB_CHAR*)_p + SIZEOF_SIZE_T <= _end) {
				size_t value = *(const size_t*)_p;
				if (value & UCS1_ASCII_CHAR_MASK)
					return 255;
				_p += SIZEOF_SIZE_T;
			}
			p_ = _p;
			if ((STRINGLIB_CHAR*)p_ == _end)
				break;
		}
		if (*p_++ & 0x80)
			return 255;
	}
	return 127;
}


#undef ASCII_CHAR_MASK


#else /* STRINGLIB_SIZEOF_CHAR == 1 */

#define MASK_ASCII 0xFFFFFF80
#define MASK_UCS1 0xFFFFFF00
#define MASK_UCS2 0xFFFF0000

#define MAX_CHAR_ASCII 0x7f
#define MAX_CHAR_UCS1  0xff
#define MAX_CHAR_UCS2  0xffff
#define MAX_CHAR_UCS4  0x10ffff

ALIF_LOCAL_INLINE(AlifUCS4)
STRINGLIB(findMaxChar)(const STRINGLIB_CHAR* begin, const STRINGLIB_CHAR* end)
{
#if STRINGLIB_SIZEOF_CHAR == 2
	const AlifUCS4 mask_limit = MASK_UCS1;
	const AlifUCS4 max_char_limit = MAX_CHAR_UCS2;
#elif STRINGLIB_SIZEOF_CHAR == 4
	const AlifUCS4 mask_limit = MASK_UCS2;
	const AlifUCS4 max_char_limit = MAX_CHAR_UCS4;
#else
#error Invalid STRINGLIB_SIZEOF_CHAR (must be 1, 2 or 4)
#endif
	AlifUCS4 mask;
	AlifSizeT n = end - begin;
	const STRINGLIB_CHAR* p = begin;
	const STRINGLIB_CHAR* unrolled_end = begin + ALIF_SIZE_ROUND_DOWN(n, 4);
	AlifUCS4 max_char;

	max_char = MAX_CHAR_ASCII;
	mask = MASK_ASCII;
	while (p < unrolled_end) {
		STRINGLIB_CHAR bits = p[0] | p[1] | p[2] | p[3];
		if (bits & mask) {
			if (mask == mask_limit) {
				/* Limit reached */
				return max_char_limit;
			}
			if (mask == MASK_ASCII) {
				max_char = MAX_CHAR_UCS1;
				mask = MASK_UCS1;
			}
			else {
				/* mask can't be MASK_UCS2 because of mask_limit above */
				max_char = MAX_CHAR_UCS2;
				mask = MASK_UCS2;
			}
			/* We check the new mask on the same chars in the next iteration */
			continue;
		}
		p += 4;
	}
	while (p < end) {
		if (p[0] & mask) {
			if (mask == mask_limit) {
				/* Limit reached */
				return max_char_limit;
			}
			if (mask == MASK_ASCII) {
				max_char = MAX_CHAR_UCS1;
				mask = MASK_UCS1;
			}
			else {
				/* mask can't be MASK_UCS2 because of mask_limit above */
				max_char = MAX_CHAR_UCS2;
				mask = MASK_UCS2;
			}
			/* We check the new mask on the same chars in the next iteration */
			continue;
		}
		p++;
	}
	return max_char;
}

#undef MASK_ASCII
#undef MASK_UCS1
#undef MASK_UCS2
#undef MAX_CHAR_ASCII
#undef MAX_CHAR_UCS1
#undef MAX_CHAR_UCS2
#undef MAX_CHAR_UCS4

#endif /* STRINGLIB_SIZEOF_CHAR == 1 */
