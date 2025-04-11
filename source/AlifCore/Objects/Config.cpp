#include "alif.h"

extern AlifObject* alifInit_math(void); // 16
extern AlifObject* alifInit_time(void);

extern AlifObject* alifInit__io(void);



class InitTable _alifImportInitTab_[] = { // 87

	{"الرياضيات", alifInit_math}, // 96

	{"الوقت", alifInit_time},


	{"_imp", alifInit__imp},

	/* These entries are here for sys.builtin_module_names */
	{"builtins", nullptr},
	{"النظام", nullptr},


	{"تبادل", alifInit__io},

	/* Sentinel */
	{0, 0}
};
