#pragma once

#include "alif.h"

#include "AlifTokenState.h"

#define LINE_ADVANCE() _tokState->lineNo++; _tokState->colOffset = 0;



char* alifTokenizer_newString(const char*, AlifSizeT, TokenState*); // 19
