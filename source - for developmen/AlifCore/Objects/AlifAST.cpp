#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Memory.h" // temp




GENERATE_SEQ_CONSTRUCTOR(Expr, expr, Expression*); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة
GENERATE_SEQ_CONSTRUCTOR(Arg, arg, Arg*); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة
GENERATE_SEQ_CONSTRUCTOR(Keyword, keyword, Keyword*); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة

Module* alifAST_module(StmtSeq* _body, AlifASTMem* _astMem) {
	Module* p{};
	p = (Module*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = ModuleK;
	p->V.module.body = _body;
	return p;
}

Module* alifAST_interactive(StmtSeq* _body, AlifASTMem* _astMem) {
	Module* p{};
	p = (Module*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = InteractiveK;
	p->V.interactive.body = _body;
	return p;
}

Statement* alifAST_assign(ExprSeq* _targets, Expression* _val,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Statement* p{};
	if (!_val) {
		// error
		return nullptr;
	}
	p = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = AssignK;
	p->V.assign.targets = _targets;
	p->V.assign.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

Statement* alifAST_expr(Expression* _val,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Statement* p{};
	if (!_val) {
		//error
		return nullptr;
	}
	p = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = ExprK;
	p->V.expression.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

Expression* alifAST_constant(AlifObject* _val, AlifObject* _type,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = ConstantK;
	p->V.constant.val = _val;
	p->V.constant.type = _type;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

Statement* alifAST_asyncFunctionDef(AlifObject* _name, Arguments* _args, StmtSeq* _body,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	if (!_name) {
		// error
		return nullptr;
	}
	if (!_args) {
		// error
		return nullptr;
	}
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtType::FunctionDefK;
	p_->V.functionDef.name = _name;
	p_->V.functionDef.args = _args;
	p_->V.functionDef.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_functionDef(AlifObject* _name, Arguments* _args, StmtSeq* _body,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	if (!_name) {
		// error
		return nullptr;
	}
	if (!_args) {
		// error
		return nullptr;
	}
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtType::AsyncFunctionDefK;
	p_->V.asyncFunctionDef.name = _name;
	p_->V.asyncFunctionDef.args = _args;
	p_->V.asyncFunctionDef.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_return(Expression* _val,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = ReturnK;
	p_->V.return_.val = _val;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_delete(ExprSeq* _targets,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::DeleteK;
	p_->V.delete_.targets = _targets;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_classDef(AlifObject* _name, ExprSeq* _bases, KeywordSeq* _keywords, StmtSeq* _body,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	if (!_name) {
		// error
		return nullptr;
	}
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtType::ClassDefK;
	p_->V.classDef.name = _name;
	p_->V.classDef.bases = _bases;
	p_->V.classDef.keywords = _keywords;
	p_->V.classDef.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_augAssign(Expression* _target, Operator _op, Expression* _val,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	if (!_target) {
		// error
		return nullptr;
	}
	if (!_op) {
		// error
		return nullptr;
	}
	if (!_val) {
		// error
		return nullptr;
	}
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtType::AugAssignK;
	p_->V.augAssign.target = _target;
	p_->V.augAssign.op = _op;
	p_->V.augAssign.val = _val;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_for(Expression* _target, Expression* _iter, StmtSeq* _body,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	if (!_target) {
		// error
		return nullptr;
	}
	if (!_iter) {
		// error
		return nullptr;
	}
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtType::ForK;
	p_->V.for_.target = _target;
	p_->V.for_.iter = _iter;
	p_->V.for_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_asyncFor(Expression* _target, Expression* _iter, StmtSeq* _body,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	if (!_target) {
		// error
		return nullptr;
	}
	if (!_iter) {
		// error
		return nullptr;
	}
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtType::ForK;
	p_->V.asyncFor.target = _target;
	p_->V.asyncFor.iter = _iter;
	p_->V.asyncFor.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_while(Expression* _condetion, StmtSeq* _body,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	if (!_condetion) {
		// error
		return nullptr;
	}
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtType::WhileK;
	p_->V.while_.condition = _condetion;
	p_->V.while_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_if(Expression* _condition, StmtSeq* _body, StmtSeq* _else,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	if (!_condition) {
		// error
		return nullptr;
	}
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtType::IfK;
	p_->V.if_.condition = _condition;
	p_->V.if_.body = _body;
	p_->V.if_.else_ = _else;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_with(WithItemSeq* _items, StmtSeq* _body,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::WithK;
	p_->V.with_.items = _items;
	p_->V.with_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_asyncWith(WithItemSeq* _items, StmtSeq* _body,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::AsyncWithK;
	p_->V.with_.items = _items;
	p_->V.with_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_import(AliasSeq* _names,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::ImportK;
	p_->V.import.names = _names;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_importFrom(AlifObject* _module, AliasSeq* _names,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::ImportFromK;
	p_->V.importFrom.module = _module;
	p_->V.importFrom.names = _names;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_global(IdentifierSeq* _names,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::GlobalK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_nonlocal(IdentifierSeq* _names,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::NonlocalK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_pass(int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::PassK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_break(int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::BreakK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Statement* alifAST_continue(int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Statement* p_{};
	p_ = (Statement*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtType::CountinueK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Expression* alifAST_boolOp(BoolOp _op, ExprSeq* _vals,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p_{};
	if (!_op) {
		// error
		return nullptr;
	}
	p_ = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = ExprType::BoolOpK;
	p_->V.boolOp.op = _op;
	p_->V.boolOp.vals = _vals;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Expression* alifAST_binOp(Expression* _left, Operator _op, Expression* _right, int _lineNo,
	int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Expression* p_{};
	if (!_left) {
		// error
		return nullptr;
	}
	if (!_op) {
		// error
		return nullptr;
	}
	if (!_right) {
		// error
		return nullptr;
	}
	p_ = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = BinOpK;
	p_->V.binOp.left = _left;
	p_->V.binOp.op = _op;
	p_->V.binOp.right = _right;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

Expression* alifAST_unaryOp(UnaryOp _op, Expression* _operand,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Expression* p{};
	if (!_op) {
		// error
		return nullptr;
	}
	if (!_operand) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = UnaryOpK;
	p->V.unaryOp.op = _op;
	p->V.unaryOp.operand = _operand;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_ifExpr(Expression* _condition, Expression* _body, Expression* _else,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Expression* p{};
	if (!_condition) {
		// error
		return nullptr;
	}
	if (!_body) {
		// error
		return nullptr;
	}
	if (!_else) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = ExprType::IfExprK;
	p->V.ifExpr.condition_ = _condition;
	p->V.ifExpr.body_ = _body;
	p->V.ifExpr.else_ = _else;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_dict(ExprSeq* _keys, ExprSeq* _vals,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = DictK;
	p->V.dict.keys = _keys;
	p->V.dict.vals = _vals;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_listComp(Expression* _elt, CompSeq* _generators,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	if (!_elt) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = ListCompK;
	p->V.listComp.elt = _elt;
	p->V.listComp.generetors = _generators;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_dictComp(Expression* _key, Expression* _val, CompSeq* _generators,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	if (!_key) {
		// error
		return nullptr;
	}
	if (!_val) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = DictCompK;
	p->V.dictComp.key = _key;
	p->V.dictComp.val = _val;
	p->V.dictComp.generetors = _generators;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_await(Expression* _val,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	if (!_val) {
		// error
		return  nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = AwaitK;
	p->V.await_.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_yield(Expression* _val,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = YieldK;
	p->V.yield.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_yieldFrom(Expression* _val,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	if (!_val) {
		// error
		return  nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = YieldFromK;
	p->V.yieldFrom.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_compare(Expression* _left, IntSeq* _ops, ExprSeq* _comparators,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Expression* p{};
	if (!_left) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	p->type = ExprType::CompareK;
	p->V.compare.left = _left;
	p->V.compare.ops = _ops;
	p->V.compare.comparators = _comparators;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_call(Expression* _func, ExprSeq* _args, KeywordSeq* _keywords,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	if (!_func) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	p->type = CallK;
	p->V.call.func = _func;
	p->V.call.args = _args;
	p->V.call.keywords = _keywords;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_formattedValue(Expression* _val, AlifIntT _conversion, Expression* _formatSpec,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	if (!_val) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = FormattedValK;
	p->V.fromattedValue.val = _val;
	p->V.fromattedValue.conversion = _conversion;
	p->V.fromattedValue.formatSpec = _formatSpec;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_joinedStr(ExprSeq* _vals,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};

	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = JoinStrK;
	p->V.joinStr.vals = _vals;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_attribute(Expression* _val, AlifObject* _attr, ExprCTX _ctx,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Expression* p{};
	if (!_val) {
		// error
		return nullptr;
	}
	if (!_attr) {
		// error
		return nullptr;
	}
	if (!_ctx) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = AttributeK;
	p->V.attribute.val = _val;
	p->V.attribute.attr = _attr;
	p->V.attribute.ctx = _ctx;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_subScript(Expression* _val, Expression* _slice, ExprCTX _ctx,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Expression* p{};
	if (!_val) {
		// error
		return nullptr;
	}
	if (!_slice) {
		// error
		return nullptr;
	}
	if (!_ctx) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = SubScriptK;
	p->V.subScript.val = _val;
	p->V.subScript.slice = _slice;
	p->V.subScript.ctx = _ctx;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Expression* alifAST_star(Expression* _val, ExprCTX _ctx,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Expression* p{};
	if (!_val) {
		// error
		return nullptr;
	}
	if (!_ctx) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = StarK;
	p->V.star.val = _val;
	p->V.star.ctx = _ctx;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

Expression* alifAST_name(AlifObject* _id, ExprCTX _ctx,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {
	Expression* p{};
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = NameK;
	p->V.name.name = _id;
	p->V.name.ctx = _ctx;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

Expression* alifAST_list(ExprSeq* _elts, ExprCTX _ctx,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	if (!_ctx) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = ListK;
	p->V.list.elts = _elts;
	p->V.list.ctx = _ctx;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

Expression* alifAST_tuple(ExprSeq* _elts, ExprCTX _ctx,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	if (!_ctx) {
		// error
		return nullptr;
	}
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = TupleK;
	p->V.tuple.elts = _elts;
	p->V.tuple.ctx = _ctx;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

Expression* alifAST_slice(Expression* _lower, Expression* _upper, Expression* _step,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Expression* p{};
	p = (Expression*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = SliceK;
	p->V.slice.lower = _lower;
	p->V.slice.upper = _upper;
	p->V.slice.step = _step;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Comprehension* alifAST_comprehension(Expression* _target, Expression* _iter, ExprSeq* _ifs,
	int _isAsync, AlifASTMem* _astMem) {

	Comprehension* p{};
	if (!_target) {
		// error
		return nullptr;
	}
	if (!_iter) {
		// error
		return nullptr;
	}
	p = (Comprehension*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->target = _target;
	p->iter = _iter;
	p->ifs = _ifs;
	p->isAsync = _isAsync;

	return p;
}

Arguments* alifAST_arguments(ArgSeq* _posOnlyArgs, ArgSeq* _args, Arg* _varArg, ArgSeq* _kwOnlyArgs,
	ExprSeq* _kwDefaults, Arg* _kwArg, ExprSeq* _defaults, AlifASTMem* _astMem) {

	Arguments* p{};
	p = (Arguments*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->posOnlyArgs = _posOnlyArgs;
	p->args = _args;
	p->varArg = _varArg;
	p->kwOnlyArgs = _kwOnlyArgs;
	p->kwDefaults = _kwDefaults;
	p->kwArg = _kwArg;
	p->defaults = _defaults;

	return p;
}

Arg* alifAST_arg(AlifObject* _arg,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Arg* p{};

	p = (Arg*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->arg = _arg;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Keyword* alifAST_keyword(AlifObject* _arg, Expression* _val,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Keyword* p{};
	if (!_val) {
		// error
		return nullptr;
	}
	p = (Keyword*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->arg = _arg;
	p->val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

Alias* alifAST_alias(AlifObject* _name, AlifObject* _asName,
	int _lineNo, int _colOffset, int _endLineNo, int _endColOffset, AlifASTMem* _astMem) {

	Alias* p{};
	p = (Alias*)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->name = _name;
	p->asName = _asName;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

WithItem* alifAST_withItem(Expression* _exprCTX, Expression* _optVars, AlifASTMem* _astMem) {

	WithItem* p_{};
	if (!_exprCTX) {
		// error
		return nullptr;
	}

	p_ = (WithItem*)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->contextExpr = _exprCTX;
	p_->optionalVars = _optVars;

	return p_;
}




AlifObject* alifAST_getDocString(StmtSeq* _body) {
	if (!SEQ_LEN(_body)) return nullptr;

	Statement* stmt = SEQ_GET(_body ,0);
	if (stmt->type != StmtType::ExprK) return nullptr;

	Expression* expr = stmt->V.expression.val;
	if (expr->type == ExprType::ConstantK and ALIFUSTR_CHECKEXACT(expr->V.constant.val)) {
		return expr->V.constant.val;
	}

	return nullptr;
}
