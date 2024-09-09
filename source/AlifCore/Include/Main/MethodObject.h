#pragma once







typedef AlifObject* (*AlifCPPFunction)(AlifObject*, AlifObject*); // 19

// 52
#define ALIF_CPPFUNCTION_CAST(_func) \
    ALIF_CAST(AlifCPPFunction, ALIF_CAST(void(*)(void), (_func)))


class AlifMethodDef { // 59
public:
	const char* name{};
	AlifCPPFunction method{};
	AlifIntT flags{};
};







#define METHOD_KEYWORDS 0x0002 // 87


#define METH_CLASS    0x0010 // 95
#define METH_STATIC   0x0020


#define METHOD_FASTCALL 0x0080 // 106
