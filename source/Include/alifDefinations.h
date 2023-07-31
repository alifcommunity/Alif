#pragma once


// تعريفات الكلمات المحجوزة

#define ENDMARKER         0   // " "
#define NUMBER            1   // 123...
#define STRING            2   // "أ ب ت ..."
#define NAME              3   // "اسم"
#define INDENT            4   // مسافة طويلة
#define DEDENT            5   // مسافة راجعة
#define NEWLINE           6   // سطر
#define RPARAN            7   // )
#define LPARAN            8   // (
#define RSQUAREPARAN      9   // ]
#define LSQUAREPARAN      10  // [ 
#define COLON             11  // :
#define COMMA             12  // ,
#define PLUS              13  // +
#define MINUS             14  // -
#define SLASH             15  // "\"
#define VERTBAR           16  // |
#define AMPER             17  // &
#define LESS              18  // >
#define GREATER           19  // <
#define EQUAL             20  // =
#define DOT               21  // .
#define RBRACE            22  // }
#define LBRACE            23  // {
#define EQEQUAL           24  // ==
#define NOTEQUAL          25  // !=
#define LESSEQUAL         26  // >=
#define GREATEREQUAL      27  // <=
#define UPARROW           28  // ^
#define RIGHTSHIFT        29  // >>
#define LEFTSHIFT         30  // <<
#define PLUSEQUAL         31  // +=
#define MINEQUAL          32  // -=
#define STAREQUAL         33  // *=
#define SLASHEQUAL        34  // \=
#define AMPEREQUAL        36  // &=
#define VBAREQUAL         37  // |=
#define UPARROWEQUAL      38  // ^=
#define LEFTSHIFTEQUAL    39  // <<=
#define RIGHTSHIFTEQUAL   40  // >>=
#define DOUBLESLASH       41  // "\\"
#define DOUBLESLASHEQUAL  42  // \\=
#define RARROW            43  // >
#define EXCLAMATION       44  // !
#define OP                45  // <~~~~~~~ مراجعة
#define AWAIT             46  
#define ASYNC             47  
#define FSTRING_START     48
#define FSTRING_MIDDLE    49
#define FSTRING_END       50
#define COMMENT           51  // #
#define NL                52  // <~~~~~~~ مراجعة
#define ERRORTOKEN        53  // <~~~~~~~ مراجعة
#define N_TOKENS          54  // <~~~~~~~ مراجعة
#define NT_OFFSET         256



#define ISTERMINAL(x)           ((x) < NT_OFFSET)
#define ISNONTERMINAL(x)        ((x) >= NT_OFFSET)
#define ISEOF(x)                ((x) == ENDMARKER)
#define ISWHITESPACE(x)         ((x) == ENDMARKER || \
                                 (x) == NEWLINE   || \
                                 (x) == INDENT    || \
                                 (x) == DEDENT)
#define ISSTRINGLIT(x)          ((x) == STRING           || \
                                 (x) == FSTRING_MIDDLE)
