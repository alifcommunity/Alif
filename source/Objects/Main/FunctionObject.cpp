#include "alif.h"



AlifTypeObject typeFunction = {
    //ALIFVarObject_HEAD_INIT(&PyType_Type, 0)
    0,
    0,
    0,
    L"function",
    sizeof(AlifFunctionObject),
    0,
    0, //(destructor)func_dealloc,                   /* tp_dealloc */
    offsetof(AlifFunctionObject, vectorCall),     /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                        /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0, //Vectorcall_Call,                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,
    0,               /* tp_flags */
    0,                            /* tp_doc */
    0,                /* tp_traverse */
    0,                        /* tp_clear */
    0,                                          /* tp_richcompare */
    offsetof(AlifFunctionObject, funcWeakRefList), /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0, //func_memberlist,                            /* tp_members */
    0, //func_getsetlist,                            /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0, //func_descr_get,                             /* tp_descr_get */
    0,                                          /* tp_descr_set */
    offsetof(AlifFunctionObject, funcDict),      /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    //func_new,                                   /* tp_new */
};