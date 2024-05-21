#include "alif.h"
#include "AlifCore_AST.h"
#include "AlifCore_AlifState.h"



class AlifASTOptimize { // 10
public:
	AlifIntT optimize{};
	AlifIntT recursionDepth{};
	AlifIntT recursionLimit{};
};


// Forward
//static AlifIntT astFold_stmt(Statement*, AlifASTMem*, AlifASTOptimize*);

#define CALL(_func, _arg) if (!_func((_arg), _ctx, _astState)) return 0 // 645


#define CALL_SEQ(_func, _type, _fullType, _arg) {\
	AlifIntT i{};	\
	_type ## Seq *seq = (_arg);		\
	for (i = 0; i < SEQ_LEN(seq); i++) { \
		_fullType elt = (_fullType)SEQ_GET(seq, i);	\
		if (elt != nullptr and !_func(elt, _ctx, _astState)) return 0;	\
	}		\
}

static AlifIntT astFold_body(StmtSeq* _stmts, AlifASTMem* _ctx, AlifASTOptimize* _astState) { // 664
	AlifIntT docString = alifAST_getDocString(_stmts) != nullptr;

	CALL_SEQ(astFold_stmt, Stmt, Statement*, _stmts);
	if (!docString) {
		Statement* stmt = (Statement*)SEQ_GET(_stmts, 0);
		ExprSeq* vals = alifNew_exprSeq(1, _ctx);
		if (!vals) return 0;

		SEQ_SET(vals, 0, stmt->V.expression.val);
		Expression* expr = alifAST_joinedStr(vals, stmt->lineNo, stmt->colOffset, stmt->endLineNo, stmt->endColOffset, _ctx);
		if (!expr) return 0;
		stmt->V.expression.val = expr;
	}

	return 1;
}

static AlifIntT astFold_module(Module* _node, AlifASTMem* _ctx, AlifASTOptimize* _astState) { // 687

	if (_node->type == ModType::ModuleK) {
		CALL(astFold_body, _node->V.module.body);
	}
	//else if (_node->type == ModType::InteractiveK) {
	//	CALL_SEQ(astFold_stmt, stmt, _node->V.interactive.body);
	//}
	//else if (_node->type == ModType::ExpressionK) {
	//	CALL(astFold_expr, Expression, _node->V.expression.body);
	//}

	return 1;
}


AlifIntT alifAST_optimize(Module* _module, AlifASTMem* _astMem, AlifIntT _optimize) { // 1103
	AlifThread* thread_{};
	AlifIntT startRecursionDepth{};

	AlifASTOptimize astState{};
	astState.optimize = _optimize;

	thread_ = alifThread_get();
	if (!thread_) return 0;

	AlifIntT recursionDepth = ALIFCPP_RECURSION_LIMIT - thread_->cppRecursionRemaining;
	startRecursionDepth = recursionDepth;
	astState.recursionDepth = startRecursionDepth;
	astState.recursionLimit = ALIFCPP_RECURSION_LIMIT;

	AlifIntT val = astFold_module(_module, _astMem, &astState);
	if (val and astState.recursionDepth != startRecursionDepth) {
		// error
		return 0;
	}

	return val;
}
