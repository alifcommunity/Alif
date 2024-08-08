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
#define SLASHCIRCUMFLEX			17	// /^
#define EQUAL					18	// =
#define AMPER					19	// &
#define LSHIFTEQUAL				20	// <<=
#define RSHIFTEQUAL				21	// >>=
#define DOUBLECIRCUMFLEXEQUAL	22	// ^^=
#define VBAREQUAL				23	// |=
#define AMPEREQUAL				24	// &=
#define DOUBLESLASHEQUAL		25	// //=
#define SLASHSTAREQUAL			26	// /*=
#define CIRCUMFLEXEQUAL			27	// ^=
#define SLASHEQUAL				28	// /=
#define STAREQUAL				29	// *=
#define MINUSEQUAL				30	// -=
#define PLUSEQUAL				31	// +=
#define DOUBLESTAR				32	// **
#define EQUALEQUAL				33  // ==
#define NOTEQUAL				34  // !=
#define LESSEQUAL				35  // <=
#define LESSTHAN				36  // <
#define GREATEREQUAL			37  // >=
#define GREATERTHAN				38  // >
#define VBAR					39  // |
#define STARVBAR				40  // *|
#define RSHIFT					41  // >>
#define LSHIFT					42  // <<
#define DOUBLESLASH				43  // //
#define SLASH					44  // /
#define SLASHSTAR				45  // /*
#define CIRCUMFLEX				46  // ^
#define SLASHCIRCUMFLEX			47  // /^
#define LBRACE					48  // {
#define RBRACE					49  // }
#define EXCLAMATION				50  // !
#define OP						51
#define COMMENT					52
#define FSTRINGSTART			53	
#define FSTRINGMIDDLE			54	
#define FSTRINGEND				55	
#define NL						56	
#define ERRORTOKEN				57	

#define ISSTRINGLITT(y) ((y) == STRING or (y) == FSTRINGMIDDLE)




int alifToken_oneChar(int);
int alifToken_twoChars(int, int);
int alifToken_threeChars(int, int, int);
