#pragma once

#include "AlifObject.h"

#define MAXINDENT 100
#define MAXLEVEL 200
#define MAXFSTRING_LEVEL 150

#define INSIDE_FSTRING(tok) (tok->tokModeStackIndex > 0)
#define INSIDE_FSTRING_EXPR(tok) (tok->curlyBracExprStartDepth >= 0)

class AlifToken { // token
public:
	int level{};
	int lineNo{}, colOffset{}, endLineNo{}, endColOffset{};
	const wchar_t* start{}, * end{};
	AlifObject* data{};
};

enum TokenizerModeType {
	Token_RegularMode,
	Token_FStringMode,
};

#define MAX_EXPR_NESTING 3

class TokenizerMode {
public:
	TokenizerModeType type{};

	int curlyBracDepth{};
	int curlyBracExprStartDepth{};

	char fStringQuote{};
	int fStringQuoteSize{};
	int fStringRaw{};

	const wchar_t* fStringStart{};
	const wchar_t* fStringMultiLineStart{};
	int fStringLineStart{};

	AlifSizeT fStringStartOffset{};
	AlifSizeT fStringMultiLineStartOffset{};

	AlifSizeT lastExprSize{};
	AlifSizeT lastExprEnd{};
	wchar_t* lastExprBuff{};
};

class TokenInfo {
public:
	wchar_t* buf{}, * cur{}, * inp{};
	const wchar_t* start{}, * end{};
	int done{};
	FILE* fp{};
	int tabSize{};
	int indent{};
	int indStack[MAXINDENT]{};
	int atBeginOfLine{ 1 };
	int pendInd{};
	const wchar_t* prompt{}, * nextPrompt{};
	int lineNo{};
	int firstLineNo{};
	int startingColOffset{ -1 };
	int colOffset{ -1 };
	int level{};
	wchar_t parenStack[MAXLEVEL]{};
	int parenLineNoStack[MAXLEVEL]{};
	int parenColStack[MAXLEVEL]{};
	AlifObject* fn;

	int alterIndStack[MAXINDENT]{};

	int countLine{};
	const wchar_t* lineStart{};
	const wchar_t* multiLineStart{};

	wchar_t* string;
	wchar_t* input;

	int comment{};

	int (*underflow) (TokenInfo*);

	TokenizerMode tokModeStack[MAXFSTRING_LEVEL]{};
	int tokModeStackIndex{};
	int tokExtraTokens{};
	int commentNewline{};
	int implicitNewline{};
};



TokenInfo* alifTokenizer_newTokenInfo();
int alifLexer_setupToken(TokenInfo*, AlifToken*, int, const wchar_t*, const wchar_t*);