#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_BytesObject.h"
#include "AlifCore_Object.h"

static AlifObject* alifSubBytes_fromSize(int64_t size_) {

	if (size_ == 0) {
		// return empty object
	}

	AlifWBytesObject* object = (AlifWBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + size_);

	alifSubObject_initVar((AlifVarObject*)object, &_typeBytes_,size_);

	ALIF_INCREF(object);

    object->value_[size_] = '\0';

    return (AlifObject*)object;

}

AlifObject* alifBytes_fromStringAndSize(const wchar_t* _str, int64_t _size)
{
    AlifWBytesObject* op_{};
    if (_size < 0) {
        return NULL;
    }
    if (_size == 1 && _str != NULL) {
        //op_ = CHARACTER(*_str & 255);
        return (AlifObject*)op_;
    }
    if (_size == 0) {
        //return bytes_get_empty();
    }

    op_ = (AlifWBytesObject*)alifSubBytes_fromSize(_size);
    if (op_ == NULL)
        return NULL;
    if (_str == NULL)
        return (AlifObject*)op_;

    memcpy(op_->value_, _str, _size);
    return (AlifObject*)op_;
}

AlifObject* alifBytes_fromString(const wchar_t* str){

    size_t size_ = wcslen(str);

    if (size_ == 0) {
        // return empty object
    }

    AlifWBytesObject* object = (AlifWBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + size_);

    alifSubObject_initVar((AlifVarObject*)object, &_typeBytes_, size_);
	ALIF_INCREF(object);
    memcpy(object->value_, str, size_ + 1);
    return (AlifObject*)object;
}

int64_t alifBytes_size(AlifObject* object) {

    if (object->type_ != &_typeBytes_) {
        std::wcout << L"متوقع تمرير كائن بايت\n" << std::endl;
        exit(-1);
    }
    return ((AlifVarObject*)object)->size_;
}

int alifWBytes_asStringAndSize(AlifObject* _obj, wchar_t** _s, int64_t* _len)
{
    if (_s == nullptr) return -1;

    if (!(_obj->type_ == &_typeBytes_)) return -1;

    //*_s = ALIFWBYTES_AS_STRING(_obj);
    if (_len != nullptr) {
        *_len = ((AlifVarObject*)_obj)->size_;
    }
    else if (wcslen(*_s) != (AlifUSizeT)((AlifVarObject*)_obj)->size_) {
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

    result_ = alifBytes_fromStringAndSize(NULL, va_.len + vb_.len);
    if (result_ != NULL) {
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

// in file codecs.c
const char* alif_hexdigits = "0123456789abcdef";

AlifObject* alifBytes_repr(AlifObject* obj, int smartQuotes)
{
    AlifWBytesObject* op = (AlifWBytesObject*)obj;
    int64_t i, length = ((AlifVarObject*)op)->size_;
    int64_t newSize, squotes, dquotes;
    AlifObject* v;
    unsigned char quote;
    const unsigned char* s;
    uint8_t* p;

    /* Compute size_ of output string */
    squotes = dquotes = 0;
    newSize = 3;
    s = (const unsigned char*)op->value_;
    for (i = 0; i < length; i++) {
        int64_t incr = 1;
        switch (s[i]) {
        case '\'': squotes++; break;
        case '"':  dquotes++; break;
        case '\\': case '\t': case '\n': case '\r':
            incr = 2; break; 
        default:
            if (s[i] < ' ' || s[i] >= 0x7f)
                incr = 4; 
        }
        if (newSize > LLONG_MAX - incr)
            goto overflow;
        newSize += incr;
    }
    quote = '\'';
    if (smartQuotes && squotes && !dquotes)
        quote = '"';
    if (squotes && quote == '\'') {
        if (newSize > LLONG_MAX - squotes)
            goto overflow;
        newSize += squotes;
    }

    v = alifNew_uStr(newSize, 127);
    if (v == NULL) {
        return NULL;
    }
    p = (uint8_t*)((AlifUStrObject*)v)->UTF;

    *p++ = 'b', *p++ = quote;
    for (i = 0; i < length; i++) {
        unsigned char c = op->value_[i];
        if (c == quote || c == '\\')
            *p++ = '\\', *p++ = c;
        else if (c == '\t')
            *p++ = '\\', *p++ = 't';
        else if (c == '\n')
            *p++ = '\\', *p++ = 'n';
        else if (c == '\r')
            *p++ = '\\', *p++ = 'r';
        else if (c < ' ' || c >= 0x7f) {
            *p++ = '\\';
            *p++ = 'x';
            *p++ = alif_hexdigits[(c & 0xf0) >> 4];
            *p++ = alif_hexdigits[c & 0xf];
        }
        else
            *p++ = c;
    }
    *p++ = quote;
    return v;

overflow:
    std::wcout << L"كائن البايت كبير جدا لعمل تمثيل نصي\n" << std::endl;
    exit(-1);
    return nullptr;
}

static AlifObject* bytes_repr(AlifObject* object)
{
    return alifBytes_repr(object, 1);
}

static int64_t bytes_length(AlifWBytesObject* value) {

    return ((AlifVarObject*)value)->size_;

}

static AlifObject* bytes_repeat(AlifWBytesObject* object, int64_t repeat) {

    if (repeat < 0){
        repeat = 0;
    }

    int64_t size_ = ((AlifVarObject*)object)->size_ * repeat;
    if (size_ == ((AlifVarObject*)object)->size_ && 
        ((AlifObject*)object)->type_ == &_typeBytes_) {
        return (AlifObject*)object;
    }
    size_t numberBytes = (size_t)size_;

    AlifWBytesObject* bytesObject = (AlifWBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + numberBytes);

    alifSubObject_initVar((AlifVarObject*)bytesObject, &_typeBytes_, size_);
    bytesObject->value_[size_] = '\0';

    bytes_subRepeat(bytesObject->value_, size_, object->value_, ((AlifVarObject*)object)->size_);

    return (AlifObject*)bytesObject;

}

static AlifObject* bytes_item(AlifWBytesObject* object, int64_t index) {

    if (index < 0 || ((AlifVarObject*)object)->size_ <= index) {
        std::wcout << L"مؤشر البايت خارج النطاق\n" << std::endl;
        exit(-1);
    }

    return alifInteger_fromLongLong((unsigned char)object->value_[index]);

}

static int bytes_compare_eq(AlifWBytesObject* a, AlifWBytesObject* b)
{
    int cmp;
    int64_t len;

    len = ((AlifVarObject*)a)->size_;
    if (((AlifVarObject*)b)->size_ != len)
        return 0;

    if (a->value_[0] != b->value_[0])
        return 0;

    cmp = memcmp(a->value_, b->value_, len);
    return (cmp == 0);
}

static AlifObject* bytes_richcompare(AlifWBytesObject* a, AlifWBytesObject* b, int op)
{
    int c;
    int64_t lenA, lenB;
    int64_t min_len;

    //if (!(Bytes_Check(a) && Bytes_Check(b))) {
    //    if (__GetConfig()->bytes_warning && (op == _EQ || op == _NE)) {
    //        if (Unicode_Check(a) || Unicode_Check(b)) {
    //            if (Err_WarnEx(Exc_BytesWarning,
    //                "Comparison between bytes and string", 1))
    //                return NULL;
    //        }
    //        if (Long_Check(a) || Long_Check(b)) {
    //            if (Err_WarnEx(Exc_BytesWarning,
    //                "Comparison between bytes and int", 1))
    //                return NULL;
    //        }
    //    }
    //    ALIF_RETURN_NOTIMPLEMENTED;
    //}
    if (a == b) {
        switch (op) {
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
    else if (op == ALIF_EQ || op == ALIF_NE) {
        int eq = bytes_compare_eq(a, b);
        eq ^= (op == ALIF_NE);
        return alifBool_fromInteger(eq);
    }
    else {
        lenA = ((AlifVarObject*)a)->size_;
        lenB = ((AlifVarObject*)b)->size_;
        min_len = min(lenA, lenB);
        if (min_len > 0) {
            c = ALIF_WCHARMASK(*a->value_) - ALIF_WCHARMASK(*b->value_);
            if (c == 0)
                c = memcmp(a->value_, b->value_, min_len);
        }
        else
            c = 0;
        if (c != 0)
            ALIF_RETURN_RICHCOMPARE(c, 0, op);
        ALIF_RETURN_RICHCOMPARE(lenA, lenB, op);
    }
}

// in file bytes_methods.c
int alif_bytes_contain(const wchar_t *str, int64_t length, AlifObject* arg) { 

    int64_t value = alifInteger_asLong(arg);

    if (value < 0 || value >= 256) {
        std::wcout << L"يجب ان يكون البايت ضمن نطاق (0, 256)\n" << std::endl;
        exit(-1);
    }

    return memchr(str, (int)value, length) != nullptr;
}

static int bytes_contains(AlifObject* self, AlifObject *arg) {

    return alif_bytes_contain(((AlifWBytesObject*)self)->value_, ((AlifVarObject*)self)->size_, arg);
}

void alifBytes_dealloc(AlifObject* object) {
    
    alifMem_objFree(object);

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
    0, //alifObject_GenericGetAttr,           
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
    if (*_pv == NULL)
        return;
    if (_w == NULL) {
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
        /* Multiple references, need to create new object */
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
        /* return early if newsize equals to v->ob_size */
        return 0;
    }
    if (oldSize == 0) {
        *_pv = alifSubBytes_fromSize(_newSize);
        ALIF_DECREF(v_);
        return (*_pv == NULL) ? -1 : 0;
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
        return (*_pv == NULL) ? -1 : 0;
    }


    * _pv = (AlifObject*)
        alifMem_objRealloc(v_, ALIFBYTESOBJECT_SIZE + _newSize);
    if (*_pv == NULL) {
        alifMem_objFree(v_);
        return -1;
    }
    sv_ = (AlifWBytesObject*)*_pv;
    ALIFSET_SIZE(sv_, _newSize);
    sv_->value_[_newSize] = '\0';
    return 0;
}

void bytes_subRepeat(wchar_t* destinaion, int64_t lengthDest,
    const wchar_t* source, int64_t lengthSrc)
{
    if (lengthDest == 0) {
        return;
    }
    if (lengthSrc == 1) {
        memset(destinaion, source[0], lengthDest);
    }
    else {
        if (source != destinaion) {
            memcpy(destinaion, source, lengthSrc);
        }
        int64_t copied = lengthSrc;
        while (copied < lengthDest) {
            int64_t bytesToCopy = min(copied, lengthDest - copied);
            memcpy(destinaion + copied, destinaion, bytesToCopy);
            copied += bytesToCopy;
        }
    }
}
