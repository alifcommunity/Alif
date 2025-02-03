#include "alif.h"

#include "AlifCore_InitConfig.h"

#include "AlifCore_Memory.h"


#include "OSDefs.h"



class AlifPathConfig { // 23
public:
	wchar_t* programFullPath{};
	wchar_t* prefix{};
	wchar_t* execPrefix{};
	wchar_t* stdlibDir{};
	wchar_t* moduleSearchPath{};
	wchar_t* calculatedModuleSearchPath{};
	wchar_t* programName{};
	wchar_t* home{};
	AlifIntT isAlifBuild{};
};

#  define ALIFPATHCONFIG_INIT \
      {.moduleSearchPath = nullptr, .isAlifBuild = 0}


AlifPathConfig _alifPathConfig_ = ALIFPATHCONFIG_INIT;






AlifIntT _alifPathConfig_updateGlobal(const AlifConfig* _config) { // 126

#define COPY(_attr) \
    do { \
        if (_config->_attr) { \
            alifMem_dataFree(_alifPathConfig_._attr); \
            _alifPathConfig_._attr = alifMem_wcsDup(_config->_attr); \
            if (!_alifPathConfig_._attr) goto error; \
        } \
    } while (0)

#define COPY2(_attr, _srcAttr) \
    do { \
        if (_config->_srcAttr) { \
            alifMem_dataFree(_alifPathConfig_._attr); \
            _alifPathConfig_._attr = alifMem_wcsDup(_config->_srcAttr); \
            if (!_alifPathConfig_._attr) goto error; \
        } \
    } while (0)

#define COPY_INT(_attr) \
    do { \
        if (_config->_attr > 0) { \
            _alifPathConfig_._attr = _config->_attr; \
        } \
    } while (0)

	//COPY(prefix);
	//COPY(execPrefix);
	//COPY(stdlibDir);
	COPY(programName);
	//COPY(home);
	//COPY2(programFullPath, executable);
	//COPY_INT(isAlifBuild);
#undef COPY
#undef COPY2
#undef COPY_INT

	alifMem_dataFree(_alifPathConfig_.moduleSearchPath);
	_alifPathConfig_.moduleSearchPath = nullptr;
	alifMem_dataFree(_alifPathConfig_.calculatedModuleSearchPath);
	_alifPathConfig_.calculatedModuleSearchPath = nullptr;

	do {
		AlifUSizeT cch = 1;
		for (AlifSizeT i = 0; i < _config->moduleSearchPaths.length; ++i) {
			cch += 1 + wcslen(_config->moduleSearchPaths.items[i]);
		}

		wchar_t* path = (wchar_t*)alifMem_dataAlloc(sizeof(wchar_t) * cch);
		if (!path) {
			goto error;
		}
		wchar_t* p = path;
		for (AlifSizeT i = 0; i < _config->moduleSearchPaths.length; ++i) {
			wcscpy(p, _config->moduleSearchPaths.items[i]);
			p = wcschr(p, L'\0');
			*p++ = DELIM;
			*p = L'\0';
		}

		do {
			*p = L'\0';
		} while (p != path && *--p == DELIM);
		_alifPathConfig_.calculatedModuleSearchPath = path;
	} while (0);

	return 1;

error:
	return -1;
}
