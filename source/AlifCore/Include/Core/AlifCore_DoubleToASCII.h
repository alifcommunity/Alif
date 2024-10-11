#pragma once




typedef uint32_t ULong; // 14


struct Bigint { // 16
	struct Bigint* next;
	int k, maxwds, sign, wds;
	ULong x[1];
};






extern double _alif_dgStrToDouble(const char*, char**); // 62
