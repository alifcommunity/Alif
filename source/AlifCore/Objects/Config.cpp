#include "alif.h"

// هنا قمنا بإستخدام extern "C"
// يقوم بعمل إخراج لان extern للدوال بوضع التسمية الخاص بـ CPP (mangaled name)
// وذلك عند استخدام "extern AlifObject*" فقط!
// وهذا يسبب خطأ في عملية الربط.
// لا تقم بإرجاعه إلى extern AlifObject*
// إلا إذا كنت تعلم ماذا تفعل.
#ifdef __cplusplus
extern "C" {
#endif

//extern AlifObject* alifInit__abc(void); // 8
extern AlifObject* alifInit_math(void); // 16
#ifdef _WINDOWS
extern AlifObject* alifInit_nt(void); // 18
#else
extern AlifObject* alifInit_posix(void); // 18
#endif
extern AlifObject* alifInit_time(void); // 28

extern AlifObject* alifInit__codecs(void);

extern AlifObject* alifInit__random(void); // 41

extern AlifObject* alifInit__io(void);


#ifdef __cplusplus
}
#endif


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
