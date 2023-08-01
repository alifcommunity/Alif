#include "alif.h"
#include "alifCore_initConfig.h"


static AlifStatus alifInit_core(const AlifConfig* _srcConfig)
{
	AlifStatus status;
	AlifConfig config = *_srcConfig;

	status = alifConfig_read(&config);

	return status;
}


AlifStatus alifInit_fromConfig(const AlifConfig* config)
{
	AlifStatus status{};

	status = alifInit_core(config);

	return ALIFSTATUS_OK();
}
