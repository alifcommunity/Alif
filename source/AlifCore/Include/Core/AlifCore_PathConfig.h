#pragma once









extern AlifStatus _alifPathConfig_readGlobal(AlifConfig*); // 14
extern AlifStatus _alifPathConfig_updateGlobal(const AlifConfig* _config); // 15
extern const wchar_t* _alifPathConfig_getGlobalModuleSearchPath(void); // 16

extern AlifIntT _alifPathConfig_computeSysPath0(const AlifWStringList*, AlifObject**); // 18
