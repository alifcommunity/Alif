#pragma once

#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifParserEngine.h"

AlifObject* alifParserEngine_parseString(AlifParser*, AlifPToken*);
AlifObject* alifParserEngine_decodeString(AlifParser*, AlifIntT, const char*, AlifUSizeT, AlifPToken*);
