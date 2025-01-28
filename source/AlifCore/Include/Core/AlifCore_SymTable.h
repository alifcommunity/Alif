#pragma once


class Module; // 11

enum BlockType_ { // 13
	Function_Block, Class_Block, Module_Block,
	Annotation_Block,
	Type_Alias_Block,
	Type_Parameters_Block,
	Type_Variable_Block,
};

enum AlifComprehensionType { // 38
	No_Comprehension = 0,
	List_Comprehension = 1,
	Dict_Comprehension = 2,
	Set_Comprehension = 3,
	Generator_Expression = 4
};


class AlifSourceLocation { // 46
public:
	AlifIntT lineNo{};
	AlifIntT endLineNo{};
	AlifIntT colOffset{};
	AlifIntT endColOffset{};
};


// 53
#define SRC_LOCATION_FROM_AST(_n) { \
               .lineNo = (_n)->lineNo, \
               .endLineNo = (_n)->endLineNo, \
               .colOffset = (_n)->colOffset, \
               .endColOffset = (_n)->endColOffset }


static const AlifSourceLocation _noLocation_ = { -1, -1, -1, -1 }; // 60

class AlifFutureFeatures { // 63
public:
	AlifIntT features{};
	AlifSourceLocation location{};
};

class SymTableEntry; // 68

class AlifSymTable { // 70
public:
	AlifObject* fileName{};
	SymTableEntry* cur{}; 
	SymTableEntry* top{};
	AlifObject* blocks{};
	AlifObject* stack{};
	AlifObject* global{};
	AlifIntT nBlocks{};
	AlifObject* private_{};
	AlifFutureFeatures* future{};
	AlifIntT recursionDepth{};
	AlifIntT recursionLimit{};
};

class SymTableEntry { // 89
public:
	ALIFOBJECT_HEAD;
	AlifObject* id{};
	AlifObject* symbols{};
	AlifObject* name{};
	AlifObject* varNames{};
	AlifObject* children{};
	AlifObject* directives{};
	AlifObject* mangledNames{};

	BlockType_ type{};

	const char* scopeInfo{};

	AlifIntT nested{};
	unsigned generator : 1;   
	unsigned coroutine : 1;  
	unsigned annotationsUsed : 1;  
	AlifComprehensionType comprehension{};
	unsigned varArgs : 1;     
	unsigned varKeywords : 1;
	unsigned returnsValue : 1;
	unsigned needsClassClosure : 1;

	unsigned needsClassDict : 1; 
	unsigned compInlined : 1; 
	unsigned compIterTarget : 1; 
	unsigned canSeeClassScope : 1;

	AlifIntT compIterExpr{};
	AlifSourceLocation loc{};
	SymTableEntry* annotationBlock{};
	AlifSymTable* table{};
};

extern AlifTypeObject _alifSTEntryType_; // 132

extern long _alifST_getSymbol(SymTableEntry*, AlifObject*); // 136
extern AlifIntT alifST_getScope(SymTableEntry*, AlifObject*); // 137
extern AlifIntT alifST_isFunctionLike(SymTableEntry* ); // 138
extern AlifSymTable* alifSymtable_build(Module*, AlifObject*, AlifFutureFeatures*); // 140

extern SymTableEntry* _alifSymtable_lookup(AlifSymTable*, void*); // 144

extern void alifSymtable_free(AlifSymTable*); // 147

extern AlifObject* alif_maybeMangle(AlifObject*, SymTableEntry*, AlifObject*); // 149
extern AlifObject* alif_mangle(AlifObject*, AlifObject*); // 150
// 154
#define DEF_GLOBAL 1            
#define DEF_LOCAL 2           
#define DEF_PARAM (2<<1)       
#define DEF_NONLOCAL (2<<2)      
#define USE (2<<3)              
#define DEF_FREE_CLASS (2<<5)  
#define DEF_IMPORT (2<<6)        
#define DEF_ANNOT (2<<7)        
#define DEF_COMP_ITER (2<<8)    
#define DEF_TYPE_PARAM (2<<9)    
#define DEF_COMP_CELL (2<<10)    

#define DEF_BOUND (DEF_LOCAL | DEF_PARAM | DEF_IMPORT)

#define SCOPE_OFFSET 12
#define SCOPE_MASK (DEF_GLOBAL | DEF_LOCAL | DEF_PARAM | DEF_NONLOCAL)
#define SYMBOL_TO_SCOPE(S) (((S) >> SCOPE_OFFSET) & SCOPE_MASK)

#define LOCAL 1
#define GLOBAL_EXPLICIT 2
#define GLOBAL_IMPLICIT 3
#define FREE 4
#define CELL 5
// 181




AlifIntT alifFuture_fromAST(Module*, AlifObject*, AlifFutureFeatures*); // 189
