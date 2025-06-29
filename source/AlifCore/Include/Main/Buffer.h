#pragma once


void _alifLexer_rememberFStringBuffers(TokenState*);
void _alifLexer_restoreFStringBuffers(TokenState*);
AlifIntT _alifLexerTok_reserveBuf(TokenState*, AlifSizeT);
