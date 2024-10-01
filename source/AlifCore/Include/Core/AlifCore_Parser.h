#pragma once









class AlifParserDureRunState { // 21
public:
	AlifMutex mutex{};
	//struct Expression dummyName{};
};


// 49
#define PARSER_DURERUN_STATE_INIT \
    { \
		/*
        .dummyName = { \
            .kind = NameK,						\
            .V.name.name = &ALIF_STR(empty),	\
            .V.name.ctx = Load,					\
            .lineNo = 1,						\
            .colOffset = 0,						\
            .endLineNo = 1,						\
            .endColOffset = 0,					\
        }, \
		*/	\
    }



ModuleTy alifParser_astFromFile(FILE*, AlifIntT, AlifObject*, const char*, const char*, const char*,
	AlifCompilerFlags*, AlifIntT*, AlifObject**, AlifASTMem*); // 70
