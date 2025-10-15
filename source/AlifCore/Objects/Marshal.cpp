#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Code.h"
#include "AlifCore_HashTable.h"
#include "AlifCore_SetObject.h"
#include "AlifCore_Long.h"





// 41
#if defined(_WINDOWS)
#  define MAX_MARSHAL_STACK_DEPTH 1000
#elif defined(__wasi__)
#  define MAX_MARSHAL_STACK_DEPTH 1500
// TARGET_OS_IPHONE covers any non-macOS Apple platform.
// It won't be defined on older macOS SDKs
#elif defined(__APPLE__) and defined(TARGET_OS_IPHONE) and TARGET_OS_IPHONE
#  define MAX_MARSHAL_STACK_DEPTH 1500
#else
#  define MAX_MARSHAL_STACK_DEPTH 2000
#endif



 // 53
#define TYPE_NULL               '0'
#define TYPE_NONE               'N'
#define TYPE_FALSE              'F'
#define TYPE_TRUE               'T'
#define TYPE_STOPITER           'S'
#define TYPE_ELLIPSIS           '.'
#define TYPE_INT                'i'
/* TYPE_INT64 is not generated anymore.
   Supported for backward compatibility only. */
#define TYPE_INT64              'I'
#define TYPE_FLOAT              'f'
#define TYPE_BINARY_FLOAT       'g'
#define TYPE_COMPLEX            'x'
#define TYPE_BINARY_COMPLEX     'y'
#define TYPE_LONG               'l'
#define TYPE_STRING             's'
#define TYPE_INTERNED           't'
#define TYPE_REF                'r'
#define TYPE_TUPLE              '('
#define TYPE_LIST               '['
#define TYPE_DICT               '{'
#define TYPE_CODE               'c'
#define TYPE_UNICODE            'u'
#define TYPE_UNKNOWN            '?'
#define TYPE_SET                '<'
#define TYPE_FROZENSET          '>'
#define TYPE_SLICE              ':'
#define FLAG_REF                '\x80' /* with a type, add obj to index */

#define TYPE_ASCII              'a'
#define TYPE_ASCII_INTERNED     'A'
#define TYPE_SMALL_TUPLE        ')'
#define TYPE_SHORT_ASCII        'z'
#define TYPE_SHORT_ASCII_INTERNED 'Z'

#define WFERR_OK 0
#define WFERR_UNMARSHALLABLE 1
#define WFERR_NESTEDTOODEEP 2
#define WFERR_NOMEMORY 3
#define WFERR_CODE_NOT_ALLOWED 4


class WFile { // 94
public:
	FILE* fp{};
	AlifIntT error{};
	AlifIntT depth{};
	AlifObject* str{};
	char* ptr{};
	const char* end{};
	char* buf{};
	AlifHashTableT* hashtable{};
	AlifIntT version{};
	AlifIntT allowCode{};
};

// 107
#define W_BYTE(_c, _p) do {                               \
        if ((_p)->ptr != (_p)->end or w_reserve((_p), 1))  \
            *(_p)->ptr++ = (_c);                          \
    } while(0)


static void w_flush(WFile* p) { // 112
	fwrite(p->buf, 1, p->ptr - p->buf, p->fp);
	p->ptr = p->buf;
}

static AlifIntT w_reserve(WFile* _p, AlifSizeT _needed) { // 120
	AlifSizeT pos{}, size{}, delta{};
	if (_p->ptr == nullptr)
		return 0; /* An error already occurred */
	if (_p->fp != nullptr) {
		w_flush(_p);
		return _needed <= _p->end - _p->ptr;
	}
	pos = _p->ptr - _p->buf;
	size = ALIFBYTES_GET_SIZE(_p->str);
	if (size > 16 * 1024 * 1024)
		delta = (size >> 3);            /* 12.5% overallocation */
	else
		delta = size + 1024;
	delta = ALIF_MAX(delta, _needed);
	if (delta > ALIF_SIZET_MAX - size) {
		_p->error = WFERR_NOMEMORY;
		return 0;
	}
	size += delta;
	if (_alifBytes_resize(&_p->str, size) != 0) {
		_p->end = _p->ptr = _p->buf = nullptr;
		return 0;
	}
	else {
		_p->buf = ALIFBYTES_AS_STRING(_p->str);
		_p->ptr = _p->buf + pos;
		_p->end = _p->buf + size;
		return 1;
	}
}


static void w_string(const void* _s, AlifSizeT _n, WFile* _p) { // 155
	AlifSizeT m{};
	if (!_n or _p->ptr == nullptr)
		return;
	m = _p->end - _p->ptr;
	if (_p->fp != nullptr) {
		if (_n <= m) {
			memcpy(_p->ptr, _s, _n);
			_p->ptr += _n;
		}
		else {
			w_flush(_p);
			fwrite(_s, 1, _n, _p->fp);
		}
	}
	else {
		if (_n <= m or w_reserve(_p, _n - m)) {
			memcpy(_p->ptr, _s, _n);
			_p->ptr += _n;
		}
	}
}



static void w_short(AlifIntT _x, WFile* _p) { // 180
	W_BYTE((char)(_x & 0xff), _p);
	W_BYTE((char)((_x >> 8) & 0xff), _p);
}

static void w_long(long _x, WFile* _p) { // 187
	W_BYTE((char)(_x & 0xff), _p);
	W_BYTE((char)((_x >> 8) & 0xff), _p);
	W_BYTE((char)((_x >> 16) & 0xff), _p);
	W_BYTE((char)((_x >> 24) & 0xff), _p);
}




#define SIZE32_MAX  0x7FFFFFFF // 195

// 198
#if SIZEOF_SIZE_T > 4
# define W_SIZE(_n, _p)  do {                     \
        if ((_n) > SIZE32_MAX) {                 \
            (_p)->depth--;                       \
            (_p)->error = WFERR_UNMARSHALLABLE;  \
            return;                             \
        }                                       \
        w_long((long)(_n), _p);                   \
    } while(0)
#else
# define W_SIZE  w_long
#endif

static void w_pstring(const void* _s, AlifSizeT _n, WFile* _p) { // 211
	W_SIZE(_n, _p);
	w_string(_s, _n, _p);
}

static void w_shortPstring(const void* _s, AlifSizeT _n, WFile* _p) { // 218
	W_BYTE(ALIF_SAFE_DOWNCAST(_n, AlifSizeT, unsigned char), _p);
	w_string(_s, _n, _p);
}

// 228
#define ALIFLONG_MARSHAL_SHIFT 15
#define ALIFLONG_MARSHAL_BASE ((short)1 << ALIFLONG_MARSHAL_SHIFT)
#define ALIFLONG_MARSHAL_MASK (ALIFLONG_MARSHAL_BASE - 1)
#if ALIFLONG_SHIFT % ALIFLONG_MARSHAL_SHIFT != 0
#error "ALIFLONG_SHIFT must be a multiple of ALIFLONG_MARSHAL_SHIFT"
#endif
#define ALIFLONG_MARSHAL_RATIO (ALIFLONG_SHIFT / ALIFLONG_MARSHAL_SHIFT)

// 237
#define W_TYPE(_t, _p) do { \
    W_BYTE((_t) | flag, (_p)); \
} while(0)


static AlifObject* _alifMarshal_writeObjectToString(AlifObject*, AlifIntT, AlifIntT); // 241

static void w_alifLong(const AlifLongObject* ob, char flag, WFile* p) { // 244
	AlifSizeT i{}, j{}, n{}, l{};
	digit d{};

	W_TYPE(TYPE_LONG, p);
	if (_alifLong_isZero(ob)) {
		w_long((long)0, p);
		return;
	}

	/* set l to number of base ALIFLONG_MARSHAL_BASE digits */
	n = alifLong_digitCount(ob);
	l = (n - 1) * ALIFLONG_MARSHAL_RATIO;
	d = ob->longValue.digit[n - 1];
	do {
		d >>= ALIFLONG_MARSHAL_SHIFT;
		l++;
	}
	while (d != 0);
	if (l > SIZE32_MAX) {
		p->depth--;
		p->error = WFERR_UNMARSHALLABLE;
		return;
	}
	w_long((long)(_alifLong_isNegative(ob) ? -l : l), p);

	for (i = 0; i < n - 1; i++) {
		d = ob->longValue.digit[i];
		for (j = 0; j < ALIFLONG_MARSHAL_RATIO; j++) {
			w_short(d & ALIFLONG_MARSHAL_MASK, p);
			d >>= ALIFLONG_MARSHAL_SHIFT;
		}
	}
	d = ob->longValue.digit[n - 1];
	do {
		w_short(d & ALIFLONG_MARSHAL_MASK, p);
		d >>= ALIFLONG_MARSHAL_SHIFT;
	}
	while (d != 0);
}

static void w_floatBin(double _v, WFile* _) { // 287
	char buf[8];
	if (alifFloat_pack8(_v, buf, 1) < 0) {
		_->error = WFERR_UNMARSHALLABLE;
		return;
	}
	w_string(buf, 8, _);
}

static void w_floatStr(double v, WFile* p) { // 298
	char* buf = alifOS_doubleToString(v, 'g', 17, 0, nullptr);
	if (!buf) {
		p->error = WFERR_NOMEMORY;
		return;
	}
	w_shortPstring(buf, strlen(buf), p);
	alifMem_dataFree(buf);
}

static AlifIntT w_ref(AlifObject* _v, char* _flag, WFile* _p) { // 310
	AlifHashTableEntryT* entry{};
	AlifIntT w{};

	if (_p->version < 3 or _p->hashtable == nullptr)
		return 0; /* not writing object references */

	if (ALIF_REFCNT(_v) == 1 and
		!(ALIFUSTR_CHECKEXACT(_v) and ALIFUSTR_CHECK_INTERNED(_v))) {
		return 0;
	}

	entry = _alifHashTable_getEntry(_p->hashtable, _v);
	if (entry != nullptr) {
		/* write the reference index to the stream */
		w = (int)(uintptr_t)entry->value;
		/* we don't store "long" indices in the dict */
		W_BYTE(TYPE_REF, _p);
		w_long(w, _p);
		return 1;
	}
	else {
		size_t s = _p->hashtable->nentries;
		/* we don't support long indices */
		if (s >= 0x7fffffff) {
			alifErr_setString(_alifExcValueError_, "too many objects");
			goto err;
		}
		w = (AlifIntT)s;
		if (_alifHashTable_set(_p->hashtable, ALIF_NEWREF(_v),
			(void*)(uintptr_t)w) < 0) {
			ALIF_DECREF(_v);
			goto err;
		}
		*_flag |= FLAG_REF;
		return 0;
	}
err:
	_p->error = WFERR_UNMARSHALLABLE;
	return 1;
}

static void w_complexObject(AlifObject*, char, WFile*); // 358

static void w_object(AlifObject* _v, WFile* _p) { // 361
	char flag = '\0';

	_p->depth++;

	if (_p->depth > MAX_MARSHAL_STACK_DEPTH) {
		_p->error = WFERR_NESTEDTOODEEP;
	}
	else if (_v == nullptr) {
		W_BYTE(TYPE_NULL, _p);
	}
	else if (_v == ALIF_NONE) {
		W_BYTE(TYPE_NONE, _p);
	}
	else if (_v == _alifExcStopIteration_) {
		W_BYTE(TYPE_STOPITER, _p);
	}
	else if (_v == ALIF_ELLIPSIS) {
		W_BYTE(TYPE_ELLIPSIS, _p);
	}
	else if (_v == ALIF_FALSE) {
		W_BYTE(TYPE_FALSE, _p);
	}
	else if (_v == ALIF_TRUE) {
		W_BYTE(TYPE_TRUE, _p);
	}
	else if (!w_ref(_v, &flag, _p))
		w_complexObject(_v, flag, _p);

	_p->depth--;
}

static void w_complexObject(AlifObject* v, char flag, WFile* p) { // 395
	AlifSizeT i{}, n{};

	if (ALIFLONG_CHECKEXACT(v)) {
		AlifIntT overflow{};
		long x = alifLong_asLongAndOverflow(v, &overflow);
		if (overflow) {
			w_alifLong((AlifLongObject*)v, flag, p);
		}
		else {
		#if SIZEOF_LONG > 4
			long y = ALIF_ARITHMETIC_RIGHT_SHIFT(long, x, 31);
			if (y and y != -1) {
				/* Too large for TYPE_INT */
				w_alifLong((AlifLongObject*)v, flag, p);
			}
			else
			#endif
			{
				W_TYPE(TYPE_INT, p);
				w_long(x, p);
			}
		}
	}
	else if (ALIFFLOAT_CHECKEXACT(v)) {
		if (p->version > 1) {
			W_TYPE(TYPE_BINARY_FLOAT, p);
			w_floatBin(ALIFFLOAT_AS_DOUBLE(v), p);
		}
		else {
			W_TYPE(TYPE_FLOAT, p);
			w_floatStr(ALIFFLOAT_AS_DOUBLE(v), p);
		}
	}
	else if (ALIFCOMPLEX_CHECKEXACT(v)) {
		if (p->version > 1) {
			W_TYPE(TYPE_BINARY_COMPLEX, p);
			//w_floatBin(alifComplex_realAsDouble(v), p);
			//w_floatBin(alifComplex_imagAsDouble(v), p);
		}
		else {
			W_TYPE(TYPE_COMPLEX, p);
			//w_floatStr(alifComplex_realAsDouble(v), p);
			//w_floatStr(alifComplex_imagAsDouble(v), p);
		}
	}
	else if (ALIFBYTES_CHECKEXACT(v)) {
		W_TYPE(TYPE_STRING, p);
		w_pstring(ALIFBYTES_AS_STRING(v), ALIFBYTES_GET_SIZE(v), p);
	}
	else if (ALIFUSTR_CHECKEXACT(v)) {
		if (p->version >= 4 and ALIFUSTR_IS_ASCII(v)) {
			AlifIntT is_short = ALIFUSTR_GET_LENGTH(v) < 256;
			if (is_short) {
				if (ALIFUSTR_CHECK_INTERNED(v))
					W_TYPE(TYPE_SHORT_ASCII_INTERNED, p);
				else
					W_TYPE(TYPE_SHORT_ASCII, p);
				w_shortPstring(ALIFUSTR_1BYTE_DATA(v),
					ALIFUSTR_GET_LENGTH(v), p);
			}
			else {
				if (ALIFUSTR_CHECK_INTERNED(v))
					W_TYPE(TYPE_ASCII_INTERNED, p);
				else
					W_TYPE(TYPE_ASCII, p);
				w_pstring(ALIFUSTR_1BYTE_DATA(v),
					ALIFUSTR_GET_LENGTH(v), p);
			}
		}
		else {
			AlifObject* utf8{};
			utf8 = alifUStr_asEncodedString(v, "utf8", "surrogatepass");
			if (utf8 == nullptr) {
				p->depth--;
				p->error = WFERR_UNMARSHALLABLE;
				return;
			}
			if (p->version >= 3 and ALIFUSTR_CHECK_INTERNED(v))
				W_TYPE(TYPE_INTERNED, p);
			else
				W_TYPE(TYPE_UNICODE, p);
			w_pstring(ALIFBYTES_AS_STRING(utf8), ALIFBYTES_GET_SIZE(utf8), p);
			ALIF_DECREF(utf8);
		}
	}
	else if (ALIFTUPLE_CHECKEXACT(v)) {
		n = ALIFTUPLE_GET_SIZE(v);
		if (p->version >= 4 and n < 256) {
			W_TYPE(TYPE_SMALL_TUPLE, p);
			W_BYTE((unsigned char)n, p);
		}
		else {
			W_TYPE(TYPE_TUPLE, p);
			W_SIZE(n, p);
		}
		for (i = 0; i < n; i++) {
			w_object(ALIFTUPLE_GET_ITEM(v, i), p);
		}
	}
	else if (ALIFLIST_CHECKEXACT(v)) {
		W_TYPE(TYPE_LIST, p);
		n = ALIFLIST_GET_SIZE(v);
		W_SIZE(n, p);
		for (i = 0; i < n; i++) {
			w_object(ALIFLIST_GET_ITEM(v, i), p);
		}
	}
	else if (ALIFDICT_CHECKEXACT(v)) {
		AlifSizeT pos{};
		AlifObject* key{}, * value{};
		W_TYPE(TYPE_DICT, p);
		/* This one is NULL object terminated! */
		pos = 0;
		while (alifDict_next(v, &pos, &key, &value)) {
			w_object(key, p);
			w_object(value, p);
		}
		w_object((AlifObject*)nullptr, p);
	}
	else if (ALIFANYSET_CHECKEXACT(v)) {
		AlifObject* value{};
		AlifSizeT pos = 0;
		AlifHashT hash{};

		if (ALIFFROZENSET_CHECKEXACT(v))
			W_TYPE(TYPE_FROZENSET, p);
		else
			W_TYPE(TYPE_SET, p);
		n = ALIFSET_GET_SIZE(v);
		W_SIZE(n, p);
		AlifObject* pairs = alifList_new(n);
		if (pairs == nullptr) {
			p->error = WFERR_NOMEMORY;
			return;
		}
		AlifSizeT i = 0;
		ALIF_BEGIN_CRITICAL_SECTION(v);
		while (_alifSet_nextEntryRef(v, &pos, &value, &hash)) {
			AlifObject* dump = _alifMarshal_writeObjectToString(value,
				p->version, p->allowCode);
			if (dump == nullptr) {
				p->error = WFERR_UNMARSHALLABLE;
				ALIF_DECREF(value);
				break;
			}
			AlifObject* pair = alifTuple_pack(2, dump, value);
			ALIF_DECREF(dump);
			ALIF_DECREF(value);
			if (pair == nullptr) {
				p->error = WFERR_NOMEMORY;
				break;
			}
			ALIFLIST_SET_ITEM(pairs, i++, pair);
		}
		ALIF_END_CRITICAL_SECTION();
		if (p->error == WFERR_UNMARSHALLABLE or p->error == WFERR_NOMEMORY) {
			ALIF_DECREF(pairs);
			return;
		}
		if (alifList_sort(pairs)) {
			p->error = WFERR_NOMEMORY;
			ALIF_DECREF(pairs);
			return;
		}
		for (AlifSizeT i = 0; i < n; i++) {
			AlifObject* pair = ALIFLIST_GET_ITEM(pairs, i);
			value = ALIFTUPLE_GET_ITEM(pair, 1);
			w_object(value, p);
		}
		ALIF_DECREF(pairs);
	}
	else if (ALIFCODE_CHECK(v)) {
		if (!p->allowCode) {
			p->error = WFERR_CODE_NOT_ALLOWED;
			return;
		}
		AlifCodeObject* co = (AlifCodeObject*)v;
		AlifObject* co_code = _alifCode_getCode(co);
		if (co_code == nullptr) {
			p->error = WFERR_NOMEMORY;
			return;
		}
		W_TYPE(TYPE_CODE, p);
		w_long(co->argCount, p);
		w_long(co->posOnlyArgCount, p);
		w_long(co->kwOnlyArgCount, p);
		w_long(co->stackSize, p);
		w_long(co->flags, p);
		w_object(co_code, p);
		w_object(co->consts, p);
		w_object(co->names, p);
		w_object(co->localsPlusNames, p);
		w_object(co->localsPlusKinds, p);
		w_object(co->filename, p);
		w_object(co->name, p);
		w_object(co->qualname, p);
		w_long(co->firstLineno, p);
		w_object(co->lineTable, p);
		w_object(co->exceptiontable, p);
		ALIF_DECREF(co_code);
	}
	else if (alifObject_checkBuffer(v)) {
		/* Write unknown bytes-like objects as a bytes object */
		AlifBuffer view{};
		if (alifObject_getBuffer(v, &view, ALIFBUF_SIMPLE) != 0) {
			W_BYTE(TYPE_UNKNOWN, p);
			p->depth--;
			p->error = WFERR_UNMARSHALLABLE;
			return;
		}
		W_TYPE(TYPE_STRING, p);
		w_pstring(view.buf, view.len, p);
		alifBuffer_release(&view);
	}
	else if (ALIFSLICE_CHECK(v)) {
		AlifSliceObject* slice = (AlifSliceObject*)v;
		W_TYPE(TYPE_SLICE, p);
		w_object(slice->start, p);
		w_object(slice->stop, p);
		w_object(slice->step, p);
	}
	else {
		W_TYPE(TYPE_UNKNOWN, p);
		p->error = WFERR_UNMARSHALLABLE;
	}
}


static void w_decrefEntry(void* _key) { // 630
	AlifObject* entryKey = (AlifObject*)_key;
	ALIF_XDECREF(entryKey);
}

static AlifIntT w_initRefs(WFile* wf, AlifIntT version) { // 637
	if (version >= 3) {
		wf->hashtable = _alifHashTable_newFull(_alifHashTable_hashPtr,
			_alifHashTable_compareDirect,
			w_decrefEntry, nullptr, nullptr);
		if (wf->hashtable == nullptr) {
			//alifErr_noMemory();
			return -1;
		}
	}
	return 0;
}

static void w_clearRefs(WFile* _wf) { // 652
	if (_wf->hashtable != nullptr) {
		_alifHashTable_destroy(_wf->hashtable);
	}
}


class RFILE { // 695
public:
	FILE* fp{};
	AlifIntT depth{};
	AlifObject* readable{};
	const char* ptr{};
	const char* end{};
	char* buf{};
	AlifSizeT buf_size{};
	AlifObject* refs{};  /* a list */
	AlifIntT allowCode{};
};




static const char* r_string(AlifSizeT n, RFILE* p) { // 707
	AlifSizeT read = -1;

	if (p->ptr != nullptr) {
		/* Fast path for loads() */
		const char* res = p->ptr;
		AlifSizeT left = p->end - p->ptr;
		if (left < n) {
			//alifErr_setString(_alifExcEOFError_,
			//	"marshal data too short");
			return nullptr;
		}
		p->ptr += n;
		return res;
	}
	if (p->buf == nullptr) {
		p->buf = (char*)alifMem_dataAlloc(n);
		if (p->buf == nullptr) {
			//alifErr_noMemory();
			return nullptr;
		}
		p->buf_size = n;
	}
	else if (p->buf_size < n) {
		char* tmp = (char*)alifMem_dataRealloc(p->buf, n);
		if (tmp == nullptr) {
			//alifErr_noMemory();
			return nullptr;
		}
		p->buf = tmp;
		p->buf_size = n;
	}

	if (!p->readable) {
		read = fread(p->buf, 1, n, p->fp);
	}
	else {
		AlifObject* res{}, * mview{};
		AlifBuffer buf{};

		if (alifBuffer_fillInfo(&buf, nullptr, p->buf, n, 0, ALIFBUF_CONTIG) == -1)
			return nullptr;
		//mview = alifMemoryView_fromBuffer(&buf);
		//if (mview == nullptr)
		//	return nullptr;

		res = _alifObject_callMethod(p->readable, &ALIF_ID(ReadInto), "N", mview);
		if (res != nullptr) {
			read = alifNumber_asSizeT(res, _alifExcValueError_);
			ALIF_DECREF(res);
		}
	}
	if (read != n) {
		if (!alifErr_occurred()) {
			//if (read > n)
			//	alifErr_format(_alifExcValueError_,
			//		"read() returned too much data: "
			//		"%zd bytes requested, %zd returned",
			//		n, read);
			//else
			//	alifErr_setString(_alifExcEOFError_,
			//		"EOF read where not expected");
		}
		return nullptr;
	}
	return p->buf;
}

static AlifIntT r_byte(RFILE* _p) { // 778
	if (_p->ptr != nullptr) {
		if (_p->ptr < _p->end) {
			return (unsigned char)*_p->ptr++;
		}
	}
	else if (!_p->readable) {
		int c = getc(_p->fp);
		if (c != EOF) {
			return c;
		}
	}
	else {
		const char* ptr = r_string(1, _p);
		if (ptr != nullptr) {
			return *(const unsigned char*)ptr;
		}
		return EOF;
	}
	//alifErr_setString(_alifExcEOFError_,
	//	"EOF read where not expected");
	return EOF;
}

static AlifIntT r_short(RFILE* p) { // 805
	short x = -1;
	const unsigned char* buffer{};

	buffer = (const unsigned char*)r_string(2, p);
	if (buffer != nullptr) {
		x = buffer[0];
		x |= buffer[1] << 8;
		/* Sign-extension, in case short greater than 16 bits */
		x |= -(x & 0x8000);
	}
	return x;
}

static long r_long(RFILE* p) { // 821
	long x = -1;
	const unsigned char* buffer{};

	buffer = (const unsigned char*)r_string(4, p);
	if (buffer != nullptr) {
		x = buffer[0];
		x |= (long)buffer[1] << 8;
		x |= (long)buffer[2] << 16;
		x |= (long)buffer[3] << 24;
	#if SIZEOF_LONG > 4
		/* Sign extension for 64-bit machines */
		x |= -(x & 0x80000000L);
	#endif
	}
	return x;
}

static AlifObject* r_long64(RFILE* p) { // 842
	const unsigned char* buffer = (const unsigned char*)r_string(8, p);
	if (buffer == nullptr) {
		return nullptr;
	}
	return _alifLong_fromByteArray(buffer, 8,
		1 /* little endian */,
		1 /* signed */);
}

static AlifObject* r_alifLong(RFILE* p) { // 854
	AlifLongObject* ob{};
	long n{}, size{}, i{};
	AlifIntT j{}, md{}, shorts_in_top_digit{};
	digit d{};

	n = r_long(p);
	if (n == 0)
		return (AlifObject*)alifLong_new(0);
	if (n == -1 and alifErr_occurred()) {
		return nullptr;
	}
	if (n < -SIZE32_MAX or n > SIZE32_MAX) {
		//alifErr_setString(_alifExcValueError_,
		//	"bad marshal data (long size out of range)");
		return nullptr;
	}

	size = 1 + (ALIF_ABS(n) - 1) / ALIFLONG_MARSHAL_RATIO;
	shorts_in_top_digit = 1 + (ALIF_ABS(n) - 1) % ALIFLONG_MARSHAL_RATIO;
	ob = alifLong_new(size);
	if (ob == nullptr)
		return nullptr;

	_alifLong_setSignAndDigitCount(ob, n < 0 ? -1 : 1, size);

	for (i = 0; i < size - 1; i++) {
		d = 0;
		for (j = 0; j < ALIFLONG_MARSHAL_RATIO; j++) {
			md = r_short(p);
			if (md < 0 or md > ALIFLONG_MARSHAL_BASE)
				goto bad_digit;
			d += (digit)md << j * ALIFLONG_MARSHAL_SHIFT;
		}
		ob->longValue.digit[i] = d;
	}

	d = 0;
	for (j = 0; j < shorts_in_top_digit; j++) {
		md = r_short(p);
		if (md < 0 or md > ALIFLONG_MARSHAL_BASE)
			goto bad_digit;
		/* topmost marshal digit should be nonzero */
		if (md == 0 and j == shorts_in_top_digit - 1) {
			ALIF_DECREF(ob);
			//alifErr_setString(_alifExcValueError_,
			//	"bad marshal data (unnormalized long data)");
			return nullptr;
		}
		d += (digit)md << j * ALIFLONG_MARSHAL_SHIFT;
	}
	ob->longValue.digit[size - 1] = d;
	return (AlifObject*)ob;
bad_digit:
	ALIF_DECREF(ob);
	if (!alifErr_occurred()) {
		//alifErr_setString(_alifExcValueError_,
		//	"bad marshal data (digit out of range in long)");
	}
	return nullptr;
}


static double r_floatBin(RFILE* p) { // 921
	const char* buf = r_string(8, p);
	if (buf == nullptr)
		return -1;
	return alifFloat_unpack8(buf, 1);
}


ALIF_NO_INLINE static double r_floatStr(RFILE* p) { // 932
	AlifIntT n{};
	char buf[256]{};
	const char* ptr{};
	n = r_byte(p);
	if (n == EOF) {
		return -1;
	}
	ptr = r_string(n, p);
	if (ptr == nullptr) {
		return -1;
	}
	memcpy(buf, ptr, n);
	buf[n] = '\0';
	return alifOS_stringToDouble(buf, nullptr, nullptr);
}

static AlifSizeT r_refReserve(AlifIntT flag, RFILE* p) { // 952
	if (flag) { /* currently only FLAG_REF is defined */
		AlifSizeT idx = ALIFLIST_GET_SIZE(p->refs);
		if (idx >= 0x7ffffffe) {
			//alifErr_setString(_alifExcValueError_, "bad marshal data (index list too large)");
			return -1;
		}
		if (alifList_append(p->refs, ALIF_NONE) < 0)
			return -1;
		return idx;
	}
	else
		return 0;
}

static AlifObject* r_refInsert(AlifObject* o,
	AlifSizeT idx, AlifIntT flag, RFILE* p) { // 976
	if (o != nullptr and flag) { /* currently only FLAG_REF is defined */
		AlifObject* tmp = ALIFLIST_GET_ITEM(p->refs, idx);
		ALIFLIST_SET_ITEM(p->refs, idx, ALIF_NEWREF(o));
		ALIF_DECREF(tmp);
	}
	return o;
}

static AlifObject* r_ref(AlifObject* _o,
	AlifIntT _flag, RFILE* _p) { // 991
	if (_o == nullptr)
		return nullptr;
	if (alifList_append(_p->refs, _o) < 0) {
		ALIF_DECREF(_o); /* release the new object */
		return nullptr;
	}
	return _o;
}



static AlifObject* r_object(RFILE* p) { // 1004
	/* nullptr is a valid return value, it does not necessarily means that
	   an exception is set. */
	AlifObject* v{}, * v2{};
	AlifSizeT idx = 0;
	long i{}, n{};
	AlifIntT type{}, code = r_byte(p);
	AlifIntT flag{}, is_interned = 0;
	AlifObject* retval = nullptr;

	if (code == EOF) {
		//if (alifErr_exceptionMatches(_alifExcEOFError_)) {
		//	alifErr_setString(_alifExcEOFError_,
		//		"EOF read where object expected");
		//}
		return nullptr;
	}

	p->depth++;

	if (p->depth > MAX_MARSHAL_STACK_DEPTH) {
		p->depth--;
		alifErr_setString(_alifExcValueError_, "recursion limit exceeded");
		return nullptr;
	}

	flag = code & FLAG_REF;
	type = code & ~FLAG_REF;

#define R_REF(_o) do{\
    if (flag) \
        _o = r_ref(_o, flag, p);\
} while (0)

	switch (type) {

	case TYPE_NULL:
		break;

	case TYPE_NONE:
		retval = ALIF_NONE;
		break;

	case TYPE_STOPITER:
		retval = ALIF_NEWREF(_alifExcStopIteration_);
		break;

	case TYPE_ELLIPSIS:
		retval = ALIF_ELLIPSIS;
		break;

	case TYPE_FALSE:
		retval = ALIF_FALSE;
		break;

	case TYPE_TRUE:
		retval = ALIF_TRUE;
		break;

	case TYPE_INT:
		n = r_long(p);
		if (n == -1 and alifErr_occurred()) {
			break;
		}
		retval = alifLong_fromLong(n);
		R_REF(retval);
		break;

	case TYPE_INT64:
		retval = r_long64(p);
		R_REF(retval);
		break;

	case TYPE_LONG:
		retval = r_alifLong(p);
		R_REF(retval);
		break;

	case TYPE_FLOAT:
	{
		double x = r_floatStr(p);
		if (x == -1.0 and alifErr_occurred())
			break;
		retval = alifFloat_fromDouble(x);
		R_REF(retval);
		break;
	}

	case TYPE_BINARY_FLOAT:
	{
		double x = r_floatBin(p);
		if (x == -1.0 and alifErr_occurred())
			break;
		retval = alifFloat_fromDouble(x);
		R_REF(retval);
		break;
	}

	case TYPE_COMPLEX:
	{
		AlifComplex c{};
		c.real = r_floatStr(p);
		if (c.real == -1.0 and alifErr_occurred())
			break;
		c.imag = r_floatStr(p);
		if (c.imag == -1.0 and alifErr_occurred())
			break;
		//retval = alifComplex_fromCComplex(c);
		R_REF(retval);
		break;
	}

	case TYPE_BINARY_COMPLEX:
	{
		AlifComplex c{};
		c.real = r_floatBin(p);
		if (c.real == -1.0 and alifErr_occurred())
			break;
		c.imag = r_floatBin(p);
		if (c.imag == -1.0 and alifErr_occurred())
			break;
		//retval = alifComplex_fromCComplex(c);
		R_REF(retval);
		break;
	}

	case TYPE_STRING:
	{
		const char* ptr;
		n = r_long(p);
		if (n < 0 or n > SIZE32_MAX) {
			if (!alifErr_occurred()) {
				//alifErr_setString(_alifExcValueError_,
				//	"bad marshal data (bytes object size out of range)");
			}
			break;
		}
		v = alifBytes_fromStringAndSize((char*)nullptr, n);
		if (v == nullptr)
			break;
		ptr = r_string(n, p);
		if (ptr == nullptr) {
			ALIF_DECREF(v);
			break;
		}
		memcpy(ALIFBYTES_AS_STRING(v), ptr, n);
		retval = v;
		R_REF(retval);
		break;
	}

	case TYPE_ASCII_INTERNED:
		is_interned = 1;
		ALIF_FALLTHROUGH;
	case TYPE_ASCII:
		n = r_long(p);
		if (n < 0 or n > SIZE32_MAX) {
			if (!alifErr_occurred()) {
				//alifErr_setString(_alifExcValueError_,
				//	"bad marshal data (string size out of range)");
			}
			break;
		}
		goto _read_ascii;

	case TYPE_SHORT_ASCII_INTERNED:
		is_interned = 1;
		ALIF_FALLTHROUGH;
	case TYPE_SHORT_ASCII:
		n = r_byte(p);
		if (n == EOF) {
			break;
		}
_read_ascii:
		{
			const char* ptr;
			ptr = r_string(n, p);
			if (ptr == nullptr)
				break;
			v = alifUStr_fromKindAndData(AlifUStrKind_::AlifUStr_1Byte_Kind, ptr, n);
			if (v == nullptr)
				break;
			if (is_interned) {
				// marshal is meant to serialize .alifc files with code
				// objects, and code-related strings are currently immortal.
				AlifInterpreter* interp = _alifInterpreter_get();
				alifUStr_internImmortal(interp, &v);
			}
			retval = v;
			R_REF(retval);
			break;
		}

	case TYPE_INTERNED:
		is_interned = 1;
		ALIF_FALLTHROUGH;
	case TYPE_UNICODE:
	{
		const char* buffer;

		n = r_long(p);
		if (n < 0 or n > SIZE32_MAX) {
			if (!alifErr_occurred()) {
				//alifErr_setString(_alifExcValueError_,
				//	"bad marshal data (string size out of range)");
			}
			break;
		}
		if (n != 0) {
			buffer = r_string(n, p);
			if (buffer == nullptr)
				break;
			v = alifUStr_decodeUTF8(buffer, n, "surrogatepass");
		}
		else {
			v = alif_getConstant(ALIF_CONSTANT_EMPTY_STR);
		}
		if (v == nullptr)
			break;
		if (is_interned) {
			// marshal is meant to serialize .alifc files with code
			// objects, and code-related strings are currently immortal.
			AlifInterpreter* interp = _alifInterpreter_get();
			alifUStr_internImmortal(interp, &v);
		}
		retval = v;
		R_REF(retval);
		break;
	}

	case TYPE_SMALL_TUPLE:
		n = r_byte(p);
		if (n == EOF) {
			break;
		}
		goto _read_tuple;
	case TYPE_TUPLE:
		n = r_long(p);
		if (n < 0 or n > SIZE32_MAX) {
			if (!alifErr_occurred()) {
				//alifErr_setString(_alifExcValueError_,
				//	"bad marshal data (tuple size out of range)");
			}
			break;
		}
_read_tuple:
		v = alifTuple_new(n);
		R_REF(v);
		if (v == nullptr)
			break;

		for (i = 0; i < n; i++) {
			v2 = r_object(p);
			if (v2 == nullptr) {
				if (!alifErr_occurred())
					//alifErr_setString(_alifExcTypeError_,
					//	"nullptr object in marshal data for tuple");
					ALIF_SETREF(v, nullptr);
				break;
			}
			ALIFTUPLE_SET_ITEM(v, i, v2);
		}
		retval = v;
		break;

	case TYPE_LIST:
		n = r_long(p);
		if (n < 0 or n > SIZE32_MAX) {
			if (!alifErr_occurred()) {
				//alifErr_setString(_alifExcValueError_,
				//	"bad marshal data (list size out of range)");
			}
			break;
		}
		v = alifList_new(n);
		R_REF(v);
		if (v == nullptr)
			break;
		for (i = 0; i < n; i++) {
			v2 = r_object(p);
			if (v2 == nullptr) {
				if (!alifErr_occurred())
					//alifErr_setString(_alifExcTypeError_,
					//	"nullptr object in marshal data for list");
					ALIF_SETREF(v, nullptr);
				break;
			}
			ALIFLIST_SET_ITEM(v, i, v2);
		}
		retval = v;
		break;

	case TYPE_DICT:
		v = alifDict_new();
		R_REF(v);
		if (v == nullptr)
			break;
		for (;;) {
			AlifObject* key, * val;
			key = r_object(p);
			if (key == nullptr)
				break;
			val = r_object(p);
			if (val == nullptr) {
				ALIF_DECREF(key);
				break;
			}
			if (alifDict_setItem(v, key, val) < 0) {
				ALIF_DECREF(key);
				ALIF_DECREF(val);
				break;
			}
			ALIF_DECREF(key);
			ALIF_DECREF(val);
		}
		if (alifErr_occurred()) {
			ALIF_SETREF(v, nullptr);
		}
		retval = v;
		break;

	case TYPE_SET:
	case TYPE_FROZENSET:
		n = r_long(p);
		if (n < 0 or n > SIZE32_MAX) {
			if (!alifErr_occurred()) {
				//alifErr_setString(_alifExcValueError_,
				//	"bad marshal data (set size out of range)");
			}
			break;
		}

		if (n == 0 && type == TYPE_FROZENSET) {
			/* call frozenset() to get the empty frozenset singleton */
			v = _alifObject_callNoArgs((AlifObject*)&_alifFrozenSetType_);
			if (v == nullptr)
				break;
			R_REF(v);
			retval = v;
		}
		else {
			v = (type == TYPE_SET) ? alifSet_new(nullptr) : alifFrozenSet_new(nullptr);
			if (type == TYPE_SET) {
				R_REF(v);
			}
			else {
				/* must use delayed registration of frozensets because they must
				 * be init with a refcount of 1
				 */
				idx = r_refReserve(flag, p);
				if (idx < 0)
					ALIF_CLEAR(v); /* signal error */
			}
			if (v == nullptr)
				break;

			for (i = 0; i < n; i++) {
				v2 = r_object(p);
				if (v2 == nullptr) {
					if (!alifErr_occurred())
						//alifErr_setString(_alifExcTypeError_,
						//	"nullptr object in marshal data for set");
						ALIF_SETREF(v, nullptr);
					break;
				}
				if (alifSet_add(v, v2) == -1) {
					ALIF_DECREF(v);
					ALIF_DECREF(v2);
					v = nullptr;
					break;
				}
				ALIF_DECREF(v2);
			}
			if (type != TYPE_SET)
				v = r_refInsert(v, idx, flag, p);
			retval = v;
		}
		break;

	case TYPE_CODE:
	{
		AlifIntT argcount{};
		AlifIntT posonlyargcount{};
		AlifIntT kwonlyargcount{};
		AlifIntT stacksize{};
		AlifIntT flags{};
		AlifObject* code = nullptr;
		AlifObject* consts = nullptr;
		AlifObject* names = nullptr;
		AlifObject* localsplusnames = nullptr;
		AlifObject* localspluskinds = nullptr;
		AlifObject* filename = nullptr;
		AlifObject* name = nullptr;
		AlifObject* qualname = nullptr;
		AlifIntT firstlineno{};
		AlifObject* linetable = nullptr;
		AlifObject* exceptiontable = nullptr;

		if (!p->allowCode) {
			//alifErr_setString(_alifExcValueError_,
			//	"unmarshalling code objects is disallowed");
			break;
		}
		idx = r_refReserve(flag, p);
		if (idx < 0)
			break;

		v = nullptr;
		AlifCodeConstructor con{}; //* alif

		/* XXX ignore long->AlifIntT overflows for now */
		argcount = (AlifIntT)r_long(p);
		if (argcount == -1 && alifErr_occurred())
			goto code_error;
		posonlyargcount = (AlifIntT)r_long(p);
		if (posonlyargcount == -1 && alifErr_occurred()) {
			goto code_error;
		}
		kwonlyargcount = (AlifIntT)r_long(p);
		if (kwonlyargcount == -1 && alifErr_occurred())
			goto code_error;
		stacksize = (AlifIntT)r_long(p);
		if (stacksize == -1 && alifErr_occurred())
			goto code_error;
		flags = (AlifIntT)r_long(p);
		if (flags == -1 && alifErr_occurred())
			goto code_error;
		code = r_object(p);
		if (code == nullptr)
			goto code_error;
		consts = r_object(p);
		if (consts == nullptr)
			goto code_error;
		names = r_object(p);
		if (names == nullptr)
			goto code_error;
		localsplusnames = r_object(p);
		if (localsplusnames == nullptr)
			goto code_error;
		localspluskinds = r_object(p);
		if (localspluskinds == nullptr)
			goto code_error;
		filename = r_object(p);
		if (filename == nullptr)
			goto code_error;
		name = r_object(p);
		if (name == nullptr)
			goto code_error;
		qualname = r_object(p);
		if (qualname == nullptr)
			goto code_error;
		firstlineno = (AlifIntT)r_long(p);
		if (firstlineno == -1 && alifErr_occurred())
			break;
		linetable = r_object(p);
		if (linetable == nullptr)
			goto code_error;
		exceptiontable = r_object(p);
		if (exceptiontable == nullptr)
			goto code_error;

		con = {
			.filename = filename,
			.name = name,
			.qualname = qualname,
			.flags = flags,

			.code = code,
			.firstLineno = firstlineno,
			.lineTable = linetable,

			.consts = consts,
			.names = names,

			.localsPlusNames = localsplusnames,
			.localsPlusKinds = localspluskinds,

			.argCount = argcount,
			.posOnlyArgCount = posonlyargcount,
			.kwOnlyArgCount = kwonlyargcount,

			.stackSize = stacksize,

			.exceptionTable = exceptiontable,
		};

		if (_alifCode_validate(&con) < 0) {
			goto code_error;
		}

		v = (AlifObject*)alifCode_new(&con);
		if (v == nullptr) {
			goto code_error;
		}

		v = r_refInsert(v, idx, flag, p);

code_error:
		if (v == nullptr and !alifErr_occurred()) {
			//alifErr_setString(_alifExcTypeError_,
			//	"nullptr object in marshal data for code object");
		}
		ALIF_XDECREF(code);
		ALIF_XDECREF(consts);
		ALIF_XDECREF(names);
		ALIF_XDECREF(localsplusnames);
		ALIF_XDECREF(localspluskinds);
		ALIF_XDECREF(filename);
		ALIF_XDECREF(name);
		ALIF_XDECREF(qualname);
		ALIF_XDECREF(linetable);
		ALIF_XDECREF(exceptiontable);
	}
	retval = v;
	break;

	case TYPE_REF:
		n = r_long(p);
		if (n < 0 or n >= ALIFLIST_GET_SIZE(p->refs)) {
			if (!alifErr_occurred()) {
				//alifErr_setString(_alifExcValueError_,
				//	"bad marshal data (invalid reference)");
			}
			break;
		}
		v = ALIFLIST_GET_ITEM(p->refs, n);
		if (v == ALIF_NONE) {
			//alifErr_setString(_alifExcValueError_, "bad marshal data (invalid reference)");
			break;
		}
		retval = ALIF_NEWREF(v);
		break;

	default:
		/* Bogus data got written, which isn't ideal.
		   This will let you keep working and recover. */
		   //alifErr_setString(_alifExcValueError_, "bad marshal data (unknown type code)");
		break;

	}
	p->depth--;
	return retval;
}


static AlifObject* read_object(RFILE* p) { // 1548
	AlifObject* v{};
	if (alifErr_occurred()) {
		fprintf(stderr, "XXX readobject called with exception set\n");
		return nullptr;
	}
	if (p->ptr and p->end) {
		//if (alifSys_audit("marshal.loads", "y#", p->ptr, (AlifSizeT)(p->end - p->ptr)) < 0) {
		//	return nullptr;
		//}
	}
	else if (p->fp or p->readable) {
		//if (alifSys_audit("marshal.load", nullptr) < 0) {
		//	return nullptr;
		//}
	}
	v = r_object(p);
	if (v == nullptr and !alifErr_occurred()) {
		//alifErr_setString(_alifExcTypeError_, "nullptr object in marshal data for object
	}

	return v;
}


AlifObject* alifMarshal_readObjectFromString(const char* _str, AlifSizeT _len) { // 1703
	RFILE rf{};
	AlifObject* result{};
	rf.allowCode = 1;
	rf.fp = nullptr;
	rf.readable = nullptr;
	rf.ptr = _str;
	rf.end = _str + _len;
	rf.buf = nullptr;
	rf.depth = 0;
	rf.refs = alifList_new(0);
	if (rf.refs == nullptr)
		return nullptr;
	result = read_object(&rf);
	ALIF_DECREF(rf.refs);
	if (rf.buf != nullptr)
		alifMem_dataFree(rf.buf);
	return result;
}




static AlifObject* _alifMarshal_writeObjectToString(AlifObject* _x, AlifIntT _version,
	AlifIntT _allowCode) { // 1725
	WFile wf{};

	//if (alifSys_audit("marshal.dumps", "Oi", x, version) < 0) {
	//	return nullptr;
	//}
	memset(&wf, 0, sizeof(wf));
	wf.str = alifBytes_fromStringAndSize((char*)nullptr, 50);
	if (wf.str == nullptr)
		return nullptr;
	wf.ptr = wf.buf = ALIFBYTES_AS_STRING(wf.str);
	wf.end = wf.ptr + ALIFBYTES_GET_SIZE(wf.str);
	wf.error = WFERR_OK;
	wf.version = _version;
	wf.allowCode = _allowCode;
	if (w_initRefs(&wf, _version)) {
		ALIF_DECREF(wf.str);
		return nullptr;
	}
	w_object(_x, &wf);
	w_clearRefs(&wf);
	if (wf.str != nullptr) {
		const char* base = ALIFBYTES_AS_STRING(wf.str);
		if (_alifBytes_resize(&wf.str, (AlifSizeT)(wf.ptr - base)) < 0)
			return nullptr;
	}
	if (wf.error != WFERR_OK) {
		ALIF_XDECREF(wf.str);
		switch (wf.error) {
		case WFERR_NOMEMORY:
			//alifErr_noMemory();
			break;
		case WFERR_NESTEDTOODEEP:
			alifErr_setString(_alifExcValueError_,
				"object too deeply nested to marshal");
			break;
		case WFERR_CODE_NOT_ALLOWED:
			alifErr_setString(_alifExcValueError_,
				"marshalling code objects is disallowed");
			break;
		default:
		case WFERR_UNMARSHALLABLE:
			alifErr_setString(_alifExcValueError_,
				"unmarshallable object");
			break;
		}
		return nullptr;
	}
	return wf.str;
}

AlifObject* alifMarshal_writeObjectToString(AlifObject* _x, AlifIntT _version) { // 1778
	return _alifMarshal_writeObjectToString(_x, _version, 1);
}
