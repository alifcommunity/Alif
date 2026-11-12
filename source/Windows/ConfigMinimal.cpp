#include "alif.h"

//* review
// هنا قمنا بإستخدام ALIFMODINIT_FUNC
// لأن استخدام extern AlifObject*
// يسبب خطأ في عملية الربط اثناء بناء اللغة
// لا تقم بإرجاعه إلى extern AlifObject*
// إلا إذا كنت تعلم ماذا تفعل.

ALIFMODINIT_FUNC alifInit_nt(void);

ALIFMODINIT_FUNC alifInit__io(void);



class InitTable _alifImportInitTab_[] = { // 87

	{"واجهة_نظام_ويندوز", alifInit_nt}, /* Use the NT os functions, not posix */


	{"_ورد", alifInit__imp},

	/* These entries are here for النظام.اسماء_الوحدات_الضمنية */
	{"الضمنيات", nullptr},
	{"النظام", nullptr},


	{"تبادل", alifInit__io},

	/* Sentinel */
	{0, 0}
};
