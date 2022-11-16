#pragma once

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum TokenType : uint8_t{
    integerT, // Integer
    floatT, // Float
    stringT, // String
    nameT, // Name
    plusT, // Plus
    plusEqualT, // Plus_equal
    minusT, // Minus
    minusEqualT, // Minus_equal
    multiplyT, // Multiply
    multiplyEqualT, // Multiply_equal
    divideT, // Divide
    divideEqualT, // Divide_equal
    powerT, // Power
    powerEqualT, // Power_equal
    remainT, // Remain
    remainEqualT, // Remain_equal
    equalT, // Equal
    lParenthesisT, // L_Parenthesis
    rParenthesisT, // R_Parenthesis
    lSquareT, // L_Square
    rSquareT, // R_Square
    lCurlyBraceT, // L_curly_brace
    rCurlyBraceT, // R_curly_brace
    equalEqualT, // Equal_equal 
    notEqualT, // Not_equal
    lessThanT, // Less_than
    greaterThanT, // Greater_than
    lessThanEqualT, // Less_than_equal
    greaterThanEqualT, // Greater_than_equal
    commaT, // Comma
    colonT, // Colon
    arrowT, // Arrow
    newlineT, // NewLine
    tabT, // Tab
    dotT, // Dot
    endOfFileT, // End_Of_File
    None,
};
//const std::vector<std::wstring> keywords = { L"مرر", L"توقف", L"استمر", L"حذف", L"استورد", L"من", L"اذا", L"واذا", L"بينما", L"لاجل", L"ارجع", L"دالة", L"صنف", L"والا", L"او", L"و", L"ليس", L"صح", L"خطا", L"عدم", L"اطبع", L"في" };


class Token {

public:
    TokenType type{};
    std::wstring value{};
    Position positionStart{};
    Position positionEnd{};

    Token(Position positionStart = Position(), Position positionEnd = Position(), TokenType type = None, std::wstring value = L"عدم")
    {
        this->type = type;
        this->value = value;
        this->positionStart = positionStart;
        this->positionEnd = positionEnd;

    }
};
