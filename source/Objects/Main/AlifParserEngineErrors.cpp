#include "alif.h"
#include "ErrorCode.h"

//#include "AlifTokenState.h"
#include "AlifLexer.h"
#include "AlifParserEngine.h"




void alifParserEngineError_setSyntaxError(AlifParser* _p, AlifPToken* _lastToken) {
	// some errors
}

void alifParserEngineError_stackOverflow(AlifParser* _p) {
	_p->errorIndicator = 1;
	//wprintf_s(L"%s %u", L"الشفرة تجاوزت حد التكديس المسموح للاستدعاء التداخلي في المحلل اللغوي،\n حجم التكديس الحالي هو: ", MAXSTACK);
	//alifError_setString(alifExcMemory, L"تجاوز حد ذاكرة المكدس، حجم ذاكرة المكدس الحالي هي ");
}
