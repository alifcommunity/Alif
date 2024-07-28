#pragma once






AlifObject* alifImport_addModuleRef(const wchar_t* );

class InitTable {
public:
	const wchar_t* name{};
	AlifObject* (*initFunc)(void);
};
