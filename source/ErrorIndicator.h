#pragma once

using wstr = std::wstring;

class ErrorIndicator {
public:
    wstr error_arrow(wstr* _input, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, uint32_t _line) {
        wstr line_, result_;
        uint32_t indexStart, indexEnd, columnEnd;


        if (_line == 1)
        {
            indexStart = 0;
        }
        else
        {
            indexStart = _input->rfind(L"\n", _posIndex); // يقوم بالبحث عن سطر جديد من _posIndex وما قبل
        }

        indexEnd = _input->find(L"\n", _posIndex); // يقوم بالبحث عن سطر جديد من _posIndex وما بعد

        line_ = _input->substr(indexStart, indexEnd - indexStart + 1);

        result_ += line_;
        result_ += wstr(_posEnd, ' ') + wstr(1, '^');

        return result_;
    }
};
