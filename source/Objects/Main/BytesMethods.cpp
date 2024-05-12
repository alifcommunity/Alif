#include "alif.h"


AlifObject* is_arabic_chars(const char* ptr, int64_t len) {
    const char* end = ptr + len;

    while (ptr < end) {
        if ((*ptr & 0x80) == 0x00) {
            return ALIF_FALSE;
        }
        else if ((*ptr & 0xE0) == 0xC0) {
            return ALIF_FALSE;
        }
        else if ((*ptr & 0xF0) == 0xE0) {
            char secondChar = *(ptr + 1);
            char thirdChar = *(ptr + 2);

            if ((secondChar & 0xC0) != 0x80 || (thirdChar & 0xC0) != 0x80) {
                return ALIF_FALSE;
            }

            uint32_t codePoint = ((*ptr & 0x0F) << 12) | ((secondChar & 0x3F) << 6) | (thirdChar & 0x3F);

            if (codePoint < 0x0621 || (codePoint > 0x064A && codePoint < 0x0660) ||
                (codePoint > 0x0669 && codePoint < 0x066E) || codePoint > 0x06D3) {
                return ALIF_FALSE;
            }

            ptr += 3;
        }
        else {
            return ALIF_FALSE;
        }
    }

    return ALIF_TRUE;
}

AlifObject* alif_chars_isDigit(const char* Ptr, int64_t length)
{
    const unsigned char* p
        = (const unsigned char*)Ptr;
    const unsigned char* e;

    if (length == 1 && ALIF_ISDIGIT(*p))
        return ALIF_TRUE;

    if (length == 0)
        return ALIF_FALSE;

    e = p + length;
    for (; p < e; p++) {
        if (!ALIF_ISDIGIT(*p))
            return ALIF_FALSE;
    }
    return ALIF_TRUE;
}

AlifObject* alif_chars_makeTrans(AlifBuffer* from, AlifBuffer* to)
{
    int64_t i;
    wchar_t* p;

    if (from->len != to->len) {
        std::wcout << L"يجب ان تكون وسيطات (جعل عبر) نفس الطول  \n" << std::endl; // بين القوسين هي اسم الفانكشن 
        exit(-1);
    }
    AlifObject *res = alifBytes_fromStringAndSize(nullptr, 256);
    if (!res)
        return nullptr;
    p = ((AlifWBytesObject*)res)->value;
    for (i = 0; i < 256; i++)
        p[i] = (char)i;
    for (i = 0; i < from->len; i++) {
        p[((unsigned char*)from->buf)[i]] = ((char*)to->buf)[i];
    }

    return res;
}