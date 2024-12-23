
static void
STRINGLIB(repr)(AlifObject* unicode, AlifUCS4 quote,
	STRINGLIB_CHAR* odata)
{
	AlifSizeT isize = ALIFUSTR_GET_LENGTH(unicode);
	const void* idata = ALIFUSTR_DATA(unicode);
	AlifIntT ikind = ALIFUSTR_KIND(unicode);

	*odata++ = quote;
	for (AlifSizeT i = 0; i < isize; i++) {
		AlifUCS4 ch = ALIFUSTR_READ(ikind, idata, i);

		/* Escape quotes and backslashes */
		if ((ch == quote) or (ch == '\\')) {
			*odata++ = '\\';
			*odata++ = ch;
			continue;
		}

		/* Map special whitespace to '\t', \n', '\r' */
		if (ch == '\t') {
			*odata++ = '\\';
			*odata++ = 't';
		}
		else if (ch == '\n') {
			*odata++ = '\\';
			*odata++ = 'n';
		}
		else if (ch == '\r') {
			*odata++ = '\\';
			*odata++ = 'r';
		}

		/* Map non-printable US ASCII to '\xhh' */
		else if (ch < ' ' or ch == 0x7F) {
			*odata++ = '\\';
			*odata++ = 'x';
			*odata++ = _alifHexDigits_[(ch >> 4) & 0x000F];
			*odata++ = _alifHexDigits_[ch & 0x000F];
		}

		/* Copy ASCII characters as-is */
		else if (ch < 0x7F) {
			*odata++ = ch;
		}

		/* Non-ASCII characters */
		else {
			/* Map Unicode whitespace and control characters
			   (categories Z* and C* except ASCII space)
			*/
			if (!ALIF_USTR_ISPRINTABLE(ch)) {
				*odata++ = '\\';
				/* Map 8-bit characters to '\xhh' */
				if (ch <= 0xff) {
					*odata++ = 'x';
					*odata++ = _alifHexDigits_[(ch >> 4) & 0x000F];
					*odata++ = _alifHexDigits_[ch & 0x000F];
				}
				/* Map 16-bit characters to '\uxxxx' */
				else if (ch <= 0xffff) {
					*odata++ = 'u';
					*odata++ = _alifHexDigits_[(ch >> 12) & 0xF];
					*odata++ = _alifHexDigits_[(ch >> 8) & 0xF];
					*odata++ = _alifHexDigits_[(ch >> 4) & 0xF];
					*odata++ = _alifHexDigits_[ch & 0xF];
				}
				/* Map 21-bit characters to '\U00xxxxxx' */
				else {
					*odata++ = 'U';
					*odata++ = _alifHexDigits_[(ch >> 28) & 0xF];
					*odata++ = _alifHexDigits_[(ch >> 24) & 0xF];
					*odata++ = _alifHexDigits_[(ch >> 20) & 0xF];
					*odata++ = _alifHexDigits_[(ch >> 16) & 0xF];
					*odata++ = _alifHexDigits_[(ch >> 12) & 0xF];
					*odata++ = _alifHexDigits_[(ch >> 8) & 0xF];
					*odata++ = _alifHexDigits_[(ch >> 4) & 0xF];
					*odata++ = _alifHexDigits_[ch & 0xF];
				}
			}
			/* Copy characters as-is */
			else {
				*odata++ = ch;
			}
		}
	}
	*odata = quote;
}
