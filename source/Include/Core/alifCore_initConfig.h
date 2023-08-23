#pragma once












/* ----- AlifStatus ------------------------------------------- */


#ifdef _MSC_VER

#  define ALIFSTATUS_GET_FUNC() __FUNCTION__
#else
#  define ALIFSTATUS_GET_FUNC() __func__
#endif

#define ALIFSTATUS_OK(status) {.type = 0,} // Ok = 0, Error = 1, Exit = 2

#define ALIFSTATUS_ERR(ERR_MSG) { \
        .type = 1, \
        .func = ALIFSTATUS_GET_FUNC(), \
        .errMsg = (ERR_MSG)} // Ok = 0, Error = 1, Exit = 2

#define ALIFSTATUS_NO_MEMORY() ALIFSTATUS_ERR("memory allocation failed")
#define ALIFSTATUS_EXIT(EXITCODE) { \
        .type = 2, \
        .exitcode = (EXITCODE)} // Ok = 0, Error = 1, Exit = 2
#define ALIFSTATUS_IS_ERROR(err) (err.type == 1) // Ok = 0, Error = 1, Exit = 2
#define ALIFSTATUS_IS_EXIT(err) (err.type == 2) // Ok = 0, Error = 1, Exit = 2
#define ALIFSTATUS_EXCEPTION(err) (err.type != 0) // Ok = 0, Error = 1, Exit = 2
#define ALIFSTATUS_UPDATE_FUNC(err) do { err.func = ALIFSTATUS_GET_FUNC(); } while (0)























/* ----- AlifArgv ------------------------------------------- */

class AlifArgv {
public:
	AlifSizeT argc;
	int useBytesArgv;
	char* const* bytesArgv;
	wchar_t* const* wcharArgv;
};




















































/* --- AlifPreConfig ------------------------------------------- */


ALIFAPI_FUNC(void) alifPreConfig_initCompatConfig(AlifPreConfig*);














/* ----- AlifConfig ------------------------------------------- */

enum AlifConfigInitEnum {
	AlifConfig_Init_Compat = 1,
	AlifConfig_Init_Alif = 2,
	AlifConfig_Init_Isolated = 3
};
