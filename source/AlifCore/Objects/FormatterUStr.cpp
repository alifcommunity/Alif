#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_Long.h"
#include <locale.h>


static void unknown_presentationType(AlifUCS4 _presentationType,
	const char* _typeName) { // 13
	/* %c might be out-of-range, hence the two cases. */
	if (_presentationType > 32 and _presentationType < 128)
		alifErr_format(_alifExcValueError_,
			"رمز تنسيق غير معروف '%c' "
			"لكائن من النوع '%.200s'",
			(char)_presentationType,
			_typeName);
	else
		alifErr_format(_alifExcValueError_,
			"رمز تنسيق غير معروف '\\x%x' "
			"لكائن من النوع '%.200s'",
			(unsigned int)_presentationType,
			_typeName);
}



static AlifIntT get_integer(AlifObject* str, AlifSizeT* ppos, AlifSizeT end,
	AlifSizeT* result) { // 59
	AlifSizeT accumulator, digitval, pos = *ppos;
	AlifIntT numdigits{};
	AlifIntT kind = ALIFUSTR_KIND(str);
	const void* data = ALIFUSTR_DATA(str);

	accumulator = numdigits = 0;
	for (; pos < end; pos++, numdigits++) {
		digitval = ALIF_USTR_TODECIMAL(ALIFUSTR_READ(kind, data, pos));
		if (digitval < 0)
			break;
		if (accumulator > (ALIF_SIZET_MAX - digitval) / 10) {
			alifErr_format(_alifExcValueError_,
				"تم تمرير الكثير من الأرقام في النص المنسق");
			*ppos = pos;
			return -1;
		}
		accumulator = accumulator * 10 + digitval;
	}
	*ppos = pos;
	*result = accumulator;
	return numdigits;
}

ALIF_LOCAL_INLINE(AlifIntT) is_alignmentToken(AlifUCS4 _c) { // 97
	switch (_c) {
	case '<': case '>': case '=': case '^':
		return 1;
	default:
		return 0;
	}
}

/* returns true if this character is a sign element */
ALIF_LOCAL_INLINE(AlifIntT) is_signElement(AlifUCS4 _c) { // 109
	switch (_c) {
	case ' ': case '+': case '-':
		return 1;
	default:
		return 0;
	}
}

enum LocaleType { // 121
	LT_NO_LOCALE = 0,
	LT_DEFAULT_LOCALE = ',',
	LT_UNDERSCORE_LOCALE = '_',
	LT_UNDER_FOUR_LOCALE,
	LT_CURRENT_LOCALE
};

class InternalFormatSpec { // 129
public:
	AlifUCS4 fillChar{};
	AlifUCS4 align{};
	AlifIntT alternate{};
	AlifIntT noNeg0{};
	AlifUCS4 sign{};
	AlifSizeT width{};
	LocaleType thousandsSeparators{};
	AlifSizeT precision{};
	AlifUCS4 type{};
};


static AlifIntT parseInternal_renderFormatSpec(AlifObject* _obj,
	AlifObject* _formatSpec, AlifSizeT _start, AlifSizeT _end,
	InternalFormatSpec* _format,
	wchar_t _defaultType /* //* alif */,
	char _defaultAlign) { // 148
	AlifSizeT pos = _start;
	AlifIntT kind = ALIFUSTR_KIND(_formatSpec);
	const void* data = ALIFUSTR_DATA(_formatSpec);
#define READ_spec(index) ALIFUSTR_READ(kind, data, index)

	AlifSizeT consumed{};
	AlifIntT align_specified = 0;
	AlifIntT fill_char_specified = 0;

	_format->fillChar = ' ';
	_format->align = _defaultAlign;
	_format->alternate = 0;
	_format->noNeg0 = 0;
	_format->sign = '\0';
	_format->width = -1;
	_format->thousandsSeparators = LocaleType::LT_NO_LOCALE;
	_format->precision = -1;
	_format->type = _defaultType;

	if (_end - pos >= 2 and is_alignmentToken(READ_spec(pos + 1))) {
		_format->align = READ_spec(pos + 1);
		_format->fillChar = READ_spec(pos);
		fill_char_specified = 1;
		align_specified = 1;
		pos += 2;
	}
	else if (_end - pos >= 1 and is_alignmentToken(READ_spec(pos))) {
		_format->align = READ_spec(pos);
		align_specified = 1;
		++pos;
	}

	if (_end - pos >= 1 and is_signElement(READ_spec(pos))) {
		_format->sign = READ_spec(pos);
		++pos;
	}

	if (_end - pos >= 1 and READ_spec(pos) == 'z') {
		_format->noNeg0 = 1;
		++pos;
	}

	if (_end - pos >= 1 and READ_spec(pos) == '#') {
		_format->alternate = 1;
		++pos;
	}

	if (!fill_char_specified and _end - pos >= 1 and READ_spec(pos) == '0') {
		_format->fillChar = '0';
		if (!align_specified and _defaultAlign == '>') {
			_format->align = '=';
		}
		++pos;
	}

	consumed = get_integer(_formatSpec, &pos, _end, &_format->width);
	if (consumed == -1)
		/* Overflow error. Exception already set. */
		return 0;

	if (consumed == 0)
		_format->width = -1;

	if (_end - pos and READ_spec(pos) == ',') {
		_format->thousandsSeparators = LT_DEFAULT_LOCALE;
		++pos;
	}
	if (_end - pos and READ_spec(pos) == '_') {
		if (_format->thousandsSeparators != LT_NO_LOCALE) {
			//invalid_commaAndUnderscore();
			return 0;
		}
		_format->thousandsSeparators = LT_UNDERSCORE_LOCALE;
		++pos;
	}
	if (_end - pos and READ_spec(pos) == ',') {
		if (_format->thousandsSeparators == LT_UNDERSCORE_LOCALE) {
			//invalid_commaAndUnderscore();
			return 0;
		}
	}

	/* Parse field precision */
	if (_end - pos and READ_spec(pos) == '.') {
		++pos;

		consumed = get_integer(_formatSpec, &pos, _end, &_format->precision);
		if (consumed == -1)
			/* Overflow error. Exception already set. */
			return 0;

		if (consumed == 0) {
			alifErr_format(_alifExcValueError_,
				"قيمة معرف التنسيق مفقودة");
			return 0;
		}

	}

	/* Finally, parse the type field. */

	if (_end - pos > 1) {
		AlifObject* actual_format_spec = alifUStr_fromKindAndData(kind,
			(char*)data + kind * _start,
			_end - _start);
		if (actual_format_spec != nullptr) {
			//alifErr_format(_alifExcValueError_,
			//	"Invalid format specifier '%U' for object of type '%.200s'",
			//	actual_format_spec, ALIF_TYPE(_obj)->name);
			ALIF_DECREF(actual_format_spec);
		}
		return 0;
	}

	if (_end - pos == 1) {
		_format->type = READ_spec(pos);
		++pos;
	}


	if (_format->thousandsSeparators) {
		switch (_format->type) { //* alif
		case L'ف':
		case L'ه':
		case L'ط':
		case L'ع':
		//case 'd':
		//case 'e':
		//case 'f':
		//case 'g':
		//case 'E':
		//case 'G':
		case '%':
		//case 'F':
		case '\0':
			/* These are allowed.*/
			break;
		case L'ث':
		case L'م':
		case L'س':
		//case 'b':
		//case 'o':
		//case 'x':
		//case 'X':
			if (_format->thousandsSeparators == LocaleType::LT_UNDERSCORE_LOCALE) {
				_format->thousandsSeparators = LocaleType::LT_UNDER_FOUR_LOCALE;
				break;
			}
			ALIF_FALLTHROUGH;
		default:
			//invalid_thousandsSeparatorType(_format->thousandsSeparators, _format->type);
			return 0;
		}
	}

	return 1;
}




/* Calculate the padding needed. */
static void calc_padding(AlifSizeT _nchars, AlifSizeT _width, AlifUCS4 _align,
	AlifSizeT *_nLPadding, AlifSizeT* _nRPadding, AlifSizeT* _nTotal) { // 336
	if (_width >= 0) {
		if (_nchars > _width)
			*_nTotal = _nchars;
		else
			*_nTotal = _width;
	}
	else {
		/* not specified, use all of the chars and no more */
		*_nTotal = _nchars;
	}

	/* Figure out how much leading space we need, based on the
	aligning */
	if (_align == '>')
		*_nLPadding = *_nTotal - _nchars;
	else if (_align == '^')
		*_nLPadding = (*_nTotal - _nchars) / 2;
	else if (_align == '<' || _align == '=')
		*_nLPadding = 0;
	else {
		/* We should never have an unspecified alignment. */
		ALIF_UNREACHABLE();
	}

	*_nRPadding = *_nTotal - _nchars - *_nLPadding;
}

/* Do the padding, and return a pointer to where the caller-supplied
content goes. */
static AlifIntT fill_padding(AlifUStrWriter* _writer,
	AlifSizeT _nchars, AlifUCS4 _fillChar,
	AlifSizeT _nLPadding, AlifSizeT _nRPadding) { // 368
	AlifSizeT pos{};

	/* Pad on left. */
	if (_nLPadding) {
		pos = _writer->pos;
		_alifUStr_fastFill(_writer->buffer, pos, _nLPadding, _fillChar);
	}

	/* Pad on right. */
	if (_nRPadding) {
		pos = _writer->pos + _nchars + _nLPadding;
		_alifUStr_fastFill(_writer->buffer, pos, _nRPadding, _fillChar);
	}

	/* Pointer to the user content. */
	_writer->pos += _nLPadding;
	return 0;
}




/************************************************************************/
/*********** common routines for numeric formatting *********************/
/************************************************************************/

/* Locale info needed for formatting integers and the part of floats
before and including the decimal. Note that locales only support
8-bit chars, not unicode. */
class LocaleInfo { // 402
public:
	AlifObject* decimalPoint{};
	AlifObject* thousandsSep{};
	const char* grouping{};
	char* groupingBuffer{};
};

#define LOCALEINFO_STATIC_INIT {0, 0, 0, 0}

class NumberFieldWidths { // 413
public:
	AlifSizeT nLPadding{};
	AlifSizeT nPrefix{};
	AlifSizeT nSPadding{};
	AlifSizeT nRPadding{};
	char sign{};
	AlifSizeT nSign{};      /* number of digits needed for sign (0/1) */
	AlifSizeT nGroupedDigits{}; /* Space taken up by the digits, including
	any grouping chars. */
	AlifSizeT nDecimal{};   /* 0 if only an integer */
	AlifSizeT nRemainder{}; /* Digits in decimal and/or exponent part,
	excluding the decimal itself, if
	present. */

	/* These 2 are not the widths of fields, but are needed by
	STRINGLIB_GROUPING. */
	AlifSizeT nDigits{};    /* The number of digits before a decimal
	or exponent. */
	AlifSizeT nMinWidth{}; /* The min_width we used when we computed
	the n_grouped_digits width. */
};

static void parse_number(AlifObject* _s, AlifSizeT _pos, AlifSizeT _end,
	AlifSizeT *_nRemainder, AlifIntT* _hasDecimal) { // 446
	AlifSizeT remainder{};
	AlifIntT kind = ALIFUSTR_KIND(_s);
	const void *data = ALIFUSTR_DATA(_s);

	while (_pos<_end and ALIF_ISDIGIT(ALIFUSTR_READ(kind, data, _pos)))
		++_pos;
	remainder = _pos;

	/* Does remainder start with a decimal point? */
	*_hasDecimal = _pos<_end and ALIFUSTR_READ(kind, data, remainder) == '.';

	/* Skip the decimal point. */
	if (*_hasDecimal)
		remainder++;

	*_nRemainder = _end - remainder;
}

static AlifSizeT calc_numberWidths(NumberFieldWidths* _spec, AlifSizeT _nPrefix,
	AlifUCS4 _signChar, AlifSizeT _nStart,
	AlifSizeT _nEnd, AlifSizeT _nRemainder,
	AlifIntT _hasDecimal, const LocaleInfo* _locale,
	const InternalFormatSpec* _format, AlifUCS4* _maxChar) { // 473
	AlifSizeT nNonDigitNonPadding{};
	AlifSizeT nPadding{};

	_spec->nDigits = _nEnd - _nStart - _nRemainder - (_hasDecimal?1:0);
	_spec->nLPadding = 0;
	_spec->nPrefix = _nPrefix;
	_spec->nDecimal = _hasDecimal ? ALIFUSTR_GET_LENGTH(_locale->decimalPoint) : 0;
	_spec->nRemainder = _nRemainder;
	_spec->nSPadding = 0;
	_spec->nRPadding = 0;
	_spec->sign = '\0';
	_spec->nSign = 0;

	/* compute the various parts we're going to write */
	switch (_format->sign) {
	case '+':
		/* always put a + or - */
		_spec->nSign = 1;
		_spec->sign = (_signChar == '-' ? '-' : '+');
		break;
	case ' ':
		_spec->nSign = 1;
		_spec->sign = (_signChar == '-' ? '-' : ' ');
		break;
	default:
		/* Not specified, or the default (-) */
		if (_signChar == '-') {
			_spec->nSign = 1;
			_spec->sign = '-';
		}
	}

	/* The number of chars used for non-digits and non-padding. */
	nNonDigitNonPadding = _spec->nSign + _spec->nPrefix + _spec->nDecimal +
		_spec->nRemainder;

	/* min_width can go negative, that's okay. format->width == -1 means
	we don't care. */
	if (_format->fillChar == '0' and _format->align == '=')
		_spec->nMinWidth = _format->width - nNonDigitNonPadding;
	else
		_spec->nMinWidth = 0;

	if (_spec->nDigits == 0)
		_spec->nGroupedDigits = 0;
	else {
		AlifUCS4 groupingMaxchar{};
		_spec->nGroupedDigits = _alifUStr_insertThousandsGrouping(
			nullptr, 0,
			nullptr, 0, _spec->nDigits,
			_spec->nMinWidth,
			_locale->grouping, _locale->thousandsSep, &groupingMaxchar);
		if (_spec->nGroupedDigits == -1) {
			return -1;
		}
		*_maxChar = ALIF_MAX(*_maxChar, groupingMaxchar);
	}

	nPadding = _format->width -
		(nNonDigitNonPadding + _spec->nGroupedDigits);
	if (nPadding > 0) {
		/* Some padding is needed. Determine if it's left, space, or right. */
		switch (_format->align) {
		case '<':
			_spec->nRPadding = nPadding;
			break;
		case '^':
			_spec->nLPadding = nPadding / 2;
			_spec->nRPadding = nPadding - _spec->nLPadding;
			break;
		case '=':
			_spec->nSPadding = nPadding;
			break;
		case '>':
			_spec->nLPadding = nPadding;
			break;
		default:
			/* Shouldn't get here */
			ALIF_UNREACHABLE();
		}
	}

	if (_spec->nLPadding or _spec->nSPadding or _spec->nRPadding)
		*_maxChar = ALIF_MAX(*_maxChar, _format->fillChar);

	if (_spec->nDecimal)
		*_maxChar = ALIF_MAX(*_maxChar, ALIFUSTR_MAX_CHAR_VALUE(_locale->decimalPoint));

	return _spec->nLPadding + _spec->nSign + _spec->nPrefix +
		_spec->nSPadding + _spec->nGroupedDigits + _spec->nDecimal +
		_spec->nRemainder + _spec->nRPadding;
}


static AlifIntT fill_number(AlifUStrWriter* _writer, const NumberFieldWidths* _spec,
	AlifObject* _digits, AlifSizeT _dStart,
	AlifObject* prefix, AlifSizeT _pStart,
	AlifUCS4 _fillChar, LocaleInfo* _locale, AlifIntT _toupper) { // 602
	/* Used to keep track of digits, decimal, and remainder. */
	AlifSizeT dPos = _dStart;
	const int kind = _writer->kind;
	const void *data = _writer->data;
	AlifSizeT r{};

	if (_spec->nLPadding) {
		_alifUStr_fastFill(_writer->buffer,
			_writer->pos, _spec->nLPadding, _fillChar);
		_writer->pos += _spec->nLPadding;
	}
	if (_spec->nSign == 1) {
		ALIFUSTR_WRITE(kind, data, _writer->pos, _spec->sign);
		_writer->pos++;
	}
	if (_spec->nPrefix) {
		_alifUStr_fastCopyCharacters(_writer->buffer, _writer->pos,
			prefix, _pStart,
			_spec->nPrefix);
		if (_toupper) {
			AlifSizeT t;
			for (t = 0; t < _spec->nPrefix; t++) {
				AlifUCS4 c = ALIFUSTR_READ(kind, data, _writer->pos + t);
				c = ALIF_TOUPPER(c);
				ALIFUSTR_WRITE(kind, data, _writer->pos + t, c);
			}
		}
		_writer->pos += _spec->nPrefix;
	}
	if (_spec->nSPadding) {
		_alifUStr_fastFill(_writer->buffer,
			_writer->pos, _spec->nSPadding, _fillChar);
		_writer->pos += _spec->nSPadding;
	}

	if (_spec->nDigits != 0) {
		/* Fill the digits with InsertThousandsGrouping. */
		r = _alifUStr_insertThousandsGrouping(
			_writer, _spec->nGroupedDigits,
			_digits, dPos, _spec->nDigits,
			_spec->nMinWidth,
			_locale->grouping, _locale->thousandsSep, nullptr);
		if (r == -1)
			return -1;
		dPos += _spec->nDigits;
	}
	if (_toupper) {
		AlifSizeT t{};
		for (t = 0; t < _spec->nGroupedDigits; t++) {
			AlifUCS4 c = ALIFUSTR_READ(kind, data, _writer->pos + t);
			c = ALIF_TOUPPER(c);
			if (c > 127) {
				alifErr_setString(_alifExcSystemError_, "non-ascii grouped digit");
				return -1;
			}
			ALIFUSTR_WRITE(kind, data, _writer->pos + t, c);
		}
	}
	_writer->pos += _spec->nGroupedDigits;

	if (_spec->nDecimal) {
		_alifUStr_fastCopyCharacters(
			_writer->buffer, _writer->pos,
			_locale->decimalPoint, 0, _spec->nDecimal);
		_writer->pos += _spec->nDecimal;
		dPos += 1;
	}

	if (_spec->nRemainder) {
		_alifUStr_fastCopyCharacters(
			_writer->buffer, _writer->pos,
			_digits, dPos, _spec->nRemainder);
		_writer->pos += _spec->nRemainder;
		/* d_pos += spec->n_remainder; */
	}

	if (_spec->nRPadding) {
		_alifUStr_fastFill(_writer->buffer,
			_writer->pos, _spec->nRPadding,
			_fillChar);
		_writer->pos += _spec->nRPadding;
	}
	return 0;
}

static const char _noGrouping_[1] = {CHAR_MAX};

static AlifIntT get_localeInfo(LocaleType _type, LocaleInfo* _localeInfo) { // 703
	switch (_type) {
	case LT_CURRENT_LOCALE: {
		struct lconv *lc = localeconv();
		if (_alif_getLocaleConvNumeric(lc,
			&_localeInfo->decimalPoint,
			&_localeInfo->thousandsSep) < 0) {
			return -1;
		}

		_localeInfo->groupingBuffer = alifMem_strDup(lc->grouping);
		if (_localeInfo->groupingBuffer == nullptr) {
			//alifErr_noMemory();
			return -1;
		}
		_localeInfo->grouping = _localeInfo->groupingBuffer;
		break;
	}
	case LT_DEFAULT_LOCALE:
	case LT_UNDERSCORE_LOCALE:
	case LT_UNDER_FOUR_LOCALE:
		_localeInfo->decimalPoint = alifUStr_fromOrdinal('.');
		_localeInfo->thousandsSep = alifUStr_fromOrdinal(
			_type == LT_DEFAULT_LOCALE ? ',' : '_');
		if (!_localeInfo->decimalPoint or !_localeInfo->thousandsSep)
			return -1;
		if (_type != LT_UNDER_FOUR_LOCALE)
			_localeInfo->grouping = "\3"; /* Group every 3 characters.  The
		(implicit) trailing 0 means repeat
		infinitely. */
		else
			_localeInfo->grouping = "\4"; /* Bin/oct/hex group every four. */
		break;
	case LT_NO_LOCALE:
		_localeInfo->decimalPoint = alifUStr_fromOrdinal('.');
		_localeInfo->thousandsSep = alif_getConstant(ALIF_CONSTANT_EMPTY_STR);
		if (!_localeInfo->decimalPoint or !_localeInfo->thousandsSep)
			return -1;
		_localeInfo->grouping = _noGrouping_;
		break;
	}
	return 0;
}

static void free_localeInfo(LocaleInfo* _localeInfo) { // 752
	ALIF_XDECREF(_localeInfo->decimalPoint);
	ALIF_XDECREF(_localeInfo->thousandsSep);
	alifMem_dataFree(_localeInfo->groupingBuffer);
}




/************************************************************************/
/*********** string formatting ******************************************/
/************************************************************************/

static AlifIntT format_stringInternal(AlifObject* _value,
	const InternalFormatSpec* _format,
	AlifUStrWriter* _writer) { // 764
	AlifSizeT lpad{};
	AlifSizeT rpad{};
	AlifSizeT total{};
	AlifSizeT len{};
	AlifIntT result = -1;
	AlifUCS4 maxchar{};

	len = ALIFUSTR_GET_LENGTH(_value);

	/* sign is not allowed on strings */
	if (_format->sign != '\0') {
		if (_format->sign == ' ') {
			alifErr_setString(_alifExcValueError_,
				"المسافة غير مسموح بها في محدد التنسيق النصي");
		}
		else {
			alifErr_setString(_alifExcValueError_,
				"الإشارة غير مسموح بها في محدد التنسيق النصي");
		}
		goto done;
	}

	/* negative 0 coercion is not allowed on strings */
	if (_format->noNeg0) {
		alifErr_setString(_alifExcValueError_,
			"الصفر السالب القسري (ز) غير مسموح به في محدد التنسيق النصي");
		goto done;
	}

	/* alternate is not allowed on strings */
	if (_format->alternate) {
		alifErr_setString(_alifExcValueError_,
			"الشكل البديل (#) غير مسموح به في محدد التنسيق النصي");
		goto done;
	}

	/* '=' alignment not allowed on strings */
	if (_format->align == '=') {
		alifErr_setString(_alifExcValueError_,
			"'=' المحاذاة باستخدام "
			"غير مسموح بها في محدد التنسيق النصي");
		goto done;
	}

	if ((_format->width == -1 or _format->width <= len)
		&& (_format->precision == -1 or _format->precision >= len)) {
		/* Fast path */
		return _alifUStrWriter_writeStr(_writer, _value);
	}

	/* if precision is specified, output no more that format.precision
	characters */
	if (_format->precision >= 0 && len >= _format->precision) {
		len = _format->precision;
	}

	calc_padding(len, _format->width, _format->align, &lpad, &rpad, &total);

	maxchar = _writer->maxChar;
	if (lpad != 0 or rpad != 0)
		maxchar = ALIF_MAX(maxchar, _format->fillChar);
	if (ALIFUSTR_MAX_CHAR_VALUE(_value) > maxchar) {
		AlifUCS4 valmaxchar = _alifUStr_findMaxChar(_value, 0, len);
		maxchar = ALIF_MAX(maxchar, valmaxchar);
	}

	/* allocate the resulting string */
	if (ALIFUSTRWRITER_PREPARE(_writer, total, maxchar) == -1)
		goto done;

	/* Write into that space. First the padding. */
	result = fill_padding(_writer, len, _format->fillChar, lpad, rpad);
	if (result == -1)
		goto done;

	/* Then the source string. */
	if (len) {
		_alifUStr_fastCopyCharacters(_writer->buffer, _writer->pos,
			_value, 0, len);
	}
	_writer->pos += (len + rpad);
	result = 0;

done:
	return result;
}


/************************************************************************/
/*********** long formatting ********************************************/
/************************************************************************/

static AlifIntT format_longInternal(AlifObject* _value, const InternalFormatSpec* _format,
	AlifUStrWriter* _writer) { // 863
	int result = -1;
	AlifUCS4 maxchar = 127;
	AlifObject *tmp = nullptr;
	AlifSizeT inumericChars{};
	AlifUCS4 signChar = '\0';
	AlifSizeT nDigits{};       /* count of digits need from the computed
	string */
	AlifSizeT nRemainder = 0; /* Used only for 'c' formatting, which
	produces non-digits */
	AlifSizeT nPrefix = 0;   /* Count of prefix chars, (e.g., '0x') */
	AlifSizeT nTotal{};
	AlifSizeT prefix = 0;
	NumberFieldWidths spec{};
	long x{};

	/* Locale settings, either from the actual locale or
	from a hard-code pseudo-locale */
	LocaleInfo locale = LOCALEINFO_STATIC_INIT;

	/* no precision allowed on integers */
	if (_format->precision != -1) {
		alifErr_setString(_alifExcValueError_,
			"الفاصلة غير مسموح بها في تركيب تنسيق العدد الصحيح");
		goto done;
	}
	/* no negative zero coercion on integers */
	if (_format->noNeg0) {
		alifErr_setString(_alifExcValueError_,
			"لا يُسمح بالتحويل القسري للصفر السالب (ص) في تركيب تنسيق الأعداد الصحيحة");
		goto done;
	}

	/* special case for character formatting */
	if (_format->type == 'c') {
		/* error to specify a sign */
		if (_format->sign != '\0') {
			alifErr_setString(_alifExcValueError_,
				"لا يسمح للإشارة مع العدد الصحيح"
				" للمركب التنسيقي 'ر'");
			goto done;
		}
		/* error to request alternate format */
		if (_format->alternate) {
				alifErr_setString(_alifExcValueError_,
				"البديل من (#) غير مسموح مع العدد الصحيح"
				" للمركب التنسيقي 'ر'");
			goto done;
		}

		/* taken from uStrobject.cpp formatchar() */
		/* Integer input truncated to a character */
		x = alifLong_asLong(_value);
		if (x == -1 and alifErr_occurred())
			goto done;
		if (x < 0 or x > 0x10ffff) {
			alifErr_setString(_alifExcOverflowError_,
				"المعامل %c ليس في مدى(0x110000)");
			goto done;
		}
		tmp = alifUStr_fromOrdinal(x);
		inumericChars = 0;
		nDigits = 1;
		maxchar = ALIF_MAX(maxchar, (AlifUCS4)x);

		nRemainder = 1;
	}
	else {
		AlifIntT base{};
		AlifIntT leading_chars_to_skip = 0; 

		/* Compute the base and how many characters will be added by
		alifNumber_toBase */
		switch (_format->type) { // DONT TOUCH!
		case 'b':
			base = 2;
			leading_chars_to_skip = 2; /* 0b */
			break;
		case 'o':
			base = 8;
			leading_chars_to_skip = 2; /* 0o */
			break;
		case 'x':
		case 'X':
			base = 16;
			leading_chars_to_skip = 2; /* 0x */
			break;
		default:  /* shouldn't be needed, but stops a compiler warning */
		case 'd':
		case 'n':
			base = 10;
			break;
		}

		if (_format->sign != '+' and _format->sign != ' '
			and _format->width == -1
			and _format->type != 'X' and _format->type != 'n'
			and !_format->thousandsSeparators
			and ALIFLONG_CHECKEXACT(_value))
		{
			/* Fast path */
			return _alifLong_formatWriter(_writer, _value, base, _format->alternate);
		}

		/* The number of prefix chars is the same as the leading
		chars to skip */
		if (_format->alternate)
			nPrefix = leading_chars_to_skip;

		/* Do the hard part, converting to a string in a given base */
		tmp = _alifLong_format(_value, base,
			0/* هذا المعامل يحدد طباعة النظام العددي بالأحرف العربية أم اللاتينية */); //* alif //* todo
		if (tmp == nullptr)
			goto done;

		inumericChars = 0;
		nDigits = ALIFUSTR_GET_LENGTH(tmp);

		prefix = inumericChars;

		/* Is a sign character present in the output?  If so, remember it
		and skip it */
		if (ALIFUSTR_READ_CHAR(tmp, inumericChars) == '-') {
			signChar = '-';
			++prefix;
			++leading_chars_to_skip;
		}

		/* Skip over the leading chars (0x, 0b, etc.) */
		nDigits -= leading_chars_to_skip;
		inumericChars += leading_chars_to_skip;
	}

	/* Determine the grouping, separator, and decimal point, if any. */
	if (get_localeInfo(_format->type == 'n' ? LT_CURRENT_LOCALE :
		_format->thousandsSeparators,
		&locale) == -1)
		goto done;

	/* Calculate how much memory we'll need. */
	nTotal = calc_numberWidths(&spec, nPrefix, signChar, inumericChars,
		inumericChars + nDigits, nRemainder, 0,
		&locale, _format, &maxchar);
	if (nTotal == -1) {
		goto done;
	}

	/* Allocate the memory. */
	if (ALIFUSTRWRITER_PREPARE(_writer, nTotal, maxchar) == -1)
		goto done;

	/* Populate the memory. */
	result = fill_number(_writer, &spec,
		tmp, inumericChars,
		tmp, prefix, _format->fillChar,
		&locale, _format->type == 'X');

done:
	ALIF_XDECREF(tmp);
	free_localeInfo(&locale);
	return result;
}


/************************************************************************/
/*********** float formatting *******************************************/
/************************************************************************/

/* much of this is taken from uStrobject.cpp */
static AlifIntT format_floatInternal(AlifObject* _value,
	const InternalFormatSpec* _format,
	AlifUStrWriter* writer) { // 1041
	char *buf = nullptr;       /* buffer returned from alifOS_doubleToString */
	AlifSizeT nDigits{};
	AlifSizeT nRemainder{};
	AlifSizeT nTotal{};
	AlifIntT hasDecimal{};
	double val{};
	AlifIntT precision{}, defaultPrecision = 6;
	AlifUCS4 type = _format->type;
	AlifIntT addPCT = 0;
	AlifSizeT index{};
	NumberFieldWidths spec{};
	AlifIntT flags = 0;
	AlifIntT result = -1;
	AlifUCS4 maxchar = 127;
	AlifUCS4 signChar = '\0';
	AlifIntT float_type; /* Used to see if we have a nan, inf, or regular float. */
	AlifObject* uStrTmp = nullptr;

	/* Locale settings, either from the actual locale or
	from a hard-code pseudo-locale */
	LocaleInfo locale = LOCALEINFO_STATIC_INIT;

	if (_format->precision > INT_MAX) {
		alifErr_setString(_alifExcValueError_, "العدد كبير جداً");
		goto done;
	}
	precision = (AlifIntT)_format->precision;

	if (_format->alternate)
		flags |= ALIF_DTSF_ALT;
	if (_format->noNeg0)
		flags |= ALIF_DTSF_NO_NEG_0;

	if (type == '\0') {
		flags |= ALIF_DTSF_ADD_DOT_0;
		type = 'r';
		defaultPrecision = 0;
	}

	if (type == 'n')
		type = 'g';

	val = alifFloat_asDouble(_value);
	if (val == -1.0 and alifErr_occurred())
		goto done;

	if (type == '%') {
		type = 'f';
		val *= 100;
		addPCT = 1;
	}

	if (precision < 0)
		precision = defaultPrecision;
	else if (type == 'r')
		type = 'g';

	/* Cast "type", because if we're in unicode we need to pass an
	8-bit char. This is safe, because we've restricted what "type"
	can be. */
	buf = alifOS_doubleToString(val, (char)type, precision, flags,
		&float_type);
	if (buf == nullptr)
		goto done;
	nDigits = strlen(buf);

	if (addPCT) {
		buf[nDigits] = '%';
		nDigits += 1;
	}

	if (_format->sign != '+' and _format->sign != ' '
		and _format->width == -1
		and _format->type != 'n'
		and !_format->thousandsSeparators)
	{
		/* Fast path */
		result = alifUStrWriter_writeASCIIString(writer, buf, nDigits);
		alifMem_dataFree(buf);
		return result;
	}

	uStrTmp = _alifUStr_fromASCII(buf, nDigits);
	alifMem_dataFree(buf);
	if (uStrTmp == nullptr)
		goto done;

	/* Is a sign character present in the output?  If so, remember it
	and skip it */
	index = 0;
	if (ALIFUSTR_READ_CHAR(uStrTmp, index) == '-') {
		signChar = '-';
		++index;
		--nDigits;
	}

	parse_number(uStrTmp, index, index + nDigits, &nRemainder, &hasDecimal);

	/* Determine the grouping, separator, and decimal point, if any. */
	if (get_localeInfo(_format->type == 'n' ? LT_CURRENT_LOCALE :
		_format->thousandsSeparators,
		&locale) == -1)
		goto done;

	/* Calculate how much memory we'll need. */
	nTotal = calc_numberWidths(&spec, 0, signChar, index,
		index + nDigits, nRemainder, hasDecimal,
		&locale, _format, &maxchar);
	if (nTotal == -1) {
		goto done;
	}

	/* Allocate the memory. */
	if (ALIFUSTRWRITER_PREPARE(writer, nTotal, maxchar) == -1)
		goto done;

	/* Populate the memory. */
	result = fill_number(writer, &spec,
		uStrTmp, index,
		nullptr, 0, _format->fillChar,
		&locale, 0);

done:
	ALIF_XDECREF(uStrTmp);
	free_localeInfo(&locale);
	return result;
}



static AlifIntT format_obj(AlifObject* _obj, AlifUStrWriter* _writer) { // 1436
	AlifObject* str{};
	AlifIntT err{};

	str = alifObject_str(_obj);
	if (str == nullptr)
		return -1;
	err = _alifUStrWriter_writeStr(_writer, str);
	ALIF_DECREF(str);
	return err;
}

AlifIntT _alifUStr_formatAdvancedWriter(AlifUStrWriter* _writer,
	AlifObject* _obj, AlifObject* _formatSpec,
	AlifSizeT _start, AlifSizeT _end) {
	InternalFormatSpec format{};

	/* check for the special case of zero length format spec, make
	it equivalent to str(obj) */
	if (_start == _end) {
		if (ALIFUSTR_CHECKEXACT(_obj))
			return _alifUStrWriter_writeStr(_writer, _obj);
		else
			return format_obj(_obj, _writer);
	}

	/* parse the format_spec */
	if (!parseInternal_renderFormatSpec(_obj, _formatSpec, _start, _end,
		&format, 's', '<'))
		return -1;

	/* type conversion? */
	switch (format.type) {
	case 's':
		/* no type conversion needed, already a string.  do the formatting */
		return format_stringInternal(_obj, &format, _writer);
	default:
		/* unknown */
		unknown_presentationType(format.type, ALIF_TYPE(_obj)->name);
		return -1;
	}
}

AlifIntT _alifLong_formatAdvancedWriter(AlifUStrWriter* _writer,
	AlifObject* _obj, AlifObject* _formatSpec,
	AlifSizeT _start, AlifSizeT _end) { // 1486
	AlifObject* tmp = nullptr;
	InternalFormatSpec format;
	AlifIntT result = -1;

	/* check for the special case of zero length format spec, make
	it equivalent to str(obj) */
	if (_start == _end) {
		if (ALIFLONG_CHECKEXACT(_obj))
			return _alifLong_formatWriter(_writer, _obj, 10, 0);
		else
			return format_obj(_obj, _writer);
	}

	/* parse the format_spec */
	if (!parseInternal_renderFormatSpec(_obj, _formatSpec, _start, _end,
		&format, L'ع', '>'))
		goto done;

	//* alif
	switch (format.type) {
	case L'ث':
		format.type = 'b';
		break;
	case L'ز':
		format.type = 'c';
		break;
	case L'ع':
		format.type = 'd';
		break;
	case L'م':
		format.type = 'o';
		break;
	case L'س':
		format.type = 'x';
		break;
	case L'ر':
		format.type = 'n';
		break;

	// for floats
	case L'ه':
		format.type = 'e';
		break;
	case L'ف':
		format.type = 'f';
		break;
	case L'ط':
		format.type = 'g';
		break;

	default:
		/* unknown */
		unknown_presentationType(format.type, ALIF_TYPE(_obj)->name);
		return -1;
	}
	//* alif

	/* type conversion? */
	switch (format.type) {
	case 'b':
	case 'c':
	case 'd':
	case 'o':
	case 'x':
	//case 'X':
	case 'n':
		/* no type conversion needed, already an int.  do the formatting */
		result = format_longInternal(_obj, &format, _writer);
		break;

	case 'e':
	//case 'E':
	case 'f':
	//case 'F':
	case 'g':
	//case 'G':
	case '%':
		/* convert to float */
		tmp = alifNumber_float(_obj);
		if (tmp == nullptr)
			goto done;
		result = format_floatInternal(tmp, &format, _writer);
		break;

	default:
		/* unknown */
		unknown_presentationType(format.type, ALIF_TYPE(_obj)->name);
		goto done;
	}

done:
	ALIF_XDECREF(tmp);
	return result;
}


AlifIntT _alifFloat_formatAdvancedWriter(AlifUStrWriter* _writer,
	AlifObject* _obj, AlifObject* _formatSpec,
	AlifSizeT _start, AlifSizeT _end) { // 1548

	InternalFormatSpec format{};

	if (_start == _end)
		return format_obj(_obj, _writer);

	/* parse the format_spec */
	if (!parseInternal_renderFormatSpec(_obj, _formatSpec, _start, _end,
		&format, '\0', '>'))
		return -1;

	/* type conversion? */
	switch (format.type) {
	case '\0': /* No format code: like 'g', but with at least one decimal. */
	case L'ف': //* alif //* todo
		// بما أنه يقبل حروف ذات ترميز عريض فبعد التحقق هنا
		// تم إستبدال القيمة بالقيمة المفترضة وهي 'f'
		// وذلك أفضل من تعديل كامل النظام بعد هذه النقطة
		// حيث أنه لا يدعم الترميز العريض للنص
		format.type = 'f';
		break;
	case L'ه':
		format.type = 'e';
		break;
	case L'ط':
		format.type = 'g';
	//case 'e':
	//case 'E':
	//case 'f':
	//case 'F':
	//case 'g':
	//case 'G':
	//case 'n':
	case '%':
		/* no conversion, already a float.  do the formatting */
		//return format_floatInternal(_obj, &format, _writer);
		break;

	default:
		/* unknown */
		unknown_presentationType(format.type, ALIF_TYPE(_obj)->name);
		return -1;
	}

	/* no conversion, already a float.  do the formatting */
	return format_floatInternal(_obj, &format, _writer);
}
