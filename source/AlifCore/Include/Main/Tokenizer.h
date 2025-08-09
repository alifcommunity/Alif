#pragma once

#include "alif.h"



TokenState* _alifTokenizer_fromUTF8(const char*, AlifIntT, AlifIntT);
TokenState* alifTokenizerInfo_fromFile(FILE*, const char*, const char*, const char*);
