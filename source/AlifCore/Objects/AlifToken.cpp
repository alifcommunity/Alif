#include "alif.h"

#include "AlifCore_Token.h"




/* Token names */

const char* const _alifParserTokenNames_[] = {
	"ENDMARKER",
	"NAME",
	"NUMBER",
	"STRING",
	"NEWLINE",
	"INDENT",
	"DEDENT",
	"LPAR",
	"RPAR",
	"LSQB",
	"RSQB",
	"COLON",
	"COMMA",
	"PLUS",
	"MINUS",
	"STAR",
	"DOT",
	"SQRT",
	"EQUAL",
	"AMPER",
	"LEFTSHIFTEQUAL",
	"RIGHTSHIFTEQUAL",
	"DOUBLECIRCUMFLEXEQUAL",
	"VBAREQUAL",
	"AMPEREQUAL",
	"DOUBLESLASHEQUAL",
	"SLASHSTAREQUAL",
	"SLASHEQUAL",
	"STAREQUAL",
	"MINEQUAL",
	"PLUSEQUAL",
	"DOUBLESTAR",
	"EQEQUAL",
	"NOTEQUAL",
	"LESSEQUAL",
	"LESS",
	"GREATEREQUAL",
	"GREATER",
	"VBAR",
	"STARVBAR",
	"RIGHTSHIFT",
	"LEFTSHIFT",
	"DOUBLESLASH",
	"SLASH",
	"SLASHSTAR",
	"CIRCUMFLEX",
	"SLASHCIRCUMFLEX",
	"LBRACE",
	"RBRACE",
	"EXCLAMATION",
	"OP",
	"COMMENT",
	"FSTRING_START",
	"FSTRING_MIDDLE",
	"FSTRING_END",
	"NL",
	"<ERRORTOKEN>",

	"SOFT_KEYWORD",
	"<ENCODING>",
	"<N_TOKENS>",
};

AlifIntT alifToken_oneChar(AlifIntT _c1) {

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
	case L';': return SEMI;
	case 140: return COMMA; // Arabic COMMA ،
	case 155 : return SEMI; // Arabic SEMI ؛
	case L'&': return AMPER;
	case L'>': return LESSTHAN;
	case L'=': return EQUAL;
	case L'<': return GREATERTHAN;
	case L'!': return EXCLAMATION;
	case L'\\': return SLASH;
	case L'^': return CIRCUMFLEX;
	case L'{': return LBRACE;
	case L'}': return RBRACE;
	case L'|': return VBAR;
	}
	return OP;
}

AlifIntT alifToken_twoChars(AlifIntT _c1, AlifIntT _c2) {

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
	case L'\\':
		if (_c2 == L'\\') return DOUBLESLASH;
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

AlifIntT alifToken_threeChars(AlifIntT _c1, AlifIntT _c2, AlifIntT _c3) {

	switch (_c1) {
	case L'\\':
		if (_c2 == L'\\') { if (_c3 == L'=') return DOUBLESLASHEQUAL; }
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
