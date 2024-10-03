#pragma once

#include "AlifCore_ASDL.h"







typedef class Mod* ModuleTy; // 15
typedef class Stmt* StmtTy;
typedef class Expr* ExprTy;
typedef enum ExprContext_ { Load = 1, Store = 2, Del = 3 };
typedef enum BoolOp_ { And = 1, Or = 2 };
typedef enum Operator_ {
	Add = 1, Sub = 2, Mult = 3, MatMult = 4, Div = 5, Mod = 6, Pow = 7,
	LShift = 8, RShift = 9, BitOr = 10, BitXor = 11, BitAnd = 12,
	FloorDiv = 13
};
typedef enum UnaryOp_ { Invert = 1, Not = 2, UAdd = 3, USub = 4, Sqrt = 5 };
typedef enum CmpOp_ {
	Equal = 1, NotEq = 2, LessThan = 3, LessThanEq = 4, GreaterThan = 5, GreaterThanEq = 6, Is = 7, IsNot = 8,
	In = 9, NotIn = 10
};
typedef class Comprehension* ComprehensionTy;
typedef class Excepthandler* ExcepthandlerTy;
typedef class Arguments* ArgumentsTy;
typedef class Arg* ArgTy;
typedef class Keyword* KeywordTy;
typedef class Alias* AliasTy;
typedef class Withitem* WithItemTy; // 46





class ASDLModSeq { // 57
public:
	ASDL_SEQ_HEAD;
	ModuleTy typedElements[1]{};
};

class ASDLStmtSeq { // 64
public:
	ASDL_SEQ_HEAD;
	StmtTy typedElements[1]{};
};

class ASDLExprSeq { // 71
public:
	ASDL_SEQ_HEAD;
	ExprTy typedElements[1]{};
};
//ASDLExprSeq* alifNew_exprSeq(AlifUSizeT, AlifASTMem*); // جسم هذه الدالة تم إنشاؤه من قبل GENERATE_SEQ_CONSTRUCTOR(Expr, expr, ExprTy*)

class ASDLComprehensionSeq { // 78
public:
	ASDL_SEQ_HEAD;
	ComprehensionTy typedElements[1]{};
};

class ASDLArgumentsSeq { // 94
	ASDL_SEQ_HEAD;
	ArgumentsTy typedElements[1]{};
};

class ASDLArgSeq { // 101
public:
	ASDL_SEQ_HEAD;
	ArgTy typedElements[1]{};
};
//ASDLArgSeq* alifNew_argSeq(AlifUSizeT, AlifASTMem*); // جسم هذه الدالة تم إنشاؤه من قبل GENERATE_SEQ_CONSTRUCTOR(Arg, arg, Arg*)

class ASDLKeywordSeq { // 108
public:
	ASDL_SEQ_HEAD;
	KeywordTy typedElements[1]{};
};
//ASDLKeywordSeq* alifNew_keywordSeq(AlifUSizeT, AlifASTMem*); // جسم هذه الدالة تم إنشاؤه من قبل GENERATE_SEQ_CONSTRUCTOR(Keyword, keyword, Keyword*)

class ASDLAliasSeq { // 115
public:
	ASDL_SEQ_HEAD;
	AliasTy typedElements[1]{};
};

class ASDLWithItemSeq { // 122
public:
	ASDL_SEQ_HEAD;
	WithItemTy typedElements[1]{};
};


enum ModK { ModuleK = 1, InteractiveK, ExpressionK, FunctionK }; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
class Mod { // 163
public:
	enum ModK type{};
	union
	{
		class {
		public:
			ASDLStmtSeq* body{};
		} module;

		class {
		public:
			ASDLStmtSeq* body{};
		} interactive;

		class {
		public:
			ExprTy body{};
		} expression;

		class {
		public:
			ASDLExprSeq* argTypes{};
			ExprTy returns{};
		} function;

	} V{};
};

enum StmtK { // 187
	ClassDefK = 1, FunctionDefK, AsyncFunctionDefK, ReturnK,
	DeleteK, AssignK, AugAssignK, ForK, AsyncForK, WhileK,
	IfK, WithK, AsyncWithK, TryK, ImportK, ImportFromK,
	ExprK, PassK, BreakK, CountinueK, GlobalK, NonlocalK,
}; 
class Stmt { // 196
public:
	enum StmtK type{};
	union
	{
		class {
		public:
			Identifier name{};
			ASDLExprSeq* bases{};
			ASDLKeywordSeq* keywords{};
			ASDLStmtSeq* body{};
		}classDef;

		class {
		public:
			Identifier name{};
			ArgumentsTy args{};
			ASDLStmtSeq* body{};
		}functionDef;

		class {
		public:
			Identifier name{};
			ArgumentsTy args{};
			ASDLStmtSeq* body{};
		}asyncFunctionDef;

		class {
		public:
			ExprTy val{};
		}return_;

		class {
		public:
			ASDLExprSeq* targets{};
		}delete_;

		class {
		public:
			ASDLExprSeq* targets{};
			ExprTy val{};
		}assign;

		class {
		public:
			ExprTy target{};
			Operator_ op{};
			ExprTy val{};
		}augAssign;

		class {
		public:
			ExprTy target{};
			ExprTy iter{};
			ASDLStmtSeq* body{};
		}for_;

		class {
		public:
			ExprTy target{};
			ExprTy iter{};
			ASDLStmtSeq* body{};
		}asyncFor;

		class {
		public:
			ExprTy condition{};
			ASDLStmtSeq* body{};
		}while_;

		class {
		public:
			ExprTy condition{};
			ASDLStmtSeq* body{};
			ASDLStmtSeq* else_{};
		}if_;

		class {
		public:
			ASDLWithItemSeq* items{};
			ASDLStmtSeq* body{};
		}with_;

		class {
		public:
			ASDLWithItemSeq* items{};
			ASDLStmtSeq* body{};
			String comment{};
		}asyncWith;

		class {
		public:
			ASDLAliasSeq* names{};
		}import;

		class {
		public:
			Identifier module{};
			ASDLAliasSeq* names{};
		}importFrom;

		class {
		public:
			ExprTy val{};
		}expression;

	} V{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

enum ExprK { // 359
	BoolOpK = 1, NamedExprK, BinOpK, UnaryOpK, IfExprK, DictK, SetK, ListK,
	DictCompK, SetCompK, ListCompK, GeneratorExprK, AwaitK, YieldK,
	YieldFromK, CompareK, CallK, FormattedValK, JoinStrK, ConstantK,
	AttributeK, SubScriptK, StarK, NameK, TupleK, SliceK
}; 
class Expr { // 367
public:
	enum ExprK type{};
	union
	{
		class {
		public:
			BoolOp_ op{};
			ASDLExprSeq* vals{};
		}boolOp;

		class {
		public:
			ExprTy target{};
			ExprTy val{};
		}namedExpr;

		class {
		public:
			ExprTy left{};
			Operator_ op{};
			ExprTy right{};
		}binOp;

		class {
		public:
			UnaryOp_ op{};
			ExprTy operand{};
		}unaryOp;

		class {
		public:
			ExprTy condition{};
			ExprTy body{};
			ExprTy else_{};
		}ifExpr;

		class {
		public:
			ASDLExprSeq* keys{};
			ASDLExprSeq* vals{};
		}dict;

		class {
		public:
			ASDLExprSeq* elts{};
		}set;

		class {
		public:
			ASDLExprSeq* elts{};
			ExprContext_ ctx{};
		}list;

		class {
		public:
			ExprTy key{};
			ExprTy val{};
			ASDLComprehensionSeq* generetors{};
		}dictComp;

		class {
		public:
			ExprTy elts{};
			ASDLComprehensionSeq* generetors{};
		}setComp;

		class {
		public:
			ExprTy elt{};
			ASDLComprehensionSeq* generetors{};
		}listComp;

		class {
		public:
			ExprTy val{};
		}await;

		class {
		public:
			ExprTy val{};
		}yield;

		class {
		public:
			ExprTy val{};
		}yieldFrom;

		class {
		public:
			ExprTy left{};
			ASDLIntSeq* ops{};
			ASDLExprSeq* comparators{};
		}compare;

		class {
		public:
			ExprTy func{};
			ASDLExprSeq* args{};
			ASDLKeywordSeq* keywords{};
		}call;

		class {
		public:
			ExprTy val{};
			AlifIntT conversion{};
			ExprTy formatSpec{};
		}fromattedValue;

		class {
		public:
			ASDLExprSeq* vals{};
		}joinStr;

		class {
		public:
			Constant val{};
			String type{};
		}constant;

		class {
		public:
			ExprTy val{};
			Identifier attr{};
			ExprContext_ ctx{};
		}attribute;

		class {
		public:
			ExprTy val{};
			ExprTy slice{};
			ExprContext_ ctx{};
		}subScript;

		class {
		public:
			ExprTy val{};
			ExprContext_ ctx{};
		}star;

		class {
		public:
			Identifier name{};
			ExprContext_ ctx{};
		}name;

		class {
		public:
			ASDLExprSeq* elts{};
			ExprContext_ ctx{};
		}tuple;

		class {
		public:
			ExprTy lower{};
			ExprTy upper{};
			ExprTy step{};
		}slice;

	} V{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

class Comprehension { // 516
public:
	ExprTy target{};
	ExprTy iter{};
	ASDLExprSeq* ifs{};
	AlifIntT isAsync{};
};

class Arguments { // 540
public:
	ASDLArgSeq* posOnlyArgs{};
	ASDLArgSeq* args{};
	ArgTy varArg{};
	ASDLArgSeq* kwOnlyArgs{};
	ASDLExprSeq* kwDefaults{};
	ArgTy kwArg{};
	ASDLExprSeq* defaults{};
};

class Arg { // 550
public:
	Identifier arg{};
	String comment{};

	AlifIntT lineNo{};
	AlifIntT colOffset{};
	AlifIntT endLineNo{};
	AlifIntT endColOffset{};
};

class Keyword { // 560
public:
	Identifier arg{};
	ExprTy val{};

	AlifIntT lineNo{};
	AlifIntT colOffset{};
	AlifIntT endLineNo{};
	AlifIntT endColOffset{};
};

class Alias { // 569
public:
	Identifier name{};
	Identifier asName{};

	AlifIntT lineNo{};
	AlifIntT colOffset{};
	AlifIntT endLineNo{};
	AlifIntT endColOffset{};
};

class WithItem { // 578
public:
	ExprTy contextExpr{};
	ExprTy optionalVars{};
};

enum TypeParamK { TypeVarK = 1, ParamSpec, TypeVarTupleK }; 
class TypeParam { // 654
public:
	enum TypeParamK type{};
	union
	{
		class {
			Identifier name{};
			ExprTy bound{};
		}typeVar;

		class {
			Identifier name{};
		}paramSpec;

		class {
			Identifier name{};
		}typeVarTuple;

	} V{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};


ModuleTy alifAST_module(ASDLStmtSeq*, AlifASTMem*);
ModuleTy alifAST_interactive(ASDLStmtSeq*, AlifASTMem*);
StmtTy alifAST_assign(ASDLExprSeq*, ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_expr(ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_constant(Constant, String, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_asyncFunctionDef(Identifier, ArgumentsTy, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_functionDef(Identifier, ArgumentsTy, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_return(ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_delete(ASDLExprSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_classDef(Identifier, ASDLExprSeq*, ASDLKeywordSeq*, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_augAssign(ExprTy, Operator_, ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_for(ExprTy, ExprTy, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_asyncFor(ExprTy, ExprTy, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_while(ExprTy, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_if(ExprTy, ASDLStmtSeq*, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_with(ASDLWithItemSeq*, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_asyncWith(ASDLWithItemSeq*, ASDLStmtSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_import(ASDLAliasSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_importFrom(Identifier, ASDLAliasSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_global(ASDLIdentifierSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_nonlocal(ASDLIdentifierSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_pass(AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_break(AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
StmtTy alifAST_continue(AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_boolOp(BoolOp_, ASDLExprSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_binOp(ExprTy, Operator_, ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_unaryOp(UnaryOp_, ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_ifExpr(ExprTy, ExprTy, ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_dict(ASDLExprSeq*, ASDLExprSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_listComp(ExprTy, ASDLComprehensionSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_dictComp(ExprTy, ExprTy, ASDLComprehensionSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_await(ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_yield(ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_yieldFrom(ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_compare(ExprTy, ASDLIntSeq*, ASDLExprSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_call(ExprTy, ASDLExprSeq*, ASDLKeywordSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_formattedValue(ExprTy, AlifIntT, ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_joinedStr(ASDLExprSeq*, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_attribute(ExprTy, Identifier, ExprContext_, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_subScript(ExprTy, ExprTy, ExprContext_, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_star(ExprTy, ExprContext_, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_name(Identifier, ExprContext_, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_list(ASDLExprSeq*, ExprContext_, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_tuple(ASDLExprSeq*, ExprContext_, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ExprTy alifAST_slice(ExprTy, ExprTy, ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
ComprehensionTy alifAST_comprehension(ExprTy, ExprTy, ASDLExprSeq*, AlifIntT, AlifASTMem*);
ArgumentsTy alifAST_arguments(ASDLArgSeq*, ASDLArgSeq*, ArgTy, ASDLArgSeq*, ASDLExprSeq*, Arg*, ASDLExprSeq*, AlifASTMem*);
ArgTy alifAST_arg(Identifier, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
KeywordTy alifAST_keyword(Identifier, ExprTy, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
AliasTy alifAST_alias(Identifier, Identifier, AlifIntT, AlifIntT, AlifIntT, AlifIntT, AlifASTMem*);
WithItemTy alifAST_withItem(ExprTy, ExprTy, AlifASTMem*);




extern AlifObject* alifAST_getDocString(ASDLStmtSeq*);
