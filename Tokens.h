#pragma once

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum TokenType {
    TTinteger, // Integer
    TTfloat, // Float
    TTstring, // String
    TTname, // Name
    TTplus, // Plus
    TTplusEqual, // Plus_equal
    TTminus, // Minus
    TTminusEqual, // Minus_equal
    TTmultiply, // Multiply
    TTmultiplyEqual, // Multiply_equal
    TTdivide, // Divide
    TTdivideEqual, // Divide_equal
    TTpower, // Power
    TTpowerEqual, // Power_equal
    TTremain, // Remain
    TTremainEqual, // Remain_equal
    TTequal, // Equal
    TTlParenthesis, // L_Parenthesis
    TTrParenthesis, // R_Parenthesis
    TTlSquare, // L_Square
    TTrSquare, // R_Square
    TTlCurlyBrace, // L_curly_brace
    TTrCurlyBrace, // R_curly_brace
    TTequalEqual, // Equal_equal 
    TTnotEqual, // Not_equal
    TTlessThan, // Less_than
    TTgreaterThan, // Greater_than
    TTlessThanEqual, // Less_than_equal
    TTgreaterThanEqual, // Greater_than_equal
    TTcomma, // Comma
    TTcolon, // Colon
    TTarrow, // Arrow
    TTnewline, // NewLine
    TTtab, // Tab
    TTdot, // Dot
    TTendOfFile, // End_Of_File
    TTbuildInFunc, // BuildInFunction
    TTkeyword, // Keyword
    TTnone, // None
};

enum KeywordType {
    Pass,
    Stop,
    Continue,
    Delete,
    From,
    Import,
    If,
    Elseif,
    Else,
    While,
    For,
    In,
    Return,
    Function,
    Class,
    Or,
    And,
    Not,
    True,
    False,
    None,
};

enum BuildInFuncType {
    Print,
};

extern std::map<BuildInFuncType, STR> buildInFunctions = { {Print , L"اطبع"} };
extern std::map<KeywordType, STR> _keywords = { {Pass , L"مرر"}, {Stop , L"توقف"}, {Continue , L"استمر"}, {Delete, L"حذف"}, {From , L"من"}, {Import , L"استورد"} , {If , L"اذا"}, {Elseif , L"واذا"}, {Else , L"والا"}, {While , L"بينما"}, {For , L"لاجل"}, {In , L"في"}, {Return , L"ارجع"}, {Function , L"دالة"}, {Class , L"صنف"}, {Or , L"او"}, {And , L"و"}, {Not , L"ليس"}, {True , L"صح"}, {False , L"خطا"}, {None , L"عدم"} };

class Token {
public:
    TokenType _type{};
    Position positionStart{}, positionEnd{};
    union Values
    {
        KeywordType keywordType;
        BuildInFuncType buildInFunc;
        STR* strVal;
        NUM numVal;
    }val{};

    Token(Position positionStart_, Position positionEnd_, TokenType type_, STR* strVal_) {
        this->positionStart = positionStart_;
        this->positionEnd = positionEnd_;
        this->_type = type_;
        this->val.strVal = strVal_;
    }

    Token(Position positionStart_, Position positionEnd_, TokenType type_, NUM numVal_) {
        this->positionStart = positionStart_;
        this->positionEnd = positionEnd_;
        this->_type = type_;
        this->val.numVal = numVal_;
    }

    Token(Position positionStart_, Position positionEnd_, TokenType type_, KeywordType keywordType_) {
        this->positionStart = positionStart_;
        this->positionEnd = positionEnd_;
        this->_type = type_;
        this->val.keywordType = keywordType_;
    }

    Token(Position positionStart_, Position positionEnd_, TokenType type_, BuildInFuncType buildInFunc_) {
        this->positionStart = positionStart_;
        this->positionEnd = positionEnd_;
        this->_type = type_;
        this->val.buildInFunc = buildInFunc_;
    }
};
