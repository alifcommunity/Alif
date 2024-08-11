#pragma once








class ImportDureRun {
public:
	class InitTable* initTable;


};

class ImportState {
public:
	AlifObject* modules_;

	AlifObject* modulesByIndex;
	AlifObject* importLib;

	int overrideFrozenModules;
	int overrideMultiInterpExtensionsCheck;
//#ifdef HAVE_DLOPEN
	//int dlopenflags;
//#endif
	AlifObject* importFunc;
	class {
	public:
		void* mutex_;
		long thread_;
		int level_;
	} lock_;
	class {
	public:
		int importLevel;
		AlifTimeT accumulated_;
		int header_;
	} findAndLoad;
};

#  define ALIF_DLOPEN_FLAGS 0
#  define DLOPENFLAGS_INIT

#define IMPORTS_INIT \
    { \
        nullptr, \
        nullptr, \
        nullptr, \
        0, \
        0, \
        nullptr, \
        { \
            nullptr, \
            -1, \
            0, \
        }, \
        { \
            1, \
        }, \
    }

AlifObject* alifImport_initModules(AlifInterpreter* );


extern AlifIntT alifImport_init();
