#pragma once

// الرموز
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum TokType {
    TTinteger,
    TTfloat,
    
    TTnumber, // لا يلزم
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

enum VisitType {
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

enum Context {
    Set,
    Get,
    Del,
};

enum KeywordType {
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

enum BuildInFuncType {
    Print,
    Push,
    Input,
};


std::map<STR, BuildInFuncType> buildInFunctions = { {L"اطبع", Print} , {L"اضف", Push} , {L"ادخل", Input} };
std::map<STR, KeywordType> keywords_ = { {L"مرر", Pass}, {L"توقف", Stop}, {L"استمر", Continue}, {L"حذف", Delete}, {L"من", From}, {L"استورد", Import} , {L"اذا", If}, {L"واذا", Elseif}, {L"والا", Else}, {L"بينما", While}, {L"لاجل", For}, {L"في", In}, {L"ارجع", Return}, {L"دالة", Function}, {L"صنف", Class}, {L"او", Or}, {L"و", And}, {L"ليس", Not}, {L"صح", True}, {L"خطا", False}, {L"عدم", None} };
const KeywordType keywordsArray[21] = { None, False, True, Not, And, Or, Class, Function, Return, In, For, While, If, Else, Elseif, Import, From, Delete, Continue, Stop, Pass }; // مصفوفة تحتوي على الكلمات المفتاحية مخصصة للتحقق ما إذا كان الاسم كلمة مفتاحية ام لا

class Token {
public:
    union Values{}val{};

    Token(){}

    Token(Position _positionStart, Position _positionEnd, TokType _type) {}

    Token(Position _positionStart, Position _positionEnd, TokType _type, STR* _strVal) {}

    Token(Position _positionStart, Position _positionEnd, TokType _type, NUM _numVal) {}

    Token(Position _positionStart, Position _positionEnd, TokType _type, KeywordType _keywordType) {}

    Token(Position _positionStart, Position _positionEnd, TokType _type, BuildInFuncType _buildInFunc) {}
};
