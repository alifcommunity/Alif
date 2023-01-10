#pragma once

// الموقع
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Position {
public:
    int index_{}, line_{}, column_{};
    wchar_t currentChar{};

    Position() {}

    Position(int _index, int _line, int _column)
    {
        this->index_ = _index;
        this->line_ = _line;
        this->column_ = _column;
    }

    void advance(wchar_t _currentChar = L'\0') {
        this->index_++;
        this->column_++;

        if (_currentChar == L'\n') {
            this->line_++;
            this->column_ = 0;
        }
    }
};
