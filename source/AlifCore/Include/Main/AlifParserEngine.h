#pragma once

#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Token.h"
#include "AlifCore_Array.h"


 // 12
#define ALIFPARSE_DONT_IMPLY_DEDENT 0x0002

#define ALIFPARSE_IGNORE_COOKIE 0x0010
#define ALIFPARSE_BARRY_AS_BDFL 0x0020
#define ALIFPARSE_TYPE_COMMENTS 0x0040
#define ALIFPARSE_ALLOW_INCOMPLETE_INPUT 0x0100



class Memo { // 27
public:
	int type{};
	void* node{};
	AlifIntT mark{};
	Memo* next{};
};

// Token
class AlifPToken {  // 34
public:
	AlifIntT type{};
	AlifObject* bytes{};
	AlifIntT level{};
	AlifIntT lineNo{}, colOffset{}, endLineNo{}, endColOffset{};
	Memo* memo{};
	AlifObject* data{};
};

class KeywordToken { // 43
public:
	const char* str{};
	AlifIntT type{};
};

class AlifParser { // 58
public:
	class TokenState* tok{};
	AlifPToken** tokens{};
	AlifIntT mark{};
	AlifIntT fill{}, size{};
	AlifASTMem* astMem{};
	KeywordToken** keywords{};
	char** softKeyword{};
	AlifIntT nKeywordList{};
	AlifIntT startRule{};
	AlifIntT* errorCode{};
	AlifIntT parsingStarted{};
	AlifObject* normalize{};
	AlifIntT startingLineNo{};
	AlifIntT startingColOffset{};
	AlifIntT errorIndicator{};
	AlifIntT flags{};
	AlifIntT featureVersion{};
	AlifPToken* KnownErrToken{};
	AlifIntT level{};
	AlifIntT callInvalidRules{};
	//AlifIntT debug{};
};

class CompExprPair { // 83
public:
	CmpOp_ cmpOp{};
	ExprTy expr{};
};

class KeyValuePair { // 88
public:
	ExprTy key{};
	ExprTy val{};
};

class NameDefaultPair { // 98
public:
	ArgTy arg{};
	ExprTy value{};
};

class SlashWithDefault { // 103
public:
	ASDLArgSeq* plainNames{};
	ASDLSeq* namesWithDefaults{};
};

class StarEtc { // 108
public:
	ArgTy varArg{};
	ASDLSeq* kwOnlyArgs{};
	ArgTy kwArg{};
};


class AugOperator { // 114
public:
	Operator_ type{};
};

class KeywordOrStar { // 115
public:
	void* element{};
	AlifIntT isKeyword{};
};

class ResultTokenWithMetadata { // 120
public:
	void* result{};
	AlifObject* metadata{};
};








AlifIntT alifParserEngine_insertMemo(AlifParser*, AlifIntT, AlifIntT, void*); // 131
AlifIntT alifParserEngine_updateMemo(AlifParser*, AlifIntT, AlifIntT, void*); // 132
AlifIntT alifParserEngine_isMemorized(AlifParser*, AlifIntT, void*);

AlifIntT alifParserEngine_lookaheadWithInt(AlifIntT, AlifPToken* (_func)(AlifParser*, AlifIntT), AlifParser*, AlifIntT); // 136
AlifIntT alifParserEngine_lookahead(AlifIntT, void* (_func)(AlifParser*), AlifParser*); // 138

AlifPToken* alifParserEngine_expectToken(AlifParser*, AlifIntT); // 140
AlifPToken* alifParserEngine_expectTokenForced(AlifParser*, AlifIntT, const char*); // 142
AlifPToken* alifParserEngine_getLastNonWhitespaceToken(AlifParser*); // 146
ExprTy alifParserEngine_nameToken(AlifParser*); // 148
ExprTy alifParserEngine_numberToken(AlifParser*); // 149
void* alifParserEngine_stringToken(AlifParser*); // 150

AlifIntT alifParserEngine_fillToken(AlifParser*); // 147




AlifIntT _alifParserEngine_tokenizerError(AlifParser*); // 164



void _alifParserEngine_setSyntaxError(AlifParser*, AlifPToken*); // 170
void alifParserEngineError_stackOverflow(AlifParser*); // 171


void* alifParserEngine_dummyName(AlifParser*, ...); // 248

#define EXTRA_EXPR(_head, _tail) (_head)->lineNo, (_head)->colOffset, (_tail)->endLineNo, (_tail)->endColOffset, _p->astMem // 254

#define EXTRA startLineNo, startColOffset, endLineNo, endColOffset, _p->astMem // 255














AlifObject* alifParserEngine_newIdentifier(AlifParser*, const char*); // 296
ASDLSeq* alifParserEngine_singletonSeq(AlifParser*, void*); // 297
ASDLSeq* alifParserEngine_seqInsertInFront(AlifParser*, void*, ASDLSeq*); // 298
ASDLSeq* alifParserEngine_seqFlatten(AlifParser*, ASDLSeq*); // 300
ExprTy alifParserEngine_joinNamesWithDot(AlifParser*, ExprTy, ExprTy); // 301
AliasTy alifParserEngine_aliasForStar(AlifParser*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*); // 303
ASDLIdentifierSeq* alifParserEngine_mapNamesToIds(AlifParser*, ASDLExprSeq*); // 304
CompExprPair* alifParserEngine_compExprPair(AlifParser*, CmpOp_, ExprTy); // 305
ASDLIntSeq* alifParserEngine_getCmpOps(AlifParser*, ASDLSeq*); // 306
ASDLExprSeq* alifParserEngine_getExprs(AlifParser*, ASDLSeq*); // 307
ExprTy alifParserEngine_setExprContext(AlifParser*, ExprTy, ExprContext_); // 308
KeyValuePair* alifParserEngine_keyValuePair(AlifParser*, ExprTy, ExprTy); // 309
ASDLExprSeq* alifParserEngine_getKeys(AlifParser*, ASDLSeq*); // 310
ASDLExprSeq* alifParserEngine_getValues(AlifParser*, ASDLSeq*); // 311
NameDefaultPair* alifParserEngine_nameDefaultPair(AlifParser*, Arg*, ExprTy); // 315
StarEtc* alifParserEngine_starEtc(AlifParser*, ArgTy, ASDLSeq*, ArgTy); // 317
ArgumentsTy alifParserEngine_makeArguments(AlifParser*, ASDLArgSeq*, SlashWithDefault*, ASDLArgSeq*, ASDLSeq*, StarEtc*); // 318
ArgumentsTy alifParserEngine_emptyArguments(AlifParser*); // 320
ExprTy alifParserEngine_formattedValue(AlifParser*, ExprTy, AlifPToken*,
	ResultTokenWithMetadata*, ResultTokenWithMetadata*, AlifPToken*, AlifIntT,
	AlifIntT, AlifIntT, AlifIntT, AlifASTMem*); // 321
AugOperator* alifParserEngine_augOperator(AlifParser*, Operator_); // 323
KeywordOrStar* alifParserEngine_keywordOrStarred(AlifParser*, void*, AlifIntT); // 326
ASDLExprSeq* alifParserEngine_seqExtractStarExprs(AlifParser*, ASDLSeq*); // 327
ASDLKeywordSeq* alifParserEngine_seqDeleteStarExprs(AlifParser*, ASDLSeq*); // 328
ExprTy alifParserEngine_collectCallSeqs(AlifParser*, ASDLExprSeq*, ASDLSeq*,AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*); // 329
ExprTy alifParserEngine_constantFromToken(AlifParser*, AlifPToken*); // 332
ExprTy alifParserEngine_decodeConstantFromToken(AlifParser*, AlifPToken*); // 333
ExprTy alifParserEngine_constantFromString(AlifParser*, AlifPToken*); // 334
ExprTy alifParserEngine_concatenateStrings(AlifParser*, ASDLExprSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*); // 335
ASDLSeq* alifParserEngine_joinSequences(AlifParser*, ASDLSeq*, ASDLSeq*); // 339
ResultTokenWithMetadata* alifParserEngine_checkFStringConversion(AlifParser*, AlifPToken*, ExprTy); // 342
ResultTokenWithMetadata* alifParserEngine_setupFullFormatSpec(AlifParser*, AlifPToken*,
	ASDLExprSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*); // 343

ModuleTy alifParserEngine_makeModule(AlifParser* p, ASDLStmtSeq* a); // 345

AlifParser* alifParserEngine_parserNew(TokenState*, AlifIntT,
	AlifIntT, AlifIntT, AlifIntT*, AlifASTMem*); // 352
void alifParserEngine_parserFree(AlifParser*); // 353
//void alifParserEngine_parserFree(AlifParser*);
//Module* alifParser_astFromFile(FILE*, AlifObject*, int, AlifASTMem*);

//void* alifParserEngine_runParser(AlifParser*);

ExprTy alifParserEngine_joinedStr(AlifParser*, AlifPToken*, ASDLExprSeq*, AlifPToken*); // 362

void* alifParserEngine_parse(AlifParser*); // 365
