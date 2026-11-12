
static inline AlifObject* return_self(AlifObject* self) {
#if !STRINGLIB_MUTABLE
	if (STRINGLIB_CHECK_EXACT(self)) {
		return ALIF_NEWREF(self);
	}
#endif
	return STRINGLIB_NEW(STRINGLIB_STR(self), STRINGLIB_LEN(self));
}

static AlifObject* stringLib_expandtabsImpl(AlifObject* self,
	AlifIntT tabsize) {
	const char* e, * p;
	char* q;
	AlifSizeT i, j;
	AlifObject* u;

	i = j = 0;
	e = STRINGLIB_STR(self) + STRINGLIB_LEN(self);
	for (p = STRINGLIB_STR(self); p < e; p++) {
		if (*p == '\t') {
			if (tabsize > 0) {
				AlifSizeT incr = tabsize - (j % tabsize);
				if (j > ALIF_SIZET_MAX - incr)
					goto overflow;
				j += incr;
			}
		}
		else {
			if (j > ALIF_SIZET_MAX - 1)
				goto overflow;
			j++;
			if (*p == '\n' || *p == '\r') {
				if (i > ALIF_SIZET_MAX - j)
					goto overflow;
				i += j;
				j = 0;
			}
		}
	}

	if (i > ALIF_SIZET_MAX - j)
		goto overflow;

	u = STRINGLIB_NEW(nullptr, i + j);
	if (!u)
		return nullptr;

	j = 0;
	q = STRINGLIB_STR(u);

	for (p = STRINGLIB_STR(self); p < e; p++) {
		if (*p == '\t') {
			if (tabsize > 0) {
				i = tabsize - (j % tabsize);
				j += i;
				while (i--)
					*q++ = ' ';
			}
		}
		else {
			j++;
			*q++ = *p;
			if (*p == '\n' || *p == '\r')
				j = 0;
		}
	}

	return u;
overflow:
	alifErr_setString(_alifExcOverflowError_, "النتائج طويلة جدا");
	return nullptr;
}

static inline AlifObject* pad(AlifObject* self,
	AlifSizeT left, AlifSizeT right, char fill) {
	AlifObject* u;

	if (left < 0)
		left = 0;
	if (right < 0)
		right = 0;

	if (left == 0 && right == 0) {
		return return_self(self);
	}

	u = STRINGLIB_NEW(nullptr, left + STRINGLIB_LEN(self) + right);
	if (u) {
		if (left)
			memset(STRINGLIB_STR(u), fill, left);
		memcpy(STRINGLIB_STR(u) + left,
			STRINGLIB_STR(self),
			STRINGLIB_LEN(self));
		if (right)
			memset(STRINGLIB_STR(u) + left + STRINGLIB_LEN(self),
				fill, right);
	}

	return u;
}

static AlifObject* stringLib_ljustImpl(AlifObject* self,
	AlifSizeT width, char fillchar) {
	if (STRINGLIB_LEN(self) >= width) {
		return return_self(self);
	}

	return pad(self, 0, width - STRINGLIB_LEN(self), fillchar);
}



static AlifObject* stringLib_rjustImpl(AlifObject* self,
	AlifSizeT width, char fillchar) {
	if (STRINGLIB_LEN(self) >= width) {
		return return_self(self);
	}

	return pad(self, width - STRINGLIB_LEN(self), 0, fillchar);
}



static AlifObject* stringLib_centerImpl(AlifObject* self,
	AlifSizeT width, char fillchar) {
	AlifSizeT marg, left;

	if (STRINGLIB_LEN(self) >= width) {
		return return_self(self);
	}

	marg = width - STRINGLIB_LEN(self);
	left = marg / 2 + (marg & width & 1);

	return pad(self, left, marg - left, fillchar);
}


static AlifObject* stringLib_zfillImpl(AlifObject* self,
	AlifSizeT width) {
	AlifSizeT fill;
	AlifObject* s;
	char* p;

	if (STRINGLIB_LEN(self) >= width) {
		return return_self(self);
	}

	fill = width - STRINGLIB_LEN(self);

	s = pad(self, fill, 0, '0');

	if (s == nullptr)
		return nullptr;

	p = STRINGLIB_STR(s);
	if (p[fill] == '+' || p[fill] == '-') {
		p[0] = p[fill];
		p[fill] = '0';
	}

	return s;
}



#define findchar(target, target_len, c)                         \
  ((char *)memchr((const void *)(target), c, target_len))


static AlifSizeT count_char(const char* target, AlifSizeT target_len,
	char c, AlifSizeT maxcount) {
	AlifSizeT count = 0;
	const char* start = target;
	const char* end = target + target_len;

	while ((start = findchar(start, end - start, c)) != nullptr) {
		count++;
		if (count >= maxcount)
			break;
		start += 1;
	}
	return count;
}



static AlifObject* stringLib_replaceInterleave(AlifObject* self,
	const char* to_s, AlifSizeT to_len, AlifSizeT maxcount) {
	const char* self_s;
	char* result_s;
	AlifSizeT self_len, result_len;
	AlifSizeT count, i;
	AlifObject* result;

	self_len = STRINGLIB_LEN(self);

	if (maxcount <= self_len) {
		count = maxcount;
	}
	else {
		count = self_len + 1;
	}

	if (to_len > (ALIF_SIZET_MAX - self_len) / count) {
		alifErr_setString(_alifExcOverflowError_,
			"ثمانيات الاستبدال طويلة جدا");
		return nullptr;
	}
	result_len = count * to_len + self_len;
	result = STRINGLIB_NEW(nullptr, result_len);
	if (result == nullptr) {
		return nullptr;
	}

	self_s = STRINGLIB_STR(self);
	result_s = STRINGLIB_STR(result);

	if (to_len > 1) {
		memcpy(result_s, to_s, to_len);
		result_s += to_len;
		count -= 1;

		for (i = 0; i < count; i++) {
			*result_s++ = *self_s++;
			memcpy(result_s, to_s, to_len);
			result_s += to_len;
		}
	}
	else {
		result_s[0] = to_s[0];
		result_s += to_len;
		count -= 1;
		for (i = 0; i < count; i++) {
			*result_s++ = *self_s++;
			result_s[0] = to_s[0];
			result_s += to_len;
		}
	}

	memcpy(result_s, self_s, self_len - i);

	return result;
}

static AlifObject* stringLib_replaceDeleteSingleCharacter(AlifObject* self,
	char from_c, AlifSizeT maxcount) {
	const char* self_s, * start, * next, * end;
	char* result_s;
	AlifSizeT self_len, result_len;
	AlifSizeT count;
	AlifObject* result;

	self_len = STRINGLIB_LEN(self);
	self_s = STRINGLIB_STR(self);

	count = count_char(self_s, self_len, from_c, maxcount);
	if (count == 0) {
		return return_self(self);
	}

	result_len = self_len - count;  /* from_len == 1 */

	result = STRINGLIB_NEW(nullptr, result_len);
	if (result == nullptr) {
		return nullptr;
	}
	result_s = STRINGLIB_STR(result);

	start = self_s;
	end = self_s + self_len;
	while (count-- > 0) {
		next = findchar(start, end - start, from_c);
		if (next == nullptr)
			break;
		memcpy(result_s, start, next - start);
		result_s += (next - start);
		start = next + 1;
	}
	memcpy(result_s, start, end - start);

	return result;
}


static AlifObject* stringLib_replaceDeleteSubstring(AlifObject* self,
	const char* from_s, AlifSizeT from_len, AlifSizeT maxcount) {
	const char* self_s, * start, * next, * end;
	char* result_s;
	AlifSizeT self_len, result_len;
	AlifSizeT count, offset;
	AlifObject* result;

	self_len = STRINGLIB_LEN(self);
	self_s = STRINGLIB_STR(self);

	count = stringLib_count(self_s, self_len,
		from_s, from_len,
		maxcount);

	if (count == 0) {
		return return_self(self);
	}

	result_len = self_len - (count * from_len);

	result = STRINGLIB_NEW(nullptr, result_len);
	if (result == nullptr) {
		return nullptr;
	}
	result_s = STRINGLIB_STR(result);

	start = self_s;
	end = self_s + self_len;
	while (count-- > 0) {
		offset = stringLib_find(start, end - start,
			from_s, from_len,
			0);
		if (offset == -1)
			break;
		next = start + offset;

		memcpy(result_s, start, next - start);

		result_s += (next - start);
		start = next + from_len;
	}
	memcpy(result_s, start, end - start);
	return result;
}

static AlifObject* stringLib_replaceSingleCharacterInPlace(AlifObject* self,
	char from_c, char to_c,
	AlifSizeT maxcount) {
	const char* self_s, * end;
	char* result_s, * start, * next;
	AlifSizeT self_len;
	AlifObject* result;

	self_s = STRINGLIB_STR(self);
	self_len = STRINGLIB_LEN(self);

	next = findchar(self_s, self_len, from_c);

	if (next == nullptr) {
		/* No matches; return the original bytes */
		return return_self(self);
	}

	/* Need to make a new bytes */
	result = STRINGLIB_NEW(nullptr, self_len);
	if (result == nullptr) {
		return nullptr;
	}
	result_s = STRINGLIB_STR(result);
	memcpy(result_s, self_s, self_len);

	/* change everything in-place, starting with this one */
	start = result_s + (next - self_s);
	*start = to_c;
	start++;
	end = result_s + self_len;

	while (--maxcount > 0) {
		next = findchar(start, end - start, from_c);
		if (next == nullptr)
			break;
		*next = to_c;
		start = next + 1;
	}

	return result;
}

static AlifObject* stringLib_replaceSubstringInPlace(AlifObject* self,
	const char* from_s, AlifSizeT from_len,
	const char* to_s, AlifSizeT to_len,
	AlifSizeT maxcount) {
	const char* self_s, * end;
	char* result_s, * start;
	AlifSizeT self_len, offset;
	AlifObject* result;


	self_s = STRINGLIB_STR(self);
	self_len = STRINGLIB_LEN(self);

	offset = stringLib_find(self_s, self_len,
		from_s, from_len,
		0);
	if (offset == -1) {
		return return_self(self);
	}

	result = STRINGLIB_NEW(nullptr, self_len);
	if (result == nullptr) {
		return nullptr;
	}
	result_s = STRINGLIB_STR(result);
	memcpy(result_s, self_s, self_len);

	start = result_s + offset;
	memcpy(start, to_s, from_len);
	start += from_len;
	end = result_s + self_len;

	while (--maxcount > 0) {
		offset = stringLib_find(start, end - start,
			from_s, from_len,
			0);
		if (offset == -1)
			break;
		memcpy(start + offset, to_s, from_len);
		start += offset + from_len;
	}

	return result;
}

static AlifObject* stringLib_replaceSingleCharacter(AlifObject* self,
	char from_c,
	const char* to_s, AlifSizeT to_len,
	AlifSizeT maxcount) {
	const char* self_s, * start, * next, * end;
	char* result_s;
	AlifSizeT self_len, result_len;
	AlifSizeT count;
	AlifObject* result;

	self_s = STRINGLIB_STR(self);
	self_len = STRINGLIB_LEN(self);

	count = count_char(self_s, self_len, from_c, maxcount);
	if (count == 0) {
		return return_self(self);
	}

	if (to_len - 1 > (ALIF_SIZET_MAX - self_len) / count) {
		alifErr_setString(_alifExcOverflowError_, "ثمانيات الإستبدال طويلة جدا");
		return nullptr;
	}
	result_len = self_len + count * (to_len - 1);

	result = STRINGLIB_NEW(nullptr, result_len);
	if (result == nullptr) {
		return nullptr;
	}
	result_s = STRINGLIB_STR(result);

	start = self_s;
	end = self_s + self_len;
	while (count-- > 0) {
		next = findchar(start, end - start, from_c);
		if (next == nullptr)
			break;

		if (next == start) {
			memcpy(result_s, to_s, to_len);
			result_s += to_len;
			start += 1;
		}
		else {
			memcpy(result_s, start, next - start);
			result_s += (next - start);
			memcpy(result_s, to_s, to_len);
			result_s += to_len;
			start = next + 1;
		}
	}
	memcpy(result_s, start, end - start);

	return result;
}

static AlifObject* stringLib_replaceSubstring(AlifObject* self,
	const char* from_s, AlifSizeT from_len,
	const char* to_s, AlifSizeT to_len,
	AlifSizeT maxcount) {
	const char* self_s, * start, * next, * end;
	char* result_s;
	AlifSizeT self_len, result_len;
	AlifSizeT count, offset;
	AlifObject* result;

	self_s = STRINGLIB_STR(self);
	self_len = STRINGLIB_LEN(self);

	count = stringLib_count(self_s, self_len,
		from_s, from_len,
		maxcount);

	if (count == 0) {
		return return_self(self);
	}

	if (to_len - from_len > (ALIF_SIZET_MAX - self_len) / count) {
		alifErr_setString(_alifExcOverflowError_, "ثمانيات الإستبدال طويلة جدا");
		return nullptr;
	}
	result_len = self_len + count * (to_len - from_len);

	result = STRINGLIB_NEW(nullptr, result_len);
	if (result == nullptr) {
		return nullptr;
	}
	result_s = STRINGLIB_STR(result);

	start = self_s;
	end = self_s + self_len;
	while (count-- > 0) {
		offset = stringLib_find(start, end - start,
			from_s, from_len,
			0);
		if (offset == -1)
			break;
		next = start + offset;
		if (next == start) {
			memcpy(result_s, to_s, to_len);
			result_s += to_len;
			start += from_len;
		}
		else {
			memcpy(result_s, start, next - start);
			result_s += (next - start);
			memcpy(result_s, to_s, to_len);
			result_s += to_len;
			start = next + from_len;
		}
	}
	memcpy(result_s, start, end - start);

	return result;
}


static AlifObject* stringLib_replace(AlifObject* self,
	const char* from_s, AlifSizeT from_len,
	const char* to_s, AlifSizeT to_len,
	AlifSizeT maxcount) {
	if (STRINGLIB_LEN(self) < from_len) {
		return return_self(self);
	}
	if (maxcount < 0) {
		maxcount = ALIF_SIZET_MAX;
	}
	else if (maxcount == 0) {
		return return_self(self);
	}

	if (from_len == 0) {
		if (to_len == 0) {
			return return_self(self);
		}
		return stringLib_replaceInterleave(self, to_s, to_len, maxcount);
	}

	if (to_len == 0) {
		if (from_len == 1) {
			return stringLib_replaceDeleteSingleCharacter(
				self, from_s[0], maxcount);
		}
		else {
			return stringLib_replaceDeleteSubstring(
				self, from_s, from_len, maxcount);
		}
	}


	if (from_len == to_len) {
		if (from_len == 1) {
			return stringLib_replaceSingleCharacterInPlace(
				self, from_s[0], to_s[0], maxcount);
		}
		else {
			return stringLib_replaceSubstringInPlace(
				self, from_s, from_len, to_s, to_len, maxcount);
		}
	}

	if (from_len == 1) {
		return stringLib_replaceSingleCharacter(
			self, from_s[0], to_s, to_len, maxcount);
	}
	else {
		return stringLib_replaceSubstring(
			self, from_s, from_len, to_s, to_len, maxcount);
	}
}

#undef findchar
