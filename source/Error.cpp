#include "Error.h"

class Error {
public:
    Position positionStart, positionEnd;
    std::wstring errorName, details, fileName, input;
    Error() {}
    Error(Position positionStart, Position positionEnd, std::wstring errorName, std::wstring details, std::wstring fileName, std::wstring input) : positionStart(positionStart), positionEnd(positionEnd), errorName(errorName), details(details), fileName(fileName), input(input) {}

    std::wstring print_() {
        std::wstring result = this->errorName + L": " + this->details + L"\n";
        result += L"الملف " + fileName + L", السطر " + std::to_wstring(this->positionStart.line_ + 1);
        result += L"\n\n" + ErrorArrow().error_arrow(input, this->positionStart, this->positionEnd);

        return result;
    }
};

class SyntaxError : public Error {
public:
    SyntaxError(Position positionStart, Position positionEnd, std::wstring details, std::wstring fileName, std::wstring input) : Error(positionStart, positionEnd, L"خطأ في النسق", details, fileName, input) {
    }
};