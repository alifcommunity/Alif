/* Implements the GetPath API for compiling with no functionality */

#include "alif.h"
#include "AlifCore_PathConfig.h"

AlifStatus _alifConfig_initPathConfig(AlifConfig* _config,
	AlifIntT _computePathConfig) {
	return alifStatus_error("تكوين المسار غير مدعوم هنا");
}
