

#include "alif.h"

//#include "alifCore_call.h"
//#include "alifCore_cEval.h"
//#include "alifCore_codeCS.h"
//#include "alifCore_context.h"
//#include "alifCore_dict.h"
//#include "alifCore_exceptions.h"
//#include "alifCore_fileUtils.h"
//#include "alifCore_floatObject.h"
//#include "alifCore_genObject.h"
//#include "alifCore_globalObjectsFiniGenerated.h"
#include "alifCore_import.h"
#include "alifCore_initConfig.h"
//#include "alifCore_list.h"
//#include "alifCore_long.h"
//#include "alifCore_object.h"
//#include "alifCore_pathcConfig.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifLifeCycle.h"
#include "alifCore_alifMem.h"
#include "alifCore_alifState.h"
#include "alifCore_runtime.h"
#include "alifCore_runtimeInit.h"
//#include "alifCore_setObject.h"
//#include "alifCore_sliceObject.h"
//#include "alifCore_sysModule.h"
//#include "alifCore_traceback.h"
//#include "alifCore_typeObject.h"
//#include "alifCore_typeVarObject.h"
//#include "alifCore_unicodeObject.h"
//#include "alifCore_weakRef.h"
//#include "opCode.h"

#include <locale.h>               // setlocale()
//#include <stdlib.h>

#if defined(__APPLE__)
#include <mach-o/loader.h>
#endif

#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif

#ifdef HAVE_LANGINFO_H
#  include <langinfo.h> 
#endif

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#ifdef MS_WINDOWS
#  undef BYTE
#endif







































AlifRuntimeState alifRuntime
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((section(".AlifRuntime")))
#endif
= ALIFRUNTIMESTATE_INIT(alifRuntime);
ALIFCOMP_DIAGPOP

static int runtimeInitialized = 0;

AlifStatus alifRuntime_initialize()
{






	if (runtimeInitialized) {
		return ALIFSTATUS_OK();
	}
	runtimeInitialized = 1;

	return alifRuntimeState_init(&alifRuntime);
}


































































































































































































































































































































































































































































































































































































































































































































































































AlifStatus alif_preInitializeFromAlifArgv(const AlifPreConfig* _srcConfig, const AlifArgv* _args)
{
	AlifStatus status{};

	if (_srcConfig == nullptr) {
		return ALIFSTATUS_ERR("preinitialization config is null");
	}

	//status = alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	//AlifRuntimeState* runtime = &alifRuntime;

	//if (runtime->preinitialized) {

	//	return ALIFSTATUS_OK();
	//}

	//runtime->preinitializing = 1;

	AlifPreConfig config{};

	status = alifPreConfig_initFromPreConfig(&config, _srcConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = alifPreConfig_read(&config, _args);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//status = alifPreConfig_write(&config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//runtime->preinitializing = 0;
	//runtime->preinitialized = 1;
	return ALIFSTATUS_OK();
}
