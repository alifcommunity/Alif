#pragma once

#include <iostream>

static const wchar_t* keywordsList[21] = { L"عدم", L"خطا", L"صح", L"ليس", L"و", L"او", L"صنف", L"دالة", L"ارجع", L"في", L"لاجل", L"بينما", L"اذا", L"والا", L"اواذا", L"استورد", L"من", L"حذف", L"استمر", L"توقف", L"مرر"};

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
};

enum ObjectType : uint8_t { // انواع الكائنات
    OTNumber,
    OTString,
    OTName,
    OTList,
    OTDictionary,
    OTBoolean,
    OTNone,
    OTContainer,
    //OTKeyword,
    //OTBuildInFunc,
};

enum VisitType : uint8_t { // انواع الزيارة في الشجرة
    VTObject,
    VTList,
    VTCall,
    VTUnaryOp,
    VTBinOp,
    VTCondExpr,
    VTExpr,
    VTExprs,
    VTAccess,
    VTAssign,
    VTAugAssign,
    VTReturn,
    VTImport,
    VTClass,
    VTFunction,
    VTIf,
    VTFor,
    VTWhile,
    VTStop,
    VTContinue,
    VTStmts,
};

//enum StateType : uint8_t { // انواع الحالات للاسماء
//    STSet,
//    STGet,
//    STDel,
//};

enum InstructionsType : uint8_t { // التعليمات
    NONE,
    
    // تعليمات الذاكرة
    GET_DATA,
    SET_DATA,

    // تعليمات العمليات الحسابية
    PLUS_NUM,
    MINUS_NUM,
    ADD_NUM,
    SUB_NUM,
    MUL_NUM,
    DIV_NUM,
    REM_NUM,
    POW_NUM,
    AUGADD_NUM,
    AUGSUB_NUM,
    AUGMUL_NUM,
    AUGDIV_NUM,
    AUGREM_NUM,
    AUGPOW_NUM,

    // تعليمات عمليات المقارنة
    EQEQ_NUM,
    NOTEQ_NUM,
    GRTHAN_NUM,
    GRTHANEQ_NUM,
    LSTHAN_NUM,
    LSTHANEQ_NUM,

    // تعليمات المنطق
    NOT_LOGIC,
    AND_LOGIC,
    OR_LOGIC,

    // تعليمات السلاسل النصية
    ADD_STR,
    MUL_STR,

    // تعليمات التعبيرات
    EXPR_OP,

    // تعليمات الإسناد
    STORE_NAME,

    // تعليمات المصفوفة
    LIST_MAKE,

    // تعليمات الحلقات التكرارية
    JUMP_TO,
    JUMP_IF,
    JUMP_FOR,

    // تعليمات جدول الاسماء
    CREATE_SCOPE,
    COPY_SCOPE,
    ENTER_SCOPE,
    GET_SCOPE,
    EXIT_SCOPE,
    RESTORE_SCOPE,


    // تعليمات الدوال والاصناف
    CALL_NAME,
    RETURN_EXPR,


    // الدوال الضمنية
    PRINT_FUNC,

};

