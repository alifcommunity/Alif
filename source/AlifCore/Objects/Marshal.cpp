#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Code.h"
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




#define SIZE32_MAX  0x7FFFFFFF // 195




 // 228
#define ALIFLONG_MARSHAL_SHIFT 15
#define ALIFLONG_MARSHAL_BASE ((short)1 << ALIFLONG_MARSHAL_SHIFT)
#define ALIFLONG_MARSHAL_MASK (ALIFLONG_MARSHAL_BASE - 1)
#if ALIFLONG_SHIFT % ALIFLONG_MARSHAL_SHIFT != 0
#error "ALIFLONG_SHIFT must be a multiple of ALIFLONG_MARSHAL_SHIFT"
#endif
#define ALIFLONG_MARSHAL_RATIO (ALIFLONG_SHIFT / ALIFLONG_MARSHAL_SHIFT)




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
			read = alifNumber_asSizeT(res, nullptr /*_alifExcValueError_*/);
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
		//alifErr_setString(_alifExcValueError_, "recursion limit exceeded");
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
		//retval = ALIF_NEWREF(_alifExcStopIteration_);
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
			v = alifUStr_new(0, 0);
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


AlifObject* alifMarshal_readObjectFromString(const char* _str, AlifSizeT _len) { // 1669
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
