#include "alif.h"

#include "ErrorCode.h"
#include "AlifCore_Token.h"

#include "AlifTokenState.h"









char* alifTokenizer_newString(const char* _s, AlifSizeT _len, TokenState* _tok) { // 180
	char* result = (char*)alifMem_dataAlloc(_len + 1);
	if (!result) {
		_tok->done = E_NOMEM;
		return nullptr;
	}
	memcpy(result, _s, _len);
	result[_len] = '\0';
	return result;
}
