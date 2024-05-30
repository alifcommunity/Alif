#include "alif.h"


static AlifObject* bool_repr(AlifObject* self) {

    const wchar_t* boolean = self == ALIF_TRUE ? L"True" : L"False";

    return alifUStr_objFromWChar((wchar_t*)boolean);

}

AlifObject* alifBool_fromInteger(long boolean) {

    return boolean ? ALIF_TRUE : ALIF_FALSE;

}

AlifObject* bool_and(AlifObject* a, AlifObject* b) {

    return alifBool_fromInteger((a == ALIF_TRUE) & (b == ALIF_TRUE));

}

AlifObject* bool_or(AlifObject* a, AlifObject* b) {

    return alifBool_fromInteger((a == ALIF_TRUE) | (b == ALIF_TRUE));

}

AlifObject* bool_xor(AlifObject* a, AlifObject* b) {

    return alifBool_fromInteger((a == ALIF_TRUE) ^ (b == ALIF_TRUE));

}

static AlifNumberMethods boolAsNumber = {
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
    bool_and,                   
    bool_xor,                  
    bool_or,                    
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
};

AlifInitObject typeBool = {
    0,
    0,
    0,
    L"bool",
    sizeof(size_t),
    sizeof(size_t),
    0,                              
    0,                                        
    0,                              
    0,                           
    bool_repr,
    &boolAsNumber,                     
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
    &_alifIntegerType_,                           
    0,                                      
    0,                                         
    0,                                         
    0,                                          
    0,                                    
    0,                                     
    0,                                  
    0,
};

class AlifIntegerObject alifFalse = {
    0, // ref of object
    &typeBool,
    0,
    1
};

class AlifIntegerObject alifTrue = {
    0, // ref of object
    &typeBool,
    1,
    1
};