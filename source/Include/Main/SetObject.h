#pragma once


extern AlifTypeObject _TypeSet_;

AlifObject* alifNew_set(AlifObject*);

int alifSet_add(AlifObject*, AlifObject*);
int alifSet_contains(AlifObject*, AlifObject*);
int alifSet_discard(AlifObject*, AlifObject*);


#define ALIFSET_MINSIZE 8

class SetEntry {
public:
    AlifObject* key_;
    size_t hash_;             
} ;

class AlifSetObject{
public:
    ALIFOBJECT_HEAD;
    int64_t fill_;          
    int64_t used_;         
    int64_t mask_;
    SetEntry* table_;
    size_t hash_;             
    int64_t finger_;         
    SetEntry smallTable[ALIFSET_MINSIZE];
    AlifObject* weakreFlist;     
} ;