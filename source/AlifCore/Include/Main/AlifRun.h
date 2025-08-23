#pragma once







void alifErr_print(void); // 12
void alifErr_printEx(AlifIntT);
void alifErr_display(AlifObject*, AlifObject*, AlifObject*); // 14


void alifErr_displayException(AlifObject* exc); // 17


















/* ----------------------------------------------------------------------------------------------------------------------- */




AlifObject* alifRun_stringFlags(const char*, AlifIntT, AlifObject*, AlifObject*, AlifCompilerFlags*); // 30


char* alifOS_readline(FILE*, FILE*, const char*); // 95
extern char* (*_alifOSReadlineFunctionPointer_)(FILE*, FILE*, const char*); // 96
