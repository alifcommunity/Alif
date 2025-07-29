#include "alif.h"

#include "AlifParserEngine.h"



ModuleTy _alifParser_astFromString(const char* _str, AlifObject* _filename, AlifIntT _mode,
	AlifCompilerFlags* _flags, AlifASTMem* _astMem) {
	//if (alifSys_audit("compile", "yO", _str, _filename) < 0) {
	//	return nullptr;
	//}

	ModuleTy result = _alifParserEngine_runParserFromString(_str, _mode,
		_filename, _flags, _astMem);
	return result;
}
