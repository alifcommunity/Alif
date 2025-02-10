#include "alif.h"

extern AlifObject* alifInit_math(void); // 16
extern AlifObject* alifInit_time(void);

class InitTable _alifImportInitTab_[] = { // 87

	{"رياضيات", alifInit_math}, // 96

	{"الزمن", alifInit_time},


	{"_imp", alifInit__imp},

	/* These entries are here for sys.builtin_module_names */
	{"builtins", nullptr},
	{"sys", nullptr},

	/* Sentinel */
	{0, 0}
};
