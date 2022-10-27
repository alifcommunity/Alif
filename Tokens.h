#pragma once

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::wstring
integerT = L"Integer",
floatT = L"Float",
stringT = L"String",
nameT = L"Name",
keywordT = L"Keyword",
plusT = L"Plus",
plusEqualT = L"Plus_equal",
minusT = L"Minus",
minusEqualT = L"Minus_equal",
multiplyT = L"Multiply",
multiplyEqualT = L"Multiply_equal",
divideT = L"Divide",
divideEqualT = L"Divide_equal",
powerT = L"Power",
powerEqualT = L"Power_equal",
remainT = L"Remain",
remainEqualT = L"Remain_equal",
equalT = L"Equal",
lParenthesisT = L"L_Parenthesis",
rParenthesisT = L"R_Parenthesis",
lSquareT = L"L_Square",
rSquareT = L"R_Square",
lCurlyBraceT = L"L_curly_brace",
rCurlyBraceT = L"R_curly_brace",
equalEqualT = L"Equal_equal",
notEqualT = L"Not_equal",
lessThanT = L"Less_than",
greaterThanT = L"Greater_than",
lessThanEqualT = L"Less_than_equal",
greaterThanEqualT = L"Greater_than_equal",
commaT = L"Comma",
colonT = L"Colon",
arrowT = L"Arrow",
newlineT = L"Newline",
tabT = L"Tab",
dotT = L"Dot",
endOfFileT = L"End_Of_File";

const std::list<std::wstring> keywords = { L"مرر", L"توقف", L"استمر", L"حذف", L"استورد", L"من", L"اذا", L"بينما", L"لاجل", L"ارجع", L"دالة", L"صنف", L"والا", L"او", L"و", L"ليس", L"صح", L"خطا", L"عدم", L"اطبع", L"في" };


class Token {
public:
    std::wstring type_, value_;
    Position positionStart, positionEnd;
    Token() {}
    Token(std::wstring type_, Position positionStart) :
        type_(type_), positionStart(positionStart), positionEnd(positionStart)
    {
        this->positionEnd.advance();
    }
    Token(std::wstring type_, std::wstring value_, Position positionStart, Position positionEnd) :
        type_(type_), value_(value_), positionStart(positionStart), positionEnd(positionEnd)
    {

    }
};
