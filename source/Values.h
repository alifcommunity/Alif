#pragma once

enum KeywordValue : uint8_t { // انواع الكلمات المفتاحية
    KVFalse,
    KVTrue,
    KVPass,
    KVStop,
    KVContinue,
    KVDelete,
    KVFrom,
    KVImport,
    KVIf,
    KVElseif,
    KVElse,
    KVWhile,
    KVFor,
    KVIn,
    KVReturn,
    KVFunction,
    KVClass,
    KVOr,
    KVAnd,
    KVNot,
    KVNone,
};

enum BuildInFuncValue : uint8_t { // انواع الدوال المضمنة
    BVPrint,
    BVPush,
    BVInput,
};
