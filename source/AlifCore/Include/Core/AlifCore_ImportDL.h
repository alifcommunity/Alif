#pragma once




extern const char* _alifImportDynLoadFiletab_[]; // 15


enum AlifExtModuleKind { // 18
	Alif_Ext_Module_Kind_UNKNOWN = 0,
	Alif_Ext_Module_Kind_SINGLEPHASE = 1,
	Alif_Ext_Module_Kind_MULTIPHASE = 2,
	Alif_Ext_Module_Kind_INVALID = 3,
};

enum AlifExtModuleOrigin { // 25
	Alif_Ext_Module_Origin_CORE = 1,
	Alif_Ext_Module_Origin_BUILTIN = 2,
	Alif_Ext_Module_Origin_DYNAMIC = 3,
};


class AlifExtModuleLoaderInfo { // 32
public:
	AlifObject* filename{};
#ifndef _WINDOWS
	AlifObject* filenameEncoded{};
#endif
	AlifObject* name{};
	AlifObject* nameEncoded{};
	AlifObject* path{};
	AlifExtModuleOrigin origin{};
	const char* hookPrefix{};
	const char* newcontext{};
};



extern void _alifExtModule_loaderInfoClear(AlifExtModuleLoaderInfo*); // 46


extern AlifIntT
_alifExtModule_loaderInfoInitForBuiltin(AlifExtModuleLoaderInfo*, AlifObject*); // 56

#ifdef HAVE_DYNAMIC_LOADING
extern AlifIntT _alifExtModule_loaderInfoInitFromSpec(
	class AlifExtModuleLoaderInfo*,
	AlifObject*); // 60
#endif

class AlifExtModuleLoaderResult { // 66
public:
	AlifModuleDef* def{};
	AlifObject* module{};
	AlifExtModuleKind kind{};
	class AlifExtModuleLoaderResultError {
	public:
		enum AlifExtModuleLoaderResultErrorKind {
			Alif_Ext_Module_Loader_Result_EXCEPTION = 0,
			Alif_Ext_Module_Loader_Result_ERR_MISSING = 1,
			Alif_Ext_Module_Loader_Result_ERR_UNREPORTED_EXC = 2,
			Alif_Ext_Module_Loader_Result_ERR_UNINITIALIZED = 3,
			Alif_Ext_Module_Loader_Result_ERR_NONASCII_NOT_MULTIPHASE = 4,
			Alif_Ext_Module_Loader_Result_ERR_NOT_MODULE = 5,
			Alif_Ext_Module_Loader_Result_ERR_MISSING_DEF = 6,
		} kind{};
		AlifObject* exc{};
	} err_{};
	AlifExtModuleLoaderResultError* err{};
};


extern void _alifExtModule_loaderResultClear(AlifExtModuleLoaderResult*); // 84

typedef AlifObject* (*AlifModInitFunction)(void); // 91
#ifdef HAVE_DYNAMIC_LOADING
extern AlifModInitFunction _alifImport_getModInitFunc(
	class AlifExtModuleLoaderInfo*, FILE*); // 92
#endif

extern AlifIntT _alifImport_runModInitFunc(AlifModInitFunction,
	AlifExtModuleLoaderInfo*, AlifExtModuleLoaderResult*); // 97



// 106
#ifdef _WINDOWS
#include <windows.h>
typedef FARPROC DLFuncPtr;


#else
typedef void (*DLFuncPtr)(void);
#endif
// 133


//#define ALIFD_TAGGED_SUFFIX ALIFD_DEBUG_SUFFIX "." ALIFD_SOABI ".alifd"
//#define ALIFD_UNTAGGED_SUFFIX ALIFD_DEBUG_SUFFIX ".alifd"
#define ALIFD_UNTAGGED_SUFFIX ".alifd" //* alif
