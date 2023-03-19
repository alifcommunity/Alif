#pragma once

#include "Types.h"

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

using wcstr = const wchar_t;

class Token 
{
public:
    wcstr* value_{};
    TokensType type_{};
    uint32_t tokLine{}, posStart{}, posEnd{}, posIndex{};

    inline Token() {}

    inline Token(uint32_t _tokLine, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, TokensType _type) 
        : tokLine(_tokLine), posStart(_posStart), posEnd(_posEnd), posIndex(_posIndex), type_(_type) {}
    
    inline Token(uint32_t _tokLine, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, TokensType _type, wcstr* _value)
        : tokLine(_tokLine), posStart(_posStart), posEnd(_posEnd), posIndex(_posIndex), type_(_type), value_(_value) {}
};
