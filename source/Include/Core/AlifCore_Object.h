#pragma once


#define ALIFSUBOBJECT_HEAD_INIT(_type)       \
    {                                        \
        .ref_ = ALIF_IMMORTAL_REFCENT,    \
        .type_ = (_type),                 \
    } \

#define ALIFSUBVAROBJECT_HEAD_INIT(_type, _size)    \
    {                                               \
        ._base_ = ALIFSUBOBJECT_HEAD_INIT(_type),   \
        .size_ = _size                              \
    } \                                              


extern void alifSub_setImmortal(AlifObject*);
extern void alifSub_setImmortalUntracked(AlifObject*);
static inline void alifSub_setMortal(AlifObject* _op, int64_t _ref)
{
    if (_op) {
        _op->ref_ = _ref;
    }
}

static inline void alifSub_clearImmortal(AlifObject* _op)
{
    if (_op) {
        alifSub_setMortal(_op, 1);
        ALIF_DECREF(_op);
    }
}
#define ALIFSUB_CLEARIMMORTAL(_op) \
    do { \
        alifSub_clearImmortal(ALIFSUBOBJECT_CAST(op)); \
        op = nullptr; \
    } while (0); \

static inline void alifSub_decref_specialized(AlifObject* _op, const Destructor _destruct)
{
    if (ALIFSUB_ISIMMORTAL(_op)) {
        return;
    }

    if (--_op->ref_ != 0) {
    }
    else {
        _destruct(_op);
    }
}

static inline int
alifSubType_hasFeature(AlifTypeObject* _type, unsigned long _feature) {
    return ((_type->flags_ & _feature) != 0);
}

static inline void alifObject_init(AlifObject* _op, AlifTypeObject* _typeObj)
{
    ALIFSET_TYPE(_op, _typeObj);
    ALIF_INCREF(_typeObj);
    //alifSub_newReference(_op);
}

static inline void alifSubObject_initVar(AlifVarObject* _op, AlifTypeObject* _typeObj, int64_t _size)
{
    alifObject_init((AlifObject*)_op, _typeObj);
    ALIFSET_SIZE(_op, _size);
}


void alifObjectGC_link(AlifObject*); // 642
