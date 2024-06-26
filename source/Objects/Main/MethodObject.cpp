#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_Memory.h"

static AlifObject* cFunction_vectorCallFastCall(
    AlifObject* , AlifObject* const* , size_t , AlifObject* );
static AlifObject* cFunctionVectorCall_fastCallKeywords(
    AlifObject* , AlifObject* const* , size_t , AlifObject* );
static AlifObject* cFunctionVectorCallFastCallKeywordsMethod(
    AlifObject* , AlifObject* const* , size_t , AlifObject* );
static AlifObject* cFunctionVectorCallNoArg(
    AlifObject* , AlifObject* const* , size_t , AlifObject* );
static AlifObject* cFunction_vectorCall(
    AlifObject* , AlifObject* const* , size_t , AlifObject* );
static AlifObject* cFunction_call(
    AlifObject* , AlifObject* , AlifObject* );

AlifObject* alifNew_cFunction(AlifMethodDef* _method, AlifObject* _self) {

	return alifNew_cMethod(_method, _self, nullptr, nullptr);

}

AlifObject* alifNew_cMethod(AlifMethodDef* _method, AlifObject* _self, AlifObject* _module, AlifInitObject* _cls)
{
    VectorCallFunc vectorCall;
    switch (_method->flags & (METHOD_VARARGS | METHOD_FASTCALL | METHOD_NOARGS |
        METHOD_O | METHOD_KEYWORDS | METHOD_METHOD))
    {
    case METHOD_VARARGS:
    case METHOD_VARARGS | METHOD_KEYWORDS:
        vectorCall = nullptr;
        break;
    case METHOD_FASTCALL:
        vectorCall = cFunction_vectorCallFastCall;
        break;
    case METHOD_FASTCALL | METHOD_KEYWORDS:
        vectorCall = cFunctionVectorCall_fastCallKeywords;
        break;
    case METHOD_NOARGS:
        vectorCall = cFunctionVectorCallNoArg;
        break;
    case METHOD_O:
        vectorCall = cFunction_vectorCall;
        break;
    case METHOD_METHOD | METHOD_FASTCALL | METHOD_KEYWORDS:
        vectorCall = cFunctionVectorCallFastCallKeywordsMethod;
        break;
    default:
        // error
        return nullptr;
    }

    AlifCFunctionObject* objectFunc = nullptr;

    if (_method->flags & METHOD_METHOD) {
        if (!_cls) {
            // error
            return nullptr;
        }
        AlifCMethodObject* objectMethod = (AlifCMethodObject*)alifMem_objAlloc(sizeof(AlifCMethodObject));
        alifSubObject_init((AlifObject*)objectMethod, &_alifCppMethodType_);
        if (objectMethod == nullptr) {
            return nullptr;
        }
        objectMethod->mMClass = (AlifInitObject*)_cls;
        objectFunc = (AlifCFunctionObject*)objectMethod;
    }
    else {
        if (_cls) {
            // error
            return nullptr;
        }
        objectFunc = (AlifCFunctionObject*)alifMem_objAlloc(sizeof(AlifCFunctionObject));
        alifSubObject_init((AlifObject*)objectFunc, &typeCFunction);
        if (objectFunc == nullptr) {
            return nullptr;
        }
    }

    objectFunc->method = _method;
    objectFunc->self = _self;
    objectFunc->module = _module;
    objectFunc->vectorCall = vectorCall;
    return (AlifObject*)objectFunc;
}

AlifCFunction alifCFunction_getFunction(AlifObject* _object) {
    return ((AlifCFunctionObject*)(_object))->method->method;
}

AlifObject* alifCFunction_getSelf(AlifObject* _object) {

    AlifCFunctionObject* func_ = (AlifCFunctionObject*)_object;
    if (func_->method->flags & METHOD_STATIC) {
        return nullptr;
    }
    
    return func_->self;
}

int alifCFunction_getFlags(AlifObject* _object) {
    return ((AlifCFunctionObject*)(_object))->method->flags;
}

AlifInitObject* alifCFunction_getClass(AlifObject* _object) {
    AlifCFunctionObject* func_ = (AlifCFunctionObject*)(_object);
    if (func_->method->flags & METHOD_METHOD) {
        return ((AlifCMethodObject *)func_)->mMClass;
    }
    return nullptr;
}

AlifObject* method_compare(AlifObject* _self, AlifObject* _other, int _op)
{
    AlifCFunctionObject* a_, * b_;
    AlifObject* res_ = nullptr;

    if ((_op != ALIF_EQ && _op != ALIF_NE) ||
        !(_self->type_ == &typeCFunction) ||
        !(_other->type_ == &typeCFunction))
    {
        // error not Implemented;
    }
    a_ = (AlifCFunctionObject*)_self;
    b_ = (AlifCFunctionObject*)_other;
    int eq_ = a_->self == b_->self;
    if (eq_)
        eq_ = a_->method->method == b_->method->method;
    if (_op == ALIF_EQ)
        res_ = eq_ ? ALIF_TRUE : ALIF_FALSE;
    else
        res_ = eq_ ? ALIF_FALSE : ALIF_TRUE;
    return res_;
}

size_t method_hash(AlifCFunctionObject* _object) {

    size_t hashPointer = (uintptr_t)_object->self ^ 1073741827ull;
    
    return hashPointer;
}

void method_dealloc(AlifCFunctionObject* _object) {
    alifMem_objFree(_object);
}

AlifTypeObject _alifTypeCFunction_ = {
    0,
	0,
	0,
    L"builtin_function_or_method",
    sizeof(AlifCFunctionObject),
    0,
    (Destructor)method_dealloc,                   /* tp_dealloc */
    offsetof(AlifCFunctionObject, vectorCall),    /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                        /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    (HashFunc)method_hash,                        /* tp_hash */
    cFunction_call,                             /* tp_call */
    0,
    0,
    0,                                          /* tp_str */
    0,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    0,                 /* tp_flags */
    0,                                          /* tp_doc */
    0,                /* tp_traverse */
    0,                                          /* tp_clear */
    0,                           /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                               /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
};

AlifInitObject _alifCppMethodType_ = {
    0,
	0,
	0,
    L"builtin_method",
    sizeof(AlifCMethodObject),
    0,                                    
    0,                                   
    0,                                         
    0,                       
    0,                                          
    0,                                          
    0,                                          
    0,
    0,
    0,
    0,                                          
    0,                    
    0,                                         
    0,                                          
    0,                 
    0,                                          
    0,                
    0,                                          
    0,                           
    0, 
    0,                                          
    0,                                          
    0,                               
    0,                              
    0,                               
    0,                                     
    &typeCFunction,
};


typedef void (*FuncPtr)(void);

FuncPtr cFunction_enterCall(AlifObject* _object) {

    return (FuncPtr)alifCFunction_getFunction(_object);

}

static AlifObject* cFunction_vectorCallFastCall(
    AlifObject* _object, AlifObject* const* _args, size_t _nArgsF, AlifObject* _kwNames)
{
    //if (cfunction_check_kwargs(object, kwnames)) {
        //return nullptr;
    //}
    int64_t nArgs = _nArgsF & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    AlifCFunctionFast method_ = (AlifCFunctionFast)
        cFunction_enterCall(_object);
    if (method_ == nullptr) {
        return nullptr;
    }
    AlifObject* result_ = method_(alifCFunction_getSelf(_object), _args, nArgs);
    return result_;
}

static AlifObject* cFunctionVectorCall_fastCallKeywords(
    AlifObject* _object, AlifObject* const* _args, size_t _nArgsF, AlifObject* _kwNames)
{
    int64_t nArgs = _nArgsF & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    AlifCFunctionFastWithKeywords method_ = (AlifCFunctionFastWithKeywords)
        cFunction_enterCall(_object);
    if (method_ == nullptr) {
        return nullptr;
    }
    AlifObject* result_ = method_(alifCFunction_getSelf(_object), _args, nArgs, _kwNames);
    return result_;
}

static AlifObject* cFunctionVectorCallFastCallKeywordsMethod(
    AlifObject* _object, AlifObject* const* _args, size_t _nArgsF, AlifObject* _kwNames)
{
    AlifInitObject* cls_ = alifCFunction_getClass(_object);
    int64_t nArgs = _nArgsF & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    AlifCMethod method_ = (AlifCMethod)cFunction_enterCall(_object);
    if (method_ == nullptr) {
        return nullptr;
    }
    AlifObject* result_ = method_(alifCFunction_getSelf(_object), cls_, _args, nArgs, _kwNames);
    return result_;
}

static AlifObject* cFunctionVectorCallNoArg(
    AlifObject* _object, AlifObject* const* _args, size_t _nArgsF, AlifObject* _kwNames)
{
    //if (cfunction_check_kwargs(object, kwnames)) {
        //return nullptr;
    //}
    int64_t nArgs = _nArgsF & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    if (nArgs != 0) {
        //AlifObject* funcStr = AlifObj_FunctionStr(object);
        //if (funcStr != nullptr) {
            // error
        //}
        return nullptr;
    }
    AlifCFunction method_ = (AlifCFunction)cFunction_enterCall(_object);
    if (method_ == nullptr) {
        return nullptr;
    }
    AlifObject* result_ = method_(alifCFunction_getSelf(_object), nullptr);
    return result_;
}

static AlifObject* cFunction_vectorCall(
    AlifObject* _object, AlifObject* const* _args, size_t _nArgsF, AlifObject* _kwNames)
{
    
    //if (cfunction_check_kwargs(func, kwnames)) {
        //return nullptr;
    //}
    int64_t nArgs = _nArgsF & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    if (nArgs != 1) {
        //AlifObject* funcstr = AlifObj_FunctionStr(func);
        //if (funcstr != nullptr) {
            // error 
        //}
        return nullptr;
    }
    AlifCFunction method_ = (AlifCFunction)cFunction_enterCall(_object);
    if (method_ == nullptr) {
        return nullptr;
    }
    AlifObject* result = method_(alifCFunction_getSelf(_object), _args[0]);
    return result;
}

static AlifObject* cFunction_call(AlifObject* _func, AlifObject* _args, AlifObject* _kwArgs) {

    int flags_ = alifCFunction_getFlags(_func);
    if (!(flags_ & METHOD_VARARGS)) {
        return alifVectorCall_call(_func, _args, _kwArgs);
    }

    AlifCFunction method_ = alifCFunction_getFunction(_func);
    AlifObject* self_ = alifCFunction_getSelf(_func);
    AlifObject* result_{};
    if (flags_ & METHOD_KEYWORDS) {
		result_ = (*(AlifCFunctionWithKeywords)(void(*)(void))method_)
            (self_, _args, _kwArgs);

    }
    else {
        if (_kwArgs != nullptr && ((AlifDictObject*)_kwArgs)->size_ != 0) {
            return nullptr;
        }
		result_ = (method_)(self_, _args);
    }

    return result_;
}
