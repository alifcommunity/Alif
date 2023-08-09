#include "alif.h"
#include "alifCore_initConfig.h"
#include "alifCore_alifMem.h"
#include "alifCore_runtime_init.h"


#ifdef HAVE_DLOPEN
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if !HAVE_DECL_RTLD_LAZY
#define RTLD_LAZY 1
#endif
#endif


AlifStatus alifRuntimeState_init(AlifRuntimeState* runtime) {




}
