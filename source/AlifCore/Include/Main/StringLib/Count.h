



#if !STRINGLIB_IS_UNICODE or STRINGLIB_MAX_CHAR > 0x7Fu

ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(count)(const STRINGLIB_CHAR* str, AlifSizeT str_len,
	const STRINGLIB_CHAR* sub, AlifSizeT sub_len,
	AlifSizeT maxcount) {
	AlifSizeT count{};

	if (str_len < 0)
		return 0; /* start > len(str) */
	if (sub_len == 0)
		return (str_len < maxcount) ? str_len + 1 : maxcount;

	count = FASTSEARCH(str, str_len, sub, sub_len, maxcount, FAST_COUNT);

	if (count < 0)
		return 0; /* no match */

	return count;
}

#endif
