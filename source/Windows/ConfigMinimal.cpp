#include "alif.h"



extern AlifObject* alifInit__io(void);



class InitTable _alifImportInitTab_[] = { // 87
	{"_imp", alifInit__imp},

	/* These entries are here for sys.builtin_module_names */
	{"builtins", nullptr},
	{"النظام", nullptr},


	{"تبادل", alifInit__io},

	/* Sentinel */
	{0, 0}
};
