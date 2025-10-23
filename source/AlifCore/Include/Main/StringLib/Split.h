



// 14
#define MAX_PREALLOC 12

/* 5 splits gives 6 elements */
#define PREALLOC_SIZE(maxsplit) (maxsplit >= MAX_PREALLOC ? MAX_PREALLOC : maxsplit+1)

// 32
#define SPLIT_ADD(data, left, right) {          \
    sub = STRINGLIB_NEW((data) + (left),        \
                        (right) - (left));      \
    if (sub == nullptr)                            \
        goto onError;                           \
    if (count < MAX_PREALLOC) {                 \
        ALIFLIST_SET_ITEM(list, count, sub);      \
    } else {                                    \
        if (alifList_append(list, sub)) {         \
            ALIF_DECREF(sub);                     \
            goto onError;                       \
        }                                       \
        else                                    \
            ALIF_DECREF(sub);                    \
    }                                           \
    count++; }


// 51
#define FIX_PREALLOC_SIZE(list) ALIF_SET_SIZE(list, count)

ALIF_LOCAL_INLINE(AlifObject*) STRINGLIB(splitWhitespace)(AlifObject* str_obj,
	const STRINGLIB_CHAR* str, AlifSizeT str_len,
	AlifSizeT maxcount) {
	AlifSizeT i{}, j{}, count = 0;
	AlifObject* list = alifList_new(PREALLOC_SIZE(maxcount));
	AlifObject* sub{};

	if (list == nullptr)
		return nullptr;

	i = j = 0;
	while (maxcount-- > 0) {
		while (i < str_len and STRINGLIB_ISSPACE(str[i]))
			i++;
		if (i == str_len) break;
		j = i; i++;
		while (i < str_len and !STRINGLIB_ISSPACE(str[i]))
			i++;
	#if !STRINGLIB_MUTABLE
		if (j == 0 and i == str_len and STRINGLIB_CHECK_EXACT(str_obj)) {
			/* No whitespace in str_obj, so just use it as list[0] */
			ALIF_INCREF(str_obj);
			ALIFLIST_SET_ITEM(list, 0, (AlifObject*)str_obj);
			count++;
			break;
		}
	#endif
		SPLIT_ADD(str, j, i);
	}

	if (i < str_len) {
		/* Only occurs when maxcount was reached */
		/* Skip any remaining whitespace and copy to end of string */
		while (i < str_len and STRINGLIB_ISSPACE(str[i]))
			i++;
		if (i != str_len)
			SPLIT_ADD(str, i, str_len);
	}
	FIX_PREALLOC_SIZE(list);
	return list;

onError:
	ALIF_DECREF(list);
	return nullptr;
}



// 1101
ALIF_LOCAL_INLINE(AlifObject*)
STRINGLIB(splitChar)(AlifObject* str_obj,
	const STRINGLIB_CHAR* str, AlifSizeT str_len,
	const STRINGLIB_CHAR ch,
	AlifSizeT maxcount) {
	AlifSizeT i{}, j{}, count = 0;
	AlifObject* list = alifList_new(PREALLOC_SIZE(maxcount));
	AlifObject* sub;

	if (list == nullptr)
		return nullptr;

	i = j = 0;
	while ((j < str_len) and (maxcount-- > 0)) {
		for (; j < str_len; j++) {
			/* I found that using memchr makes no difference */
			if (str[j] == ch) {
				SPLIT_ADD(str, i, j);
				i = j = j + 1;
				break;
			}
		}
	}
#if !STRINGLIB_MUTABLE
	if (count == 0 && STRINGLIB_CHECK_EXACT(str_obj)) {
		/* ch not in str_obj, so just use str_obj as list[0] */
		ALIF_INCREF(str_obj);
		ALIFLIST_SET_ITEM(list, 0, (AlifObject*)str_obj);
		count++;
	}
	else
	#endif
		if (i <= str_len) {
			SPLIT_ADD(str, i, str_len);
		}
	FIX_PREALLOC_SIZE(list);
	return list;

onError:
	ALIF_DECREF(list);
	return nullptr;
}



// 144
ALIF_LOCAL_INLINE(AlifObject*) STRINGLIB(split)(AlifObject* str_obj,
	const STRINGLIB_CHAR* str, AlifSizeT str_len,
	const STRINGLIB_CHAR* sep, AlifSizeT sep_len,
	AlifSizeT maxcount) {
	AlifSizeT i{}, j{}, pos{}, count = 0;
	AlifObject* list{}, * sub{};

	if (sep_len == 0) {
		alifErr_setString(_alifExcValueError_, "لا يوجد فاصل");
		return nullptr;
	}
	else if (sep_len == 1)
		return STRINGLIB(splitChar)(str_obj, str, str_len, sep[0], maxcount);

	list = alifList_new(PREALLOC_SIZE(maxcount));
	if (list == nullptr)
		return nullptr;

	i = j = 0;
	while (maxcount-- > 0) {
		pos = FASTSEARCH(str + i, str_len - i, sep, sep_len, -1, FAST_SEARCH);
		if (pos < 0)
			break;
		j = i + pos;
		SPLIT_ADD(str, i, j);
		i = j + sep_len;
	}
#if !STRINGLIB_MUTABLE
	if (count == 0 and STRINGLIB_CHECK_EXACT(str_obj)) {
		/* No match in str_obj, so just use it as list[0] */
		ALIF_INCREF(str_obj);
		ALIFLIST_SET_ITEM(list, 0, (AlifObject*)str_obj);
		count++;
	}
	else
	#endif
	{
		SPLIT_ADD(str, i, str_len);
	}
	FIX_PREALLOC_SIZE(list);
	return list;

onError:
	ALIF_DECREF(list);
	return nullptr;
}
