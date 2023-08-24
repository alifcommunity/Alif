#include "alif.h"
//#include "alifCore_fileUtils.h"
//#include "alifCore_getOpt.h"
#include "alifCore_initConfig.h"
//#include "alifCore_interp.h"
//#include "alifCore_long.h"   
//#include "alifCore_pathConfig.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifLifeCycle.h"
//#include "alifCore_alifMem.h" 
//#include "alifCore_alifState.h"

//#include "osdefs.h"

#include <locale.h>
//#include <stdlib.h>
#if defined(MS_WINDOWS) || defined(__CYGWIN__)
#  ifdef HAVE_IO_H
#    include <io.h>
#  endif
#  ifdef HAVE_FCNTL_H
#    include <fcntl.h>
#  endif
#endif














































































































































































/* --- Global configuration variables ----------------------------- */

int alifUTF8Mode = 0;
int alifDebugFlag = 0;
int alifVerboseFlag = 0;
int alifQuietFlag = 0;
int alifInteractiveFlag = 0;
int alifInspectFlag = 0;
int alifOptimizeFlag = 0;
int alifNoSiteFlag = 0;
int alifBytesWarningFlag = 0;
int alifFrozenFlag = 0;
int alifIgnoreEnvironmentFlag = 0;
int alifDontWriteBytecodeFlag = 0;
int alifNoUserSiteDirectory = 0;
int alifUnbufferedStdioFlag = 0;
int alifHashRandomizationFlag = 0;
int alifIsolatedFlag = 0;
#ifdef MS_WINDOWS
int alifLegacyWindowsFSEncodingFlag = 0;
int alifLegacyWindowsStdioFlag = 0;
#endif
