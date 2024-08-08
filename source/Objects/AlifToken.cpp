
#include "alif.h"
#include "AlifCore_AlifToken.h"




/* Token names */

const wchar_t* const alifParserTokenNames[] = {
	L"ENDMARKER",
	L"NAME",
	L"NUMBER",
	L"STRING",
	L"NEWLINE",
	L"INDENT",
	L"DEDENT",
	L"LPAR",
	L"RPAR",
	L"LSQB",
	L"RSQB",
	L"COLON",
	L"COMMA",
	L"PLUS",
	L"MINUS",
	L"STAR",
	L"DOT",
	L"SQRT",
	L"EQUAL",
	L"AMPER",
	L"LEFTSHIFTEQUAL",
	L"RIGHTSHIFTEQUAL",
	L"DOUBLECIRCUMFLEXEQUAL",
	L"VBAREQUAL",
	L"AMPEREQUAL",
	L"DOUBLESLASHEQUAL",
	L"SLASHSTAREQUAL",
	L"SLASHEQUAL",
	L"STAREQUAL",
	L"MINEQUAL",
	L"PLUSEQUAL",
	L"DOUBLESTAR",
	L"EQEQUAL",
	L"NOTEQUAL",
	L"LESSEQUAL",
	L"LESS",
	L"GREATEREQUAL",
	L"GREATER",
	L"VBAR",
	L"STARVBAR",
	L"RIGHTSHIFT",
	L"LEFTSHIFT",
	L"DOUBLESLASH",
	L"SLASH",
	L"SLASHSTAR",
	L"CIRCUMFLEX",
	L"SLASHCIRCUMFLEX",
	L"LBRACE",
	L"RBRACE",
	L"EXCLAMATION",
	L"OP",
	L"COMMENT",
	L"FSTRING_START",
	L"FSTRING_MIDDLE",
	L"FSTRING_END",
	L"NL",
	L"<ERRORTOKEN>",

	L"SOFT_KEYWORD",
	L"<ENCODING>",
	L"<N_TOKENS>",
};

int alifToken_oneChar(int _c1) {

	switch (_c1) {
	case L'(': return LPAR;
	case L')': return RPAR;
	case L'[': return LSQR;
	case L']': return RSQR;
	case L'*': return STAR;
	case L'+': return PLUS;
	case L'-': return MINUS;
	case L'.': return DOT;
	case L':': return COLON;
	case L',': return COMMA;
	case L'&': return AMPER;
	case L'<': return LESSTHAN;
	case L'=': return EQUAL;
	case L'>': return GREATERTHAN;
	case L'!': return EXCLAMATION;
	case L'/': return SLASH;
	case L'^': return CIRCUMFLEX;
	case L'{': return LBRACE;
	case L'}': return RBRACE;
	case L'|': return VBAR;
	}
	return OP;
}

int alifToken_twoChars(int _c1, int _c2) {

	switch (_c1) {
	case L'!':
		if (_c2 == L'=') return NOTEQUAL;
		break;
	case L'&':
		if (_c2 == L'=') return AMPEREQUAL;
		break;
	case L'*':
		if (_c2 == L'*') return DOUBLESTAR;
		else if (_c2 == L'=') return STAREQUAL;
		else if (_c2 == L'|') return STARVBAR;
		break;
	case L'+':
		if (_c2 == L'=') return PLUSEQUAL;
		break;
	case L'-':
		if (_c2 == L'=') return MINUSEQUAL;
		break;
	case L'/':
		if (_c2 == L'/') return DOUBLESLASH;
		else if (_c2 == L'=') return SLASHEQUAL;
		else if (_c2 == L'^') return SLASHCIRCUMFLEX;
		else if (_c2 == L'*') return SLASHSTAR;
		break;
	case L'<':
		if (_c2 == L'<') return LSHIFT;
		else if (_c2 == L'=') return LESSEQUAL;
		break;
	case L'=':
		if (_c2 == L'=') return EQUALEQUAL;
		break;
	case L'>':
		if (_c2 == L'=') return GREATEREQUAL;
		else if (_c2 == L'>') return RSHIFT;
		break;
	case L'^':
		if (_c2 == L'=') return CIRCUMFLEXEQUAL;
		break;
	case L'|':
		if (_c2 == L'=') return VBAREQUAL;
		break;
	}
	return OP;
}

int alifToken_threeChars(int _c1, int _c2, int _c3) {

	switch (_c1) {
	case L'/':
		if (_c2 == L'/') { if (_c3 == L'=') return DOUBLESLASHEQUAL; }
		else if (_c2 == L'*') { if (_c3 == L'=') return SLASHSTAREQUAL; }
		break;
	case L'<':
		if (_c2 == L'<') { if (_c3 == L'=') return LSHIFTEQUAL; }
		break;
	case L'>':
		if (_c2 == L'>') { if (_c3 == L'=') return RSHIFTEQUAL; }
		break;
	case L'^':
		if (_c2 == L'^') { if (_c3 == L'=') return DOUBLECIRCUMFLEXEQUAL; }
		break;
	}
	return OP;
}
