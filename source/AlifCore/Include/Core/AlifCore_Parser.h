#pragma once

#include "AlifCore_AST.h"
#include "AlifCore_GlobalString.h"
#include "AlifCore_Memory.h"






class AlifParserDureRunState { // 21
public:
	AlifMutex mutex{};
	Expr dummyName{};
};


// 49
#define PARSER_DURERUN_STATE_INIT \
    { \
		.mutex = {0},							\
        .dummyName = {							\
            .type = ExprK_::NameK,				\
			.V = {								\
				.name = {						\
					.name = &ALIF_STR(Empty),	\
					.ctx = ExprContext_::Load,	\
				},								\
			},									\
            .lineNo = 1,						\
            .colOffset = 0,						\
            .endLineNo = 1,						\
            .endColOffset = 0,					\
        }, \
    }


extern ModuleTy _alifParser_astFromString(const char*, AlifObject*,
	AlifIntT, AlifCompilerFlags*, AlifASTMem*); // 63

extern ModuleTy alifParser_astFromFile(FILE*, AlifIntT, AlifObject*, const char*, const char*, const char*,
	AlifCompilerFlags*, AlifIntT*, AlifObject**, AlifASTMem*); // 70
