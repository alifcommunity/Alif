#pragma once

#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_AlifToken.h"


class Memo {
public:
	int type{};
	void* node{};
	AlifIntT mark_{};
	Memo* next{};
};

class AlifPToken { // Token
public:
	int type{};
	AlifObject* bytes{};
	int level{};
	int lineNo{}, colOffset{}, endLineNo{}, endColOffset{};
	Memo* memo{};
	AlifObject* data{};
};

class KeywordToken {
public:
	const wchar_t* str{};
	int type{};
};

class GrowableCommentArr {
public:
	class Items {
	public:
		AlifSizeT lineNo{};
		char* comment{};
	} *items;
	AlifSizeT size{};
	AlifSizeT numItems{};
};

class AlifParser {
public:
	class TokenInfo* tok{};
	AlifPToken** tokens{};
	AlifIntT mark_{};
	AlifIntT fill_{}, size_{};
	AlifASTMem* astMem{};
	KeywordToken** keywords{};
	wchar_t** softKeyword{};
	int nKeywordList{};
	int startRule{};
	int* errorCode{};
	int parsingStarted{};
	AlifObject* normalize{};
	int startingLineNo{};
	int startingColOffset{};
	int errorIndicator{};
	int flags{};
	int featureVersion{};
	GrowableCommentArr typeIgnoreComments{};
	AlifPToken* KnownErrToken{};
	int level{};
	//int callInvalidRules{}; // we'll need it
};

class CompExprPair {
public:
	CmpOp cmpOp{};
	Expression* expr_{};
};

class KeyValuePair {
public:
	Expression* key_{};
	Expression* val_{};
};

class NameDefaultPair {
public:
	Arg* arg_{};
	Expression* value_{};
};

class SlashWithDefault {
public:
	ArgSeq* plainNames{};
	Seq* namesWithDefaults{};
};

class StarEtc {
public:
	Arg* varArg{};
	Seq* kwOnlyArgs{};
	Arg* kwArg{};
};


class AugOperator {
public:
	Operator type{};
};

class KeywordOrStar {
public:
	void* element{};
	AlifIntT isKeyword{};
};

class ResultTokenWithMetadata {
public:
	void* result{};
	AlifObject* metadata{};
};












int alifParserEngine_insertMemo(AlifParser*, int, int, void*);
int alifParserEngine_updateMemo(AlifParser*, int, int, void*);
int alifParserEngine_isMemorized(AlifParser*, int, void*);

int alifParserEngine_lookaheadWithInt(int, AlifPToken* (_func)(AlifParser*, int), AlifParser*, int);
int alifParserEngine_lookahead(int, void* (_func)(AlifParser*), AlifParser*);

AlifPToken* alifParserEngine_expectToken(AlifParser*, int);
AlifPToken* alifParserEngine_expectTokenForced(AlifParser*, int, const wchar_t*);
AlifPToken* alifParserEngine_getLastNonWhitespaceToken(AlifParser*);
Expression* alifParserEngine_nameToken(AlifParser*);
void* alifParserEngine_stringToken(AlifParser*);
Expression* alifParserEngine_numberToken(AlifParser*);

int alifParserEngine_fillToken(AlifParser*);






void alifParserEngineError_stackOverflow(AlifParser*);




#define EXTRA_EXPR(head, tail) (head)->lineNo, (head)->colOffset, (tail)->endLineNo, (tail)->endColOffset, _p->astMem

#define EXTRA startLineNo, startColOffset, endLineNo, endColOffset, _p->astMem














AlifObject* alifParserEngine_newIdentifier(AlifParser*, const wchar_t*);
Seq* alifParserEngine_singletonSeq(AlifParser*, void*);
Seq* alifParserEngine_seqInsertInFront(AlifParser*, void*, Seq*);
Seq* alifParserEngine_seqFlatten(AlifParser*, Seq*);
Expression* alifParserEngine_joinNamesWithDot(AlifParser*, Expression*, Expression*);
Alias* alifParserEngine_aliasForStar(AlifParser*, int, int, int, int, AlifASTMem*);
IdentifierSeq* alifParserEngine_mapNamesToIds(AlifParser*, ExprSeq*);
CompExprPair* alifParserEngine_compExprPair(AlifParser*, CmpOp, Expression*);
IntSeq* alifParserEngine_getCmpOps(AlifParser*, Seq*);
ExprSeq* alifParserEngine_getExprs(AlifParser*, Seq*);
Expression* alifParserEngine_setExprContext(AlifParser*, Expression*, ExprCTX);
KeyValuePair* alifParserEngine_keyValuePair(AlifParser*, Expression*, Expression*);
ExprSeq* alifParserEngine_getKeys(AlifParser*, Seq*);
ExprSeq* alifParserEngine_getValues(AlifParser*, Seq*);
NameDefaultPair* alifParserEngine_nameDefaultPair(AlifParser*, Arg*, Expression*);
StarEtc* alifParserEngine_starEtc(AlifParser*, Arg*, Seq*, Arg*);
Arguments* alifParserEngine_makeArguments(AlifParser*, ArgSeq*, SlashWithDefault*, ArgSeq*, Seq*, StarEtc*);
Arguments* alifParserEngine_emptyArguments(AlifParser*);
Expression* alifParserEngine_formattedValue(AlifParser*, Expression*, AlifPToken*,
	ResultTokenWithMetadata*, ResultTokenWithMetadata*, AlifPToken*, int, int, int, int, AlifASTMem*);
AugOperator* alifParserEngine_augOperator(AlifParser*, Operator);
KeywordOrStar* alifParserEngine_keywordOrStarred(AlifParser*, void*, AlifIntT);
ExprSeq* alifParserEngine_seqExtractStarExprs(AlifParser*, Seq*);
KeywordSeq* alifParserEngine_seqDeleteStarExprs(AlifParser*, Seq*);
Expression* alifParserEngine_collectCallSeqs(AlifParser*, ExprSeq*, Seq*,int, int, int, int, AlifASTMem*);
Expression* alifParserEngine_constantFromToken(AlifParser*, AlifPToken*);
Expression* alifParserEngine_decodeConstantFromToken(AlifParser*, AlifPToken*);
Expression* alifParserEngine_constantFromString(AlifParser*, AlifPToken*);
Expression* alifParserEngine_combineStrings(AlifParser*, ExprSeq*, int, int, int, int, AlifASTMem*);
Seq* alifParserEngine_joinSequences(AlifParser*, Seq*, Seq*);
ResultTokenWithMetadata* alifParserEngine_checkFStringConversion(AlifParser*, AlifPToken*, Expression*);
ResultTokenWithMetadata* alifParserEngine_setupFullFormatSpec(AlifParser*, AlifPToken*, ExprSeq*, int, int, int, int, AlifASTMem*);

Module* alifParserEngine_makeModule(AlifParser*, StmtSeq*);


AlifParser* alifParserEngine_newParser(class TokenInfo*, int, AlifASTMem*);
void alifParserEngine_parserFree(AlifParser*);
Module* alifParser_astFromFile(FILE*, AlifObject*, int, AlifASTMem*);

void* alifParserEngine_runParser(AlifParser*);

Expression* alifParserEngine_joinedStr(AlifParser*, AlifPToken*, ExprSeq*, AlifPToken*);

void* alifParserEngine_parse(AlifParser*);