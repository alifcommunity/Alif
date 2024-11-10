#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Long.h"
#include "AlifCore_State.h"




class AlifASTOptimizeState { // 10
public:
	AlifIntT optimize{};
	AlifIntT features{};

	AlifIntT recursionDepth{};            /* current recursion depth */
	AlifIntT recursionLimit{};            /* recursion limit */
};

#define ENTER_RECURSIVE(ST) \
    do { \
        if (++(ST)->recursionDepth > (ST)->recursionLimit) { \
            /*alifErr_setString(_alifExcRecursionError_, \
                "maximum recursion depth exceeded during compilation");*/ \
            return 0; \
        } \
    } while(0)

#define LEAVE_RECURSIVE(ST) \
    do { \
        --(ST)->recursionDepth; \
    } while(0)



static AlifIntT astFold_mod(ModuleTy, AlifASTMem*, AlifASTOptimizeState*); // 644
static AlifIntT astFold_stmt(StmtTy, AlifASTMem*, AlifASTOptimizeState*);
static AlifIntT astFold_expr(ExprTy, AlifASTMem*, AlifASTOptimizeState*);


 // 657
#define CALL(_func, _type, _arg) \
    if (!_func((_arg), _ctx, _state)) \
        return 0;

#define CALL_OPT(_func, _type, _arg) \
    if ((_arg) != nullptr and !_func((_arg), _ctx, _state)) \
        return 0;

#define CALL_SEQ(_func, _type, _arg) { \
    AlifSizeT i{}; \
    ASDL ## _type ## Seq *seq = (_arg); /* avoid variable capture */ \
    for (i = 0; i < ASDL_SEQ_LEN(seq); i++) { \
        _type ## Ty elt = (_type ## Ty)ASDL_SEQ_GET(seq, i); \
        if (elt != nullptr and !_func(elt, _ctx, _state)) \
            return 0; \
    } \
}



static AlifIntT astFold_body(ASDLStmtSeq* _stmts, AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 676
	AlifIntT docstring = alifAST_getDocString(_stmts) != nullptr;
	CALL_SEQ(astFold_stmt, Stmt, _stmts);
	if (!docstring and alifAST_getDocString(_stmts) != nullptr) {
		StmtTy st = (StmtTy)ASDL_SEQ_GET(_stmts, 0);
		ASDLExprSeq* values = alifNew_exprSeq(1, _ctx);
		if (!values) {
			return 0;
		}
		ASDL_SEQ_SET(values, 0, st->V.expression.val);
		ExprTy expr = alifAST_joinedStr(values, st->lineNo, st->colOffset,
			st->endLineNo, st->endColOffset,
			_ctx);
		if (!expr) {
			return 0;
		}
		st->V.expression.val = expr;
	}
	return 1;
}


static AlifIntT astfold_mod(ModuleTy _node, AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 699
	switch (_node->type) {
	case ModK_::ModuleK:
		CALL(astFold_body, ASDLSeq, _node->V.module.body);
		break;
	case ModK_::InteractiveK:
		CALL_SEQ(astFold_stmt, Stmt, _node->V.interactive.body);
		break;
	//case ModK_::ExpressionK:
	//	CALL(astFold_expr, ExprTy, _node->V.expression.body);
	//	break;
		// The following top level nodes don't participate in constant folding
	case ModK_::FunctionK:
		break;
		// No default case, so the compiler will emit a warning if new top level
		// compilation nodes are added without being handled here
	}
	return 1;
}


static AlifIntT astFold_expr(ExprTy _node, AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 721
	ENTER_RECURSIVE(_state);
	switch (_node->type) {
	case ExprK_::BoolOpK:
		CALL_SEQ(astFold_expr, Expr, _node->V.boolOp.vals);
		break;
	case ExprK_::BinOpK:
		CALL(astFold_expr, ExprTy, _node->V.binOp.left);
		CALL(astFold_expr, ExprTy, _node->V.binOp.right);
		CALL(fold_binOp, ExprTy, _node);
		break;
	case ExprK_::UnaryOpK:
		CALL(astFold_expr, ExprTy, _node->V.unaryOp.operand);
		CALL(fold_unaryOp, ExprTy, _node);
		break;
	case ExprK_::IfExprK:
		CALL(astFold_expr, ExprTy, _node->V.ifExpr.condition);
		CALL(astFold_expr, ExprTy, _node->V.ifExpr.body);
		CALL(astFold_expr, ExprTy, _node->V.ifExpr.else_);
		break;
	case ExprK_::DictK:
		CALL_SEQ(astFold_expr, Expr, _node->V.dict.keys);
		CALL_SEQ(astFold_expr, Expr, _node->V.dict.vals);
		break;
	case ExprK_::SetK:
		CALL_SEQ(astFold_expr, Expr, _node->V.set.elts);
		break;
	//case ExprK_::ListCompK:
	//	CALL(astFold_expr, ExprTy, _node->V.listComp.elt);
	//	CALL_SEQ(astFold_comprehension, Comprehension, _node->V.listComp.generators);
	//	break;
	//case ExprK_::SetCompK:
	//	CALL(astFold_expr, ExprTy, _node->V.setComp.elts);
	//	CALL_SEQ(astFold_comprehension, Comprehension, _node->V.setComp.generators);
	//	break;
	//case ExprK_::DictCompK:
	//	CALL(astFold_expr, ExprTy, _node->V.dictComp.key);
	//	CALL(astFold_expr, ExprTy, _node->V.dictComp.val);
	//	CALL_SEQ(astFold_comprehension, Comprehension, _node->V.dictComp.generators);
	//	break;
	case ExprK_::AwaitK:
		CALL(astFold_expr, ExprTy, _node->V.await.val);
		break;
	case ExprK_::YieldK:
		CALL_OPT(astFold_expr, ExprTy, _node->V.yield.val);
		break;
	case ExprK_::YieldFromK:
		CALL(astFold_expr, ExprTy, _node->V.yieldFrom.val);
		break;
	case ExprK_::CompareK:
		CALL(astFold_expr, ExprTy, _node->V.compare.left);
		CALL_SEQ(astFold_expr, Expr, _node->V.compare.comparators);
		CALL(fold_compare, ExprTy, _node);
		break;
	case ExprK_::CallK:
		CALL(astFold_expr, ExprTy, _node->V.call.func);
		CALL_SEQ(astFold_expr, Expr, _node->V.call.args);
		CALL_SEQ(astFold_keyword, Keyword, _node->V.call.keywords);
		break;
	case ExprK_::FormattedValK:
		CALL(astFold_expr, ExprTy, _node->V.formattedValue.val);
		CALL_OPT(astFold_expr, ExprTy, _node->V.formattedValue.formatSpec);
		break;
	case ExprK_::JoinStrK:
		CALL_SEQ(astFold_expr, Expr, _node->V.joinStr.vals);
		break;
	case ExprK_::AttributeK:
		CALL(astFold_expr, ExprTy, _node->V.attribute.val);
		break;
	//case ExprK_::SubScriptK:
	//	CALL(astFold_expr, ExprTy, _node->V.subScript.val);
	//	CALL(astFold_expr, ExprTy, _node->V.subScript.slice);
	//	CALL(fold_subScr, ExprTy, _node);
	//	break;
	case ExprK_::StarK:
		CALL(astFold_expr, ExprTy, _node->V.star.val);
		break;
	case ExprK_::SliceK:
		CALL_OPT(astFold_expr, ExprTy, _node->V.slice.lower);
		CALL_OPT(astFold_expr, ExprTy, _node->V.slice.upper);
		CALL_OPT(astFold_expr, ExprTy, _node->V.slice.step);
		break;
	case ExprK_::ListK:
		CALL_SEQ(astFold_expr, Expr, _node->V.list.elts);
		break;
	case ExprK_::TupleK:
		CALL_SEQ(astFold_expr, Expr, _node->V.tuple.elts);
		CALL(fold_tuple, ExprTy, _node);
		break;
	case ExprK_::NameK:
		if (_node->V.name.ctx == ExprContext_::Load and
			alifUStr_equalToASCIIString(_node->V.name.name, "__debug__")) {
			LEAVE_RECURSIVE(_state);
			return make_const(_node, alifBool_fromLong(!_state->optimize), _ctx);
		}
		break;
	case ExprK_::NamedExprK:
		CALL(astFold_expr, ExprTy, _node->V.namedExpr.val);
		break;
	case ExprK_::ConstantK:
		// Already a constant, nothing further to do
		break;
		// No default case, so the compiler will emit a warning if new expression
		// kinds are added without being handled here
	}
	LEAVE_RECURSIVE(_state);;
	return 1;
}

static AlifIntT astFold_stmt(StmtTy _node, AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 880
	ENTER_RECURSIVE(_state);
	switch (_node->type) {
	//case StmtK_::FunctionDefK:
	//	CALL_SEQ(astFold_typeParam, TypeParam, _node->V.functionDef.typeParams);
	//	CALL(astFold_arguments, ArgumentsTy, _node->V.functionDef.args);
	//	CALL(astFold_body, ASDLSeq, _node->V.functionDef.body);
	//	CALL_SEQ(astFold_expr, Expr, _node->V.functionDef.decoratorList);
	//	if (!(_state->features & CO_FUTURE_ANNOTATIONS)) {
	//		CALL_OPT(astFold_expr, ExprTy, _node->V.functionDef.returns);
	//	}
	//	break;
	//case StmtK_::AsyncFunctionDefK:
	//	CALL_SEQ(astfold_typeParam, TypeParam, _node->V.asyncFunctionDef.typeParams);
	//	CALL(astFold_arguments, ArgumentsTy, _node->V.asyncFunctionDef.args);
	//	CALL(astFold_body, ASDLSeq, _node->V.asyncFunctionDef.body);
	//	CALL_SEQ(astFold_expr, Expr, _node->V.asyncFunctionDef.decoratorList);
	//	if (!(_state->features & CO_FUTURE_ANNOTATIONS)) {
	//		CALL_OPT(astFold_expr, ExprTy, _node->V.asyncFunctionDef.returns);
	//	}
	//	break;
	//case StmtK_::ClassDefK:
	//	CALL_SEQ(astFold_typeParam, TypeParam, _node->V.classDef.typeParams);
	//	CALL_SEQ(astFold_expr, Expr, _node->V.classDef.bases);
	//	CALL_SEQ(astFold_keyword, Keyword, _node->V.classDef.keywords);
	//	CALL(astFold_body, asdl_seq, _node->V.classDef.body);
	//	CALL_SEQ(astFold_expr, Expr, _node->V.classDef.decoratorList);
	//	break;
	case StmtK_::ReturnK:
		CALL_OPT(astFold_expr, ExprTy, _node->V.return_.val);
		break;
	case StmtK_::DeleteK:
		CALL_SEQ(astFold_expr, Expr, _node->V.delete_.targets);
		break;
	case StmtK_::AssignK:
		CALL_SEQ(astFold_expr, Expr, _node->V.assign.targets);
		CALL(astFold_expr, ExprTy, _node->V.assign.val);
		break;
	case StmtK_::AugAssignK:
		CALL(astFold_expr, ExprTy, _node->V.augAssign.target);
		CALL(astFold_expr, ExprTy, _node->V.augAssign.val);
		break;
	case StmtK_::ForK:
		CALL(astFold_expr, ExprTy, _node->V.for_.target);
		CALL(astFold_expr, ExprTy, _node->V.for_.iter);
		CALL_SEQ(astFold_stmt, Stmt, _node->V.for_.body);
		//CALL_SEQ(astFold_stmt, Stmt, _node->V.for_.else_);

		CALL(fold_iter, ExprTy, _node->V.for_.iter);
		break;
	case StmtK_::AsyncForK:
		CALL(astFold_expr, ExprTy, _node->V.asyncFor.target);
		CALL(astFold_expr, ExprTy, _node->V.asyncFor.iter);
		CALL_SEQ(astFold_stmt, Stmt, _node->V.asyncFor.body);
		//CALL_SEQ(astFold_stmt, Stmt, _node->V.asyncFor.else_);
		break;
	case StmtK_::WhileK:
		CALL(astFold_expr, ExprTy, _node->V.while_.condition);
		CALL_SEQ(astFold_stmt, Stmt, _node->V.while_.body);
		//CALL_SEQ(astFold_stmt, Stmt, _node->V.while_.);
		break;
	case StmtK_::IfK:
		CALL(astFold_expr, ExprTy, _node->V.if_.condition);
		CALL_SEQ(astFold_stmt, Stmt, _node->V.if_.body);
		CALL_SEQ(astFold_stmt, Stmt, _node->V.if_.else_);
		break;
	//case StmtK_::WithK:
	//	CALL_SEQ(astFold_withItem, WithItem, _node->V.with_.items);
	//	CALL_SEQ(astFold_stmt, Stmt, _node->V.with_.body);
	//	break;
	//case StmtK_::AsyncWithK:
	//	CALL_SEQ(astFold_withItem, withitem, _node->V.asyncWith.items);
	//	CALL_SEQ(astFold_stmt, stmt, _node->v.AsyncWith.body);
	//	break;
	//case StmtK_::TryK:
	//	CALL_SEQ(astFold_stmt, stmt, _node->v.Try.body);
	//	CALL_SEQ(astfold_excepthandler, excepthandler, _node->v.Try.handlers);
	//	CALL_SEQ(astFold_stmt, stmt, _node->v.Try.orelse);
	//	CALL_SEQ(astFold_stmt, stmt, _node->v.Try.finalbody);
	//	break;
	case StmtK_::ExprK:
		CALL(astFold_expr, ExprTy, _node->V.expression.val);
		break;
		// The following statements don't contain any subexpressions to be folded
	case StmtK_::ImportK:
	case StmtK_::ImportFromK:
	case StmtK_::GlobalK:
	case StmtK_::NonlocalK:
	case StmtK_::PassK:
	case StmtK_::BreakK:
	case StmtK_::ContinueK:
		break;
		// No default case, so the compiler will emit a warning if new statement
		// kinds are added without being handled here
	}
	LEAVE_RECURSIVE(_state);
	return 1;
}


AlifIntT alifAST_optimize(ModuleTy _mod, AlifASTMem* _astMem,
	AlifIntT _optimize, AlifIntT _features) { // 1103
	AlifThread* thread{};
	AlifIntT startingRecursionDepth{};

	AlifASTOptimizeState state{};
	state.optimize = _optimize;
	state.features = _features;

	/* Setup recursion depth check counters */
	thread = _alifThread_get();
	if (!thread) {
		return 0;
	}
	/* Be careful here to prevent overflow. */
	AlifIntT recursion_depth = ALIFCPP_RECURSION_LIMIT - thread->cppRecursionRemaining;
	startingRecursionDepth = recursion_depth;
	state.recursionDepth = startingRecursionDepth;
	state.recursionLimit = ALIFCPP_RECURSION_LIMIT;

	AlifIntT ret = astfold_mod(_mod, _astMem, &state);

	/* Check that the recursion depth counting balanced correctly */
	if (ret and state.recursionDepth != startingRecursionDepth) {
		//alifErr_format(_alifExcSystemError_,
		//	"AST optimizer recursion depth mismatch (before=%d, after=%d)",
		//	startingRecursionDepth, state.recursionDepth);
		return 0;
	}

	return ret;
}
