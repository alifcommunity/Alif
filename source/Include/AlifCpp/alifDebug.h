#pragma once

ALIFAPI_DATA(int) alifDebugFlag;
ALIFAPI_DATA(int) alifVerboseFlag;
ALIFAPI_DATA(int) alifQuietFlag;
ALIFAPI_DATA(int) alifInteractiveFlag;
ALIFAPI_DATA(int) alifInspectFlag;
ALIFAPI_DATA(int) alifOptimizeFlag;
ALIFAPI_DATA(int) alifNoSiteFlag;
ALIFAPI_DATA(int) alifBytesWarningFlag;
ALIFAPI_DATA(int) alifFrozenFlag;
ALIFAPI_DATA(int) alifIgnoreEnvironmentFlag;
ALIFAPI_DATA(int) alifDontWriteBytecodeFlag;
ALIFAPI_DATA(int) alifNoUserSiteDirectory;
ALIFAPI_DATA(int) alifUnbufferedStdioFlag;
ALIFAPI_DATA(int) alifHashRandomizationFlag;
ALIFAPI_DATA(int) alifIsolatedFlag;

#ifdef MS_WINDOWS
ALIFAPI_DATA(int) alifLegacyWindowsFsEncodingFlag;
ALIFAPI_DATA(int) alifLegacyWindowsStdioFlag;
#endif
