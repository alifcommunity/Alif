#pragma once

class AlifLongValue { // 93
public:
	uintptr_t tag{};
	uint32_t digit[1]{};
};

class AlifLongObject { // 98
public:
	ALIFOBJECT_HEAD;
	AlifLongValue longValue{};
};
