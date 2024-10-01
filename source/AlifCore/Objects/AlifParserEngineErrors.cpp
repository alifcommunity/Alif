#include "alif.h"
#include "ErrorCode.h"

#include "AlifTokenState.h"
#include "AlifLexer.h"
#include "AlifParserEngine.h"





void alifParserEngineError_stackOverflow(AlifParser* _p) { // 448
	_p->errorIndicator = 1;
	//alifErr_setString(_alifExcMemoryError_,
	//	"Parser stack overflowed - Alif source too complex to parse");
}
