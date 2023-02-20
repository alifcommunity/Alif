#pragma once

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum TokType : uint16_t { // انواع الرموز
    TTinteger,
    TTfloat,
    
    TTlist,
    
    TTstring, 

    TTname,

    TTplus, 
    TTplusEqual, 
    TTminus, 
    TTminusEqual, 
    TTmultiply, 
    TTmultiplyEqual, 
    TTdivide, 
    TTdivideEqual, 
    TTpower, 
    TTpowerEqual, 
    TTremain, 
    TTremainEqual,

    TTlParenthesis, 
    TTrParenthesis, 
    TTlSquare, 
    TTrSquare, 
    TTlCurlyBrace, 
    TTrCurlyBrace, 
    TTequalEqual, 
    TTnotEqual, 
    TTlessThan, 
    TTgreaterThan, 
    TTlessThanEqual, 
    TTgreaterThanEqual, 

    TTequal,
    TTcomma, 
    TTcolon, 
    TTarrow, 
    TTdot, 

    TTnewline, 

    TTindent, 
    TTdedent, 

    TTendOfFile, 

    TTbuildInFunc, 
    TTkeyword, 

    TTnone, 
};

enum VisitType : uint8_t {
    VObject,
    VList,
    VCall,
    VUnaryOp,
    VBinOp,
    VExpr,
    VExprs,
    VAccess,
    VAssign,
    VAugAssign,
    VReturn,
    VClass,
    VFunction,
    VIf,
    VElseIf,
    VFor,
    VWhile,
    VStmts,
};

enum Context : uint8_t {
    Set,
    Get,
    Del,
};

enum KeywordType : uint8_t {
    False,
    True,
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
    None,
};

enum BuildInFuncType : uint8_t {
    Print,
    Push,
    Input,
};


const std::map<STR, BuildInFuncType> buildInFunctions = { {L"اطبع", Print} , {L"اضف", Push} , {L"ادخل", Input} };
const std::map<STR, KeywordType> keywords_ = { {L"مرر", Pass}, {L"توقف", Stop}, {L"استمر", Continue}, {L"حذف", Delete}, {L"من", From}, {L"استورد", Import} , {L"اذا", If}, {L"واذا", Elseif}, {L"والا", Else}, {L"بينما", While}, {L"لاجل", For}, {L"في", In}, {L"ارجع", Return}, {L"دالة", Function}, {L"صنف", Class}, {L"او", Or}, {L"و", And}, {L"ليس", Not}, {L"صح", True}, {L"خطا", False}, {L"عدم", None} };
const KeywordType keywordsArray[21] = { None, False, True, Not, And, Or, Class, Function, Return, In, For, While, If, Else, Elseif, Import, From, Delete, Continue, Stop, Pass }; // مصفوفة تحتوي على الكلمات المفتاحية مخصصة للتحقق ما إذا كان الاسم كلمة مفتاحية ام لا

class Token {
public:
    TokType type_{};
    unsigned int tokLine{}, tokStart{}, tokEnd{}, tokIndex{};

    union Values
    {
        KeywordType keywordType;
        BuildInFuncType buildInFunc;
        STR* strVal;
        NUM numVal;
    }V{};

    inline Token() {}

    inline Token(unsigned int _tokLine, unsigned int _tokStart, unsigned int _tokEnd, unsigned int _tokIndex, TokType _type) :
        tokLine(_tokLine), tokStart(_tokStart), tokEnd(_tokEnd), tokIndex(_tokIndex), type_(_type) {}

    inline Token(Position _positionStart, Position _positionEnd, TokType _type, STR* _strVal) {}

    inline Token(Position _positionStart, Position _positionEnd, TokType _type, NUM _numVal) {}

    inline Token(Position _positionStart, Position _positionEnd, TokType _type, KeywordType _keywordType) {}

    inline Token(Position _positionStart, Position _positionEnd, TokType _type, BuildInFuncType _buildInFunc) {}
};
