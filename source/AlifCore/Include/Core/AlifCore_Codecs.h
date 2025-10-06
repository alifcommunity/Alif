#pragma once




extern AlifStatus _alifCodec_initRegistry(AlifInterpreter*); // 16

extern AlifObject* _alifCodec_lookup(const char*); // 22


extern AlifObject* _alifCodec_encodeText(AlifObject*, const char*, const char*); // 41



extern AlifObject* _alifCodec_decodeText(AlifObject*, const char*, const char*); // 46

extern AlifObject* _alifCodec_lookupTextEncoding(const char*, const char*); // 48

extern AlifObject* _alifCodecInfo_getIncrementalEncoder(AlifObject*, const char*); // 69


class CodecsState { // 74
public:
	AlifObject* searchPath{};
	AlifObject* searchCache{};
	AlifObject* errorRegistry{};
	AlifMutex searchPathMutex{};
	AlifIntT initialized{};
};
