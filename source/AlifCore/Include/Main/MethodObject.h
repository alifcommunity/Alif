#pragma once







typedef AlifObject* (*AlifCPPFunction)(AlifObject*, AlifObject*); // 19



class AlifMethod { // 59
public:
	const char* name{};
	AlifCPPFunction method{};
	AlifIntT flags{};
};
