#include "alif.h"


extern AlifObject* alifInit_time(void);

class InitTable _alifImportInitTab_[] = { // 87


	{"الزمن", alifInit_time},


	{"_imp", alifInit__imp},

	/* These entries are here for sys.builtin_module_names */
	{"builtins", nullptr},
	{"sys", nullptr},

	/* Sentinel */
	{0, 0}
};
