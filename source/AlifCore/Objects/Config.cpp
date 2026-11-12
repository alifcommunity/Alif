#include "alif.h"

//* review
// هنا قمنا بإستخدام ALIFMODINIT_FUNC
// لأن استخدام extern AlifObject*
// يسبب خطأ في عملية الربط اثناء بناء اللغة
// لا تقم بإرجاعه إلى extern AlifObject*
// إلا إذا كنت تعلم ماذا تفعل.

//extern AlifObject* alifInit__abc(void); // 8
ALIFMODINIT_FUNC alifInit_math(void); // 16
#ifdef _WINDOWS
ALIFMODINIT_FUNC alifInit_nt(void); // 18
#else
ALIFMODINIT_FUNC alifInit_posix(void); // 18
#endif
ALIFMODINIT_FUNC alifInit_time(void); // 28

ALIFMODINIT_FUNC alifInit__codecs(void);

ALIFMODINIT_FUNC alifInit__random(void); // 41

ALIFMODINIT_FUNC alifInit__io(void);



class InitTable _alifImportInitTab_[] = { // 87
	//{"صنف_اساس_مجرد", alifInit__abc},

	{"الرياضيات", alifInit_math}, // 96

#ifdef _WINDOWS
	{"واجهة_نظام_ويندوز", alifInit_nt}, /* Use the NT os functions, not posix */
#else
	{"واجهة_نظام_يونكس", alifInit_posix},
#endif

	{"الوقت", alifInit_time},


	{"برمجيات_ترميز", alifInit__codecs},
	{"عشوائي", alifInit__random},

	{"_ورد", alifInit__imp}, // _imp *import support system

	/* These entries are here for النظام.اسماء_الوحدات_الضمنية */
	{"الضمنيات", nullptr},
	{"النظام", nullptr},


	{"تبادل", alifInit__io},

	/* Sentinel */
	{0, 0}
};
