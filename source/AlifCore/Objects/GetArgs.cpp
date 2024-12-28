#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Dict.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Tuple.h"


#define FLAG_COMPAT 1 // 23

typedef AlifIntT (*DestrT)(AlifObject*, void*); // 25

class FreeListEntryT { // 32
public:
	void* item{};
	DestrT destructor{};
};

class FreeListT { // 37
public:
	FreeListEntryT* entries{};
	AlifIntT first_available{};
	AlifIntT entries_malloced{};
};


#define STATIC_FREELIST_ENTRIES 8 // 43



AlifIntT alifArg_parseTuple(AlifObject* _args, const char* _format, ...) { // 94
	AlifIntT retval{};
	va_list va{};

	va_start(va, _format);
	retval = vGetArgs1(_args, _format, &va, 0);
	va_end(va);
	return retval;
}


static AlifIntT clean_return(AlifIntT _retval, FreeListT* _freelist) { // 193
	AlifIntT index{};

	if (_retval == 0) {
		for (index = 0; index < _freelist->first_available; ++index) {
			_freelist->entries[index].destructor(nullptr,
				_freelist->entries[index].item);
		}
	}
	if (_freelist->entries_malloced)
		alifMem_dataFree(_freelist->entries);
	return _retval;
}


static AlifIntT vGetArgs1_impl(AlifObject* _compatArgs,
	AlifObject* const* _stack, AlifSizeT _nargs, const char* _format,
	va_list* _pVa, AlifIntT _flags) { // 213
	char msgbuf[256]{};
	AlifIntT levels[32]{};
	const char* fname = nullptr;
	const char* message = nullptr;
	AlifIntT min = -1;
	AlifIntT max = 0;
	AlifIntT level = 0;
	AlifIntT endfmt = 0;
	const char* formatsave = _format;
	AlifSizeT i{};
	const char* msg;
	AlifIntT compat = _flags & FLAG_COMPAT;
	FreeListEntryT static_entries[STATIC_FREELIST_ENTRIES];
	FreeListT freelist;


	freelist.entries = static_entries;
	freelist.first_available = 0;
	freelist.entries_malloced = 0;

	_flags = _flags & ~FLAG_COMPAT;

	while (endfmt == 0) {
		int c = *_format++;
		switch (c) {
		case '(':
			if (level == 0)
				max++;
			level++;
			if (level >= 30)
			{ /*alif_fatalError("too many tuple nesting levels "
					"in argument format string");*/ }
			break;
		case ')':
			if (level == 0)
			{ /*alif_fatalError("excess ')' in getargs format");*/ }
			else
				level--;
			break;
		case '\0':
			endfmt = 1;
			break;
		case ':':
			fname = _format;
			endfmt = 1;
			break;
		case ';':
			message = _format;
			endfmt = 1;
			break;
		case '|':
			if (level == 0)
				min = max;
			break;
		default:
			if (level == 0) {
				if (ALIF_ISALPHA(c))
					if (c != 'e') /* skip encoded */
						max++;
			}
			break;
		}
	}

	//if (level != 0)
	//{ alif_fatalError(/* '(' */ "missing ')' in getargs format"); }

	if (min < 0)
		min = max;

	_format = formatsave;

	if (max > STATIC_FREELIST_ENTRIES) {
		freelist.entries = ((AlifUSizeT)max > ALIF_SIZET_MAX / sizeof(FreeListEntryT)) ? nullptr : \
			(FreeListEntryT*)alifMem_dataAlloc(max * sizeof(FreeListEntryT)); // alif
		if (freelist.entries == nullptr) {
			//alifErr_noMemory();
			return 0;
		}
		freelist.entries_malloced = 1;
	}

	if (compat) {
		if (max == 0) {
			if (_compatArgs == nullptr)
				return 1;
			//alifErr_format(_alifExcTypeError_,
			//	"%.200s%s takes no arguments",
			//	fname == nullptr ? "function" : fname,
			//	fname == nullptr ? "" : "()");
			return clean_return(0, &freelist);
		}
		else if (min == 1 && max == 1) {
			if (_compatArgs == nullptr) {
				//alifErr_format(_alifExcTypeError_,
				//	"%.200s%s takes at least one argument",
				//	fname == nullptr ? "function" : fname,
				//	fname == nullptr ? "" : "()");
				return clean_return(0, &freelist);
			}
			msg = convert_item(_compatArgs, &_format, _pVa, _flags, levels,
				msgbuf, sizeof(msgbuf), &freelist);
			if (msg == nullptr)
				return clean_return(1, &freelist);
			//seterror(levels[0], msg, levels + 1, fname, message);
			return clean_return(0, &freelist);
		}
		else {
			//alifErr_setString(_alifExcSystemError_,
			//	"old style getargs format uses new features");
			return clean_return(0, &freelist);
		}
	}

	if (_nargs < min || max < _nargs) {
		//if (message == nullptr)
			//alifErr_format(_alifExcTypeError_,
			//	"%.150s%s takes %s %d argument%s (%zd given)",
			//	fname == nullptr ? "function" : fname,
			//	fname == nullptr ? "" : "()",
			//	min == max ? "exactly"
			//	: _nargs < min ? "at least" : "at most",
			//	_nargs < min ? min : max,
			//	(_nargs < min ? min : max) == 1 ? "" : "s",
			//	_nargs);
		//else
			//alifErr_setString(_alifExcTypeError_, message);
		return clean_return(0, &freelist);
	}

	for (i = 0; i < _nargs; i++) {
		if (*_format == '|')
			_format++;
		msg = convert_item(_stack[i], &_format, _pVa,
			_flags, levels, msgbuf,
			sizeof(msgbuf), &freelist);
		if (msg) {
			//seterror(i + 1, msg, levels, fname, message);
			return clean_return(0, &freelist);
		}
	}

	if (*_format != '\0' and !ALIF_ISALPHA(*_format) and
		*_format != '(' and
		*_format != '|' and *_format != ':' and *_format != ';') {
		//alifErr_format(_alifExcSystemError_,
		//	"bad format string: %.200s", formatsave);
		return clean_return(0, &freelist);
	}

	return clean_return(1, &freelist);
}



static AlifIntT vGetArgs1(AlifObject* _args, const char* _format,
	va_list* _pVa, AlifIntT _flags) { // 370
	AlifObject** stack{};
	AlifSizeT nargs{};

	if (!(_flags & FLAG_COMPAT)) {

		if (!ALIFTUPLE_CHECK(_args)) {
			//alifErr_setString(_alifExcSystemError_,
			//	"new style getargs format but argument is not a tuple");
			return 0;
		}

		stack = ALIFTUPLE_ITEMS(_args);
		nargs = ALIFTUPLE_GET_SIZE(_args);
	}
	else {
		stack = nullptr;
		nargs = 0;
	}

	return vGetArgs1_impl(_args, stack, nargs, _format, _pVa, _flags);
}



static const char* convert_tuple(AlifObject* _arg, const char** _pFormat,
	va_list* _pVa, AlifIntT _flags, AlifIntT* _levels, char* _msgbuf,
	AlifUSizeT _bufsize, FreeListT* _freelist) { // 458
	AlifIntT level = 0;
	AlifIntT n = 0;
	const char* format = *_pFormat;
	AlifIntT i{};
	AlifSizeT len{};

	for (;;) {
		int c = *format++;
		if (c == '(') {
			if (level == 0)
				n++;
			level++;
		}
		else if (c == ')') {
			if (level == 0)
				break;
			level--;
		}
		else if (c == ':' || c == ';' || c == '\0')
			break;
		else if (level == 0 and ALIF_ISALPHA(c) and c != 'e')
			n++;
	}

	if (!alifSequence_check(_arg) or ALIFBYTES_CHECK(_arg)) {
		_levels[0] = 0;
		//alifOS_snprintf(_msgbuf, _bufsize,
		//	"must be %d-item sequence, not %.50s",
		//	n,
		//	_arg == ALIF_NONE ? "None" : ALIF_TYPE(_arg)->name);
		return _msgbuf;
	}

	len = alifSequence_size(_arg);
	if (len != n) {
		_levels[0] = 0;
		//alifOS_snprintf(_msgbuf, _bufsize,
		//	"must be sequence of length %d, not %zd",
		//	n, len);
		return _msgbuf;
	}

	format = *_pFormat;
	for (i = 0; i < n; i++) {
		const char* msg{};
		AlifObject* item{};
		item = alifSequence_getItem(_arg, i);
		if (item == nullptr) {
			//alifErr_clear();
			_levels[0] = i + 1;
			_levels[1] = 0;
			strncpy(_msgbuf, "is not retrievable", _bufsize);
			return _msgbuf;
		}
		msg = convert_item(item, &format, _pVa, _flags, _levels + 1,
			_msgbuf, _bufsize, _freelist);
		ALIF_XDECREF(item);
		if (msg != nullptr) {
			_levels[0] = i + 1;
			return msg;
		}
	}

	*_pFormat = format;
	return nullptr;
}



static const char* convert_item(AlifObject* _arg, const char** _pFormat, va_list* _pVa, AlifIntT _flags,
	AlifIntT* _levels, char* _msgbuf, AlifUSizeT _bufsize, FreeListT* _freelist) { // 534
	const char* msg{};
	const char* format = *_pFormat;

	if (*format == '(' /* ')' */) {
		format++;
		msg = convert_tuple(_arg, &format, _pVa, _flags, _levels, _msgbuf,
			_bufsize, _freelist);
		if (msg == nullptr)
			format++;
	}
	else {
		//msg = convert_simple(_arg, &format, _pVa, _flags,
		//	_msgbuf, _bufsize, _freelist);
		if (msg != nullptr)
			_levels[0] = 0;
	}
	if (msg == nullptr)
		*_pFormat = format;
	return msg;
}






#define IS_END_OF_FORMAT(_c) (_c == '\0' or _c == ';' or _c == ':') // 1510





static AlifIntT scan_keywords(const char* const* _keywords, AlifIntT* _ptotal, AlifIntT* _pposonly) { // 1820
	AlifIntT i{};
	for (i = 0; _keywords[i] and !*_keywords[i]; i++) {
	}
	*_pposonly = i;

	for (; _keywords[i]; i++) {
		if (!*_keywords[i]) {
			//alifErr_setString(_alifExcSystemError_,
			//	"Empty keyword parameter name");
			return -1;
		}
	}
	*_ptotal = i;
	return 0;
}

static AlifIntT parse_format(const char* _format, AlifIntT _total,
	AlifIntT _npos, const char** _pfname, const char** _pCustomMsg,
	AlifIntT* _pmin, AlifIntT* _pmax) { // 1841
	const char* custommsg{};
	const char* fname = strchr(_format, ':');
	if (fname) {
		fname++;
		custommsg = nullptr;
	}
	else {
		custommsg = strchr(_format, ';');
		if (custommsg) {
			custommsg++;
		}
	}

	AlifIntT min = INT_MAX;
	AlifIntT max = INT_MAX;
	for (int i = 0; i < _total; i++) {
		if (*_format == '|') {
			if (min != INT_MAX) {
				//alifErr_setString(_alifExcSystemError_,
				//	"Invalid format string (| specified twice)");
				return -1;
			}
			if (max != INT_MAX) {
				//alifErr_setString(_alifExcSystemError_,
				//	"Invalid format string ($ before |)");
				return -1;
			}
			min = i;
			_format++;
		}
		if (*_format == '$') {
			if (max != INT_MAX) {
				//alifErr_setString(_alifExcSystemError_,
				//	"Invalid format string ($ specified twice)");
				return -1;
			}
			if (i < _npos) {
				//alifErr_setString(_alifExcSystemError_,
				//	"Empty parameter name after $");
				return -1;
			}
			max = i;
			_format++;
		}
		if (IS_END_OF_FORMAT(*_format)) {
			//alifErr_format(_alifExcSystemError_,
			//	"More keyword list entries (%d) than "
			//	"format specifiers (%d)", _total, i);
			return -1;
		}

		//const char* msg = skipitem(&_format, nullptr, 0);
		//if (msg) {
		//	//alifErr_format(_alifExcSystemError_, "%s: '%s'", msg,
		//	//	_format);
		//	return -1;
		//}
	}
	min = ALIF_MIN(min, _total);
	max = ALIF_MIN(max, _total);

	if (!IS_END_OF_FORMAT(*_format) && (*_format != '|') && (*_format != '$')) {
		//alifErr_format(_alifExcSystemError_,
		//	"more argument specifiers than keyword list entries "
		//	"(remaining format:'%s')", _format);
		return -1;
	}

	*_pfname = fname;
	*_pCustomMsg = custommsg;
	*_pmin = min;
	*_pmax = max;
	return 0;
}


static AlifIntT _parser_init(void* _arg) { // 1944
	AlifArgParser* parser = (AlifArgParser*)_arg;
	const char* const* keywords = parser->keywords;

	AlifIntT len{}, pos{};
	if (scan_keywords(keywords, &len, &pos) < 0) {
		return -1;
	}

	const char* fname, * custommsg = nullptr;
	AlifIntT min = 0, max = 0;
	if (parser->format) {
		if (parse_format(parser->format, len, pos,
			&fname, &custommsg, &min, &max) < 0) {
			return -1;
		}
	}
	else {
		fname = parser->fname;
	}

	AlifIntT owned{};
	AlifObject* kwtuple = parser->kwTuple;
	if (kwtuple == nullptr) {
		AlifThread* saveThread = nullptr;
		AlifThread* tempThread = nullptr;
		//if (!alif_isMainInterpreter(alifInterpreter_get())) {
		//	tempThread = alifThreadState_new(alifInterpreter_main());
		//	if (tempThread == nullptr) {
		//		return -1;
		//	}
		//	saveThread = alifThreadState_swap(tempThread);
		//}
		//kwtuple = new_kwTuple(keywords, len, pos);
		//if (tempThread != nullptr) {
		//	alifThreadState_clear(tempThread);
		//	(void)alifThreadState_swap(saveThread);
		//	alifThreadState_delete(tempThread);
		//}
		if (kwtuple == nullptr) {
			return -1;
		}
		owned = 1;
	}
	else {
		owned = 0;
	}

	parser->pos = pos;
	parser->fname = fname;
	parser->customMsg = custommsg;
	parser->min = min;
	parser->max = max;
	parser->kwTuple = kwtuple;
	parser->isKwTupleOwned = owned;

	parser->next = (AlifArgParser*)alifAtomic_loadPtr(&_alifDureRun_.getArgs.staticParsers); // alif
	do {
		// compare-exchange updates parser->next on failure
	} while (!alifAtomic_compareExchangePtr(&_alifDureRun_.getArgs.staticParsers,
		&parser->next, parser));
	return 0;
}


static AlifIntT parser_init(AlifArgParser* _parser) { // 2021
	return _alifOnceFlag_callOnce(&_parser->once, &_parser_init, _parser);
}


static AlifObject* find_keyword(AlifObject* kwnames, AlifObject* const* kwstack, AlifObject* key) { // 2048
	AlifSizeT i{}, nkwargs{};

	nkwargs = ALIFTUPLE_GET_SIZE(kwnames);
	for (i = 0; i < nkwargs; i++) {
		AlifObject* kwname = ALIFTUPLE_GET_ITEM(kwnames, i);
		if (kwname == key) {
			return ALIF_NEWREF(kwstack[i]);
		}
	}

	for (i = 0; i < nkwargs; i++) {
		AlifObject* kwname = ALIFTUPLE_GET_ITEM(kwnames, i);
		if (alifUStr_eq(kwname, key)) {
			return ALIF_NEWREF(kwstack[i]);
		}
	}
	return nullptr;
}







AlifObject* const* _alifArg_unpackKeywordsWithVarArg(AlifObject* const* _args, AlifSizeT _nargs,
	AlifObject* _kwargs, AlifObject* _kwnames, AlifArgParser* _parser,
	AlifIntT _minpos, AlifIntT _maxpos, AlifIntT _minkw, AlifIntT _vararg, AlifObject** _buf) { // 2489
	AlifObject* kwtuple{};
	AlifObject* keyword{};
	AlifSizeT varargssize = 0;
	AlifIntT i{}, posonly{}, minposonly{}, maxargs{};
	AlifIntT reqlimit = _minkw ? _maxpos + _minkw : _minpos;
	AlifSizeT nkwargs{};
	AlifObject* const* kwstack = nullptr;

	if (_parser == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	if (_kwnames != nullptr and !ALIFTUPLE_CHECK(_kwnames)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}

	if (_args == nullptr && _nargs == 0) {
		_args = _buf;
	}

	if (parser_init(_parser) < 0) {
		return nullptr;
	}

	kwtuple = _parser->kwTuple;
	posonly = _parser->pos;
	minposonly = ALIF_MIN(posonly, _minpos);
	maxargs = posonly + (AlifIntT)ALIFTUPLE_GET_SIZE(kwtuple);
	if (_kwargs != nullptr) {
		nkwargs = ALIFDICT_GET_SIZE(_kwargs);
	}
	else if (_kwnames != nullptr) {
		nkwargs = ALIFTUPLE_GET_SIZE(_kwnames);
		kwstack = _args + _nargs;
	}
	else {
		nkwargs = 0;
	}
	if (_nargs < minposonly) {
		//alifErr_format(_alifExcTypeError_,
		//	"%.200s%s takes %s %d positional argument%s"
		//	" (%zd given)",
		//	(parser->fname == nullptr) ? "function" : parser->fname,
		//	(parser->fname == nullptr) ? "" : "()",
		//	minposonly < maxpos ? "at least" : "exactly",
		//	minposonly,
		//	minposonly == 1 ? "" : "s",
		//	nargs);
		return nullptr;
	}

	/* create varargs tuple */
	varargssize = _nargs - _maxpos;
	if (varargssize < 0) {
		varargssize = 0;
	}
	_buf[_vararg] = alifTuple_new(varargssize);
	if (!_buf[_vararg]) {
		return nullptr;
	}

	/* copy tuple args */
	for (i = 0; i < _nargs; i++) {
		if (i >= _vararg) {
			ALIFTUPLE_SET_ITEM(_buf[_vararg], i - _vararg, ALIF_NEWREF(_args[i]));
			continue;
		}
		else {
			_buf[i] = _args[i];
		}
	}

	/* copy keyword args using kwtuple to drive process */
	for (i = ALIF_MAX((AlifIntT)_nargs, posonly) - ALIF_SAFE_DOWNCAST(varargssize, AlifSizeT, AlifIntT); i < maxargs; i++) {
		AlifObject* currentArg{};
		if (nkwargs) {
			keyword = ALIFTUPLE_GET_ITEM(kwtuple, i - posonly);
			if (_kwargs != nullptr) {
				if (alifDict_getItemRef(_kwargs, keyword, &currentArg) < 0) {
					goto exit;
				}
			}
			else {
				currentArg = find_keyword(_kwnames, kwstack, keyword);
			}
		}
		else {
			currentArg = nullptr;
		}

		if (i < _vararg) {
			_buf[i] = currentArg;
		}
		else {
			_buf[i + 1] = currentArg;
		}

		if (currentArg) {
			ALIF_DECREF(currentArg);
			--nkwargs;
		}
		else if (i < _minpos or (_maxpos <= i and i < reqlimit)) {
			/* Less arguments than required */
			keyword = ALIFTUPLE_GET_ITEM(kwtuple, i - posonly);
			//alifErr_format(_alifExcTypeError_, "%.200s%s missing required "
			//	"argument '%U' (pos %d)",
			//	(parser->fname == nullptr) ? "function" : parser->fname,
			//	(parser->fname == nullptr) ? "" : "()",
			//	keyword, i + 1);
			goto exit;
		}
	}

	if (nkwargs > 0) {
		//errorUnexpected_keywordArg(kwargs, kwnames, kwtuple, parser->fname);
		goto exit;
	}

	return _buf;

exit:
	ALIF_XDECREF(_buf[_vararg]);
	return nullptr;
}
