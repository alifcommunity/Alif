#pragma once

#include "alif.h"

#include "AlifTokenState.h"

// 8
#define LINE_ADVANCE() _tokState->lineNo++; _tokState->colOffset = 0;

AlifIntT alifTokenizer_syntaxError(TokenState*, const char*, ...); // 12

AlifIntT _alifTokenizer_indentError(TokenState*); // 14

char* alifTokenizer_newString(const char*, AlifSizeT, TokenState*); // 19
char* _alifTokenizer_translateNewlines(const char*, AlifIntT, AlifIntT, class TokenState*); // 20
