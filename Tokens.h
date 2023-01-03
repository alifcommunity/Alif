#pragma once

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum TokenType {
    TTinteger, // Integer
    TTfloat, // Float
    
    TTnumber, // Number
    TTlist, // List
    
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
    TTindent, // INDENT
    TTdedent, // DEDENT
    TTdot, // Dot
    TTendOfFile, // End_Of_File
    TTbuildInFunc, // BuildInFunction
    TTkeyword, // Keyword
    TTnone, // None
};

enum VisitType {
    VObject,
    VList,
    VUnaryOp,
    VBinOp,
    VExprs,
    VAssign,
    VAugAssign,
    VReturn,
};

//enum Context {
//    Set,
//    Get,
//    Del,
//};

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


std::map<STR, BuildInFuncType> buildInFunctions = { {L"اطبع", Print} };
std::map<STR, KeywordType> keywords_ = { {L"مرر", Pass}, {L"توقف", Stop}, {L"استمر", Continue}, {L"حذف", Delete}, {L"من", From}, {L"استورد", Import} , {L"اذا", If}, {L"واذا", Elseif}, {L"والا", Else}, {L"بينما", While}, {L"لاجل", For}, {L"في", In}, {L"ارجع", Return}, {L"دالة", Function}, {L"صنف", Class}, {L"او", Or}, {L"و", And}, {L"ليس", Not}, {L"صح", True}, {L"خطا", False}, {L"عدم", None} };

class Token {
public:
    TokenType type_{};
    Position positionStart{}, positionEnd{};

    union Values
    {
        KeywordType keywordType;
        BuildInFuncType buildInFunc;
        STR* strVal;
        NUM numVal;
    }val{};

    Token(){}

    Token(Position _positionStart, Position _positionEnd, TokenType _type) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
    }

    Token(Position _positionStart, Position _positionEnd, TokenType _type, STR* _strVal) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
        this->val.strVal = _strVal;
    }

    Token(Position _positionStart, Position _positionEnd, TokenType _type, NUM _numVal) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
        this->val.numVal = _numVal;
    }

    Token(Position _positionStart, Position _positionEnd, TokenType _type, KeywordType _keywordType) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
        this->val.keywordType = _keywordType;
    }

    Token(Position _positionStart, Position _positionEnd, TokenType _type, BuildInFuncType _buildInFunc) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
        this->val.buildInFunc = _buildInFunc;
    }
};
