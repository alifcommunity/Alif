#pragma once















class AlifNumberMethods {


public:

	BinaryFunc nbAdd;
	BinaryFunc nbSubtract;
	BinaryFunc nbMultiply;
	BinaryFunc nbRemainder;
	BinaryFunc nbDivmod;
	TernaryFunc nbPower;
	UnaryFunc nbNegative;
	UnaryFunc nbPositive;
	UnaryFunc nbAbsolute;
	Inquiry nbBool;
	UnaryFunc nbInvert;
	BinaryFunc nbLshift;
	BinaryFunc nbRshift;
	BinaryFunc nbAnd;
	BinaryFunc nbXor;
	BinaryFunc nbOr;
	UnaryFunc nbInt;
	void* nbReserved;  /* the slot formerly known as nb_long */
	UnaryFunc nbFloat;

	BinaryFunc nbInplaceAdd;
	BinaryFunc nbInplaceSubtract;
	BinaryFunc nbInplaceMultiply;
	BinaryFunc nbInplaceRemainder;
	TernaryFunc nbInplacePower;
	BinaryFunc nbInplaceLshift;
	BinaryFunc nbInplaceRshift;
	BinaryFunc nbInplaceAnd;
	BinaryFunc nbInplaceXor;
	BinaryFunc nbInplaceOr;

	BinaryFunc nbFloorDivide;
	BinaryFunc nbTrueDivide;
	BinaryFunc nbInplaceFloorDivide;
	BinaryFunc nbInplaceTrueDivide;

	UnaryFunc nbIndex;

	BinaryFunc nbMatrixMultiply;
	BinaryFunc nbInplaceMatrixMultiply;
};

class AlifSequenceMethods {
public:
	LenFunc sqLength;
	BinaryFunc sqConcat;
	SSizeArgFunc sqRepeat;
	SSizeArgFunc sqItem;
	void* wasSqSlice;
	SSizeObjArgProc sqAssItem;
	void* wasSqAssSlice;
	ObjObjProc sqContains;
	BinaryFunc sqInplaceConcat;
	SSizeArgFunc sqInplaceRepeat;
} ;

class AlifMappingMethods {
public:
	LenFunc mpLength;
	BinaryFunc mpSubscript;
	ObjObjArgProc mpAssSubscript;
};
typedef AlifSendResult(*SendFunc)(AlifObject* iter, AlifObject* value, AlifObject** result);

class AlifAsyncMethods {
public:
	UnaryFunc amAwait;
	UnaryFunc amAiter;
	UnaryFunc amAnext;
	SendFunc amSend;
} ;

class AlifBufferProcs {
public:
	GetBufferProc bf_getbuffer;
	ReleaseBufferProc bf_releasebuffer;
};





class TypeObject {
public:
	ALIFObject_VAR_HEAD
	const char* tpName; /* printing in format "<module>.<name>" */
	AlifSizeT tpBasicSize, tpItemSize; 



	Destructor tpDealloc;
	AlifSizeT tpVectorCallOffset;
	GetAttrFunc tpGetAttr;
	SetAttrFunc tpSetAttr;
	AlifAsyncMethods* tpAsAsync;

	ReprFunc tpRepr;



	AlifNumberMethods* tpAsNumber;
	AlifSequenceMethods* tpAsSequence;
	AlifMappingMethods* tpAsMapping;



	HashFunc tpHash;
	TernaryFunc tpCall;
	ReprFunc tpStr;
	GetAttroFunc tpGetAttro;
	SetAttroFunc tpSetAttro;


	AlifBufferProcs* tpAsBuffer;


	unsigned long tpFlags;

	const char* tpDoc;



	TraverseProc tpTraverse;


	Inquiry tpClear;



	RichCmpFunc tpRichCompare;


	AlifSizeT tpWeakListOffset;


	GetIterFunc tpIter;
	IterNextFunc tpIterNext;


	//AlifMethodDef* tpMethods;
	//AlifMemberDef* tpMembers;
	//AlifGetSetDef* tpGetSet;

	AlifTypeObject* tpBase;
	AlifObject* tpDict;
	DescrGetFunc tpDescrGet;
	DescrGetFunc tpDescrSet;
	AlifSizeT tpDictOffset;
	InitProc tpInit;
	AllocFunc tpAlloc;
	NewFunc tpNew;
	FreeFunc tpNree;
	Inquiry tpIs_Gc;
	AlifObject* tpBases;
	AlifObject* tpMro; 
	AlifObject* tpCache; 
	void* tpSubClasses;  
	AlifObject* tpWeakList; 
	Destructor tpDel;


	unsigned int tpVersionTag;

	Destructor tpFinalize;
	VectorCallFunc tpVectorCall;


	unsigned char tpWatched;
};
