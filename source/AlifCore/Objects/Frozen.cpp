#include "alif.h"

#include "AlifCore_Import.h"



#include "FrozenModules/Importlib._bootstrap.h"
#include "FrozenModules/Importlib._bootstrap_external.h"




static const Frozen _bootstrapModules_[] = { // 69
	// _frozen_importlib
	{"_مكتبة_استيراد_مصرفة", _ALIF_M__Importlib__bootstrap, (AlifIntT)sizeof(_ALIF_M__Importlib__bootstrap), false},
	// _frozen_importlib_external
	{"_مكتبة_استيراد_خارجي_مصرفة", _ALIF_M__Importlib__bootstrap_external, (AlifIntT)sizeof(_ALIF_M__Importlib__bootstrap_external), false},
	{0, 0, 0} /* bootstrap sentinel */
};





const Frozen* _alifImportFrozenBootstrap_ = _bootstrapModules_; // 112


static const ModuleAlias aliases[] = { // 116
	{"_مكتبة_استيراد_مصرفة", "importlib._bootstrap"},
	{"_مكتبة_استيراد_خارجي_مصرفة", "importlib._bootstrap_external"},
	{0, 0} /* aliases sentinel */
};
const ModuleAlias* _alifImportFrozenAliases_ = aliases; // 128
