#pragma once



enum AlifBlockType {
    FunctionBlock, ClassBlock, ModuleBlock,
    TypeVarBoundBlock, TypeAliasBlock, TypeParamBlock
};

enum AlifComprehensionType {
    NoComprehension = 0,
    ListComprehension,
    DictComprehension,
    SetComprehension,
    GeneratorExpression
};

/* source location information */
class SourceLocation {
public:
    AlifSizeT lineNo;
    AlifSizeT endLineNo;
    AlifSizeT colOffset;
    AlifSizeT endColOffset;
};



class AlifSTEntryObject { // 76
public:
    ALIFOBJECT_HEAD;
    AlifObject* steID;        /* int: key in ste_table->st_blocks */
    AlifObject* steSymbols;   /* dict: variable names to flags */
    AlifObject* steName;      /* string: name of current block */
    AlifObject* steVarNames;  /* list of function parameters */
    AlifObject* steChildren;  /* list of child blocks */
    AlifObject* steDirectives;/* locations of global and nonlocal statements */
    AlifBlockType steType;
    int steNested;      /* true if block is nested */
    unsigned steFree : 1;        /* true if block has free variables */
    unsigned steChildFree : 1;  /* true if a child block has free vars,
                                     including free refs to globals */
    unsigned steGenerator : 1;   /* true if namespace is a generator */
    unsigned steCoroutine : 1;   /* true if namespace is a coroutine */
    AlifComprehensionType stEComprehension;  /* Kind of comprehension (if any) */
    unsigned steVarArgs : 1;     /* true if block has varargs */
    unsigned steVarKeywords : 1; /* true if block has varkeywords */
    unsigned steReturnsValue : 1;  /* true if namespace uses return with
                                        an argument */
    unsigned steNeedsClassClosure : 1; /* for class scopes, true if a
                                             closure over __class__
                                             should be created */
    unsigned steNeedsClassDict : 1; /* for class scopes, true if a closure
                                         over the class dict should be created */
    unsigned steCompInlined : 1; /* true if this comprehension is inlined */
    unsigned steCompIterTarget : 1; /* true if visiting comprehension target */
    unsigned steCanSeeClassScope : 1; /* true if this block can see names bound in an
                                             enclosing class scope */
    int steCompIterExpr; /* non-zero if visiting a comprehension range expression */
    int steLineNo;          /* first line of block */
    int steColOffset;      /* offset of first line of block */
    int steEndLineNo;      /* end line of block */
    int steEndColOffset;  /* end offset of first line of block */
    int steOptLineNo;      /* lineno of last exec or import * */
    int steOptColOffset;  /* offset of last exec or import * */
    class AlifSymTable* stETable;
};



class AlifSymTable { // 57
public:
    AlifObject* stFileName;
    AlifSTEntryObject* stCur;
    AlifSTEntryObject* stTop;
    AlifObject* stBlocks;
    AlifObject* stStack;
    AlifObject* stGlobal;
    AlifIntT stNBlocks;
    AlifObject* stPrivate;
    AlifIntT recursionDepth;
    AlifIntT recursionLimit;
};


extern AlifTypeObject _alifSTEntryType_;

extern AlifIntT alifST_getSymbol(AlifSTEntryObject*, AlifObject*);
extern AlifIntT alifST_getScope(AlifSTEntryObject*, AlifObject*);
extern AlifIntT alifST_isFunctionLike(AlifSTEntryObject*);

extern class AlifSymTable* alifSymTable_build(class Module*, AlifObject*);
extern AlifSTEntryObject* alifSymTable_lookup(AlifSymTable*, void*);
extern void alifSymtable_free(AlifSymTable*);
extern AlifObject* alif_mangle(AlifObject*, AlifObject*);



#define DEF_GLOBAL 1         
#define DEF_LOCAL 2              
#define DEF_PARAM (2<<1)         
#define DEF_NONLOCAL (2<<2)   
#define USE (2<<3)              
#define DEF_FREE (2<<4)         
#define DEF_FREE_CLASS (2<<5)   
#define DEF_IMPORT (2<<6)      
#define DEF_ANNOT (2<<7)      
#define DEF_COMP_ITER (2<<8)     
#define DEF_TYPE_PARAM (2<<9)    
#define DEF_COMP_CELL (2<<10)  

#define DEF_BOUND (DEF_LOCAL | DEF_PARAM | DEF_IMPORT)


#define SCOPE_OFFSET 12
#define SCOPE_MASK (DEF_GLOBAL | DEF_LOCAL | DEF_PARAM | DEF_NONLOCAL)

#define LOCAL 1
#define GLOBAL_EXPLICIT 2
#define GLOBAL_IMPLICIT 3
#define FREE 4
#define CELL 5