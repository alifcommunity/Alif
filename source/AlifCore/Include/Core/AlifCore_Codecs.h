#pragma once







extern AlifObject* _alifCodec_encodeText(AlifObject*, const char*, const char*); // 41



extern AlifObject* _alifCodec_decodeText(AlifObject*, const char*, const char*); // 46

extern AlifObject* _alifCodecInfo_getIncrementalEncoder(AlifObject*, const char*); // 69


class CodecsState { // 74
public:
	AlifObject* searchPath{};
	AlifObject* searchCache{};
	AlifObject* errorRegistry{};
	AlifMutex searchPathMutex{};
	AlifIntT initialized{};
};
