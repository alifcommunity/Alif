#include "alif.h"

class TypeNew {
public:
    AlifInitObject* metatype;
    AlifObject* args;
    AlifObject* kwds;
    AlifObject* origDict;
    AlifObject* name;
    AlifObject* bases;
    AlifInitObject* base;
    AlifObject* slots;
    int64_t nSlot;
    int addDict;
    int addWeak;
    int mayAddDict;
    int mayAddWeak;
};

AlifObject* alifNew_type(AlifInitObject* metatype, AlifObject* args, AlifObject* kwds)
{

    AlifObject* name, * bases, * origDict;
    //if (!alifArg_ParseTuple(args, "UO!O!:type_.__new__",
    //    &name,
    //    &alifTuple_Type, &bases,
    //    &alifDict_Type, &orig_dict))
    //{
    //    return NULL;
    //}

    //TypeNew ctx = {
    //    metatype,
    //    args,
    //    kwds,
    //    origDict,
    //    name,
    //    bases,
    //    nullptr,
    //    nullptr,
    //    0,
    //    0,
    //    0,
    //    0,
    //    0 
    //};
    AlifObject* type_ = nullptr;
    //int res = type_new_get_bases(&ctx, &type_);
    //if (res < 0) {
        //return nullptr;
    //}
    //if (res == 1) {
        //return type_;
    //}

    //type_ = type_new_impl(&ctx);
    return type_;
}


AlifInitObject typeType = {
    0,0,0,
    //ALIFVAROBJECT_HEAD_INIT(&typeType, 0)
    L"type",                                     
    sizeof(AlifHeapTypeObject),                  
    sizeof(AlifMemberDef),                        
    0,//estructor)type_dealloc,                   
    offsetof(AlifInitObject, vectorCall),      
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                        /* tp_repr */
    0, //&type_as_number,                           
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0, //(ternaryfunc)type_call,                   
    0,                                          /* tp_str */
    0,//(getattrofunc)_alif_type_getattro,           
    0,//(setattrofunc)type_setattro,               
    0,                    /* tp_flags */
    0,                                   /* tp_doc */
    0,                /* tp_traverse */
    0,                        /* tp_clear */
    0,                                         /* tp_richcompare */
    0,
    offsetof(AlifInitObject, weakList),       
    0,                                         
    0,                                          
    0, //type_methods,                              
    0, //type_members,                               
    0, //type_getsets,                               
    0,                                         
    0,                                         
    0,                                          
    0,                                          
    offsetof(AlifInitObject, dict_),           
    0, //type_init,                                  
    0,                                         
    0, //type_new,                                  
    0,                            
    0,                      
    //.tp_vectorcall = type_vectorcall,
};
