#pragma once


#include "AlifCore_GlobalString.h"









// 27
#define ALIF_GLOBAL_OBJECT(_name) \
    _alifDureRun_.staticObjects._name
#define ALIF_SINGLETON(_name) \
    ALIF_GLOBAL_OBJECT(singletons._name)













class AlifStaticObjects { // 37
public:
	class {
	public:
		//AlifLongObject smallInts[ALIF_NSMALLNEGINTS + ALIF_NSMALLPOSINTS];

		//AlifBytesObject bytesEmpty;
		//struct {
		//	AlifBytesObject ob;
		//	char eos;
		//} bytesCharacters[256];

		AlifGlobalStrings strings;

		//AlifTupleObject tupleEmpty;
	} singletons;
};
