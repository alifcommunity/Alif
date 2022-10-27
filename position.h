#pragma once

// الموقع
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Position {
public:
    int index, lineNumber = 0;
    int16_t columnNumber = 0;
    std::wstring input_, fileName;
    wchar_t currentChar = L'\0';
    Position(){}
    Position(int index, int lineNumber, int16_t columnNumber, std::wstring fileName, std::wstring input_) :
        index(index), lineNumber(lineNumber), columnNumber(columnNumber), fileName(fileName), input_(input_){}

    void advance(wchar_t currentChar = L'\0') {
        this->index++;
        this->columnNumber++;

        if (currentChar == L'\n') {
            this->lineNumber++;
            this->columnNumber = 0;
        }
    }
};