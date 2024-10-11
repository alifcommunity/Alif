#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"
#include "AlifCore_Memory.h"



 // 382
GENERATE_SEQ_CONSTRUCTOR(Expr, expr, ExprTy); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة
GENERATE_SEQ_CONSTRUCTOR(Arg, arg, ArgTy); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة
GENERATE_SEQ_CONSTRUCTOR(Keyword, keyword, KeywordTy); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة

ModuleTy alifAST_module(ASDLStmtSeq* _body, AlifASTMem* _astMem) { // 6673
	ModuleTy p{};
	p = (ModuleTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = ModuleK;
	p->V.module.body = _body;
	return p;
}

ModuleTy alifAST_interactive(ASDLStmtSeq* _body, AlifASTMem* _astMem) { // 6687
	ModuleTy p{};
	p = (ModuleTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = InteractiveK;
	p->V.interactive.body = _body;
	return p;
}

StmtTy alifAST_assign(ASDLExprSeq* _targets, ExprTy _val,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 6869
	StmtTy p{};
	if (!_val) {
		//alifErr_setString(_alifExcValueError_,
		//	"field 'value' is required for Assign");
		return nullptr;
	}
	p = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

StmtTy alifAST_expr(ExprTy _val,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 7321
	StmtTy p{};
	if (!_val) {
		//alifErr_setString(_alifExcValueError_,
		//	"field 'value' is required for Expr");
		return nullptr;
	}
	p = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;
	p->type = ExprK;
	p->V.expression.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

ExprTy alifAST_constant(AlifObject* _val, AlifObject* _type,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 7856

	ExprTy p{};
	if (!_val) {
		//alifErr_setString(_alifExcValueError_,
		//	"field 'value' is required for Constant");
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

StmtTy alifAST_asyncFunctionDef(AlifObject* _name, Arguments* _args,
	ASDLStmtSeq* _body, AlifIntT _lineNo, AlifIntT _colOffset,
	AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) { // 6770

	StmtTy p_{};
	if (!_name) {
		//alifErr_setString(_alifExcValueError_,
		//	"field 'name' is required for AsyncFunctionDef");
		return nullptr;
	}
	if (!_args) {
		//alifErr_setString(_alifExcValueError_,
		//	"field 'args' is required for AsyncFunctionDef");
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK::FunctionDefK;
	p_->V.functionDef.name = _name;
	p_->V.functionDef.args = _args;
	p_->V.functionDef.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_functionDef(AlifObject* _name, Arguments* _args, ASDLStmtSeq* _body,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	if (!_name) {
		// error
		return nullptr;
	}
	if (!_args) {
		// error
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK::AsyncFunctionDefK;
	p_->V.asyncFunctionDef.name = _name;
	p_->V.asyncFunctionDef.args = _args;
	p_->V.asyncFunctionDef.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_return(ExprTy _val,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
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

StmtTy alifAST_delete(ASDLExprSeq* _targets,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::DeleteK;
	p_->V.delete_.targets = _targets;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_classDef(AlifObject* _name, ASDLExprSeq* _bases, ASDLKeywordSeq* _keywords, ASDLStmtSeq* _body,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	if (!_name) {
		// error
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK::ClassDefK;
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

StmtTy alifAST_augAssign(ExprTy _target, Operator_ _op, ExprTy _val,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
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
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK::AugAssignK;
	p_->V.augAssign.target = _target;
	p_->V.augAssign.op = _op;
	p_->V.augAssign.val = _val;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_for(ExprTy _target, ExprTy _iter, ASDLStmtSeq* _body,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	if (!_target) {
		// error
		return nullptr;
	}
	if (!_iter) {
		// error
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK::ForK;
	p_->V.for_.target = _target;
	p_->V.for_.iter = _iter;
	p_->V.for_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_asyncFor(ExprTy _target, ExprTy _iter, ASDLStmtSeq* _body,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	if (!_target) {
		// error
		return nullptr;
	}
	if (!_iter) {
		// error
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK::ForK;
	p_->V.asyncFor.target = _target;
	p_->V.asyncFor.iter = _iter;
	p_->V.asyncFor.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_while(ExprTy _condetion, ASDLStmtSeq* _body,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	if (!_condetion) {
		// error
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK::WhileK;
	p_->V.while_.condition = _condetion;
	p_->V.while_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_if(ExprTy _condition, ASDLStmtSeq* _body, ASDLStmtSeq* _else,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	if (!_condition) {
		// error
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK::IfK;
	p_->V.if_.condition = _condition;
	p_->V.if_.body = _body;
	p_->V.if_.else_ = _else;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_with(ASDLWithItemSeq* _items, ASDLStmtSeq* _body,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::WithK;
	p_->V.with_.items = _items;
	p_->V.with_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_asyncWith(ASDLWithItemSeq* _items, ASDLStmtSeq* _body,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::AsyncWithK;
	p_->V.with_.items = _items;
	p_->V.with_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_import(ASDLAliasSeq* _names,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::ImportK;
	p_->V.import.names = _names;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_importFrom(AlifObject* _module, ASDLAliasSeq* _names,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::ImportFromK;
	p_->V.importFrom.module = _module;
	p_->V.importFrom.names = _names;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_global(ASDLIdentifierSeq* _names,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::GlobalK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_nonlocal(ASDLIdentifierSeq* _names,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::NonlocalK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_pass(AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::PassK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_break(AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::BreakK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_continue(AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK::CountinueK;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

ExprTy alifAST_boolOp(BoolOp_ _op, ASDLExprSeq* _vals,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p_{};
	if (!_op) {
		// error
		return nullptr;
	}
	p_ = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = ExprK::BoolOpK;
	p_->V.boolOp.op = _op;
	p_->V.boolOp.vals = _vals;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

ExprTy alifAST_binOp(ExprTy _left, Operator_ _op, ExprTy _right, AlifIntT _lineNo,
	AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	ExprTy p_{};
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
	p_ = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p_));
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

ExprTy alifAST_unaryOp(UnaryOp_ _op, ExprTy _operand,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	ExprTy p{};
	if (!_op) {
		// error
		return nullptr;
	}
	if (!_operand) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_ifExpr(ExprTy _condition, ExprTy _body, ExprTy _else,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	ExprTy p{};
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
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = ExprK::IfExprK;
	p->V.ifExpr.condition = _condition;
	p->V.ifExpr.body = _body;
	p->V.ifExpr.else_ = _else;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

ExprTy alifAST_dict(ASDLExprSeq* _keys, ASDLExprSeq* _vals,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_listComp(ExprTy _elt, ASDLComprehensionSeq* _generators,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	if (!_elt) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_dictComp(ExprTy _key, ExprTy _val, ASDLComprehensionSeq* _generators,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	if (!_key) {
		// error
		return nullptr;
	}
	if (!_val) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_await(ExprTy _val,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	if (!_val) {
		// error
		return  nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = AwaitK;
	p->V.await.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

ExprTy alifAST_yield(ExprTy _val,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = YieldK;
	p->V.yield.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

ExprTy alifAST_yieldFrom(ExprTy _val,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	if (!_val) {
		// error
		return  nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = YieldFromK;
	p->V.yieldFrom.val = _val;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

ExprTy alifAST_compare(ExprTy _left, ASDLIntSeq* _ops, ASDLExprSeq* _comparators,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	ExprTy p{};
	if (!_left) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
	p->type = ExprK::CompareK;
	p->V.compare.left = _left;
	p->V.compare.ops = _ops;
	p->V.compare.comparators = _comparators;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

ExprTy alifAST_call(ExprTy _func, ASDLExprSeq* _args, ASDLKeywordSeq* _keywords,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	if (!_func) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_formattedValue(ExprTy _val, AlifIntT _conversion, ExprTy _formatSpec,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	if (!_val) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_joinedStr(ASDLExprSeq* _vals,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};

	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p) return nullptr;

	p->type = JoinStrK;
	p->V.joinStr.vals = _vals;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;

	return p;
}

ExprTy alifAST_attribute(ExprTy _val, AlifObject* _attr, ExprContext_ _ctx,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	ExprTy p{};
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
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_subScript(ExprTy _val, ExprTy _slice, ExprContext_ _ctx,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	ExprTy p{};
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
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_star(ExprTy _val, ExprContext_ _ctx,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	ExprTy p{};
	if (!_val) {
		// error
		return nullptr;
	}
	if (!_ctx) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_name(AlifObject* _id, ExprContext_ _ctx,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {
	ExprTy p{};
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_list(ASDLExprSeq* _elts, ExprContext_ _ctx,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	if (!_ctx) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_tuple(ASDLExprSeq* _elts, ExprContext_ _ctx,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	if (!_ctx) {
		// error
		return nullptr;
	}
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

ExprTy alifAST_slice(ExprTy _lower, ExprTy _upper, ExprTy _step,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

	ExprTy p{};
	p = (ExprTy)alifASTMem_malloc(_astMem, sizeof(*p));
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

Comprehension* alifAST_comprehension(ExprTy _target, ExprTy _iter, ASDLExprSeq* _ifs,
	AlifIntT _isAsync, AlifASTMem* _astMem) {

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

Arguments* alifAST_arguments(ASDLArgSeq* _posOnlyArgs, ASDLArgSeq* _args, Arg* _varArg, ASDLArgSeq* _kwOnlyArgs,
	ASDLExprSeq* _kwDefaults, Arg* _kwArg, ASDLExprSeq* _defaults, AlifASTMem* _astMem) {

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
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

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

Keyword* alifAST_keyword(AlifObject* _arg, ExprTy _val,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

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
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) {

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

WithItem* alifAST_withItem(ExprTy _exprCTX, ExprTy _optVars, AlifASTMem* _astMem) {

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




AlifObject* alifAST_getDocString(ASDLStmtSeq* _body) {
	if (!ASDL_SEQ_LEN(_body)) return nullptr;

	StmtTy stmt = ASDL_SEQ_GET(_body ,0);
	if (stmt->type != StmtK::ExprK) return nullptr;

	ExprTy expr = stmt->V.expression.val;
	if (expr->type == ExprK::ConstantK and ALIFUSTR_CHECKEXACT(expr->V.constant.val)) {
		return expr->V.constant.val;
	}

	return nullptr;
}
