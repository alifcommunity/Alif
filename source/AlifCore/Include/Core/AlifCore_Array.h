#pragma once






// مصفوفة خاصة بالمحلل اللغوي
class AlifPArray {
public:
	void** data{};
	AlifSizeT size{};
	AlifSizeT capacity{};

	AlifPArray();
	~AlifPArray();

	bool push_back(void*& value);

	void* operator[](AlifUSizeT _index) const {
		return data[_index];
	}
};
