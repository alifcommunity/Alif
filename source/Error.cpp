#include "Error.h"


Error::Error(uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, uint32_t _line, wstr errorName, wstr details, wstr fileName, wstr* input) :
    posStart(_posStart), posEnd(_posEnd), posIndex(_posIndex), line_(_line), errorName(errorName), details(details), fileName(fileName), input_(input) {}

wstr Error::print_() {
    wstr result = this->errorName + L": " + this->details + L"\n";
    result += L"الملف " + fileName + L", السطر " + std::to_wstring(line_);
    result += L"\n\n" + ErrorIndicator().error_arrow(input_, posStart, posEnd, posIndex, line_);

    return result;
}

SyntaxError::SyntaxError(uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, uint32_t _line, wstr details, wstr fileName, wstr* input) :
        Error(_posStart, _posEnd, _posIndex, _line, L"خطأ في النسق", details, fileName, input) {}