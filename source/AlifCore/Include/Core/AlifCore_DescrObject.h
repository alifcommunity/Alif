#pragma once




class PropertyObject {
public:
	ALIFOBJECT_HEAD;
	AlifObject* propGet{};
	AlifObject* propSet{};
	AlifObject* propDel{};
	AlifObject* propDoc{};
	AlifObject* propName{};
	AlifIntT getterDoc{};
};
