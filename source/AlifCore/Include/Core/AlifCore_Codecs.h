#pragma once







extern AlifObject* _alifCodec_encodeText(AlifObject*, const char*, const char*); // 41



extern AlifObject* _alifCodec_decodeText(AlifObject*, const char*, const char*); // 46




class CodecsState { // 63
public:
	AlifObject* searchPath{};
	AlifObject* searchCache{};
	AlifObject* errorRegistry{};
	AlifMutex searchPathMutex{};
	AlifIntT initialized{};
};
