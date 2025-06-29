#include "alif.h"

#include "AlifCore_Import.h"



#include "FrozenModules/Importlib._bootstrap.h"





static const Frozen _bootstrapModules_[] = { // 69
	{"_frozen_importlib", _alif_M__importlib__bootstrap, (AlifIntT)sizeof(_alif_M__importlib__bootstrap), false},
	//{"_frozen_importlib_external", _alif_M__importlib__bootstrap_external, (AlifIntT)sizeof(_alif_M__importlib__bootstrap_external), false},
	{0, 0, 0} /* bootstrap sentinel */
};





const Frozen* _alifImportFrozenBootstrap_ = _bootstrapModules_; // 112


static const ModuleAlias aliases[] = { // 116
	{"_frozen_importlib", "importlib._bootstrap"},
	{"_frozen_importlib_external", "importlib._bootstrap_external"},
	{0, 0} /* aliases sentinel */
};
const ModuleAlias* _alifImportFrozenAliases_ = aliases; // 128
