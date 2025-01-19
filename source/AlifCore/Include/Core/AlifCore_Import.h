#pragma once








class ImportDureRunState { // 40
public:
	class InitTable* initTable{};

	AlifSizeT lastModuleIndex{};
};

class ImportState { // 63
public:
	AlifObject* modules{};

	AlifObject* importFunc{};
	//class {
	//public:
	//	AlifIntT importLevel{};
	//	AlifTimeT accumulated{};
	//	AlifIntT header{};
	//} findAndLoad;
};

// 119
/*
#define IMPORTS_INIT \
    { \
        .findAndLoad = { \
            .header = 1, \
        }, \
    }
*/


AlifSizeT alifImport_getNextModuleIndex(); // 129
extern const char* alifImport_resolveNameWithPackageContext(const char*);

extern AlifObject* alifImport_initModules(AlifInterpreter*); // 136


extern AlifIntT _alifImport_isDefaultImportFunc(AlifInterpreter*, AlifObject*); // 143

extern AlifIntT alifImport_init(); // 161
