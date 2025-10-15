#pragma once

#include "alif.h"

#include "AlifTokenState.h"

// 8
#define LINE_ADVANCE() _tokState->lineNo++; _tokState->colOffset = 0;

AlifIntT alifTokenizer_syntaxError(TokenState*, const char*, ...); // 12

AlifIntT _alifTokenizer_indentError(TokenState*); // 14

char* _alifTokenizer_errorRet(TokenState*); // 17

char* _alifTokenizer_newString(const char*, AlifSizeT, TokenState*); // 19
char* _alifTokenizer_translateNewlines(const char*, AlifIntT, AlifIntT, class TokenState*); // 20
AlifObject* _alifTokenizer_translateIntoUTF8(const char*, const char*); // 21

AlifIntT _alifTokenizer_checkBom(AlifIntT get_char(TokenState*),
	void unget_char(AlifIntT, TokenState*),
	AlifIntT set_readline(TokenState*, const char*), TokenState*); // 23


AlifIntT _alifTokenizer_checkCodingSpec(const char*, AlifSizeT, TokenState*,
	AlifIntT set_readline(TokenState*, const char*)); // 27
