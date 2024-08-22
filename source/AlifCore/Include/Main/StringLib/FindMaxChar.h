

// 10
#if (SIZEOF_SIZE_T == 8)
# define UCS1_ASCII_CHAR_MASK 0x8080808080808080ULL
#elif (SIZEOF_SIZE_T == 4)
# define UCS1_ASCII_CHAR_MASK 0x80808080U
#else
# error CPP 'size_t' size should be either 4 or 8!
#endif






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
