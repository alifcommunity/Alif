#pragma once


extern AlifTypeObject _alifDictType_; // 15

AlifObject* alifDict_new(); // 21





/* ---------------------------------------------------------------------------------------------------------------- */











typedef class DictKeysObject AlifDictKeysObject; // 5
typedef class DictValues AlifDictValues; // 6


class AlifDictObject { // 11
public:
	ALIFOBJECT_HEAD{};

	AlifSizeT used{};
	uint64_t versionTag{};

	AlifDictKeysObject* keys{};

	AlifDictValues* values{};
};


