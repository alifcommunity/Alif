#pragma once

#include "AlifCore_ASDL.h"

// Forward Declaration
class WithItem;
class Alias;
class Keyword;
class Arg;
class Arguments;
//class ExceptHandler;
class Comprehension;
class Expression;
class Statement;
class Module;

class ModSeq {
public:
	SEQ_HEAD;
	Module* typedElements[1]{};
};

class StmtSeq {
public:
	SEQ_HEAD;
	Statement* typedElements[1];
};

class ExprSeq {
public:
	SEQ_HEAD;
	Expression* typedElements[1];
};
ExprSeq* alifNew_exprSeq(AlifUSizeT, AlifASTMem*); // جسم هذه الدالة تم إنشاؤه من قبل GENERATE_SEQ_CONSTRUCTOR(Expr, expr, Expression*)

class CompSeq {
public:
	SEQ_HEAD;
	Comprehension* typedElements[1];
};

class ArgSeq {
public:
	SEQ_HEAD;
	Arg* typedElements[1];
};
ArgSeq* alifNew_argSeq(AlifUSizeT, AlifASTMem*); // جسم هذه الدالة تم إنشاؤه من قبل GENERATE_SEQ_CONSTRUCTOR(Arg, arg, Arg*)

class KeywordSeq {
public:
	SEQ_HEAD;
	Keyword* typedElements[1];
};
KeywordSeq* alifNew_keywordSeq(AlifUSizeT, AlifASTMem*); // جسم هذه الدالة تم إنشاؤه من قبل GENERATE_SEQ_CONSTRUCTOR(Keyword, keyword, Keyword*)

class AliasSeq {
public:
	SEQ_HEAD;
	Alias* typedElements[1];
};

class WithItemSeq {
public:
	SEQ_HEAD;
	WithItem* typedElements[1];
};


enum ModType { ModuleK = 1, InteractiveK, ExpressionK, FunctionK }; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
class Module {
public:
	enum ModType type {};
	union
	{
		class {
		public:
			StmtSeq* body{};
		} module;

		class {
		public:
			StmtSeq* body{};
		} interactive;

		class {
		public:
			Expression* body{};
		} expression;

		class {
		public:
			ExprSeq* argTypes{};
			Expression* returns{};
		} function;

	} V;
};

enum StmtType {
	ClassDefK = 1, FunctionDefK, AsyncFunctionDefK, ReturnK,
	DeleteK, AssignK, AugAssignK, ForK, AsyncForK, WhileK,
	IfK, WithK, AsyncWithK, TryK, ImportK, ImportFromK,
	ExprK, PassK, BreakK, CountinueK, GlobalK, NonlocalK,
}; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
enum Operator {
	Add = 1, Sub, Mult, MatMult, Div, FloorDiv, Mod, Pow, LShift,
	RShift, BitOr, BitXor, BitAnd
}; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
class Statement {
public:
	enum StmtType type {};
	union
	{
		class {
		public:
			AlifObject* name{};
			ExprSeq* bases{};
			KeywordSeq* keywords{};
			StmtSeq* body{};
		}classDef;

		class {
		public:
			AlifObject* name{};
			Arguments* args{};
			StmtSeq* body{};
		}functionDef;

		class {
		public:
			AlifObject* name{};
			Arguments* args{};
			StmtSeq* body{};
		}asyncFunctionDef;

		class {
		public:
			Expression* val{};
		}return_;

		class {
		public:
			ExprSeq* targets{};
		}delete_;

		class {
		public:
			ExprSeq* targets{};
			Expression* val{};
		}assign;

		class {
		public:
			Expression* target{};
			Operator op{};
			Expression* val{};
		}augAssign;

		class {
		public:
			Expression* target{};
			Expression* iter{};
			StmtSeq* body{};
		}for_;

		class {
		public:
			Expression* target{};
			Expression* iter{};
			StmtSeq* body{};
		}asyncFor;

		class {
		public:
			Expression* condition{};
			StmtSeq* body{};
		}while_;

		class {
		public:
			Expression* condition{};
			StmtSeq* body{};
			StmtSeq* else_{};
		}if_;

		class {
		public:
			WithItemSeq* items{};
			StmtSeq* body{};
		}with_;

		class {
		public:
			WithItemSeq items{};
			StmtSeq* body{};
			AlifObject* comment{};
		}asyncWith;

		class {
		public:
			AliasSeq* names{};
		}import;

		class {
		public:
			AlifObject* module{};
			AliasSeq* names{};
			//int level{};
		}importFrom;

		class {
		public:
			Expression* val{};
		}expression;

	} V;

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

enum ExprType {
	BoolOpK = 1, NamedExprK, BinOpK, UnaryOpK, IfExprK, DictK, SetK, ListK,
	DictCompK, SetCompK, ListCompK, GeneratorExprK, AwaitK, YieldK,
	YieldFromK, CompareK, CallK, FormattedValK, JoinStrK, ConstantK,
	AttributeK, SubScriptK, StarK, NameK, TupleK, SliceK
}; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
enum BoolOp { And = 1, Or }; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
enum UnaryOp { UAdd = 1, USub, Not, Invert, Sqrt }; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
enum CmpOp { Equal = 1, NotEq, LessThan, LessThanEq, GreaterThan, GreaterThanEq, Is, IsNot, In, NotIn }; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
enum ExprCTX { Store = 1, Load, Del }; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
class Expression {
public:
	enum ExprType type {};
	union
	{
		class {
		public:
			BoolOp op{};
			ExprSeq* vals{};
		}boolOp;

		class {
		public:
			Expression* target{};
			Expression* val{};
		}expr;

		class {
		public:
			Expression* left{};
			Operator op{};
			Expression* right{};
		}binOp;

		class {
		public:
			UnaryOp op{};
			Expression* operand{};
		}unaryOp;

		class {
		public:
			Expression* condition_{};
			Expression* body_{};
			Expression* else_{};
		}ifExpr;

		class {
		public:
			ExprSeq* keys{};
			ExprSeq* vals{};
		}dict;

		class {
		public:
			ExprSeq* elts{};
		}set;

		class {
		public:
			ExprSeq* elts{};
			ExprCTX ctx{};
		}list;

		class {
		public:
			Expression* key{};
			Expression* val{};
			CompSeq* generetors{};
		}dictComp;

		class {
		public:
			Expression* elts{};
			CompSeq* generetors{};
		}setComp;

		class {
		public:
			Expression* elt{};
			CompSeq* generetors{};
		}listComp;

		class {
		public:
			Expression* val{};
		}await_;

		class {
		public:
			Expression* val{};
		}yield;

		class {
		public:
			Expression* val{};
		}yieldFrom;

		class {
		public:
			Expression* left{};
			IntSeq* ops{};
			ExprSeq* comparators{};
		}compare;

		class {
		public:
			Expression* func{};
			ExprSeq* args{};
			KeywordSeq* keywords{};
		}call;

		class {
		public:
			Expression* val{};
			AlifIntT conversion{};
			Expression* formatSpec{};
		}fromattedValue;

		class {
		public:
			ExprSeq* vals{};
		}joinStr;

		class {
		public:
			AlifObject* val{};
			AlifObject* type{};
		}constant;

		class {
		public:
			Expression* val{};
			AlifObject* attr{};
			ExprCTX ctx{};
		}attribute;

		class {
		public:
			Expression* val{};
			Expression* slice{};
			ExprCTX ctx{};
		}subScript;

		class {
		public:
			Expression* val{};
			ExprCTX ctx{};
		}star;

		class {
		public:
			AlifObject* name{};
			ExprCTX ctx{};
		}name;

		class {
		public:
			ExprSeq* elts{};
			ExprCTX ctx{};
		}tuple;

		class {
		public:
			Expression* lower{};
			Expression* upper{};
			Expression* step{};
		}slice;

	} V;

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

class Comprehension {
public:
	Expression* target{};
	Expression* iter{};
	ExprSeq* ifs{};
	AlifSizeT isAsync{};
};

class Arguments {
public:
	ArgSeq* posOnlyArgs{};
	ArgSeq* args{};
	Arg* varArg{};
	ArgSeq* kwOnlyArgs{};
	ExprSeq* kwDefaults{};
	Arg* kwArg{};
	ExprSeq* defaults{};
};

class Arg {
public:
	AlifObject* arg{};
	AlifObject* comment{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

class Keyword {
public:
	AlifObject* arg{};
	Expression* val{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

class Alias {
public:
	AlifObject* name{};
	AlifObject* asName{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

class WithItem {
public:
	Expression* contextExpr{};
	Expression* optionalVars{};
};

enum TypeParamType { TypeVarK = 1, ParamSpec, TypeVarTupleK }; // يجب تحديد رقم البداية 1 لكي لا يبدأ من 0 حيث يتم التحقق فيما بعد ولا يجب أن تكون الحالة 0
class TypeParam {
public:
	enum TypeParamType type {};
	union
	{
		class {
			AlifObject* name{};
			Expression* bound{};
		}typeVar;

		class {
			AlifObject* name{};
		}paramSpec;

		class {
			AlifObject* name{};
		}typeVarTuple;

	} V;

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};


Module* alifAST_module(StmtSeq*, AlifASTMem*);
Statement* alifAST_assign(ExprSeq*, Expression*, int, int, int, int, AlifASTMem*);
Statement* alifAST_expr(Expression*, int, int, int, int, AlifASTMem*);
Expression* alifAST_constant(AlifObject*, AlifObject*, int, int, int, int, AlifASTMem*);
Statement* alifAST_asyncFunctionDef(AlifObject*, Arguments*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_functionDef(AlifObject*, Arguments*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_return(Expression*, int, int, int, int, AlifASTMem*);
Statement* alifAST_delete(ExprSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_classDef(AlifObject*, ExprSeq*, KeywordSeq*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_augAssign(Expression*, Operator, Expression*, int, int, int, int, AlifASTMem*);
Statement* alifAST_for(Expression*, Expression*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_asyncFor(Expression*, Expression*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_while(Expression*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_if(Expression*, StmtSeq*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_with(WithItemSeq*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_asyncWith(WithItemSeq*, StmtSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_import(AliasSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_importFrom(AlifObject*, AliasSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_global(IdentifierSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_nonlocal(IdentifierSeq*, int, int, int, int, AlifASTMem*);
Statement* alifAST_pass(int, int, int, int, AlifASTMem*);
Statement* alifAST_break(int, int, int, int, AlifASTMem*);
Statement* alifAST_continue(int, int, int, int, AlifASTMem*);
Expression* alifAST_boolOp(BoolOp, ExprSeq*, int, int, int, int, AlifASTMem*);
Expression* alifAST_binOp(Expression*, Operator, Expression*, int, int, int, int, AlifASTMem*);
Expression* alifAST_unaryOp(UnaryOp, Expression*, int, int, int, int, AlifASTMem*);
Expression* alifAST_ifExpr(Expression*, Expression*, Expression*, int, int, int, int, AlifASTMem*);
Expression* alifAST_dict(ExprSeq*, ExprSeq*, int, int, int, int, AlifASTMem*);
Expression* alifAST_listComp(Expression*, CompSeq*, int, int, int, int, AlifASTMem*);
Expression* alifAST_dictComp(Expression*, Expression*, CompSeq*, int, int, int, int, AlifASTMem*);
Expression* alifAST_await(Expression*, int, int, int, int, AlifASTMem*);
Expression* alifAST_yield(Expression*, int, int, int, int, AlifASTMem*);
Expression* alifAST_yieldFrom(Expression*, int, int, int, int, AlifASTMem*);
Expression* alifAST_compare(Expression*, IntSeq*, ExprSeq*, int, int, int, int, AlifASTMem*);
Expression* alifAST_call(Expression*, ExprSeq*, KeywordSeq*, int, int, int, int, AlifASTMem*);
Expression* alifAST_formattedValue(Expression*, AlifIntT, Expression*, int, int, int, int, AlifASTMem*);
Expression* alifAST_joinedStr(ExprSeq*, int, int, int, int, AlifASTMem*);
Expression* alifAST_attribute(Expression*, AlifObject*, ExprCTX, int, int, int, int, AlifASTMem*);
Expression* alifAST_subScript(Expression*, Expression*, ExprCTX, int, int, int, int, AlifASTMem*);
Expression* alifAST_star(Expression*, ExprCTX, int, int, int, int, AlifASTMem*);
Expression* alifAST_name(AlifObject*, ExprCTX, int, int, int, int, AlifASTMem*);
Expression* alifAST_list(ExprSeq*, ExprCTX, int, int, int, int, AlifASTMem*);
Expression* alifAST_tuple(ExprSeq*, ExprCTX, int, int, int, int, AlifASTMem*);
Expression* alifAST_slice(Expression*, Expression*, Expression*, int, int, int, int, AlifASTMem*);
Comprehension* alifAST_comprehension(Expression*, Expression*, ExprSeq*, int, AlifASTMem*);
Arguments* alifAST_arguments(ArgSeq*, ArgSeq*, Arg*, ArgSeq*, ExprSeq*, Arg*, ExprSeq*, AlifASTMem*);
Arg* alifAST_arg(AlifObject*, int, int, int, int, AlifASTMem*);
Keyword* alifAST_keyword(AlifObject*, Expression*, int, int, int, int, AlifASTMem*);
Alias* alifAST_alias(AlifObject*, AlifObject*, int, int, int, int, AlifASTMem*);
WithItem* alifAST_withItem(Expression*, Expression*, AlifASTMem*);