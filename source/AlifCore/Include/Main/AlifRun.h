#pragma once







ALIFAPI_FUNC(void) alifErr_print(void); // 12
ALIFAPI_FUNC(void) alifErr_printEx(AlifIntT);
ALIFAPI_FUNC(void) alifErr_display(AlifObject*, AlifObject*, AlifObject*); // 14


ALIFAPI_FUNC(void) alifErr_displayException(AlifObject* exc); // 17


















/* ----------------------------------------------------------------------------------------------------------------------- */




ALIFAPI_FUNC(AlifObject*) alifRun_stringFlags(const char*, AlifIntT,
	AlifObject*, AlifObject*, AlifCompilerFlags*); // 30


ALIFAPI_FUNC(AlifObject*) alif_compileStringExFlags(const char*, const char*, AlifIntT,
	AlifCompilerFlags*, AlifIntT); // 43
ALIFAPI_FUNC(AlifObject*) alif_compileStringObject(const char*, AlifObject*, AlifIntT,
	AlifCompilerFlags*, AlifIntT); // 49



#define ALIFRUN_STRING(_str, _s, _g, _l) alifRun_stringFlags((_str), (_s), (_g), (_l), nullptr) // 76

ALIFAPI_FUNC(char*) alifOS_readline(FILE*, FILE*, const char*); // 95
ALIFAPI_DATA(char*) (*_alifOSReadlineFunctionPointer_)(FILE*, FILE*, const char*); // 96
