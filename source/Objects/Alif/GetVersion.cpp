/*
يقوم بإرجاع قيمة النسخة الحالية
بالإضافة الى المترجم المستخدم في بناء اللغة
*/

#include "alif.h"

#ifndef COMPILER
#if defined(__clang__)
#define COMPILER L"[Clang " __clang_version__ L"]"
#elif defined(__GNUC__)
#define COMPILER L"[GCC " __VERSION__ L"]"
#elif defined(__cplusplus)
#define COMPILER L"[C++]"
#else
#define COMPILER L"Unknown"
#endif
#endif

const wchar_t* alif_getCompiler()
{
	return COMPILER;
}


static wchar_t version[100]{};


const wchar_t* alif_getVersion() {
	swprintf(version, sizeof(version),
		L"%ls %ls", ALIF_VERSION, alif_getCompiler());
	return version;
}


const unsigned long alifVersion = ALIF_VERSION_HEX; // يستخرج ك بيانات تستعمل في ملفات خارج اللغة
