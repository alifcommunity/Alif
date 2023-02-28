#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
//#include <algorithm> // لعمل تتالي على المصفوفات

#include "Tokens.h"

/*
تم تعريف متغيرات الاسماء خارج الصنف لكي لا يتم إعادة ضبطها عند 
استخدام الطرفية في تنفيذ الشفرة او عند استيراد المكتبات 
*/

static uint32_t name = 0; // متغير اسماء على شكل ارقام
static std::map<wstr, int> namesAlter{};

// المعرب اللغوي
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DedentSpecifier { // صنف يقوم بتحديد المسافة البادئة الحالية والاخيرة
public:
    int spaces = 0;
    DedentSpecifier* previous = nullptr;
};

class Lexer {
public:
    wstr fileName{}, input_{};
    wchar_t currentChar{};
    //Position position_{}, positionEnd{};
    std::vector<Token> tokens_{};
    DedentSpecifier* dedentSpec = new DedentSpecifier; // حساب المسافات البادئة والراجعة

    ////////////

    Lexer(wstr _fileName, wstr _input);

    void advance();

    void make_token();

    void skip_space();

    void make_indent();

    void make_newline();

    void make_number();

    void make_name();

    void make_string();

    void make_plus_equal();

    void make_minus_equal();

    void make_multiply_equal();

    void make_power_equal();

    void make_divide();

    void make_not_equals();

    void make_equals();

    void make_less_than();

    void make_greater_than();

    void skip_comment();
};
