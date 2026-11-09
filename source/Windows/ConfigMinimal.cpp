#include "alif.h"

// هنا قمنا بإستخدام ALIFMODINIT_FUNC
// لان FreezeModule تستخدم ALIF_NO_ENABLE_SHARED
// وبالتالي يقوم بعمل إخراج extern للدوال بوضع التسمية الخاص بـ CPP (mangaled name)
// وذلك عند استخدام "extern AlifObject*" فقط!
// وهذا يسبب خطأ في عملية الربط.
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
