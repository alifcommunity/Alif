#pragma once

enum UnicodeKind {
	UNICODE_2BYTE = 2,
	UNICODE_4BYTE = 4,
};

class AlifUStrObject{
public:

	AlifObj object;

	size_t length;

	size_t hash;

	uint8_t kind;

	void* UTF;
};

extern AlifInitObject typeUnicode;

#define ALIFUNICODE_CAST(unicode) ((AlifUStrObject*)(unicode))

#define ALIF_SIZE_ROUND_DOWN(n, a) ((size_t)(n) & ~(size_t)((a) - 1))

// هنا يتم تحويل من utf16 الى utf32 والعكس 
#define ALIFUNICODE_CONVERT_BYTES(fromType, toType, begin, end, to) \
    do {                                                \
        toType *_to = (toType *)(to);                 \
        const fromType *_iter = (const fromType *)(begin);\
        const fromType *_end = (const fromType *)(end);\
        SSIZE_T n = (_end) - (_iter);                \
        const fromType *_unrolledEnd =                \
            _iter + ALIF_SIZE_ROUND_DOWN(n, 4);          \
        while (_iter < (_unrolledEnd)) {               \
            _to[0] = (toType) _iter[0];                \
            _to[1] = (toType) _iter[1];                \
            _to[2] = (toType) _iter[2];                \
            _to[3] = (toType) _iter[3];                \
            _iter += 4; _to += 4;                       \
        }                                               \
        while (_iter < (_end))                          \
            *_to++ = (toType) *_iter++;                \
    } while (0)
	


AlifUStrObject* alifUnicode_decodeStringToUTF(const wchar_t* str);
uint8_t find_MaxChar(const wchar_t* str);
size_t hash_unicode(AlifObj* unicode);





AlifObj* alifUStr_objFromWChar(wchar_t*);
