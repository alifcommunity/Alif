#pragma once


class AlifHamtNode {
public:
	ALIFOBJECT_HEAD
};

class AlifHamtObject {
public:
	ALIFOBJECT_HEAD
	AlifHamtNode* root{};
	AlifObject* weakRefList{};
	AlifSizeT count{};
};


class AlifHamtNodeBitmap {
public:
	ALIFOBJECT_VAR_HEAD
		uint32_t bitmap{};
	AlifObject* array[1]{};
};
