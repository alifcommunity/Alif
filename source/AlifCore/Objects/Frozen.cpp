#include "alif.h"

#include "AlifCore_Import.h"













static const ModuleAlias aliases[] = { // 116
	{"_frozen_importlib", "importlib._bootstrap"},
	{"_frozen_importlib_external", "importlib._bootstrap_external"},
	{0, 0} /* aliases sentinel */
};
const ModuleAlias* _alifImportFrozenAliases_ = aliases; // 128
