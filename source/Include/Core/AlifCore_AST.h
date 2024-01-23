#pragma once


enum ModType {ModuleK=1, InteractiveK=2, ExpressionK=3, FunctionK=4};
class Module {
public:
	enum ModType type {};
	union
	{
		class {
		public:
			StatementSeq* body{};
		} Module;

		class {
		public:
			StatementSeq* body{};
		}Interactive;

		class {
		public:
			Expr body{};
		}Expression;

		class {
		public:
			ExprSeq* argTypes{};
			Expr returns{};
		}Function;

	} val;
};

enum StmtType {ClassDefK=1, FunctionDefK=2, AsyncFunctionDefK=3, ReturnK=4,
				DeleteK=5, AssignK=6, AugAssignK=7, ForK=8, AsyncForK=9, WhileK=10,
				IfK=11, WithK=12, AsyncWithK=13, TryK=14, ImportK=15, ImportFromK=16,
				ExprK=17, PassK=18, BreakK=19, CountinueK=20 };
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
			StatementSeq* body{};
			TypeParamSeq* typeParams{};
		}ClassDef;

		class {
		public:
			AlifObj* name{};
			Arguments args{};
			StatementSeq* body{};
			Expr returns{};
			AlifObj* comment{};
			TypeParamSeq typeParams{};
		}FunctionDef;

		class {
		public:
			AlifObj* name{};
			Arguments args{};
			StatementSeq* body{};
			Expr returns{};
			AlifObj* comment{};
			TypeParamSeq typeParams{};
		}AsyncFunctionDef;

		class {
		public:
			Expr val{};
		}Return;

		class {
		public:
			ExprSeq* targets{};
		}Delete;

		class {
		public:
			ExprSeq* targets{};
			Expr val{};
			AlifObj* comment{};
		}Assign;

		class {
		public:
			Expr target{};
			Operator op{};
			Expr val{};
		}AugAssign;

		class {
		public:
			Expr target{};
			Expr iter{};
			StatementSeq* body{};
			AlifObj* comment{};
		}For;

		class {
		public:
			Expr target{};
			Expr iter{};
			StatementSeq* body{};
			AlifObj* comment{};
		}AsyncFor;

		class {
		public:
			Expr condition{};
			StatementSeq* body{};
		}While;

		class {
		public:
			Expr condition{};
			StatementSeq* body{};
			StatementSeq* else_{};
		}If;

		class {
		public:
			WithItemSeq items{};
			StatementSeq* body{};
			AlifObj* comment{};
		}With;

		class {
		public:
			WithItemSeq items{};
			StatementSeq* body{};
			AlifObj* comment{};
		}AsyncWith;

		class {
		public:
			StatementSeq* body{};
			ExceptHandlerSeq* handler{};
			StatementSeq* else_{};
			StatementSeq* finalBody{};
		}Try;

		class {
		public:
			AliasSeq* names{};
		}Import;

		class {
		public:
			AlifObj* module{};
			AliasSeq* names{};
			int level{};
		}ImportFrom;

		class {
		public:
			Expr* val{};
		}Expr;

	} val;

	AlifSizeT lineNo{};
	AlifSizeT colOffset{};
	AlifSizeT endLineNo{};
	AlifSizeT endColOffset{};
};

enum ExprType {BoolOpK=1, ExprK=2, BinOpK=3, UnaryOpK=4, LambdaK=5, IfExprK=6,
			   DictK=7, SetK=8, ListK=9, DictComp=10, SetComp=11, ListComp=12,
			   GeneratorExprK=13, AwaitK=14, YieldK=15, CompareK=16, CallK=17,
			   FormattedValK=18, JoinStrK=19, ConstantK=20, AttributeK=21,
			   SubScriptK=22, NameK=23, TupleK=24, SliceK=25 };
