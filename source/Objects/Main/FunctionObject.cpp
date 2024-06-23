#include "alif.h"

#include "AlifCore_AlifEval.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"



AlifFunctionObject* alifFunction_fromConstructor(AlifFrameConstructor* _constr) { // 101

	AlifObject* module{};
	//AlifObject* name = alifUStr_decodeStringToUTF8(L"__name__"); // temp
	//if (alifDict_getItemRef(_constr->fcGlobals, name, &module) < 0) {
	//	return nullptr;
	//}

	AlifFunctionObject* op = ALIFOBJECT_GC_NEW(AlifFunctionObject, &_alifFunctionType_);
	if (op == nullptr) {
		ALIF_XDECREF(module);
		return nullptr;
	}
	//op->funcGlobals = ALIF_NEWREF(_constr->fcGlobals);
	//op->funcBuiltins = ALIF_NEWREF(_constr->fcBuiltins);
	op->funcName = ALIF_NEWREF(_constr->fcName);
	op->funcQualname = ALIF_NEWREF(_constr->fcQualname);
	op->funcCode = ALIF_NEWREF(_constr->fcCode);
	//op->funcDefaults = ALIF_XNEWREF(_constr->fcDefaults);
	//op->funcKwdefaults = ALIF_XNEWREF(_constr->fcKwdefaults);
	//op->funcClosure = ALIF_XNEWREF(_constr->fcClosure);
	op->funcDoc = ALIF_NEWREF(ALIF_NONE);
	op->funcDict = nullptr;
	op->funcWeakRefList = nullptr;
	op->funcModule = module;
	op->funcTypeParams = nullptr;
	//op->vectorCall = alifFunction_vectorCall;
	op->funcVersion = 0;

	ALIFOBJECT_GC_TRACK(op);
	//handle_funcEvent(AlifFunction_Event_Create, op, nullptr);
	return op;
}



AlifTypeObject _alifFunctionType_ = {
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
