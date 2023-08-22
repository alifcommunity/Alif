#pragma once






/* ----- AlifStatus ------------------------------------------- */

class AlifStatus {
public:
	enum {
		AlifStatus_Type_Ok = 0,
		AlifStatus_Type_Error = 1,
		AlifStatus_Type_EXIT = 2
	} type;
	const char* func;
	const char* errMsg;
	int exitcode;
};













#define ALIF_BUILD_CORE
































#define MS_WIN32
#define MS_WINDOWS





























































#ifdef _WIN64
#define MS_WIN64
#endif





























































#ifdef MS_WIN64
typedef __int64 AlifSizeT;
#   define ALIF_SIZE_T_MAX LLONG_MAX
#else
typedef _W64 int AlifSizeT;
#   define ALIF_SIZE_T_MAX INT_MAX
#endif





























































































#if !defined(MS_NO_COREDLL) && !defined(ALIF_NO_ENABLE_SHARED)
#       define ALIF_ENABLE_SHARED 1
#       define MS_COREDLL
#endif


#define HAVE_DECLSPEC_DLL
