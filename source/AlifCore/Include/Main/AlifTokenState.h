#pragma once

#include "AlifObject.h"

// 6
#define MAXINDENT 100
#define MAXLEVEL 200
#define MAXFSTRING_LEVEL 150

#define INSIDE_FSTRING(_tok) (_tok->tokModeStackIndex > 0)
#define INSIDE_FSTRING_EXPR(_tok) (_tok->curlyBracExprStartDepth >= 0)


enum DecodingState_ { // 13
	State_Init,
	State_Seek_Coding,
	State_Normal
};

enum InteractiveUnderflow_ { // 19
	IUnderflow_Normal,
	IUnderflow_Stop,
};



 // token
class AlifToken { // 27
public:
	AlifIntT level{};
	AlifIntT lineNo{}, colOffset{}, endLineNo{}, endColOffset{};
	const char* start{}, * end{};
	AlifObject* data{};
};

enum TokenizerModeType_ { // 34
	Token_RegularMode,
	Token_FStringMode,
};

#define MAX_EXPR_NESTING 3 // 39

class TokenizerMode { // 41
public:
	TokenizerModeType_ type{};

	AlifIntT curlyBracDepth{};
	AlifIntT curlyBracExprStartDepth{};

	char fStringQuote{};
	AlifIntT fStringQuoteSize{};
	AlifIntT fStringRaw{};

	const char* fStringStart{};
	const char* fStringMultiLineStart{};
	AlifIntT fStringLineStart{};

	AlifSizeT fStringStartOffset{};
	AlifSizeT fStringMultiLineStartOffset{};

	AlifSizeT lastExprSize{};
	AlifSizeT lastExprEnd{};
	char* lastExprBuff{};
	AlifIntT fStringDebug;
	AlifIntT inFormatSpec;
};

class TokenState { // 65
public:
	char* buf{}, * cur{}, * inp{};
	AlifIntT interactive{};
	char* interactiveSrcStart{};
	char* interactiveSrcEnd{};
	const char* start{}, * end{};
	AlifIntT done{};
	FILE* fp{};
	AlifIntT tabSize{};
	AlifIntT indent{};
	AlifIntT indStack[MAXINDENT]{};
	AlifIntT atBeginOfLine{ 1 };
	AlifIntT pendInd{};
	const char* prompt{}, * nextPrompt{};
	AlifIntT lineNo{};
	AlifIntT firstLineNo{};
	AlifIntT startingColOffset{};
	AlifIntT colOffset{};
	AlifIntT level{};
	char parenStack[MAXLEVEL]{};
	AlifIntT parenLineNoStack[MAXLEVEL]{};
	AlifIntT parenColStack[MAXLEVEL]{};
	AlifObject* fn{};

	AlifIntT alterIndStack[MAXINDENT]{};

	DecodingState_ decodingState{};
	AlifIntT decodingErred{};
	char* encoding{};
	AlifIntT contLine{};
	const char* lineStart{};
	const char* multiLineStart{};
	AlifObject* decodingReadline{};
	AlifObject* decodingBuffer{};
	AlifObject* readline{};
	const char* enc{};
	char* string{};
	char* input{};

	AlifIntT comment{};

	InteractiveUnderflow_ interactiveUnderflow{};
	AlifIntT (*underflow)(TokenState*);

	TokenizerMode tokModeStack[MAXFSTRING_LEVEL]{};
	AlifIntT tokModeStackIndex{};
	AlifIntT tokExtraTokens{};
	AlifIntT commentNewline{};
	AlifIntT implicitNewline{};
};



AlifIntT alifLexer_setupToken(TokenState*, AlifToken*, AlifIntT, const char*, const char*); // 134

TokenState* alifTokenizer_tokNew(); // 136
void alifTokenizer_free(TokenState*); // 137
void _alifToken_free(AlifToken*); // 138
