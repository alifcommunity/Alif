


ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(find)(const STRINGLIB_CHAR* str, AlifSizeT str_len,
	const STRINGLIB_CHAR* sub, AlifSizeT sub_len,
	AlifSizeT offset) {
	AlifSizeT pos;

	if (sub_len == 0)
		return offset;

	pos = FASTSEARCH(str, str_len, sub, sub_len, -1, FAST_SEARCH);

	if (pos >= 0)
		pos += offset;

	return pos;
}

ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(rfind)(const STRINGLIB_CHAR* str, AlifSizeT str_len,
	const STRINGLIB_CHAR* sub, AlifSizeT sub_len,
	AlifSizeT offset) {
	AlifSizeT pos;

	if (sub_len == 0)
		return str_len + offset;

	pos = FASTSEARCH(str, str_len, sub, sub_len, -1, FAST_RSEARCH);

	if (pos >= 0)
		pos += offset;

	return pos;
}

ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(find_slice)(const STRINGLIB_CHAR* str, AlifSizeT str_len,
	const STRINGLIB_CHAR* sub, AlifSizeT sub_len,
	AlifSizeT start, AlifSizeT end) {
	return STRINGLIB(find)(str + start, end - start, sub, sub_len, start);
}

ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(rfind_slice)(const STRINGLIB_CHAR* str, AlifSizeT str_len,
	const STRINGLIB_CHAR* sub, AlifSizeT sub_len,
	AlifSizeT start, AlifSizeT end) {
	return STRINGLIB(rfind)(str + start, end - start, sub, sub_len, start);
}
