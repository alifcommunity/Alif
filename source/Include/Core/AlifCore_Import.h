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
	struct {
		void* mutex_;
		unsigned long thread_;
		int level_;
	} lock_;
	struct {
		int importLevel;
		AlifTimeT accumulated_;
		int header_;
	} findAndLoad;
};






extern AlifIntT alifImport_init();
