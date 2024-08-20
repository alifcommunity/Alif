#pragma once








class ImportDureRun { // 40
public:
	class InitTable* initTable;


};

class ImportState { // 63
public:
	//AlifObject* modules_;

	//AlifObject* modulesByIndex;
	//AlifObject* importLib;

	//int overrideFrozenModules;
	//int overrideMultiInterpExtensionsCheck;
	//AlifObject* importFunc;
	//class {
	//public:
	//	void* mutex_;
	//	long thread_;
	//	int level_;
	//} lock_;
	//class {
	//public:
	//	int importLevel;
	//	AlifTimeT accumulated_;
	//	int header_;
	//} findAndLoad;
};

// 119
#define IMPORTS_INIT \
    {	\
		\
    }

//extern AlifObject* alifImport_initModules(AlifInterpreter*); // 136


//extern AlifIntT alifImport_init(); // 161
