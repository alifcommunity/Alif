#pragma once



typedef AlifObject* (*Getter)(AlifObject*, void*); // 8
typedef AlifIntT (*Setter)(AlifObject*, AlifObject*, void*);

class AlifGetSetDef { // 11
public:
	const char* name{};
	Getter get{};
	Setter set{};
	const char* doc{};
	void* closure{};
};


extern AlifTypeObject _alifClassMethodDescrType_; // 19

extern AlifTypeObject _alifMemberDescrType_;

AlifObject* alifDescr_newMethod(AlifTypeObject*, AlifMethodDef*); // 27
AlifObject* alifDescr_newClassMethod(AlifTypeObject*, AlifMethodDef*); // 28

class AlifMemberDef { // 41
public:
	const char* name{};
	AlifIntT type{};
	AlifSizeT offset{};
	AlifIntT flags{};
};




#define ALIF_T_OBJECT   6  // 59





#define ALIF_T_OBJECT_EX 16 // 75

#define ALIF_READONLY	1 // 83








/* ------------------------------------------------------------------------------------ */



class AlifDescrObject {
public:
	ALIFOBJECT_HEAD{};
	AlifTypeObject* type{};
	AlifObject* name{};
	AlifObject* qualname{};
};


#define ALIFDESCR_COMMON AlifDescrObject common // 33

#define ALIFDESCR_TYPE(_x) (((AlifDescrObject *)(_x))->type)
#define ALIFDESCR_NAME(_x) (((AlifDescrObject *)(_x))->name)

class AlifMethodDescrObject { // 38
public:
	ALIFDESCR_COMMON{};
	AlifMethodDef* method{};
	VectorCallFunc vectorCall{};
};




AlifIntT alifDescr_isData(AlifObject*); // 62
