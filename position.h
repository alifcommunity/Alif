#pragma once

// الموقع
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Position {
public:
    int index_{}, line_{}, column_{};
    wchar_t currentChar{};

    Position(int _index = -1, int _line = 0, int _column = -1)
    {
        this->index_ = _index;
        this->line_ = _line;
        this->column_ = _column;
    }

    void advance(wchar_t currentChar = L'\0') {
        this->index_++;
        this->column_++;

        if (currentChar == L'\n') {
            this->line_++;
            this->column_ = 0;
        }
    }
};
