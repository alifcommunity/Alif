#pragma once

#include "Types.h";

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

using wstr = std::wstring;

const std::map<wstr, BuildInFuncType> buildInFunctions = { {L"اطبع", BTPrint} , {L"اضف", BTPush} , {L"ادخل", BTInput} };
const std::map<wstr, KeywordType> keywords_ = { {L"مرر", KTPass}, {L"توقف", KTStop}, {L"استمر", KTContinue}, {L"حذف", KTDelete}, {L"من", KTFrom}, {L"استورد", KTImport} , {L"اذا", KTIf}, {L"واذا", KTElseif}, {L"والا", KTElse}, {L"بينما", KTWhile},
                                                {L"لاجل", KTFor}, {L"في", KTIn}, {L"ارجع", KTReturn}, {L"دالة", KTFunction}, {L"صنف", KTClass}, {L"او", KTOr}, {L"و", KTAnd}, {L"ليس", KTNot}, {L"صح", KTTrue}, {L"خطا", KTFalse}, {L"عدم", KTNone} };

class Token {
public:
    TokensType type_{};
    uint32_t tokLine{}, tokStart{}, tokEnd{}, tokIndex{};

    union
    {
        KeywordType keywordType;
        BuildInFuncType buildInFunc;
        wstr* strVal;
        int64_t numVal;
    }V{};

    inline Token() {}

    inline Token(uint32_t _tokLine, uint32_t _tokStart, uint32_t _tokEnd, uint32_t _tokIndex, TokensType _type) :
        tokLine(_tokLine), tokStart(_tokStart), tokEnd(_tokEnd), tokIndex(_tokIndex), type_(_type) {}
    
    inline Token(uint32_t _tokLine, uint32_t _tokStart, uint32_t _tokEnd, uint32_t _tokIndex, TokensType _type, wstr* _strVal) :
        tokLine(_tokLine), tokStart(_tokStart), tokEnd(_tokEnd), tokIndex(_tokIndex), type_(_type) { 
        this->V.strVal = _strVal;
    }

    inline Token(uint32_t _tokLine, uint32_t _tokStart, uint32_t _tokEnd, uint32_t _tokIndex, TokensType _type, int64_t _numVal) :
        tokLine(_tokLine), tokStart(_tokStart), tokEnd(_tokEnd), tokIndex(_tokIndex), type_(_type) {
        this->V.numVal = _numVal;
    }

    inline Token(uint32_t _tokLine, uint32_t _tokStart, uint32_t _tokEnd, uint32_t _tokIndex, TokensType _type, KeywordType _keywordType) :
        tokLine(_tokLine), tokStart(_tokStart), tokEnd(_tokEnd), tokIndex(_tokIndex), type_(_type) {
        this->V.keywordType = _keywordType;
    }

    inline Token(uint32_t _tokLine, uint32_t _tokStart, uint32_t _tokEnd, uint32_t _tokIndex, TokensType _type, BuildInFuncType _buildInFunc) :
        tokLine(_tokLine), tokStart(_tokStart), tokEnd(_tokEnd), tokIndex(_tokIndex), type_(_type) {
        this->V.buildInFunc = _buildInFunc;
    }
};
