#pragma once

class ErrorArrow {
public:
    std::wstring error_arrow(std::wstring input_, Position positionStart, Position positionEnd) {
        std::wstring line, result;
        int indexStart, indexEnd, columnEnd;

        indexStart = input_.rfind(L"\n", positionStart.index_);
        indexEnd = input_.find(L"\n", positionEnd.index_);
        columnEnd = positionEnd.column_;

        line = input_.substr(indexStart + 1, indexEnd - indexStart);

        result += line;
        result += std::wstring(columnEnd, ' ') + std::wstring(1, '^');

        replace(result.begin(), result.end(), '\t', ' ');
        return result;
    }
};
