#pragma once

#include <string>

#include "ErrorIndicator.h"

// أخطاء
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

using wstr = std::wstring;

class Error {
    //Position positionStart, positionEnd;
    uint32_t posStart, posEnd, posIndex, line_;
public:
    wstr errorName, details, fileName, input;
    Error() {}
    Error(Position positionStart, Position positionEnd, wstr errorName, wstr details, wstr fileName, wstr input) : 
        positionStart(positionStart), positionEnd(positionEnd), errorName(errorName), details(details), fileName(fileName), input(input) {}

    wstr print_() {
        wstr result = this->errorName + L": " + this->details + L"\n";
        result += L"الملف " + fileName + L", السطر " + std::to_wstring(this->positionStart.line_ + 1);
        result += L"\n\n" + ErrorArrow().error_arrow(input, this->positionStart, this->positionEnd);

        return result;
    }
};

class SyntaxError : public Error {
public:
    SyntaxError(uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, uint32_t _line, wstr details, wstr fileName, wstr &input) : 
        Error(_posStart, _posEnd, _posIndex, _line, L"خطأ في النسق", details, fileName, &input) {}
};
