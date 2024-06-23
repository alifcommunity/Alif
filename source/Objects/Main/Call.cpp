#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_Call.h"
#include "AlifCore_Dict.h"

#define ALIFVECTORCALL_ARGUMENTS_OFFSET \
    ((size_t)1 << (8 * sizeof(size_t) - 1))

static AlifObject* alifVectorCall_callSub(VectorCallFunc func,
    AlifObject* callable, AlifObject* tuple, AlifObject* kwArgs)
{

    int64_t nargs = ((AlifTupleObject*)tuple)->_base_.size_;

    /* Fast path for no keywords */
    if (kwArgs == nullptr || ((AlifDictObject*)kwArgs)->size_ == 0) {
        return func(callable, ((AlifTupleObject*)tuple)->items_, nargs, nullptr);
    }

    /* Convert arguments & call */
    AlifObject* const* args{};
    AlifObject* kwNames{};
    args = alifStack_unpackDict(
        ((AlifTupleObject*)tuple)->items_, nargs,
        kwArgs, &kwNames);
    if (args == nullptr) {
        return nullptr;
    }
    AlifObject* result = func(callable, args,
        nargs | ALIFVECTORCALL_ARGUMENTS_OFFSET, kwNames);
    //alifStack_unpackDict_free(args, nargs, kwNames);

    return result;
}

AlifObject* alifVectorCall_call(AlifObject* callable, AlifObject* tuple, AlifObject* kwArgs)
{

    int64_t offset = callable->type_->vectorCallOffset;
    if (offset <= 0) {
        //_PyErr_Format(tstate, PyExc_TypeError,
        //    "'%.200s' object does not support vectorcall",
        //    Py_TYPE(callable)->tp_name);
        return nullptr;
    }

    VectorCallFunc func{};
    memcpy(&func, (char*)callable + offset, sizeof(func));
    if (func == nullptr) {
        //_PyErr_Format(tstate, PyExc_TypeError,
        //    "'%.200s' object does not support vectorcall",
        //    Py_TYPE(callable)->tp_name);
        return nullptr;
    }

    return alifVectorCall_callSub(func, callable, tuple, kwArgs);
}

AlifObject* const* alifStack_unpackDict(AlifObject* const* args, int64_t nArgs,
    AlifObject* kwArgs, AlifObject** p_kwnames)
{

    int64_t nKwArgs = ((AlifDictObject*)kwArgs)->size_;

    int64_t maxnargs = INT64_MAX / sizeof(args[0]) - 1;
    if (nArgs > maxnargs - nKwArgs) {
        return nullptr;
    }

    AlifObject** stack = (AlifObject**)alifMem_objAlloc((1 + nArgs + nKwArgs) * sizeof(args[0]));
    if (stack == nullptr) {
        return nullptr;
    }

    AlifObject* kwNames = alifNew_tuple(nKwArgs);
    if (kwNames == nullptr) {
        alifMem_objFree(stack);
        return nullptr;
    }

    stack++; 

    for (int64_t i = 0; i < nArgs; i++) {
        stack[i] = args[i];
    }

    AlifObject** kwStack = stack + nArgs;

    int64_t pos = 0, i = 0;
    AlifObject* key{}, * value{};
    unsigned long keys_are_strings = ALIFTPFLAGS_USTR_SUBCLASS;
    while (alifDict_next(kwArgs, &pos, &key, &value, nullptr)) {
        keys_are_strings &= key->type_->flags_;
        ((AlifTupleObject*)kwNames)->items_[i] = key;
        kwStack[i] = value;
        i++;
    }

    //if (!keys_are_strings) {
    //    _PyErr_SetString(tstate, PyExc_TypeError,
    //        "keywords must be strings");
    //    _PyStack_UnpackDict_Free(stack, nargs, kwNames);
    //    return nullptr;
    //}

    *p_kwnames = kwNames;
    return stack;
}

void alifStack_unpackDict_free(AlifObject* const* stack, int64_t nArgs,
    AlifObject* kwNames)
{
    int64_t n = ((AlifTupleObject*)kwNames)->_base_.size_ +nArgs;
    //for (int64_t i = 0; i < n; i++) {
    //    ALIF_DECREF(stack[i]);
    //}
    //alifStack_unpackDict_freeNoDecRef(stack, kwNames);
}

void alifStack_unpackDict_freeNoDecRef(AlifObject* const* stack, AlifObject* kwNames)
{
    alifMem_objFree((AlifObject**)stack - 1);
    //ALIF_DECREF(kwNames);
}
