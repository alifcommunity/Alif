#pragma once
#include "alif.h"
#include "alifCore_runtime.h"

#if defined(MS_WINDOWS)

#pragma section("alifRuntime", read, write)
__declspec(allocate("alifRuntime"))

#elif defined(__APPLE__)

__attribute__((
	section(SEG_DATA ",alifRuntime")
	))

#endif

static AlifRuntimeState runtime{};
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((section(".alifRuntime")))
#endif
// هنا تم عمل متغير يتم القراءة منه والكتابة يسمى alifRuntime 
// ومن ثم التحقق من النظام لوضع بيانات كائن AlifRuntimeState 
// ثم يتم استخراجه في ملف compiler لاحقا مع بيانات النظام كاملة

static int runtimeInitialized = 0;

AlifStatus alifInit_fromConfig(const AlifConfig*);
