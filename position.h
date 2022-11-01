#pragma once

// الموقع
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Position {
public:
    int index, lineNumber;
    int16_t columnNumber;
    wchar_t currentChar;
    Position(int index = -1, int lineNumber = 0, int16_t columnNumber = -1) : index(index), lineNumber(lineNumber), columnNumber(columnNumber), currentChar(L'\0') {}

    void advance(wchar_t currentChar = L'\0') {
        this->index++;
        this->columnNumber++;

        if (currentChar == L'\n') {
            this->lineNumber++;
            this->columnNumber = 0;
        }
    }
};

// هل يفضل وضع قيم افتراضية للمتغيرات ام لا؟