#pragma once

#include "Object.h"

#define MAXINDENT 100
#define MAXLEVEL 200
#define MAXFSTRING_LEVEL 150

#define INSIDE_FSTRING(tok) (tok->tokModeStackIndex > 0)
#define INSIDE_FSTRING_EXPR(tok) (tok->curlyBracExprStartDepth >= 0)

class AlifToken {
public:
	int level{};
	int lineNo{}, colOffset{}, endLineNo{}, endColOffset{};
	const char* start{}, * end{};
	AlifObj* data{};
};

enum TokenizerModeType {
	Token_RegularMode,
	Token_FStringMode,
};

class TokenizerMode {
public:
	TokenizerModeType type{};

	int curlyBracDepth{};
	int curlyBracExprStartDepth{};

	char fStringQuote{};
	int fStringQuoteSize{};
	int fStringRaw{};

	const char* fStringStart{};
	const char* fStringMultiLineStart{};
	int fStringLineStart{};

	AlifSizeT fStringStartOffset{};
	AlifSizeT fStringMultiLineStartOffset{};

	AlifSizeT lastExprSize{};
	AlifSizeT lastExprEnd{};
	char* lastExprBuff{};
};

class TokenInfo {
public:
	char* buf{}, *cur{}, *inp{};
	const char* start{}, * end{};
	int done{};
	FILE* fp{};
	int tabSize{};
	int indent{};
	int indStack[MAXINDENT]{};
	int atBeginOfLine{};
	int pendInd{};
	const char* prompt{}, *nextPrompt{};
	int lineNo{};
	int firstLineNo{};
	int startingColOffset{};
	int colOffset{};
	int level{};
	char parenStack[MAXLEVEL]{};
	int parenLineNoStack[MAXLEVEL]{};
	int parenColStack[MAXLEVEL]{};
	AlifObj* fn;

	int alterIndStack[MAXINDENT]{};

	int countLine{};
	const char* lineStart{};
	const char* multiLineStart{};

	char* string;
	char* input;

	int comment{};

	TokenizerMode tokModeStack[MAXFSTRING_LEVEL]{};
	int tokModeStackIndex{};
	int tokExtraTokens{};
	int commentNewline{};
	int implicitNewline{};
};
