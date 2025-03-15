#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_Long.h"




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
			//alifErr_format(_alifExcValueError_,
			//	"Too many decimal digits in format string");
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
	InternalFormatSpec* _format, char _defaultType, char _defaultAlign) { // 148
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
			//alifErr_format(_alifExcValueError_,
			//	"Format specifier missing precision");
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
		switch (_format->type) {
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'E':
		case 'G':
		case '%':
		case 'F':
		case '\0':
			/* These are allowed. See PEP 378.*/
			break;
		case 'b':
		case 'o':
		case 'x':
		case 'X':
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
	case 'e':
	case 'E':
	case 'f':
	case 'F':
	case 'g':
	case 'G':
	case 'n':
	case '%':
		/* no conversion, already a float.  do the formatting */
		//return format_floatInternal(_obj, &format, _writer);

	default:
		/* unknown */
		//unknown_presentation_type(format.type, ALIF_TYPE(_obj)->name);
		return -1;
	}
}
