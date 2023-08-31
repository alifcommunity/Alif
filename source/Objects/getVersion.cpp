


#include "alif.h"

#include "alifVersion.h"

static int initialized = 0;
static char version[250];

void alif_initVersion(void)
{
	if (initialized) {
		return;
	}
	initialized = 1;
	//alifOSSnPrintf(version, sizeof(version), "%.80s (%.80s) %.80s", ALIF_VERSION, alif_getBuildInfo(), alif_getCompiler());
}

const char* alif_getVersion(void)
{
	alif_initVersion();
	return version;
}

const unsigned long alifVersion = ALIF_VERSION_HEX;
