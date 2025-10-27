




// 12
ALIF_LOCAL_INLINE(AlifObject*)
STRINGLIB(partition)(AlifObject* str_obj,
	const STRINGLIB_CHAR* str, AlifSizeT str_len,
	AlifObject* sep_obj,
	const STRINGLIB_CHAR* sep, AlifSizeT sep_len)
{
	AlifObject* out{};
	AlifSizeT pos{};

	if (sep_len == 0) {
		alifErr_setString(_alifExcValueError_, "لا يوجد فاصل");
		return nullptr;
	}

	out = alifTuple_new(3);
	if (!out)
		return nullptr;

	pos = FASTSEARCH(str, str_len, sep, sep_len, -1, FAST_SEARCH);

	if (pos < 0) {
#if STRINGLIB_MUTABLE
		ALIFTUPLE_SET_ITEM(out, 0, STRINGLIB_NEW(str, str_len));
		ALIFTUPLE_SET_ITEM(out, 1, STRINGLIB_NEW(nullptr, 0));
		ALIFTUPLE_SET_ITEM(out, 2, STRINGLIB_NEW(nullptr, 0));

		if (alifErr_occurred()) {
			ALIF_DECREF(out);
			return nullptr;
		}
#else
		ALIF_INCREF(str_obj);
		ALIFTUPLE_SET_ITEM(out, 0, (AlifObject*)str_obj);
		AlifObject* empty = (AlifObject*)STRINGLIB_GET_EMPTY();
		ALIF_INCREF(empty);
		ALIFTUPLE_SET_ITEM(out, 1, empty);
		ALIF_INCREF(empty);
		ALIFTUPLE_SET_ITEM(out, 2, empty);
#endif
		return out;
	}

	ALIFTUPLE_SET_ITEM(out, 0, STRINGLIB_NEW(str, pos));
	ALIF_INCREF(sep_obj);
	ALIFTUPLE_SET_ITEM(out, 1, sep_obj);
	pos += sep_len;
	ALIFTUPLE_SET_ITEM(out, 2, STRINGLIB_NEW(str + pos, str_len - pos));

	if (alifErr_occurred()) {
		ALIF_DECREF(out);
		return nullptr;
	}

	return out;
}
