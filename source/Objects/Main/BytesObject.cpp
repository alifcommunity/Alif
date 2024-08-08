#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_BytesObject.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_Object.h"

static AlifObject* alifSubBytes_fromSize(int64_t _size) {

	if (_size == 0) {
		// return empty _object
	}

	AlifWBytesObject* _object = (AlifWBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + _size);

	alifSubObject_initVar((AlifVarObject*)_object, &_typeBytes_, _size);

	ALIF_INCREF(_object);

    //_object->value_[_size] = '\0'; // alifMem_objAlloc reset all bytes of allocated var

    return (AlifObject*)_object;

}

AlifObject* alifBytes_fromStringAndSize(const wchar_t* _str, int64_t _size)
{
    AlifWBytesObject* op_{};
    if (_size < 0) {
        return nullptr;
    }
    if (_size == 1 && _str != nullptr) {
        //op_ = CHARACTER(*_str & 255);
        return (AlifObject*)op_;
    }
    if (_size == 0) {
        //return bytes_get_empty();
    }

    op_ = (AlifWBytesObject*)alifSubBytes_fromSize(_size);
    if (op_ == nullptr)
        return nullptr;
    if (_str == nullptr)
        return (AlifObject*)op_;

    memcpy(op_->value_, _str, _size);
    return (AlifObject*)op_;
}

AlifObject* alifBytes_fromString(const wchar_t* _str){

    size_t size_ = wcslen(_str);

    if (size_ == 0) {
        // return empty _object
    }

    AlifWBytesObject* _object = (AlifWBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + size_);

    alifSubObject_initVar((AlifVarObject*)_object, &_typeBytes_, size_);
	ALIF_INCREF(_object);
    memcpy(_object->value_, _str, size_ + 1);
    return (AlifObject*)_object;
}

int64_t alifBytes_size(AlifObject* _object) {

    if (_object->type_ != &_typeBytes_) {
        std::wcout << L"متوقع تمرير كائن بايت\n" << std::endl;
        exit(-1);
    }
    return ALIF_SIZE(_object);
}

int alifWBytes_asStringAndSize(AlifObject* _obj, wchar_t** _s, AlifSizeT* _len)
{
    if (_s == nullptr) return -1;

    if (!(_obj->type_ == &_typeBytes_)) return -1;

    *_s = ALIFWBYTES_AS_STRING(_obj);
    if (_len != nullptr) {
        *_len = ALIF_SIZE(_obj);
    }
    else if (wcslen(*_s) != (AlifUSizeT)ALIF_SIZE(_obj)) {
        return -1;
    }
    return 0;
}

static AlifObject* bytes_concat(AlifObject* _a, AlifObject* _b)
{
    AlifBuffer va_{}, vb_{};
    AlifObject* result_ = nullptr;

    va_.len = -1;
    vb_.len = -1;
    if (alifObject_getBuffer(_a, &va_, 0) != 0 or alifObject_getBuffer(_b, &vb_, 0) != 0) {
        goto done;
    }

    /* Optimize end cases */
    if (va_.len == 0 && (_b->type_ == &_typeBytes_)) {
        result_ = ALIF_NEWREF(_b);
        goto done;
    }
    if (vb_.len == 0 && (_a->type_ == &_typeBytes_)) {
        result_ = ALIF_NEWREF(_a);
        goto done;
    }

    if (va_.len > LLONG_MAX - vb_.len) {
        goto done;
    }

    result_ = alifBytes_fromStringAndSize(nullptr, va_.len + vb_.len);
    if (result_ != nullptr) {
        memcpy(ALIFWBYTES_AS_STRING(result_), va_.buf, va_.len);
        memcpy(ALIFWBYTES_AS_STRING(result_) + va_.len, vb_.buf, vb_.len);
    }

done:
    if (va_.len != -1)
        alifBuffer_release(&va_);
    if (vb_.len != -1)
        alifBuffer_release(&vb_);
    return result_;
}

// in file codecs.c_
const char* alif_hexdigits = "0123456789abcdef";

AlifObject* alifBytes_repr(AlifObject* _obj, int _smartQuotes)
{
    AlifWBytesObject* op_ = (AlifWBytesObject*)_obj;
    int64_t i_, length_ = ((AlifVarObject*)op_)->size_;
    int64_t newSize, squotes_, dquotes_;
    AlifObject* v_;
    unsigned char quote_;
    const unsigned char* s_;
    uint8_t* p_;

    /* Compute size_ of output string */
    squotes_ = dquotes_ = 0;
    newSize = 3;
    s_ = (const unsigned char*)op_->value_;
    for (i_ = 0; i_ < length_; i_++) {
        int64_t incr_ = 1;
        switch (s_[i_]) {
        case '\'': squotes_++; break;
        case '"':  dquotes_++; break;
        case '\\': case '\t': case '\n': case '\r':
            incr_ = 2; break; 
        default:
            if (s_[i_] < ' ' || s_[i_] >= 0x7f)
                incr_ = 4; 
        }
        if (newSize > LLONG_MAX - incr_)
            goto overflow;
        newSize += incr_;
    }
    quote_ = '\'';
    if (_smartQuotes && squotes_ && !dquotes_)
        quote_ = '"';
    if (squotes_ && quote_ == '\'') {
        if (newSize > LLONG_MAX - squotes_)
            goto overflow;
        newSize += squotes_;
    }

    v_ = alifNew_uStr(newSize, 127);
    if (v_ == nullptr) {
        return nullptr;
    }
    p_ = (uint8_t*)((AlifUStrObject*)v_)->UTF;

    *p_++ = 'b', *p_++ = quote_;
    for (i_ = 0; i_ < length_; i_++) {
        unsigned char c_ = op_->value_[i_];
        if (c_ == quote_ || c_ == '\\')
            *p_++ = '\\', *p_++ = c_;
        else if (c_ == '\t')
            *p_++ = '\\', *p_++ = 't';
        else if (c_ == '\n')
            *p_++ = '\\', *p_++ = 'n';
        else if (c_ == '\r')
            *p_++ = '\\', *p_++ = 'r';
        else if (c_ < ' ' || c_ >= 0x7f) {
            *p_++ = '\\';
            *p_++ = 'x';
            *p_++ = alif_hexdigits[(c_ & 0xf0) >> 4];
            *p_++ = alif_hexdigits[c_ & 0xf];
        }
        else
            *p_++ = c_;
    }
    *p_++ = quote_;
    return v_;

overflow:
    std::wcout << L"كائن البايت كبير جدا لعمل تمثيل نصي\n" << std::endl;
    exit(-1);
    return nullptr;
}

static AlifObject* bytes_repr(AlifObject* _object)
{
    return alifBytes_repr(_object, 1);
}

static int64_t bytes_length(AlifWBytesObject* _value) {

    return ALIF_SIZE(_value);

}

static AlifObject* bytes_repeat(AlifWBytesObject* _object, int64_t _repeat) {

    if (_repeat < 0){
        _repeat = 0;
    }

    int64_t size_ = ((AlifVarObject*)_object)->size_ * _repeat;
    if (size_ == ((AlifVarObject*)_object)->size_ && 
        ((AlifObject*)_object)->type_ == &_typeBytes_) {
        return (AlifObject*)_object;
    }
    size_t numberBytes = (size_t)size_;

    AlifWBytesObject* bytesObject = (AlifWBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + numberBytes);

    alifSubObject_initVar((AlifVarObject*)bytesObject, &_typeBytes_, size_);
    bytesObject->value_[size_] = '\0';

    bytes_subRepeat(bytesObject->value_, size_, _object->value_, ((AlifVarObject*)_object)->size_);

    return (AlifObject*)bytesObject;

}

static AlifObject* bytes_item(AlifWBytesObject* _object, int64_t _index) {

    if (_index < 0 || ((AlifVarObject*)_object)->size_ <= _index) {
        std::wcout << L"مؤشر البايت خارج النطاق\n" << std::endl;
        exit(-1);
    }

    return alifInteger_fromLongLong((unsigned char)_object->value_[_index]);

}

static int bytes_compare_eq(AlifWBytesObject* _a, AlifWBytesObject* _b)
{
    int cmp;
    int64_t len;

    len = ((AlifVarObject*)_a)->size_;
    if (((AlifVarObject*)_b)->size_ != len)
        return 0;

    if (_a->value_[0] != _b->value_[0])
        return 0;

    cmp = memcmp(_a->value_, _b->value_, len);
    return (cmp == 0);
}

static AlifObject* bytes_richcompare(AlifWBytesObject* _a, AlifWBytesObject* _b, int _op)
{
    int c_;
    int64_t lenA, lenB;
    int64_t minLen;

    //if (!(Bytes_Check(_a) && Bytes_Check(_b))) {
    //    if (__GetConfig()->bytes_warning && (_op == _EQ || _op == _NE)) {
    //        if (Unicode_Check(_a) || Unicode_Check(_b)) {
    //            if (Err_WarnEx(Exc_BytesWarning,
    //                "Comparison between bytes and string", 1))
    //                return nullptr;
    //        }
    //        if (Long_Check(_a) || Long_Check(_b)) {
    //            if (Err_WarnEx(Exc_BytesWarning,
    //                "Comparison between bytes and int", 1))
    //                return nullptr;
    //        }
    //    }
    //    ALIF_RETURN_NOTIMPLEMENTED;
    //}
    if (_a == _b) {
        switch (_op) {
        case ALIF_EQ:
        case ALIF_LE:
        case ALIF_GE:
            return ALIF_TRUE;
        case ALIF_NE:
        case ALIF_LT:
        case ALIF_GT:
            return ALIF_FALSE;
        default:
            return nullptr;
        }
    }
    else if (_op == ALIF_EQ || _op == ALIF_NE) {
        int eq_ = bytes_compare_eq(_a, _b);
        eq_ ^= (_op == ALIF_NE);
        return alifBool_fromInteger(eq_);
    }
    else {
        lenA = ((AlifVarObject*)_a)->size_;
        lenB = ((AlifVarObject*)_b)->size_;
        minLen = ALIF_MIN(lenA, lenB);
        if (minLen > 0) {
            c_ = ALIF_WCHARMASK(*_a->value_) - ALIF_WCHARMASK(*_b->value_);
            if (c_ == 0)
                c_ = memcmp(_a->value_, _b->value_, minLen);
        }
        else
            c_ = 0;
        if (c_ != 0)
            ALIF_RETURN_RICHCOMPARE(c_, 0, _op);
        ALIF_RETURN_RICHCOMPARE(lenA, lenB, _op);
    }

	return nullptr;
}

// in file bytes_methods.c_
int alif_bytes_contain(const wchar_t *_str, int64_t _length, AlifObject* _arg) { 

    int64_t value = alifInteger_asLong(_arg);

    if (value < 0 || value >= 256) {
        std::wcout << L"يجب ان يكون البايت ضمن نطاق (0, 256)\n" << std::endl;
        exit(-1);
    }

    return memchr(_str, (int)value, _length) != nullptr;
}

static int bytes_contains(AlifObject* _self, AlifObject * _arg) {

    return alif_bytes_contain(((AlifWBytesObject*)_self)->value_, ((AlifVarObject*)_self)->size_, _arg);
}

void alifBytes_dealloc(AlifObject* _object) {
    
    alifMem_objFree(_object);

}

static AlifSequenceMethods _bytesAsSequence_ = {
    (LenFunc)bytes_length, 
    (BinaryFunc)bytes_concat, 
    (SSizeArgFunc)bytes_repeat, 
    (SSizeArgFunc)bytes_item, 
    0,                  
    0,                  
    0,                  
    (ObjObjProc)bytes_contains 
};

static AlifMappingMethods _bytesAsmapping_ = {
    (LenFunc)bytes_length,
    //(BinaryFunc)bytes_subscript,
    0,
};

AlifTypeObject _typeBytes_ = {
    0,
    0,
    0,
    L"bytes",
    ALIFBYTESOBJECT_SIZE,
    sizeof(char),
    0,                                
    0,                                          
    0,                                
    0,                                
    bytes_repr,
    0,                   
    &_bytesAsSequence_,                   
    &_bytesAsmapping_,                   
    0,         
    0,                             
    0,                    
    alifObject_genericGetAttr,           
    0,                                 
    0,      
    0,                       
    0,                                    
    0,
    0,
    (RichCmpFunc)bytes_richcompare,          
    0,                                          
    0, //bytes_iter,                       
    0,                                    
    0, //bytes_methods,                          
    0,                                      
    0,                                     
    0,                                   
    0,                                   
    0,                                        
    0,                                        
    0,                                         
    0,                                   
    0, //bytes_alloc,                          
    0, //bytes_new,                                   
    (FreeFunc)alifBytes_dealloc,                               
};

void alifBytes_concat(AlifObject** _pv, AlifObject* _w)
{
    if (*_pv == nullptr)
        return;
    if (_w == nullptr) {
        ALIF_CLEAR(*_pv);
        return;
    }

    if (ALIF_REFCNT(*_pv) == 1 && ((*_pv)->type_ == &_typeBytes_)) {
        int64_t oldSize;
        AlifBuffer wb_;

        if (alifObject_getBuffer(_w, &wb_, 0) != 0) {
            ALIF_CLEAR(*_pv);
            return;
        }

        oldSize = ((AlifVarObject*)*_pv)->size_;
        if (oldSize > LLONG_MAX - wb_.len) {
       
            goto error;
        }
        if (alifSubBytes_resize(_pv, oldSize + wb_.len) < 0)
            goto error;

        //memcpy(ALIFWBYTES_AS_STRING(*_pv) + oldSize, wb_.buf, wb_.len);
        alifBuffer_release(&wb_);
        return;

    error:
        alifBuffer_release(&wb_);
        ALIF_CLEAR(*_pv);
        return;
    }

    else {
        /* Multiple references, need to create new _object */
        AlifObject* v_;
        v_ = bytes_concat(*_pv, _w);
        ALIF_SETREF(*_pv, v_);
    }
}

int alifSubBytes_resize(AlifObject** _pv, int64_t _newSize)
{
    AlifObject* v_;
    AlifWBytesObject* sv_;
    v_ = *_pv;
    if (!(v_->type_ == &_typeBytes_) || _newSize < 0) {
        *_pv = 0;
        ALIF_DECREF(v_);
        return -1;
    }
    int64_t oldSize = ((AlifVarObject*)v_)->size_;
    if (oldSize == _newSize) {
        /* return early if newsize equals to v_->ob_size */
        return 0;
    }
    if (oldSize == 0) {
        *_pv = alifSubBytes_fromSize(_newSize);
        ALIF_DECREF(v_);
        return (*_pv == nullptr) ? -1 : 0;
    }
    if (_newSize == 0) {
        //*_pv = bytes_get_empty();
        ALIF_DECREF(v_);
        return 0;
    }
    if (ALIF_REFCNT(v_) != 1) {
        if (oldSize < _newSize) {
            *_pv = alifSubBytes_fromSize(_newSize);
            if (*_pv) {
                memcpy(ALIFWBYTES_AS_STRING(*_pv), ALIFWBYTES_AS_STRING(v_), oldSize);
            }
        }
        else {
            *_pv = alifBytes_fromStringAndSize(ALIFWBYTES_AS_STRING(v_), _newSize);
        }
        ALIF_DECREF(v_);
        return (*_pv == nullptr) ? -1 : 0;
    }


    * _pv = (AlifObject*)
        alifMem_objRealloc(v_, ALIFBYTESOBJECT_SIZE + _newSize);
    if (*_pv == nullptr) {
        alifMem_objFree(v_);
        return -1;
    }
    sv_ = (AlifWBytesObject*)*_pv;
    ALIFSET_SIZE(sv_, _newSize);
    sv_->value_[_newSize] = '\0';
    return 0;
}

void bytes_subRepeat(wchar_t* _destinaion, int64_t _lengthDest,
    const wchar_t* _source, int64_t _lengthSrc)
{
    if (_lengthDest == 0) {
        return;
    }
    if (_lengthSrc == 1) {
        memset(_destinaion, _source[0], _lengthDest);
    }
    else {
        if (_source != _destinaion) {
            memcpy(_destinaion, _source, _lengthSrc);
        }
        int64_t copied_ = _lengthSrc;
        while (copied_ < _lengthDest) {
            int64_t bytesToCopy = ALIF_MIN(copied_, _lengthDest - copied_);
            memcpy(_destinaion + copied_, _destinaion, bytesToCopy);
            copied_ += bytesToCopy;
        }
    }
}
