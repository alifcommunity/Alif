#include "alif.h"

// هنا قمنا بإستخدام extern "C"
// لان FreezeModule تستخدم ALIF_NO_ENABLE_SHARED
// وبالتالي يقوم بعمل إخراج extern للدوال بوضع التسمية الخاص بـ CPP (mangaled name)
// وذلك عند استخدام "extern AlifObject*" فقط!
// وهذا يسبب خطأ في عملية الربط.
// لا تقم بإرجاعه إلى extern AlifObject*
// إلا إذا كنت تعلم ماذا تفعل.
#ifdef __cplusplus
extern "C" {
#endif

extern AlifObject* alifInit_nt(void);

extern AlifObject* alifInit__io(void);

#ifdef __cplusplus
}
#endif


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
