#pragma once

#include <string>

#include "ErrorIndicator.h"

// أخطاء
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Error {
    uint32_t posStart, posEnd, posIndex, line_;
public:
    wstr errorName, details, fileName;
    wstr* input_;

    Error();
    Error(uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, uint32_t _line, wstr errorName, wstr details, wstr fileName, wstr* input);

    wstr print_();
};

class SyntaxError : public Error {
public:
    SyntaxError(uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, uint32_t _line, wstr details, wstr fileName, wstr* input);
};
