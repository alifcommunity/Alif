#pragma once








class InitTable {
public:
	const wchar_t* name{};
	AlifObject* (*initFunc)(void);
};