#pragma once

#include "AlifTokenState.h"

int alifLexer_updateFStringExpr(TokenInfo*, wchar_t);
int alifTokenizer_get(TokenInfo*, AlifToken*);