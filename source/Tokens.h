#pragma once

#include <map>

#include "Types.h"
#include "Values.h"

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

using wstr = std::wstring;
using double64_t = long double;

const std::map<wstr, BuildInFuncValue> buildInFunctions = { {L"اطبع", BVPrint} , {L"اضف", BVPush} , {L"ادخل", BVInput} };
const std::map<wstr, KeywordValue> keywords_ = { {L"مرر", KVPass}, {L"توقف", KVStop}, {L"استمر", KVContinue}, {L"حذف", KVDelete}, {L"من", KVFrom}, {L"استورد", KVImport} , {L"اذا", KVIf}, {L"واذا", KVElseif}, {L"والا", KVElse}, {L"بينما", KVWhile},
                                                {L"لاجل", KVFor}, {L"في", KVIn}, {L"ارجع", KVReturn}, {L"دالة", KVFunction}, {L"صنف", KVClass}, {L"او", KVOr}, {L"و", KVAnd}, {L"ليس", KVNot}, {L"صح", KVTrue}, {L"خطا", KVFalse}, {L"عدم", KVNone} };

class Token 
{
public:
    TokensType type_{};
    uint32_t tokLine{}, posStart{}, posEnd{}, posIndex{};

    union
    {
        KeywordValue keywordType;
        BuildInFuncValue buildInFunc;
        wstr* strVal;
        double64_t numVal;
    }V{};

    inline Token() {}

    inline Token(uint32_t _tokLine, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, TokensType _type) :
        tokLine(_tokLine), posStart(_posStart), posEnd(_posEnd), posIndex(_posIndex), type_(_type) {}
    
    inline Token(uint32_t _tokLine, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, TokensType _type, wstr* _strVal) :
        tokLine(_tokLine), posStart(_posStart), posEnd(_posEnd), posIndex(_posIndex), type_(_type) { 
        this->V.strVal = _strVal;
    }

    inline Token(uint32_t _tokLine, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, TokensType _type, double64_t _numVal) :
        tokLine(_tokLine), posStart(_posStart), posEnd(_posEnd), posIndex(_posIndex), type_(_type) {
        this->V.numVal = _numVal;
    }

    inline Token(uint32_t _tokLine, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, TokensType _type, KeywordValue _keywordType) :
        tokLine(_tokLine), posStart(_posStart), posEnd(_posEnd), posIndex(_posIndex), type_(_type) {
        this->V.keywordType = _keywordType;
    }

    inline Token(uint32_t _tokLine, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, TokensType _type, BuildInFuncValue _buildInFunc) :
        tokLine(_tokLine), posStart(_posStart), posEnd(_posEnd), posIndex(_posIndex), type_(_type) {
        this->V.buildInFunc = _buildInFunc;
    }
};
