#pragma once

// الموقع
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Position {
public:
    int index, lineNumber, columnNumber;
    std::wstring input_, fileName;
    wchar_t currentChar;
    Position() {}
    Position(int index, int lineNumber, int columnNumber, std::wstring fileName, std::wstring input_) :
        index(index), lineNumber(lineNumber), columnNumber(columnNumber), fileName(fileName), input_(input_)
    {

    }

    void advance(wchar_t currentChar = L'\0') {
        this->index++;
        this->columnNumber++;

        if (currentChar == L'\n') {
            this->lineNumber++;
            this->columnNumber = 0;
        }
    }

    void reverse(wchar_t currentChar = L'\0') {
        this->index--;
        this->columnNumber--;

        if (currentChar == L'\n') {
            this->lineNumber--;
            this->columnNumber = 0;
        }
    }
};