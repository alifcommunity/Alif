#pragma once





class AlifCodeConstructor {
public:

    /* data */
    AlifObject* fileName{};
    AlifObject* name{};
    AlifObject* qualName{};
    //int flags;

    /* the code */
    AlifObject* code{};
    AlifIntT firstlineno{};
    AlifObject* lineTable{};

    /* used by the code */
    AlifObject* consts{};
    AlifObject* names{};

    /* mapping frame offsets to information */
    AlifObject* localsPlusNames{};  // Tuple of strings
    AlifObject* localsPlusKinds{};  // Bytes object, one byte per variable

    /* args (within varnames) */
    AlifIntT argCount;
    AlifIntT posOnlyArgCount;
    AlifIntT kwOnlyArgCount;

    /* needed to create the frame */
    AlifIntT stacksize;

    /* used by the eval loop */
    //AlifObject* exceptionTable;
};




extern AlifCodeObject* alifCode_new(AlifCodeConstructor*);