#pragma once


#define ENDMARKER				0
#define NAME					1
#define NUMBER					2
#define STRING					3
#define NEWLINE					4	// /n
#define INDENT					5
#define DEDENT					6
#define LPAR					7	// (
#define RPAR					8	// )
#define LSQR					9	// [
#define RSQR					10	// ]
#define COLON					11	// :
#define COMMA					12	// ,
#define PLUS					13	// +
#define MINUS					14	// -
#define STAR					15	// *
#define DOT						16	// .
#define EQUAL					17	// =
#define AMPER					18	// &
#define LSHIFTEQUAL				19	// <<=
#define RSHIFTEQUAL				20	// >>=
#define DOUBLECIRCUMFLEXEQUAL	21	// ^^=
#define VBAREQUAL				22	// |=
#define AMPEREQUAL				23	// &=
#define DOUBLESLASHEQUAL		24	// //=
#define SLASHSTAREQUAL			25	// /*=
#define CIRCUMFLEXEQUAL			26	// ^=
#define SLASHEQUAL				27	// /=
#define STAREQUAL				28	// *=
#define MINUSEQUAL				29	// -=
#define PLUSEQUAL				30	// +=
#define DOUBLESTAR				31	// **
#define EQUALEQUAL				32  // ==
#define NOTEQUAL				33  // !=
#define LESSEQUAL				34  // <=
#define LESSTHAN				35  // <
#define GREATEREQUAL			36  // >=
#define GREATERTHAN				37  // >
#define VBAR					38  // |
#define STARVBAR				39  // *|
#define RSHIFT					40  // >>
#define LSHIFT					41  // <<
#define DOUBLESLASH				42  // //
#define SLASH					43  // /
#define SLASHSTAR				44  // /*
#define CIRCUMFLEX				45  // ^
#define SLASHCIRCUMFLEX			46  // /^
#define LBRACE					47  // {
#define RBRACE					48  // }
#define EXCLAMATION				49  // !
#define OP						50
#define ELLIPSIS				51	// ...
#define COMMENT					52
#define FSTRINGSTART			53	
#define FSTRINGMIDDLE			54	
#define FSTRINGEND				55	
#define NL						56	
#define ERRORTOKEN				57	
#define NTOFFSET				256


#define ISTERMINAL(x)           ((x) < NTOFFSET)
#define ISNONTERMINAL(x)        ((x) >= NTOFFSET)
#define ISEOF(x)                ((x) == ENDMARKER)
#define ISWHITESPACE(x)         ((x) == ENDMARKER || \
                                 (x) == NEWLINE   || \
                                 (x) == INDENT    || \
                                 (x) == DEDENT)
#define ISSTRINGLIT(x)          ((x) == STRING		|| \
                                 (x) == FSTRINGMIDDLE)



extern const char* const _alifParserTokenNames_[];
AlifIntT alifToken_oneChar(AlifIntT);
AlifIntT alifToken_twoChars(AlifIntT, AlifIntT);
AlifIntT alifToken_threeChars(AlifIntT, AlifIntT, AlifIntT);
