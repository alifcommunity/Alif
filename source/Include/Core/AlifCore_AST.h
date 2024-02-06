#pragma once

#define SEQ_HEAD AlifSizeT size{}; void** element{}

// Forward Declaration
class TypeParam;
class WithItem;
class Alias;
class Keyword;
class Arg;
class Arguments;
class ExceptHandler;
class Comprehension;
class Expression;
class Statement;
class Module;

class ModSeq {
public:
	SEQ_HEAD;
	Module* elements[1]{};
};

class StmtSeq {
public:
	SEQ_HEAD;
	Statement* elements[1];
};

class ExprSeq {
public:
	SEQ_HEAD;
	Expression* elements[1];
};

class CompSeq {
public:
	SEQ_HEAD;
	Comprehension* elements[1];
};

class ArgSeq {
public:
	SEQ_HEAD;
	Arg* elements[1];
};

class KeywordsSeq {
public:
	SEQ_HEAD;
	Keyword* elements[1];
};

class AliasSeq {
public:
	SEQ_HEAD;
	Alias* elements[1];
};

class WithItemSeq {
public:
	SEQ_HEAD;
	WithItem* elements[1];
};

class ExceptHandlerSeq {
public:
	SEQ_HEAD;
	ExceptHandler* elements[1];
};

class TypeParamSeq {
public:
	SEQ_HEAD;
	TypeParamSeq* elements[1];
};

class IntSeq {
public:
	SEQ_HEAD;
	AlifSizeT elements[1];
};


enum ModType {ModuleK=1, InteractiveK=2, ExpressionK=3, FunctionK=4};
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

	} val;
};

enum StmtType {ClassDefK=1, FunctionDefK=2, AsyncFunctionDefK=3, ReturnK=4,
				DeleteK=5, AssignK=6, AugAssignK=7, ForK=8, AsyncForK=9, WhileK=10,
				IfK=11, WithK=12, AsyncWithK=13, TryK=14, ImportK=15, ImportFromK=16,
				ExprK=17, PassK=18, BreakK=19, CountinueK=20 };
enum Operator {Add=1, Sub=2, Mult=3, MatMult=4, Div=5, FloorDiv = 6, Mod=7, Pow=8, LShift=9,
			   RShift=10, BitOr=11, BitXor=12, BitAnd=13 };
class Statement {
public:
	enum StmtType type {};
	union
	{
		class {
		public:
			AlifObj* name{};
			ExprSeq* bases{};
			KeywordsSeq* keywords{};
			StmtSeq* body{};
			TypeParamSeq* typeParams{};
		}classDef;

		class {
		public:
			AlifObj* name{};
			Arguments* args{};
			StmtSeq* body{};
			Expression* returns{};
			AlifObj* comment{};
			TypeParamSeq typeParams{};
		}functionDef;

		class {
		public:
			AlifObj* name{};
			Arguments* args{};
			StmtSeq* body{};
			Expression* returns{};
			AlifObj* comment{};
			TypeParamSeq typeParams{};
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
			AlifObj* comment{};
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
			AlifObj* comment{};
		}for_;

		class {
		public:
			Expression* target{};
			Expression* iter{};
			StmtSeq* body{};
			AlifObj* comment{};
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
			WithItemSeq items{};
			StmtSeq* body{};
			AlifObj* comment{};
		}with_;

		class {
		public:
			WithItemSeq items{};
			StmtSeq* body{};
			AlifObj* comment{};
		}asyncWith;

		class {
		public:
			StmtSeq* body{};
			ExceptHandlerSeq* handler{};
			StmtSeq* else_{};
			StmtSeq* finalBody{};
		}try_;

		class {
		public:
			AliasSeq* names{};
		}import;

		class {
		public:
			AlifObj* module{};
			AliasSeq* names{};
			int level{};
		}importFrom;

		class {
		public:
			Expression* val{};
		}expression;

	} val;

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

enum ExprType {BoolOpK=1, NamedExprK=2, BinOpK=3, UnaryOpK=4, LambdaK=5, IfExprK=6,
			   DictK=7, SetK=8, ListK=9, DictComp=10, SetComp=11, ListComp=12,
			   GeneratorExprK=13, AwaitK=14, YieldK=15, CompareK=16, CallK=17,
			   FormattedValK=18, JoinStrK=19, ConstantK=20, AttributeK=21,
			   SubScriptK=22, NameK=23, TupleK=24, SliceK=25 };
enum BoolOp {And=1, Or=2};
enum Unary {UAdd=1, USub=2, Not=3, Invert=4};
enum ExprCTX {Store=1, Load=2, Del=3};
class Expression {
public:
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
			Operator* op{};
			Expression* right{};
		}binOp;

		class {
		public:
			Unary op{};
			Expression* operand{};
		}unaryOp;

		class {
		public:
			Arguments* args{};
			Expression* body{};
		}lambda;

		class {
		public:
			Expression* condetion{};
			Expression* body{};
			Expression* else_{};
		}if_;

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
			ExprCTX* ctx{};
		}list;

		class {
		public:
			Expression* keys{};
			Expression* vals{};
			CompSeq* generetors{};
		}dictComp;

		class {
		public:
			Expression* elts{};
			CompSeq* generetors{};
		}setComp;

		class {
		public:
			Expression* elts{};
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
			Expression* left{};
			IntSeq* ops{};
			ExprSeq* comparators{};
		}compare;

		class {
		public:
			Expression* func{};
			ExprSeq* args{};
			KeywordsSeq* keywords{};
		}call;

		class {
		public:
			Expression* val{};
			AlifSizeT* conversion{};
			Expression* formatSpec{};
		}fromattedValue;

		class {
		public:
			ExprSeq* vals{};
		}joinStr;

		class {
		public:
			AlifObj* val{};
			AlifObj* type{};
		}constat;

		class {
		public:
			Expression* val{};
			AlifObj* attr{};
			ExprCTX ctx{};
		}attribute;

		class {
		public:
			Expression* val{};
			Expression* slice{};
			ExprCTX* ctx{};
		}subScript;

		class {
		public:
			AlifObj* name{};
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

	} val;

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

enum ExceptHandlerType {ExeptHandlerK=1};
class ExceptHandler {
public:
	ExceptHandlerType type{};

	union
	{
		class {
		public:
			Expression* type{};
			AlifObj* name{};
			StmtSeq* body{};
		}exceptHandler;
	} val;

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
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
	AlifObj* arg{};
	//Expression* annotation{};
	AlifObj* comment{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

class Keyword {
public:
	AlifObj* arg{};
	Expression* val{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

class Alias {
public:
	AlifObj* name{};
	AlifObj* asName{};

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

class WithItem {
	Expression* contextExpr{};
	Expression* optionalVars{};
};

enum TypeParamType {TypeVarK=1, ParamSpec=2, TypeVarTupleK=3};
class TypeParam {
public:
	enum TypeParamType type {};
	union
	{
		class {
			AlifObj* name{};
			Expression* bound{};
		}typeVar;

		class {
			AlifObj* name{};
		}paramSpec;

		class {
			AlifObj* name{};
		}typeVarTuple;

	} val;

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

