#pragma once












/* ----- AlifStatus ------------------------------------------- */


#ifdef _MSC_VER

#  define ALIFSTATUS_GET_FUNC() __FUNCTION__
#else
#  define ALIFSTATUS_GET_FUNC() __func__
#endif

#define ALIFSTATUS_OK() {.type = AlifStatus::AlifStatus_Type_Ok,}

#define ALIFSTATUS_ERR(msg) { \
        .type = AlifStatus::AlifStatus_Type_Error, \
        .func = ALIFSTATUS_GET_FUNC(), \
        .errMsg = (msg)}

#define ALIFSTATUS_NO_MEMORY() ALIFSTATUS_ERR("memory allocation failed")
#define ALIFSTATUS_EXIT(exit) { \
        .type = AlifStatus::AlifStatus_Type_Exit, \
        .exitcode = (exit)}
#define ALIFSTATUS_IS_ERROR(err) (err.type == AlifStatus::AlifStatus_Type_Error)
#define ALIFSTATUS_IS_EXIT(err) (err.type == AlifStatus::AlifStatus_Type_Exit)
#define ALIFSTATUS_EXCEPTION(err) (err.type != AlifStatus::AlifStatus_Type_Ok)
#define ALIFSTATUS_UPDATE_FUNC(err) do { err.func = ALIFSTATUS_GET_FUNC(); } while (0)























/* ----- AlifArgv ------------------------------------------- */

class AlifArgv {
public:
	AlifSizeT argc;
	int useBytesArgv;
	char* const* bytesArgv;
	wchar_t* const* wcharArgv;
};




















































/* ----- AlifPreConfig ------------------------------------------- */


ALIFAPI_FUNC(void) alifPreConfig_initCompatConfig(AlifPreConfig*);



extern AlifStatus alifPreConfig_initFromPreConfig(AlifPreConfig*,const AlifPreConfig*);





extern AlifStatus alifPreConfig_read(AlifPreConfig*,const AlifArgv*);

extern AlifStatus alifPreConfig_write(const AlifPreConfig*);


/* ----- AlifConfig ------------------------------------------- */

enum AlifConfigInitEnum {
	AlifConfig_Init_Compat = 1,
	AlifConfig_Init_Alif = 2,
	AlifConfig_Init_Isolated = 3
};
