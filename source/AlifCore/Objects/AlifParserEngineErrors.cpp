#include "alif.h"
#include "ErrorCode.h"

#include "AlifCore_Errors.h"
#include "AlifTokenState.h"
#include "AlifLexer.h"
#include "AlifParserEngine.h"





AlifIntT _alifParserEngine_tokenizerError(AlifParser* p) { // 61
	if (alifErr_occurred()) {
		return -1;
	}

	const char* msg = nullptr;
	AlifObject* errtype = _alifExcSyntaxError_;
	AlifSizeT col_offset = -1;
	p->errorIndicator = 1;
	switch (p->tok->done) {
	case E_TOKEN:
		msg = "invalid token";
		break;
	case E_EOF:
		if (p->tok->level) {
			//raiseUnclosedParentheses_error(p);
		}
		else {
			//RAISE_SYNTAX_ERROR("unexpected EOF while parsing");
		}
		return -1;
	case E_DEDENT:
		//RAISE_INDENTATION_ERROR("unindent does not match any outer indentation level");
		return -1;
	case E_INTR:
		if (!alifErr_occurred()) {
			//alifErr_setNone(_alifExcKeyboardInterrupt_);
		}
		return -1;
	case E_NOMEM:
		//alifErr_noMemory();
		return -1;
	case E_TABSPACE:
		//errtype = _alifExcTabError_;
		msg = "inconsistent use of tabs and spaces in indentation";
		break;
	case E_TOODEEP:
		//errtype = _alifExcIndentationError_;
		msg = "too many levels of indentation";
		break;
	case E_LINECONT: {
		col_offset = p->tok->cur - p->tok->buf - 1;
		msg = "unexpected character after line continuation character";
		break;
	}
	case E_COLUMNOVERFLOW:
		//alifErr_setString(_alifExcOverflowError_,
		//	"Parser column offset overflow - source line is too big");
		return -1;
	default:
		msg = "unknown parsing error";
	}

	//RAISE_ERROR_KNOWN_LOCATION(p, errtype, p->tok->lineNo,
	//	col_offset >= 0 ? col_offset : 0,
	//	p->tok->lineNo, -1, msg);
	return -1;
}









void _alifParserEngine_setSyntaxError(AlifParser* _p, AlifPToken* _lastToken) { // 405
	// Existing syntax error
	if (alifErr_occurred()) {
		AlifIntT is_tok_ok = (_p->tok->done == E_DONE or _p->tok->done == E_OK);
		//if (is_tok_ok and alifErr_exceptionMatches(_alifExcSyntaxError_)) {
		//	_alifParserEngine_tokenizeFullSourceToCheckForErrors(p);
		//}
		// Propagate the existing syntax error.
		return;
	}
	// Initialization error
	if (_p->fill == 0) {
		//RAISE_SYNTAX_ERROR("error at start before reading any input");
	}

	if (_lastToken->type == ERRORTOKEN and _p->tok->done == E_EOF) {
		if (_p->tok->level) {
			//raise_unclosedParenthesesError(p);
		}
		else {
			//RAISE_SYNTAX_ERROR("unexpected EOF while parsing");
		}
		return;
	}

	if (_lastToken->type == INDENT or _lastToken->type == DEDENT) {
		//RAISE_INDENTATION_ERROR(last_token->type == INDENT ? "unexpected indent" : "unexpected unindent");
		return;
	}
	// Unknown error (generic case)

	//RAISE_SYNTAX_ERROR_KNOWN_LOCATION(last_token, "invalid syntax");

	//_alifParserEngine_tokenizeFullSourceToCheckForErrors(p);
}




void alifParserEngineError_stackOverflow(AlifParser* _p) { // 448
	_p->errorIndicator = 1;
	//alifErr_setString(_alifExcMemoryError_,
	//	"Parser stack overflowed - Alif source too complex to parse");
}
