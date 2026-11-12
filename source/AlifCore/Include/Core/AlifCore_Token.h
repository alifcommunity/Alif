#pragma once


#undef TILDE   /* Prevent clash of our definition with system macro. Ex AIX, ioctl.h */

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
#define SEMI					13  // ؛
#define PLUS					14	// +
#define MINUS					15	// -
#define STAR					16	// *
#define DOT						17	// .
#define EQUAL					18	// =
#define AMPER					19	// &
#define LSHIFTEQUAL				20	// <<=
#define RSHIFTEQUAL				21	// >>=
#define STARVBAREQUAL       	22	// *|=
#define VBAREQUAL				23	// |=
#define AMPEREQUAL				24	// &=
#define DOUBLESLASHEQUAL		25	// //=
#define SLASHSTAREQUAL			26	// /*=
#define CIRCUMFLEXEQUAL			27	// ^=
#define SLASHEQUAL				29	// /=
#define STAREQUAL				30	// *=
#define MINUSEQUAL				31	// -=
#define PLUSEQUAL				32	// +=
#define DOUBLESTAR				33	// **
#define EQUALEQUAL				34  // ==
#define NOTEQUAL				35  // !=
#define LESSEQUAL				36  // <=
#define LESSTHAN				37  // <
#define GREATEREQUAL			38  // >=
#define GREATERTHAN				39  // >
#define VBAR					40  // |
#define STARVBAR				41  // *|
#define RSHIFT					42  // >>
#define LSHIFT					43  // <<
#define TILDE                   44  // ~
#define DOUBLESLASH				45  // //
#define SLASH					46  // /
#define SLASHSTAR				47  // /*
#define CIRCUMFLEX				48  // ^
#define SLASHCIRCUMFLEX			49  // /^
#define LBRACE					50  // {
#define RBRACE					51  // }
#define AT                      52  // @
#define EXCLAMATION				53  // !
#define OP						54
#define ELLIPSIS				55	// ...
#define TYPE_COMMENT            56
#define COMMENT					57
#define FSTRINGSTART			58	
#define FSTRINGMIDDLE			59	
#define FSTRINGEND				60	
#define NL						61	
#define ERRORTOKEN				62	
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



ALIFAPI_DATA(const char* const) _alifParserTokenNames_[];
ALIFAPI_FUNC(AlifIntT) alifToken_oneChar(AlifIntT);
ALIFAPI_FUNC(AlifIntT) alifToken_twoChars(AlifIntT, AlifIntT);
ALIFAPI_FUNC(AlifIntT) alifToken_threeChars(AlifIntT, AlifIntT, AlifIntT);
