#pragma once

#include "AlifCore_EvalState.h"
#include "AlifCore_Eval.h" // remove if include other hreader with AlifCore_Eval.h inside it
//#include "AlifCore_Import.h"
#include "AlifCore_Parser.h"
#include "AlifCore_Thread.h"


#define ALIF_DURERUNSTATE_INIT(dureRun)												\
	{																				\
        .selfInitialized = 0,														\
		.interpreters = {.nextID = -1},												\
		.mainThreadID = 0,															\
		.threads = ALIFTHREAD_DURERUN_INIT(dureRun.threads),						\
		.autoTSSKey = 0,															\
		.trashTSSKey = 0,															\
		.parser = PARSER_DURERUN_STATE_INIT,										\
        .staticObjects = { \
            .singletons = { \
                /*.smallInts = ALIF_SMALL_INTS_INIT,*/ \
                /*.bytesEmpty = ALIFBYTES_SIMPLE_INIT(0, 0),*/ \
                /*.bytesCharacters = ALIF_BYTES_CHARACTERS_INIT,*/ \
                .strings = { \
                    .literals = ALIF_STR_LITERALS_INIT, \
                    .identifiers = ALIF_STR_IDENTIFIERS_INIT, \
                    /*.ascii = ALIF_STR_ASCII_INIT,*/ \
                    /*.latin1 = ALIF_STR_LATIN1_INIT,*/ \
                }, \
                .tupleEmpty = { \
                    .objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, 0), \
                }, \
                /*.hamtBitmapNodeEmpty = { \
                    .objBase = ALIFVAROBJECT_HEAD_INIT(&_alifHamtBitmapNodeType_, 0), \
                },*/ \
                /*.contextTokenMissing = { \
                    .objBase = ALIFOBJECT_HEAD_INIT(&_alifContextTokenMissingType_), \
                },*/ \
            }, \
        }, \
        .mainInterpreter = ALIF_INTERPRETERSTATE_INIT(_dureRun_.mainInterpreter),	\
	}


#define ALIF_INTERPRETERSTATE_INIT(interpreter)						\
    {																\
		.eval = {.recursionLimit = ALIF_DEFAULT_RECURSION_LIMIT},	\
		.qsbr = {													\
			.wrSeq = QSBR_INITIAL,									\
			.rdSeq = QSBR_INITIAL,									\
		},															\
    }
        //IMPORTS_INIT,



#define ALIFBYTES_SIMPLE_INIT(_ch, _len) \
    { \
        ALIFVAROBJECT_HEAD_INIT(&_alifBytesType_, (_len)), \
        .hash = -1, \
        .val = { (_ch) }, \
    }
#define ALIFBYTES_CHAR_INIT(_ch) \
    { \
        ALIFBYTES_SIMPLE_INIT((_ch), 1) \
    }

#define ALIFUSTR_ASCII_BASE_INIT(_litr, ASCII) \
    { \
        .objBase = ALIFOBJECT_HEAD_INIT(&_alifUStrType_), \
        .length = sizeof(_litr) - 1, \
        .hash = -1, \
        .state = { \
            .kind = 1, \
            .compact = 1, \
            .ascii = (ASCII), \
            .staticallyAllocated = 1, \
        }, \
    }
#define ALIFASCIIOBJECT_INIT(_litr)									\
	{																\
		.ascii = ALIFUSTR_ASCII_BASE_INIT(_litr, 1),				\
		.data = _litr												\
	}
#define INIT_STR(_name, _litr) \
    .alif ## _name = ALIFASCIIOBJECT_INIT(_litr)
#define INIT_ID(_name) \
    .alif ## _name = ALIFASCIIOBJECT_INIT(#_name)
#define ALIFUSTR_LATIN1_INIT(_litr, UTF8) \
    { \
        .latin1 = { \
            .base = ALIFUSTR_ASCII_BASE_INIT((_litr), 0), \
            .utf8 = (UTF8), \
            .utf8Length = sizeof(UTF8) - 1, \
        }, \
        .data = (_litr), \
    }




















/* ------------------------------ AlifCore_DureRunInitGenerated.h ------------------------------ */



#define ALIF_STR_LITERALS_INIT { \
    INIT_STR(Empty, ""), \
}

#define ALIF_STR_IDENTIFIERS_INIT { \
    INIT_ID(CANCELLED), \
    INIT_ID(__name__), \
	INIT_ID(__doc__), \
	INIT_ID(__package__), \
	INIT_ID(__loader__), \
	INIT_ID(__spec__), \
	INIT_ID(__hash__), \
}
