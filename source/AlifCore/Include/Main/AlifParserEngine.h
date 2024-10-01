#pragma once

#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Token.h"


 // 12
#define ALIFPARSE_DONT_IMPLY_DEDENT       0x0002

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
	OperatorTy type{};
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



//// يجب إيجاد الملف المناسب لنقل هذا الصنف له
//class AlifPArray {
//public:
//	void** data_{};
//	AlifSizeT size_{};
//	AlifSizeT capacity_{};
//
//	AlifPArray() {
//		capacity_ = 4;
//		size_ = 0;
//		data_ = (void**)alifMem_dataAlloc(capacity_ * sizeof(void**));
//	}
//
//	~AlifPArray() {
//		alifMem_dataFree(data_);
//	}
//
//	void push_back(void*& value) {
//		if (size_ == capacity_) {
//			capacity_ *= 2;
//			data_ = (void**)alifMem_dataRealloc(data_, capacity_ * sizeof(void**));
//			if (data_ == nullptr) return;
//		}
//		data_[size_++] = value;
//	}
//
//	void* operator[](AlifUSizeT _index) const {
//		return data_[_index];
//	}
//};







//int alifParserEngine_insertMemo(AlifParser*, int, int, void*);
//int alifParserEngine_updateMemo(AlifParser*, int, int, void*);
//int alifParserEngine_isMemorized(AlifParser*, int, void*);
//
//int alifParserEngine_lookaheadWithInt(int, AlifPToken* (_func)(AlifParser*, AlifIntT), AlifParser*, int);
//int alifParserEngine_lookahead(int, void* (_func)(AlifParser*), AlifParser*);
//
//AlifPToken* alifParserEngine_expectToken(AlifParser*, AlifIntT);
//AlifPToken* alifParserEngine_expectTokenForced(AlifParser*, int, const wchar_t*);
//AlifPToken* alifParserEngine_getLastNonWhitespaceToken(AlifParser*);
//Expression* alifParserEngine_nameToken(AlifParser*);
//void* alifParserEngine_stringToken(AlifParser*);
//Expression* alifParserEngine_numberToken(AlifParser*);
//
//int alifParserEngine_fillToken(AlifParser*);






void alifParserEngineError_stackOverflow(AlifParser*); // 171




//#define EXTRA_EXPR(head, tail) (head)->lineNo, (head)->colOffset, (tail)->endLineNo, (tail)->endColOffset, _p->astMem
//
//#define EXTRA startLineNo, startColOffset, endLineNo, endColOffset, _p->astMem
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//AlifObject* alifParserEngine_newIdentifier(AlifParser*, const wchar_t*);
//Seq* alifParserEngine_singletonSeq(AlifParser*, void*);
//Seq* alifParserEngine_seqInsertInFront(AlifParser*, void*, Seq*);
//Seq* alifParserEngine_seqFlatten(AlifParser*, Seq*);
//Expression* alifParserEngine_joinNamesWithDot(AlifParser*, Expression*, Expression*);
//Alias* alifParserEngine_aliasForStar(AlifParser*, int, int, int, int, AlifASTMem*);
//IdentifierSeq* alifParserEngine_mapNamesToIds(AlifParser*, ExprSeq*);
//CompExprPair* alifParserEngine_compExprPair(AlifParser*, CmpOp, Expression*);
//IntSeq* alifParserEngine_getCmpOps(AlifParser*, Seq*);
//ExprSeq* alifParserEngine_getExprs(AlifParser*, Seq*);
//Expression* alifParserEngine_setExprContext(AlifParser*, Expression*, ExprCTX);
//KeyValuePair* alifParserEngine_keyValuePair(AlifParser*, Expression*, Expression*);
//ExprSeq* alifParserEngine_getKeys(AlifParser*, Seq*);
//ExprSeq* alifParserEngine_getValues(AlifParser*, Seq*);
//NameDefaultPair* alifParserEngine_nameDefaultPair(AlifParser*, Arg*, Expression*);
//StarEtc* alifParserEngine_starEtc(AlifParser*, Arg*, Seq*, Arg*);
//Arguments* alifParserEngine_makeArguments(AlifParser*, ArgSeq*, SlashWithDefault*, ArgSeq*, Seq*, StarEtc*);
//Arguments* alifParserEngine_emptyArguments(AlifParser*);
//Expression* alifParserEngine_formattedValue(AlifParser*, Expression*, AlifPToken*,
//	ResultTokenWithMetadata*, ResultTokenWithMetadata*, AlifPToken*, int, int, int, int, AlifASTMem*);
//AugOperator* alifParserEngine_augOperator(AlifParser*, Operator);
//KeywordOrStar* alifParserEngine_keywordOrStarred(AlifParser*, void*, AlifIntT);
//ExprSeq* alifParserEngine_seqExtractStarExprs(AlifParser*, Seq*);
//KeywordSeq* alifParserEngine_seqDeleteStarExprs(AlifParser*, Seq*);
//Expression* alifParserEngine_collectCallSeqs(AlifParser*, ExprSeq*, Seq*,int, int, int, int, AlifASTMem*);
//Expression* alifParserEngine_constantFromToken(AlifParser*, AlifPToken*);
//Expression* alifParserEngine_decodeConstantFromToken(AlifParser*, AlifPToken*);
//Expression* alifParserEngine_constantFromString(AlifParser*, AlifPToken*);
//Expression* alifParserEngine_combineStrings(AlifParser*, ExprSeq*, int, int, int, int, AlifASTMem*);
//Seq* alifParserEngine_joinSequences(AlifParser*, Seq*, Seq*);
//ResultTokenWithMetadata* alifParserEngine_checkFStringConversion(AlifParser*, AlifPToken*, Expression*);
//ResultTokenWithMetadata* alifParserEngine_setupFullFormatSpec(AlifParser*, AlifPToken*, ExprSeq*, int, int, int, int, AlifASTMem*);
//
//
AlifParser* alifParserEngine_parserNew(TokenState*, AlifIntT, AlifIntT,
	AlifIntT, AlifIntT*, AlifASTMem*); // 352
void alifParserEngine_parserFree(AlifParser*); // 353
//void alifParserEngine_parserFree(AlifParser*);
//Module* alifParser_astFromFile(FILE*, AlifObject*, int, AlifASTMem*);
//
//void* alifParserEngine_runParser(AlifParser*);
//
//Expression* alifParserEngine_joinedStr(AlifParser*, AlifPToken*, ExprSeq*, AlifPToken*);
//
//void* alifParserEngine_parse(AlifParser*);
