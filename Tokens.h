#pragma once

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::wstring
integerT = L"Int", // Integer
floatT = L"Float", // Float
stringT = L"Str", // String
nameT = L"Nam", // Name
//keywordT = L"KW", // Keyword
plusT = L"Pls", // Plus
plusEqualT = L"Pls_eq", // Plus_equal
minusT = L"Mins", // Minus
minusEqualT = L"Mins_eq", // Minus_equal
multiplyT = L"Multi", // Multiply
multiplyEqualT = L"Multi_eq", // Multiply_equal
divideT = L"Div", // Divide
divideEqualT = L"Div_eq", // Divide_equal
powerT = L"Pow", // Power
powerEqualT = L"Pow_eq", // Power_equal
remainT = L"Remn", // Remain
remainEqualT = L"Remn_eq", // Remain_equal
equalT = L"Eq", // Equal
lParenthesisT = L"L_Paren", // L_Parenthesis
rParenthesisT = L"R_Paren", // R_Parenthesis
lSquareT = L"L_Sq", // L_Square
rSquareT = L"R_Sq", // R_Square
lCurlyBraceT = L"L_cur_br", // L_curly_brace
rCurlyBraceT = L"R_cur_br", // R_curly_brace
equalEqualT = L"Eq_eq", // Equal_equal 
notEqualT = L"Not_eq", // Not_equal
lessThanT = L"Les_tha", // Less_than
greaterThanT = L"Gre_tha", // Greater_than
lessThanEqualT = L"Les_tha_eq", // Less_than_equal
greaterThanEqualT = L"Gre_tha_eq", // Greater_than_equal
commaT = L"Com", // Comma
colonT = L"Coln", // Colon
arrowT = L"Arr", // Arrow
newlineT = L"NL", // NewLine
tabT = L"Tab", // Tab
dotT = L"Dot", // Dot
endOfFileT = L"EOF"; // End_Of_File

//const std::vector<std::wstring> keywords = { L"مرر", L"توقف", L"استمر", L"حذف", L"استورد", L"من", L"اذا", L"واذا", L"بينما", L"لاجل", L"ارجع", L"دالة", L"صنف", L"والا", L"او", L"و", L"ليس", L"صح", L"خطا", L"عدم", L"اطبع", L"في" };


class Token {

public:
    std::wstring type = L"";
    std::wstring value = L"";
    Position positionStart;
    Position positionEnd;

    Token() {}
    Token(Position positionStart, Position positionEnd, std::wstring type)
    {
        this->type = type;
        this->positionStart = positionStart;
        this->positionEnd = positionEnd;
    }
    Token(Position positionStart, Position positionEnd, std::wstring type, std::wstring value)
    {
        this->type = type;
        this->value = value;
        this->positionStart = positionStart;
        this->positionEnd = positionEnd;

    }
};
