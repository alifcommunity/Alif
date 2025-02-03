#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Format.h"
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

static AlifIntT make_const(ExprTy _node,
	AlifObject* _val, AlifASTMem* _astMem) { // 32
	// Even if no new value was calculated, make_const may still
	// need to clear an error (e.g. for division by zero)
	if (_val == nullptr) {
		//if (alifErr_exceptionMatches(_alifExcKeyboardInterrupt_)) {
		//	return 0;
		//}
		//alifErr_clear();
		return 1;
	}
	if (alifASTMem_listAddAlifObj(_astMem, _val) < 0) {
		ALIF_DECREF(_val);
		return 0;
	}
	_node->type = ExprK_::ConstantK;
	_node->V.constant.type = nullptr;
	_node->V.constant.val = _val;
	return 1;
}

#define COPY_NODE(_to, _from) (memcpy((_to), (_from), sizeof(Expr)))

static AlifIntT has_starred(ASDLExprSeq* _elts) { // 56
	AlifSizeT n = ASDL_SEQ_LEN(_elts);
	for (AlifSizeT i = 0; i < n; i++) {
		ExprTy e = (ExprTy)ASDL_SEQ_GET(_elts, i);
		if (e->type == ExprK_::StarK) {
			return 1;
		}
	}
	return 0;
}

static AlifObject* unary_not(AlifObject* _v) { // 70
	AlifIntT r = alifObject_isTrue(_v);
	if (r < 0) return nullptr;
	return alifBool_fromLong(!r);
}

static AlifIntT fold_unaryOp(ExprTy _node,
	AlifASTMem* _astMem, AlifASTOptimizeState* _state) { // 79
	ExprTy arg = _node->V.unaryOp.operand;

	if (arg->type != ExprK_::ConstantK) {
		/* Fold not into comparison */
		if (_node->V.unaryOp.op == UnaryOp_::Not and arg->type == ExprK_::CompareK and
			ASDL_SEQ_LEN(arg->V.compare.ops) == 1) {
			CmpOp_ op = (CmpOp_)ASDL_SEQ_GET(arg->V.compare.ops, 0);
			switch (op) {
			case CmpOp_::Is:
				op = CmpOp_::IsNot;
				break;
			case CmpOp_::IsNot:
				op = CmpOp_::Is;
				break;
			case CmpOp_::In:
				op = CmpOp_::NotIn;
				break;
			case CmpOp_::NotIn:
				op = CmpOp_::In;
				break;
				// The remaining comparison operators can't be safely inverted
			case CmpOp_::Equal:
			case CmpOp_::NotEq:
			case CmpOp_::LessThan:
			case CmpOp_::LessThanEq:
			case CmpOp_::GreaterThan:
			case CmpOp_::GreaterThanEq:
				op = (CmpOp_)0; // The AST enums leave "0" free as an "unused" marker
				break;
				// No default case, so the compiler will emit a warning if new
				// comparison operators are added without being handled here
			}
			if (op) {
				ASDL_SEQ_SET(arg->V.compare.ops, 0, op);
				COPY_NODE(_node, arg);
				return 1;
			}
		}
		return 1;
	}

	typedef AlifObject* (*UnaryOp)(AlifObject*);
	static const UnaryOp ops[] = {
		nullptr,
		alifNumber_invert,			// [UnaryOp_::Invert]
		unary_not,					// [UnaryOp_::Not] 
		alifNumber_positive,		// [UnaryOp_::UAdd] 
		alifNumber_negative,		// [UnaryOp_::USub] 
		alifNumber_sqrt,			// [UnaryOp_::Sqrt]  //* alif
	};
	AlifObject* newval = ops[_node->V.unaryOp.op](arg->V.constant.val);
	return make_const(_node, newval, _astMem);
}

static AlifSizeT check_complexity(AlifObject* _obj, AlifSizeT _limit) { // 147
	if (ALIFTUPLE_CHECK(_obj)) {
		AlifSizeT i{};
		_limit -= ALIFTUPLE_GET_SIZE(_obj);
		for (i = 0; _limit >= 0 and i < ALIFTUPLE_GET_SIZE(_obj); i++) {
			_limit = check_complexity(ALIFTUPLE_GET_ITEM(_obj, i), _limit);
		}
		return _limit;
	}
	return _limit;
}

 // 161
#define MAX_INT_SIZE           128  /* bits */
#define MAX_COLLECTION_SIZE    256  /* items */
#define MAX_STR_SIZE          4096  /* characters */
#define MAX_TOTAL_ITEMS       1024  /* including nested collections */

static AlifObject* safe_multiply(AlifObject* _v, AlifObject* _w) { // 166
	if (ALIFLONG_CHECK(_v) and ALIFLONG_CHECK(_w) and
		!_alifLong_isZero((AlifLongObject*)_v) and !_alifLong_isZero((AlifLongObject*)_w)) {
		int64_t vBits = _alifLong_numBits(_v);
		int64_t wBits = _alifLong_numBits(_w);
		if (vBits + wBits > MAX_INT_SIZE) {
			return nullptr;
		}
	}
	else if (ALIFLONG_CHECK(_v) and ALIFTUPLE_CHECK(_w)) {
		AlifSizeT size = ALIFTUPLE_GET_SIZE(_w);
		if (size) {
			long n = alifLong_asLong(_v);
			if (n < 0 or n > MAX_COLLECTION_SIZE / size) {
				return nullptr;
			}
			if (n and check_complexity(_w, MAX_TOTAL_ITEMS / n) < 0) {
				return nullptr;
			}
		}
	}
	else if (ALIFLONG_CHECK(_v) and (ALIFUSTR_CHECK(_w) or ALIFBYTES_CHECK(_w))) {
		AlifSizeT size = ALIFUSTR_CHECK(_w) ? ALIFUSTR_GET_LENGTH(_w) :
			ALIFBYTES_GET_SIZE(_w);
		if (size) {
			long n = alifLong_asLong(_v);
			if (n < 0 or n > MAX_STR_SIZE / size) {
				return nullptr;
			}
		}
	}
	else if (ALIFLONG_CHECK(_w)
		and (ALIFTUPLE_CHECK(_v)
			or ALIFUSTR_CHECK(_v)
			or ALIFBYTES_CHECK(_v))
		) {
		return safe_multiply(_w, _v);
	}

	return alifNumber_multiply(_v, _w);
}


static AlifObject* safe_power(AlifObject* _v, AlifObject* _w) { // 212
	if (ALIFLONG_CHECK(_v) and ALIFLONG_CHECK(_w) and
		!_alifLong_isZero((AlifLongObject*)_v) and _alifLong_isPositive((AlifLongObject*)_w)) {
		int64_t vBits = _alifLong_numBits(_v);
		AlifUSizeT wBits = alifLong_asSizeT(_w);
		if (wBits == (AlifUSizeT)-1) {
			return nullptr;
		}
		if ((uint64_t)vBits > MAX_INT_SIZE / wBits) {
			return nullptr;
		}
	}

	return alifNumber_power(_v, _w, ALIF_NONE);
}

static AlifObject* safe_mod(AlifObject* _v, AlifObject* _w) { // 250
	if (ALIFUSTR_CHECK(_v) or ALIFBYTES_CHECK(_v)) {
		return nullptr;
	}

	return alifNumber_remainder(_v, _w);
}

static ExprTy parse_literal(AlifObject* _fmt,
	AlifSizeT* _ppos, AlifASTMem* _astMem) { // 261
	const void* data = ALIFUSTR_DATA(_fmt);
	AlifIntT kind = ALIFUSTR_KIND(_fmt);
	AlifSizeT size = ALIFUSTR_GET_LENGTH(_fmt);
	AlifSizeT start, pos;
	AlifIntT hasPercents = 0;
	start = pos = *_ppos;
	while (pos < size) {
		if (ALIFUSTR_READ(kind, data, pos) != '%') {
			pos++;
		}
		else if (pos + 1 < size and ALIFUSTR_READ(kind, data, pos + 1) == '%') {
			hasPercents = 1;
			pos += 2;
		}
		else {
			break;
		}
	}
	*_ppos = pos;
	if (pos == start) {
		return nullptr;
	}
	AlifObject* str = alifUStr_subString(_fmt, start, pos);
	/* str = str.replace('%%', '%') */
	if (str and hasPercents) {
		ALIF_SETREF(str, alifUStr_replace(str, &ALIF_STR(DblPercent),
			ALIF_LATIN1_CHR('%'), -1));
	}
	if (!str) {
		return nullptr;
	}

	if (alifASTMem_listAddAlifObj(_astMem, str) < 0) {
		ALIF_DECREF(str);
		return nullptr;
	}
	return alifAST_constant(str, nullptr, -1, -1, -1, -1, _astMem);
}

#define MAXDIGITS 3 // 304

static AlifIntT simpleFormat_argParse(AlifObject* _fmt, AlifSizeT* _ppos,
	AlifIntT* _spec, AlifIntT* _flags, AlifIntT* _width, AlifIntT* _prec) { // 306
	AlifSizeT pos = *_ppos, len = ALIFUSTR_GET_LENGTH(_fmt);
	AlifUCS4 ch{};

#define NEXTC do {                      \
    if (pos >= len) {                   \
        return 0;                       \
    }                                   \
    ch = ALIFUSTR_READ_CHAR(_fmt, pos); \
    pos++;                              \
} while (0)

	*_flags = 0;
	while (1) {
		NEXTC;
		switch (ch) {
		case '-': *_flags |= F_LJUST; continue;
		case '+': *_flags |= F_SIGN; continue;
		case ' ': *_flags |= F_BLANK; continue;
		case '#': *_flags |= F_ALT; continue;
		case '0': *_flags |= F_ZERO; continue;
		}
		break;
	}
	if ('0' <= ch && ch <= '9') {
		*_width = 0;
		AlifIntT digits = 0;
		while ('0' <= ch and ch <= '9') {
			*_width = *_width * 10 + (ch - '0');
			NEXTC;
			if (++digits >= MAXDIGITS) {
				return 0;
			}
		}
	}

	if (ch == '.') {
		NEXTC;
		*_prec = 0;
		if ('0' <= ch and ch <= '9') {
			AlifIntT digits = 0;
			while ('0' <= ch and ch <= '9') {
				*_prec = *_prec * 10 + (ch - '0');
				NEXTC;
				if (++digits >= MAXDIGITS) {
					return 0;
				}
			}
		}
	}
	*_spec = ch;
	*_ppos = pos;
	return 1;

#undef NEXTC
}

static ExprTy parse_format(AlifObject* _fmt, AlifSizeT* _ppos,
	ExprTy _arg, AlifASTMem* _astMem) { // 366
	AlifIntT spec{}, flags{}, width = -1, prec = -1;
	if (!simpleFormat_argParse(_fmt, _ppos, &spec, &flags, &width, &prec)) {
		// Unsupported format.
		return nullptr;
	}
	if (spec == 's' or spec == 'r' or spec == 'a') {
		char buf[1 + MAXDIGITS + 1 + MAXDIGITS + 1], * p = buf;
		if (!(flags & F_LJUST) && width > 0) {
			*p++ = '>';
		}
		if (width >= 0) {
			p += snprintf(p, MAXDIGITS + 1, "%d", width);
		}
		if (prec >= 0) {
			p += snprintf(p, MAXDIGITS + 2, ".%d", prec);
		}
		ExprTy format_spec = nullptr;
		if (p != buf) {
			AlifObject* str = alifUStr_fromString(buf);
			if (str == nullptr) {
				return nullptr;
			}
			if (alifASTMem_listAddAlifObj(_astMem, str) < 0) {
				ALIF_DECREF(str);
				return nullptr;
			}
			format_spec = alifAST_constant(str, nullptr, -1, -1, -1, -1, _astMem);
			if (format_spec == nullptr) {
				return nullptr;
			}
		}
		return alifAST_formattedValue(_arg, spec, format_spec,
			_arg->lineNo, _arg->colOffset,
			_arg->endLineNo, _arg->endColOffset,
			_astMem);
	}
	// Unsupported format.
	return nullptr;
}

static AlifIntT optimize_format(ExprTy _node,
	AlifObject* _fmt, ASDLExprSeq* _elts, AlifASTMem* _astMem) { // 409
	AlifSizeT pos = 0;
	AlifSizeT cnt = 0;
	ASDLExprSeq* seq = alifNew_exprSeq(ASDL_SEQ_LEN(_elts) * 2 + 1, _astMem);
	if (!seq) {
		return 0;
	}
	seq->size = 0;

	while (1) {
		ExprTy lit = parse_literal(_fmt, &pos, _astMem);
		if (lit) {
			ASDL_SEQ_SET(seq, seq->size++, lit);
		}
		else if (alifErr_occurred()) {
			return 0;
		}

		if (pos >= ALIFUSTR_GET_LENGTH(_fmt)) {
			break;
		}
		if (cnt >= ASDL_SEQ_LEN(_elts)) {
			// More format units than items.
			return 1;
		}
		pos++;
		ExprTy expr = parse_format(_fmt, &pos, ASDL_SEQ_GET(_elts, cnt), _astMem);
		cnt++;
		if (!expr) {
			return !alifErr_occurred();
		}
		ASDL_SEQ_SET(seq, seq->size++, expr);
	}
	if (cnt < ASDL_SEQ_LEN(_elts)) {
		// More items than format units.
		return 1;
	}
	ExprTy res = alifAST_joinedStr(seq,
		_node->lineNo, _node->colOffset,
		_node->endLineNo, _node->endColOffset,
		_astMem);
	if (!res) {
		return 0;
	}
	COPY_NODE(_node, res);
	return 1;
}


static AlifIntT fold_binOp(ExprTy _node,
	AlifASTMem* _astMem, AlifASTOptimizeState* _state) { // 461
	ExprTy lhs{}, rhs{};
	lhs = _node->V.binOp.left;
	rhs = _node->V.binOp.right;
	if (lhs->type != ExprK_::ConstantK) {
		return 1;
	}
	AlifObject* lv = lhs->V.constant.val;

	if (_node->V.binOp.op == Mod and
		rhs->type == ExprK_::TupleK and
		ALIFUSTR_CHECK(lv) and
		!has_starred(rhs->V.tuple.elts))
	{
		return optimize_format(_node, lv, rhs->V.tuple.elts, _astMem);
	}

	if (rhs->type != ExprK_::ConstantK) {
		return 1;
	}

	AlifObject* rv = rhs->V.constant.val;
	AlifObject* newval = nullptr;

	switch (_node->V.binOp.op) {
	case Operator_::Add:
		newval = alifNumber_add(lv, rv);
		break;
	case Operator_::Sub:
		newval = alifNumber_subtract(lv, rv);
		break;
	case Operator_::Mult:
		newval = safe_multiply(lv, rv);
		break;
	case Operator_::Div:
		newval = alifNumber_trueDivide(lv, rv);
		break;
	case Operator_::Mod:
		newval = safe_mod(lv, rv);
		break;
	case Operator_::Pow:
		newval = safe_power(lv, rv);
		break;
	case Operator_::LShift:
		//newval = safe_lshift(lv, rv);
		break;
	case Operator_::RShift:
		newval = alifNumber_rshift(lv, rv);
		break;
	case Operator_::BitOr:
		newval = alifNumber_or(lv, rv);
		break;
	case Operator_::BitXor:
		newval = alifNumber_xor(lv, rv);
		break;
	case Operator_::BitAnd:
		newval = alifNumber_and(lv, rv);
		break;
	case Operator_::FloorDiv:
		newval = alifNumber_floorDivide(lv, rv);
		break;
		// No builtin constants implement the following operators
	//case Operator_::MatMult:
	//	return 1;
		// No default case, so the compiler will emit a warning if new binary
		// operators are added without being handled here
	}

	return make_const(_node, newval, _astMem);
}

static AlifObject* make_constTuple(ASDLExprSeq* _elts) { // 534
	for (AlifSizeT i = 0; i < ASDL_SEQ_LEN(_elts); i++) {
		ExprTy e = (ExprTy)ASDL_SEQ_GET(_elts, i);
		if (e->type != ExprK_::ConstantK) {
			return nullptr;
		}
	}

	AlifObject* newVal = alifTuple_new(ASDL_SEQ_LEN(_elts));
	if (newVal == nullptr) {
		return nullptr;
	}

	for (AlifSizeT i = 0; i < ASDL_SEQ_LEN(_elts); i++) {
		ExprTy e = (ExprTy)ASDL_SEQ_GET(_elts, i);
		AlifObject* v = e->V.constant.val;
		ALIFTUPLE_SET_ITEM(newVal, i, ALIF_NEWREF(v));
	}
	return newVal;
}

static AlifIntT fold_tuple(ExprTy _node,
	AlifASTMem* _astMem, AlifASTOptimizeState* _state) { // 558
	AlifObject* newVal{};

	if (_node->V.tuple.ctx != ExprContext_::Load)
		return 1;

	newVal = make_constTuple(_node->V.tuple.elts);
	return make_const(_node, newVal, _astMem);
}

static AlifIntT fold_subScr(ExprTy _node,
	AlifASTMem* _astMem, AlifASTOptimizeState* _state) { // 569
	AlifObject* newval{};
	ExprTy arg{}, idx{};

	arg = _node->V.subScript.val;
	idx = _node->V.subScript.slice;
	if (_node->V.subScript.ctx != ExprContext_::Load or
		arg->type != ExprK_::ConstantK or
		idx->type != ExprK_::ConstantK)
	{
		return 1;
	}

	newval = alifObject_getItem(arg->V.constant.val, idx->V.constant.val);
	return make_const(_node, newval, _astMem);
}

static AlifIntT fold_iter(ExprTy _arg,
	AlifASTMem* _astMem, AlifASTOptimizeState* _state) { // 594
	AlifObject* newVal{};
	if (_arg->type == ExprK_::ListK) {
		/* First change a list into tuple. */
		ASDLExprSeq* elts = _arg->V.list.elts;
		if (has_starred(elts)) {
			return 1;
		}
		ExprContext_ ctx = _arg->V.list.ctx;
		_arg->type = ExprK_::TupleK;
		_arg->V.tuple.elts = elts;
		_arg->V.tuple.ctx = ctx;
		/* Try to create a constant tuple. */
		newVal = make_constTuple(elts);
	}
	else if (_arg->type == ExprK_::SetK) {
		newVal = make_constTuple(_arg->V.set.elts);
		if (newVal) {
			ALIF_SETREF(newVal, alifFrozenSet_new(newVal));
		}
	}
	else {
		return 1;
	}
	return make_const(_arg, newVal, _astMem);
}

static AlifIntT fold_compare(ExprTy _node,
	AlifASTMem* _astMem, AlifASTOptimizeState* _state) { // 623
	ASDLIntSeq* ops{};
	ASDLExprSeq* args{};
	AlifSizeT i{};

	ops = _node->V.compare.ops;
	args = _node->V.compare.comparators;
	/* Change literal list or set in 'in' or 'not in' into
	   tuple or frozenset respectively. */
	i = ASDL_SEQ_LEN(ops) - 1;
	AlifIntT op = ASDL_SEQ_GET(ops, i);
	if (op == CmpOp_::In or op == CmpOp_::NotIn) {
		if (!fold_iter((ExprTy)ASDL_SEQ_GET(args, i), _astMem, _state)) {
			return 0;
		}
	}
	return 1;
}


static AlifIntT astFold_mod(ModuleTy, AlifASTMem*, AlifASTOptimizeState*); // 644
static AlifIntT astFold_stmt(StmtTy, AlifASTMem*, AlifASTOptimizeState*);
static AlifIntT astFold_expr(ExprTy, AlifASTMem*, AlifASTOptimizeState*);
static AlifIntT astFold_arguments(ArgumentsTy, AlifASTMem*, AlifASTOptimizeState*);
static AlifIntT astFold_comprehension(ComprehensionTy, AlifASTMem*, AlifASTOptimizeState*); // 648
static AlifIntT astFold_keyword(KeywordTy, AlifASTMem*, AlifASTOptimizeState*); //  649
static AlifIntT astFold_arg(ArgTy, AlifASTMem*, AlifASTOptimizeState*); // 650
static AlifIntT astFold_typeParam(TypeParamTy, AlifASTMem*, AlifASTOptimizeState*); // 655



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



static AlifIntT stmtSeq_removeItem(ASDLStmtSeq* _stmts, AlifSizeT _idx) { // 676
	if (_idx >= ASDL_SEQ_LEN(_stmts)) {
		return 0;
	}
	for (AlifSizeT i = _idx; i < ASDL_SEQ_LEN(_stmts) - 1; i++) {
		StmtTy st = (StmtTy)ASDL_SEQ_GET(_stmts, i + 1);
		ASDL_SEQ_SET(_stmts, i, st);
	}
	_stmts->size--;
	return 1;
}


static AlifIntT astFold_body(ASDLStmtSeq* _stmts,
	AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 690
	AlifIntT docstring = alifAST_getDocString(_stmts) != nullptr;
	if (docstring and (_state->optimize >= 2)) {
		/* remove the docstring */
		if (!stmtSeq_removeItem(_stmts, 0)) {
			return 0;
		}
		docstring = 0;
	}
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


static AlifIntT astfold_mod(ModuleTy _node,
	AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 699
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


static AlifIntT astFold_expr(ExprTy _node,
	AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 721
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
	case ExprK_::ListCompK:
		CALL(astFold_expr, ExprTy, _node->V.listComp.elt);
		CALL_SEQ(astFold_comprehension, Comprehension, _node->V.listComp.generators);
		break;
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
	case ExprK_::SubScriptK:
		CALL(astFold_expr, ExprTy, _node->V.subScript.val);
		CALL(astFold_expr, ExprTy, _node->V.subScript.slice);
		CALL(fold_subScr, ExprTy, _node);
		break;
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
	LEAVE_RECURSIVE(_state);
	return 1;
}

static AlifIntT astFold_keyword(KeywordTy _node,
	AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 840
	CALL(astFold_expr, ExprTy, _node->val);
	return 1;
}

static AlifIntT astFold_comprehension(ComprehensionTy _node,
	AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 847
	CALL(astFold_expr, ExprTy, _node->target);
	CALL(astFold_expr, ExprTy, _node->iter);
	CALL_SEQ(astFold_expr, Expr, _node->ifs);

	CALL(fold_iter, ExprTy, _node->iter);
	return 1;
}

static AlifIntT astFold_arguments(ArgumentsTy _node,
	AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 858
	CALL_SEQ(astFold_arg, Arg, _node->posOnlyArgs);
	CALL_SEQ(astFold_arg, Arg, _node->args);
	CALL_OPT(astFold_arg, ArgTy, _node->varArg);
	CALL_SEQ(astFold_arg, Arg, _node->kwOnlyArgs);
	CALL_SEQ(astFold_expr, Expr, _node->kwDefaults);
	CALL_OPT(astFold_arg, ArgTy, _node->kwArg);
	CALL_SEQ(astFold_expr, Expr, _node->defaults);
	return 1;
}

static AlifIntT astFold_arg(ArgTy _node, AlifASTMem* _ctx,
	AlifASTOptimizeState* _state) { // 871
	if (!(_state->features & CO_FUTURE_ANNOTATIONS)) {
		//CALL_OPT(astFold_expr, ExprTy, _node->annotation);
	}
	return 1;
}

static AlifIntT astFold_stmt(StmtTy _node,
	AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 880
	ENTER_RECURSIVE(_state);
	switch (_node->type) {
	case StmtK_::FunctionDefK:
		CALL_SEQ(astFold_typeParam, TypeParam, _node->V.functionDef.typeParams);
		CALL(astFold_arguments, ArgumentsTy, _node->V.functionDef.args);
		CALL(astFold_body, ASDLSeq, _node->V.functionDef.body);
		//CALL_SEQ(astFold_expr, Expr, _node->V.functionDef.decoratorList);
		if (!(_state->features & CO_FUTURE_ANNOTATIONS)) {
			CALL_OPT(astFold_expr, ExprTy, _node->V.functionDef.returns);
		}
		break;
	//case StmtK_::AsyncFunctionDefK:
	//	CALL_SEQ(astfold_typeParam, TypeParam, _node->V.asyncFunctionDef.typeParams);
	//	CALL(astFold_arguments, ArgumentsTy, _node->V.asyncFunctionDef.args);
	//	CALL(astFold_body, ASDLSeq, _node->V.asyncFunctionDef.body);
	//	CALL_SEQ(astFold_expr, Expr, _node->V.asyncFunctionDef.decoratorList);
	//	if (!(_state->features & CO_FUTURE_ANNOTATIONS)) {
	//		CALL_OPT(astFold_expr, ExprTy, _node->V.asyncFunctionDef.returns);
	//	}
	//	break;
	case StmtK_::ClassDefK:
		CALL_SEQ(astFold_typeParam, TypeParam, _node->V.classDef.typeParams);
		CALL_SEQ(astFold_expr, Expr, _node->V.classDef.bases);
		CALL_SEQ(astFold_keyword, Keyword, _node->V.classDef.keywords);
		CALL(astFold_body, asdl_seq, _node->V.classDef.body);
		//CALL_SEQ(astFold_expr, Expr, _node->V.classDef.decoratorList);
		break;
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




static AlifIntT astFold_typeParam(TypeParamTy _node,
	AlifASTMem* _ctx, AlifASTOptimizeState* _state) { // 1084
	switch (_node->type) {
	case TypeParamK::TypeVarK:
		CALL_OPT(astFold_expr, ExprTy, _node->V.typeVar.bound);
		//CALL_OPT(astFold_expr, ExprTy, _node->V.typeVar.defaultValue);
		break;
	case TypeParamK::ParamSpecK:
		//CALL_OPT(astFold_expr, ExprTy, _node->V.paramSpec.defaultValue);
		break;
	case TypeParamK::TypeVarTupleK:
		//CALL_OPT(astFold_expr, ExprTy, _node->V.typeVarTuple.defaultValue);
		break;
	}
	return 1;
}

#undef CALL
#undef CALL_OPT
#undef CALL_SEQ




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
