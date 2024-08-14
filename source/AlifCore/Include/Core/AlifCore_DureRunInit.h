#pragma once

//#include "AlifCore_Import.h"

#define ALIF_DURERUNSTATE_INIT(dureRun)							\
	{															\
        .selfInitialized = 0,									\
		.mainThread = 0,										\
		.threads = ALIFTHREAD_DURERUN_INIT(dureRun.threads),	\
		.autoTSSKey = 0,										\
		.trashTSSKey = 0,										\
	}
  //      ALIF_INTERPRETERSTATE_INIT(_dureRun_.mainInterpreter),  \


#define ALIF_INTERPRETERSTATE_INIT(interpreter)					\
    {															\
    }
        //IMPORTS_INIT,    \
