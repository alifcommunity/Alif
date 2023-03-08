#pragma once


enum TokensType : uint8_t { // انواع الرموز
    TTInteger,
    TTFloat,

    TTString,

    TTName,

    TTPlus,
    TTPlusEqual,
    TTMinus,
    TTMinusEqual,
    TTMultiply,
    TTMultiplyEqual,
    TTDivide,
    TTDivideEqual,
    TTPower,
    TTPowerEqual,
    TTRemain,
    TTRemainEqual,

    TTLeftParenthesis,
    TTRrightParenthesis,
    TTLeftSquare,
    TTRightSquare,
    TTLeftCurlyBrace,
    TTRightCurlyBrace,

    TTEqualEqual,
    TTNotEqual,
    TTLessThan,
    TTGreaterThan,
    TTLessThanEqual,
    TTGreaterThanEqual,

    TTEqual,
    TTComma,
    TTColon,
    TTDot,

    TTNewline,

    TTIndent,
    TTDedent,

    TTEndOfFile,

    TTBuildInFunc,
    TTKeyword,
};

enum ObjectType : uint8_t { // انواع الكائنات
    OTNumber,
    OTString,
    OTName,
    OTList,
    OTDictionary,
    OTBoolean,
    OTNone,
    OTKeyword,

};

enum VisitType : uint8_t { // انواع الزيارة في الشجرة
    VTObject,
    VTList,
    VTCall,
    VTUnaryOp,
    VTBinOp,
    VTExpr,
    VTExprs,
    VTAccess,
    VTAssign,
    VTAugAssign,
    VTReturn,
    VTClass,
    VTFunction,
    VTIf,
    VTElseIf,
    VTFor,
    VTWhile,
    VTStmts,
};

enum StateType : uint8_t { // انواع الحالات للاسماء
    STSet,
    STGet,
    STDel,
};

