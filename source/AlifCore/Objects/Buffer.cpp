#include "alif.h"

#include "ErrorCode.h"
#include "AlifTokenState.h"





void _alifLexer_rememberFStringBuffers(TokenState* _tok) {
	AlifIntT index{};
	TokenizerMode* mode{};

	for (index = _tok->tokModeStackIndex; index >= 0; --index) {
		mode = &(_tok->tokModeStack[index]);
		mode->fStringStartOffset = mode->fStringStart - _tok->buf;
		mode->fStringMultiLineStartOffset = mode->fStringMultiLineStart - _tok->buf;
	}
}

void _alifLexer_restoreFStringBuffers(TokenState* _tok) {
	AlifIntT index{};
	TokenizerMode* mode{};

	for (index = _tok->tokModeStackIndex; index >= 0; --index) {
		mode = &(_tok->tokModeStack[index]);
		mode->fStringStart = _tok->buf + mode->fStringStartOffset;
		mode->fStringMultiLineStart = _tok->buf + mode->fStringMultiLineStartOffset;
	}
}






AlifIntT _alifLexerTok_reserveBuf(TokenState* _tok, AlifSizeT _size) { // 49
	AlifSizeT cur = _tok->cur - _tok->buf;
	AlifSizeT oldsize = _tok->inp - _tok->buf;
	AlifSizeT newsize = oldsize + ALIF_MAX(_size, oldsize >> 1);
	if (newsize > _tok->end - _tok->buf) {
		char* newbuf = _tok->buf;
		AlifSizeT start = _tok->start == nullptr ? -1 : _tok->start - _tok->buf;
		AlifSizeT lineStart = _tok->start == nullptr ? -1 : _tok->lineStart - _tok->buf;
		AlifSizeT multiLineStart = _tok->multiLineStart - _tok->buf;
		_alifLexer_rememberFStringBuffers(_tok);
		newbuf = (char*)alifMem_dataRealloc(newbuf, newsize);
		if (newbuf == nullptr) {
			_tok->done = E_NOMEM;
			return 0;
		}
		_tok->buf = newbuf;
		_tok->cur = _tok->buf + cur;
		_tok->inp = _tok->buf + oldsize;
		_tok->end = _tok->buf + newsize;
		_tok->start = start < 0 ? nullptr : _tok->buf + start;
		_tok->lineStart = lineStart < 0 ? nullptr : _tok->buf + lineStart;
		_tok->multiLineStart = multiLineStart < 0 ? nullptr : _tok->buf + multiLineStart;
		_alifLexer_restoreFStringBuffers(_tok);
	}
	return 1;
}
