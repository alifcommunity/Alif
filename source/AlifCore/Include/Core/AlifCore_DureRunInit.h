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
        .mainInterpreter = ALIF_INTERPRETERSTATE_INIT(_dureRun_.mainInterpreter),	\
	}


#define ALIF_INTERPRETERSTATE_INIT(interpreter)						\
    {																\
		.eval = {.recursionLimit = ALIF_DEFAULT_RECURSION_LIMIT},	\
    }
        //IMPORTS_INIT,
