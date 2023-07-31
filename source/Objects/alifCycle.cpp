#include "alif.h"
#include "alifCore_initConfig.h"


static AlifStatus alifInit_core(const AlifConfig* _srcConfig)
{
	AlifStatus status;
	AlifConfig config;

	status = alifConfig_read(&config, 0);

done:
	return status;
}


AlifStatus alifInit_fromConfig(const AlifConfig* config)
{
	AlifStatus status{};

	status = alifInit_core(config);

	return _AlifStatus_OK();
}