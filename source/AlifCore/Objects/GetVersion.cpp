#include "alif.h"

/*
يقوم بإرجاع قيمة النسخة الحالية
بالإضافة الى المترجم المستخدم في بناء اللغة
*/


#ifndef COMPILER
#if defined(__clang__)
#define COMPILER "[Clang " __clang_version__ "]"
#elif defined(__GNUC__)
#define COMPILER "[GCC " __VERSION__ "]"
#elif defined(__cplusplus)
#define COMPILER "[C++]"
#else
#define COMPILER "غير معرف"
#endif
#endif

static const char* alif_getCompiler() { return COMPILER; }

static char version[100]{};

const char* alif_getVersion() {
	sprintf(version, "%s %s\n", ALIF_VERSION, alif_getCompiler());
	return version;
}

// يستخرج ك بيانات تستعمل في ملفات خارج اللغة
const unsigned long alifVersion = ALIF_VERSION_HEX;
