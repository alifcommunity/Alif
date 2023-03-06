#pragma once

#include <algorithm> // لاستخدام دالة replace()

using wstr = std::wstring;

/*
    يقوم محدد الخطأ بفرز السطر الذي يحتوي على الخطأ مع السطر الجديد الخاص بالسطر
    ثم يقوم بإنشاء سهم ليشير الى المكان الذي ظهر فيه الخطأ
*/

class ErrorIndicator {
public:
    wstr error_arrow(wstr* _input, uint32_t _posStart, uint32_t _posEnd, uint32_t _posIndex, uint32_t _line) {
        wstr line_, result_;
        uint32_t indexStart, indexEnd;

        /*
            .rfind
            تقوم بالبحث عن الحرف من الموقع المعطى وما قبل حتى يجده يرجع موقعه
            .find
            تقوم بالبحث عن الحرف من الموقع المعطى وما بعد حتى يجده فيرجع موقعه
        */

        _line == 1 ? indexStart = 0 : indexStart = _input->rfind(L'\n', _posIndex);

        indexEnd = _input->find(L'\n', _posIndex);

        line_ = _input->substr(indexStart, indexEnd - indexStart + 1); // يقوم بإقتطاع النص بين موقعين

        result_ += line_;
        result_ += wstr(_posEnd, L' ') + wstr(1, L'^');
        replace(result_.begin(), result_.end(), L'\t', L' '); // يجب استبدال كل مسافة طويلة بمسافة لان المركب اللغوي يعتبر المسافة الطويلة مسافة فقط

        return result_;
    }
};
