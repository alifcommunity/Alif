#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_Memory.h"

static AlifObject* cFunction_vectorCall_fastCall(
    AlifObject* func, AlifObject* const* args, size_t nargsf, AlifObject* kwnames);
static AlifObject* cFunction_vectorCall_fastCall_keywords(
    AlifObject* func, AlifObject* const* args, size_t nargsf, AlifObject* kwnames);
static AlifObject* cFunction_vectorCall_fastCall_keywords_method(
    AlifObject* func, AlifObject* const* args, size_t nargsf, AlifObject* kwnames);
static AlifObject* cFunction_vectorCall_noArg(
    AlifObject* func, AlifObject* const* args, size_t nargsf, AlifObject* kwnames);
static AlifObject* cFunction_vectorCall(
    AlifObject* func, AlifObject* const* args, size_t nargsf, AlifObject* kwnames);
static AlifObject* cFunction_call(
    AlifObject* func, AlifObject* args, AlifObject* kwargs);

AlifObject* alifNew_cFunction(AlifMethodDef* method, AlifObject* self) {

	return alifNew_cMethod(method, self, nullptr, nullptr);

}

AlifObject* alifNew_cMethod(AlifMethodDef* method, AlifObject* self, AlifObject* module, AlifInitObject* cls)
{
    VectorCallFunc vectorCall;
    switch (method->flags & (METHOD_VARARGS | METHOD_FASTCALL | METHOD_NOARGS |
        METHOD_O | METHOD_KEYWORDS | METHOD_METHOD))
    {
    case METHOD_VARARGS:
    case METHOD_VARARGS | METHOD_KEYWORDS:
        vectorCall = nullptr;
        break;
    case METHOD_FASTCALL:
        vectorCall = cFunction_vectorCall_fastCall;
        break;
    case METHOD_FASTCALL | METHOD_KEYWORDS:
        vectorCall = cFunction_vectorCall_fastCall_keywords;
        break;
    case METHOD_NOARGS:
        vectorCall = cFunction_vectorCall_noArg;
        break;
    case METHOD_O:
        vectorCall = cFunction_vectorCall;
        break;
    case METHOD_METHOD | METHOD_FASTCALL | METHOD_KEYWORDS:
        vectorCall = cFunction_vectorCall_fastCall_keywords_method;
        break;
    default:
        // error
        return nullptr;
    }

    AlifCFunctionObject* objectFunc = nullptr;

    if (method->flags & METHOD_METHOD) {
        if (!cls) {
            // error
            return nullptr;
        }
        AlifCMethodObject* objectMethod = (AlifCMethodObject*)alifMem_objAlloc(sizeof(AlifCMethodObject));
        alifSubObject_init((AlifObject*)objectMethod, &typeCMethod);
        if (objectMethod == nullptr) {
            return nullptr;
        }
        objectMethod->mMClass = (AlifInitObject*)cls;
        objectFunc = (AlifCFunctionObject*)objectMethod;
    }
    else {
        if (cls) {
            // error
            return nullptr;
        }
        objectFunc = (AlifCFunctionObject*)alifMem_objAlloc(sizeof(AlifCFunctionObject));
        alifSubObject_init((AlifObject*)objectFunc, &typeCFunction);
        if (objectFunc == nullptr) {
            return nullptr;
        }
    }

    objectFunc->method = method;
    objectFunc->self = self;
    objectFunc->module = module;
    objectFunc->vectorCall = vectorCall;
    return (AlifObject*)objectFunc;
}

AlifCFunction alifCFunction_getFunction(AlifObject* object) {
    return ((AlifCFunctionObject*)(object))->method->method;
}

AlifObject* alifCFunction_getSelf(AlifObject* object) {

    AlifCFunctionObject* func = (AlifCFunctionObject*)object;
    if (func->method->flags & METHOD_STATIC) {
        return nullptr;
    }
    
    return func->self;
}

int alifCFunction_getFlags(AlifObject* object) {
    return ((AlifCFunctionObject*)(object))->method->flags;
}

AlifInitObject* alifCFunction_getClass(AlifObject* object) {
    AlifCFunctionObject* func = (AlifCFunctionObject*)(object);
    if (func->method->flags & METHOD_METHOD) {
        return ((AlifCMethodObject *)func)->mMClass;
    }
    return nullptr;
}

AlifObject* method_compare(AlifObject* self, AlifObject* other, int op)
{
    AlifCFunctionObject* a, * b;
    AlifObject* res = nullptr;

    if ((op != ALIF_EQ && op != ALIF_NE) ||
        !(self->type_ == &typeCFunction) ||
        !(other->type_ == &typeCFunction))
    {
        // error not Implemented;
    }
    a = (AlifCFunctionObject*)self;
    b = (AlifCFunctionObject*)other;
    int eq = a->self == b->self;
    if (eq)
        eq = a->method->method == b->method->method;
    if (op == ALIF_EQ)
        res = eq ? ALIF_TRUE : ALIF_FALSE;
    else
        res = eq ? ALIF_FALSE : ALIF_TRUE;
    return res;
}

size_t method_hash(AlifCFunctionObject* object) {

    size_t hashPointer = (uintptr_t)object->self ^ 1073741827ull;
    
    return hashPointer;
}

void method_dealloc(AlifCFunctionObject* object) {
    alifMem_objFree(object);
}

AlifTypeObject typeCFunction = {
    0,0,0,
    //PyVarObject_HEAD_INIT(&PyType_Type, 0)
    L"builtin_function_or_method",
    sizeof(AlifCFunctionObject),
    0,
    (Destructor)method_dealloc,                   /* tp_dealloc */
    offsetof(AlifCFunctionObject, vectorCall),    /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    0,                        /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    //(hashfunc)meth_hash,                        /* tp_hash */
    0,
    //cfunction_call,                             /* tp_call */
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

AlifInitObject typeCMethod = {
    0,0,0,
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

FuncPtr cFunction_enterCall(AlifObject* object) {

    return (FuncPtr)alifCFunction_getFunction(object);

}

static AlifObject* cFunction_vectorCall_fastCall(
    AlifObject* object, AlifObject* const* args, size_t nargsf, AlifObject* kwnames)
{
    //if (cfunction_check_kwargs(object, kwnames)) {
        //return NULL;
    //}
    int64_t nargs = nargsf & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    AlifCFunctionFast method = (AlifCFunctionFast)
        cFunction_enterCall(object);
    if (method == nullptr) {
        return nullptr;
    }
    AlifObject* result = method(alifCFunction_getSelf(object), args, nargs);
    return result;
}

static AlifObject* cFunction_vectorCall_fastCall_keywords(
    AlifObject* object, AlifObject* const* args, size_t nArgsF, AlifObject* kWNames)
{
    int64_t nArgs = nArgsF & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    AlifCFunctionFastWithKeywords method = (AlifCFunctionFastWithKeywords)
        cFunction_enterCall(object);
    if (method == nullptr) {
        return nullptr;
    }
    AlifObject* result = method(alifCFunction_getSelf(object), args, nArgs, kWNames);
    return result;
}

static AlifObject* cFunction_vectorCall_fastCall_keywords_method(
    AlifObject* object, AlifObject* const* args, size_t nArgsF, AlifObject* kWNames)
{
    AlifInitObject* cls = alifCFunction_getClass(object);
    int64_t nArgs = nArgsF & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    AlifCMethod method = (AlifCMethod)cFunction_enterCall(object);
    if (method == nullptr) {
        return nullptr;
    }
    AlifObject* result = method(alifCFunction_getSelf(object), cls, args, nArgs, kWNames);
    return result;
}

static AlifObject* cFunction_vectorCall_noArg(
    AlifObject* object, AlifObject* const* args, size_t nargsf, AlifObject* kwnames)
{
    //if (cfunction_check_kwargs(object, kwnames)) {
        //return NULL;
    //}
    int64_t nArgs = nargsf & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    if (nArgs != 0) {
        //AlifObject* funcStr = AlifObj_FunctionStr(object);
        //if (funcStr != nullptr) {
            // error
        //}
        return nullptr;
    }
    AlifCFunction method = (AlifCFunction)cFunction_enterCall(object);
    if (method == nullptr) {
        return nullptr;
    }
    AlifObject* result = method(alifCFunction_getSelf(object), nullptr);
    return result;
}

static AlifObject* cFunction_vectorCall(
    AlifObject* object, AlifObject* const* args, size_t nArgsF, AlifObject* kWNames)
{
    
    //if (cfunction_check_kwargs(func, kwnames)) {
        //return NULL;
    //}
    int64_t nArgs = nArgsF & ~((size_t)1 << (8 * sizeof(size_t) - 1));
    if (nArgs != 1) {
        //AlifObject* funcstr = AlifObj_FunctionStr(func);
        //if (funcstr != NULL) {
            // error 
        //}
        return nullptr;
    }
    AlifCFunction method = (AlifCFunction)cFunction_enterCall(object);
    if (method == nullptr) {
        return nullptr;
    }
    AlifObject* result = method(alifCFunction_getSelf(object), args[0]);
    return result;
}

static AlifObject* cFunction_call(AlifObject* func, AlifObject* args, AlifObject* kwArgs) {

    int flags = alifCFunction_getFlags(func);
    if (!(flags & METHOD_VARARGS)) {
        return alifVectorCall_call(func, args, kwArgs);
    }

    AlifCFunction method = alifCFunction_getFunction(func);
    AlifObject* self = alifCFunction_getSelf(func);
    AlifObject* result{};
    if (flags & METHOD_KEYWORDS) {
        result = (*(AlifCFunctionWithKeywords)(void(*)(void))method)
            (self, args, kwArgs);

    }
    else {
        //if (kwargs != NULL && Dict_GET_SIZE(kwargs) != 0) {
        //    Err_Format(tstate, Exc_TypeError,
        //        "%.200s() takes no keyword arguments",
        //        ((AlifCFunctionObject*)func)->m_ml->ml_name);
        //    return NULL;
        //}
        result = (method)(self, args);
    }

    return result;
}