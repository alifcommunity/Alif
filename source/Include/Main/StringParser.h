#pragma once

#include "alif.h"
#include "AlifCore_AST.h"
#include "AlifParserEngine.h"

AlifObject* alifParserEngine_parseString(AlifParser*, AlifPToken*);
AlifObject* alifParserEngine_decodeString(AlifParser*, int, const wchar_t*, size_t, AlifPToken*);