#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Dict.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Tuple.h"




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
		custommsg = NULL;
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
