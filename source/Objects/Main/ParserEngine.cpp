#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_AST.h"

#include "Tokenizer.h"
#include "ParserEngine.h"














Module* alifParser_astFromFile(FILE* _fp, AlifObj* _fn, int _start, AlifMemory* alifMem) {

	TokenInfo* tokInfo = alifTokenizerInfo_fromFile(_fp);

	return nullptr;//
}
