#pragma once

#define PRINT_(a){std::wcout << a << std::endl;}

#include <iostream>
#include <string>
#include <vector>

#include "Constants.h"
#include "Tokens.h"
#include "Error.h"
#include "MemoryBlock.h"

using wstr = std::wstring;

class DedentSpecifier { // صنف يقوم بتحديد المسافة البادئة الحالية والاخيرة
public:
    int spaces = 0;
    DedentSpecifier* previous = nullptr;
};

class Lexer {
    wstr fileName{}, *input_{};
    wchar_t currentChar{};
    uint32_t tokLine = 1;
    uint32_t tokIndex = -1, tokPos = -1;
    DedentSpecifier* dedentSpec = (DedentSpecifier*)alifMemory.allocate(sizeof(DedentSpecifier)); // حساب المسافات البادئة والراجعة


public:
    MemoryBlock alifMemory;
    std::vector<Token> tokens_{};

    /////////////////////////////////////////

    Lexer(wstr _fileName, wstr* _input);

    void advance();

    void make_token();


    bool word_lex();
    
    bool symbol_lex();


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

    void make_not_equal();

    void make_equals();

    void make_less_than();

    void make_greater_than();

    void skip_comment();
};
