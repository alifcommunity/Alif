#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_ASTState.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_State.h"
#include "AlifCore_Memory.h"

class Validator { // 16
public:
	AlifIntT recursionDepth;            /* current recursion depth */
	AlifIntT recursionLimit;            /* recursion limit */
};

// Forward declaration
static AlifIntT init_types(void*);

static class ASTState* get_astState(void) { // 24
	AlifInterpreter* interp = _alifInterpreter_get();
	class ASTState* state = &interp->ast;
	if (_alifOnceFlag_callOnce(&state->once, (AlifOnceFnT*)&init_types, state) < 0) {
		return nullptr;
	}
	return state;
}


static AlifIntT init_identifiers(ASTState* _state) { // 286
	if ((_state->__dict__ = alifUStr_internFromString("__dict__")) == nullptr) return -1;
	if ((_state->__doc__ = alifUStr_internFromString("__doc__")) == nullptr) return -1;
	if ((_state->__match_args__ = alifUStr_internFromString("__match_args__")) == nullptr) return -1;
	if ((_state->__module__ = alifUStr_internFromString("__module__")) == nullptr) return -1;
	if ((_state->_attributes = alifUStr_internFromString("_attributes")) == nullptr) return -1;
	if ((_state->_fields = alifUStr_internFromString("_fields")) == nullptr) return -1;
	if ((_state->annotation = alifUStr_internFromString("annotation")) == nullptr) return -1;
	if ((_state->arg = alifUStr_internFromString("arg")) == nullptr) return -1;
	if ((_state->args = alifUStr_internFromString("args")) == nullptr) return -1;
	if ((_state->argtypes = alifUStr_internFromString("argtypes")) == nullptr) return -1;
	if ((_state->asname = alifUStr_internFromString("asname")) == nullptr) return -1;
	if ((_state->ast = alifUStr_internFromString("ast")) == nullptr) return -1;
	if ((_state->attr = alifUStr_internFromString("attr")) == nullptr) return -1;
	if ((_state->bases = alifUStr_internFromString("bases")) == nullptr) return -1;
	if ((_state->body = alifUStr_internFromString("body")) == nullptr) return -1;
	if ((_state->bound = alifUStr_internFromString("bound")) == nullptr) return -1;
	if ((_state->cases = alifUStr_internFromString("cases")) == nullptr) return -1;
	if ((_state->cause = alifUStr_internFromString("cause")) == nullptr) return -1;
	if ((_state->cls = alifUStr_internFromString("cls")) == nullptr) return -1;
	if ((_state->colOffset = alifUStr_internFromString("col_offset")) == nullptr) return -1;
	if ((_state->comparators = alifUStr_internFromString("comparators")) == nullptr) return -1;
	if ((_state->context_expr = alifUStr_internFromString("context_expr")) == nullptr) return -1;
	if ((_state->conversion = alifUStr_internFromString("conversion")) == nullptr) return -1;
	if ((_state->ctx = alifUStr_internFromString("ctx")) == nullptr) return -1;
	if ((_state->decorator_list = alifUStr_internFromString("decorator_list")) == nullptr) return -1;
	if ((_state->default_value = alifUStr_internFromString("default_value")) == nullptr) return -1;
	if ((_state->defaults = alifUStr_internFromString("defaults")) == nullptr) return -1;
	if ((_state->elt = alifUStr_internFromString("elt")) == nullptr) return -1;
	if ((_state->elts = alifUStr_internFromString("elts")) == nullptr) return -1;
	if ((_state->endColOffset = alifUStr_internFromString("end_col_offset")) == nullptr) return -1;
	if ((_state->endLineNo = alifUStr_internFromString("end_lineno")) == nullptr) return -1;
	if ((_state->exc = alifUStr_internFromString("exc")) == nullptr) return -1;
	if ((_state->finalbody = alifUStr_internFromString("finalbody")) == nullptr) return -1;
	if ((_state->format_spec = alifUStr_internFromString("format_spec")) == nullptr) return -1;
	if ((_state->func = alifUStr_internFromString("func")) == nullptr) return -1;
	if ((_state->generators = alifUStr_internFromString("generators")) == nullptr) return -1;
	if ((_state->guard = alifUStr_internFromString("guard")) == nullptr) return -1;
	if ((_state->handlers = alifUStr_internFromString("handlers")) == nullptr) return -1;
	if ((_state->id = alifUStr_internFromString("id")) == nullptr) return -1;
	if ((_state->ifs = alifUStr_internFromString("ifs")) == nullptr) return -1;
	if ((_state->isAsync = alifUStr_internFromString("isAsync")) == nullptr) return -1;
	if ((_state->items = alifUStr_internFromString("items")) == nullptr) return -1;
	if ((_state->iter = alifUStr_internFromString("iter")) == nullptr) return -1;
	if ((_state->key = alifUStr_internFromString("key")) == nullptr) return -1;
	if ((_state->keys = alifUStr_internFromString("keys")) == nullptr) return -1;
	if ((_state->keywords = alifUStr_internFromString("keywords")) == nullptr) return -1;
	if ((_state->kind = alifUStr_internFromString("kind")) == nullptr) return -1;
	if ((_state->kwDefaults = alifUStr_internFromString("kwDefaults")) == nullptr) return -1;
	if ((_state->kwarg = alifUStr_internFromString("kwarg")) == nullptr) return -1;
	if ((_state->kwd_attrs = alifUStr_internFromString("kwd_attrs")) == nullptr) return -1;
	if ((_state->kwd_patterns = alifUStr_internFromString("kwd_patterns")) == nullptr) return -1;
	if ((_state->kwonlyargs = alifUStr_internFromString("kwonlyargs")) == nullptr) return -1;
	if ((_state->left = alifUStr_internFromString("left")) == nullptr) return -1;
	if ((_state->level = alifUStr_internFromString("level")) == nullptr) return -1;
	if ((_state->lineno = alifUStr_internFromString("lineno")) == nullptr) return -1;
	if ((_state->lower = alifUStr_internFromString("lower")) == nullptr) return -1;
	if ((_state->module = alifUStr_internFromString("module")) == nullptr) return -1;
	if ((_state->msg = alifUStr_internFromString("msg")) == nullptr) return -1;
	if ((_state->name = alifUStr_internFromString("name")) == nullptr) return -1;
	if ((_state->names = alifUStr_internFromString("names")) == nullptr) return -1;
	if ((_state->op = alifUStr_internFromString("op")) == nullptr) return -1;
	if ((_state->operand = alifUStr_internFromString("operand")) == nullptr) return -1;
	if ((_state->ops = alifUStr_internFromString("ops")) == nullptr) return -1;
	if ((_state->optional_vars = alifUStr_internFromString("optional_vars")) == nullptr) return -1;
	if ((_state->else_ = alifUStr_internFromString("orelse")) == nullptr) return -1;
	if ((_state->pattern = alifUStr_internFromString("pattern")) == nullptr) return -1;
	if ((_state->patterns = alifUStr_internFromString("patterns")) == nullptr) return -1;
	if ((_state->posonlyargs = alifUStr_internFromString("posonlyargs")) == nullptr) return -1;
	if ((_state->rest = alifUStr_internFromString("rest")) == nullptr) return -1;
	if ((_state->returns = alifUStr_internFromString("returns")) == nullptr) return -1;
	if ((_state->right = alifUStr_internFromString("right")) == nullptr) return -1;
	if ((_state->simple = alifUStr_internFromString("simple")) == nullptr) return -1;
	if ((_state->slice = alifUStr_internFromString("slice")) == nullptr) return -1;
	if ((_state->step = alifUStr_internFromString("step")) == nullptr) return -1;
	if ((_state->subject = alifUStr_internFromString("subject")) == nullptr) return -1;
	if ((_state->tag = alifUStr_internFromString("tag")) == nullptr) return -1;
	if ((_state->target = alifUStr_internFromString("target")) == nullptr) return -1;
	if ((_state->targets = alifUStr_internFromString("targets")) == nullptr) return -1;
	if ((_state->test = alifUStr_internFromString("test")) == nullptr) return -1;
	if ((_state->type = alifUStr_internFromString("type")) == nullptr) return -1;
	if ((_state->typeComment = alifUStr_internFromString("typeComment")) == nullptr) return -1;
	if ((_state->type_ignores = alifUStr_internFromString("type_ignores")) == nullptr) return -1;
	if ((_state->typeParams = alifUStr_internFromString("type_params")) == nullptr) return -1;
	if ((_state->upper = alifUStr_internFromString("upper")) == nullptr) return -1;
	if ((_state->value = alifUStr_internFromString("value")) == nullptr) return -1;
	if ((_state->values = alifUStr_internFromString("values")) == nullptr) return -1;
	if ((_state->vararg = alifUStr_internFromString("vararg")) == nullptr) return -1;
	return 0;
};


// 382
GENERATE_SEQ_CONSTRUCTOR(Expr, expr, ExprTy); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة
GENERATE_SEQ_CONSTRUCTOR(Arg, arg, ArgTy); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة
GENERATE_SEQ_CONSTRUCTOR(Keyword, keyword, KeywordTy); // هذه الماكرو تقوم بإنشاء جسم دالة عن طريق تمرير المعاملات ك نصوص لإستخدامها ضمن مولد الدالة


static const char* const _moduleFields_[] = { // 394
	"body",
	"type_ignores",
};
static const char* const _interactiveFields_[] = {
	"body",
};
static const char* const _expressionFields_[] = {
	"body",
};
static const char* const _functionTypeFields_[] = {
	"argtypes",
	"returns",
};
static const char* const _stmtAttributes_[] = {
	"lineno",
	"col_offset",
	"end_lineno",
	"end_col_offset",
};
static AlifObject* ast2obj_stmt(ASTState*, Validator*, void*);
static const char* const _functionDefFields_[] = {
	"name",
	"args",
	"body",
	"decorator_list",
	"returns",
	"typeComment",
	"type_params",
};
static const char* const _asyncFunctionDefFields_[] = {
	"name",
	"args",
	"body",
	"decorator_list",
	"returns",
	"typeComment",
	"type_params",
};
static const char* const _classDefFields_[] = {
	"name",
	"bases",
	"keywords",
	"body",
	"decorator_list",
	"type_params",
};
static const char* const _returnFields_[] = {
	"value",
};
static const char* const _deleteFields_[] = {
	"targets",
};
static const char* const _assignFields_[] = {
	"targets",
	"value",
	"typeComment",
};
static const char* const _typeAliasFields_[] = {
	"name",
	"type_params",
	"value",
};
static const char* const _augAssignFields_[] = {
	"target",
	"op",
	"value",
};
static const char* const _annAssignFields_[] = {
	"target",
	"annotation",
	"value",
	"simple",
};
static const char* const _forFields_[] = {
	"target",
	"iter",
	"body",
	"orelse",
	"typeComment",
};
static const char* const _asyncForFields_[] = {
	"target",
	"iter",
	"body",
	"orelse",
	"typeComment",
};
static const char* const _whileFields_[] = {
	"test",
	"body",
	"orelse",
};
static const char* const _ifFields_[] = {
	"test",
	"body",
	"orelse",
};
static const char* const _withFields_[] = {
	"items",
	"body",
	"typeComment",
};
static const char* const _asyncWithFields_[] = {
	"items",
	"body",
	"typeComment",
};
static const char* const _matchFields_[] = {
	"subject",
	"cases",
};
static const char* const _raiseFields_[] = {
	"exc",
	"cause",
};
static const char* const _tryFields_[] = {
	"body",
	"handlers",
	"orelse",
	"finalbody",
};
static const char* const _tryStarFields_[] = {
	"body",
	"handlers",
	"orelse",
	"finalbody",
};
static const char* const _assertFields_[] = {
	"test",
	"msg",
};
static const char* const _importFields_[] = {
	"names",
};
static const char* const _importFromFields_[] = {
	"module",
	"names",
	"level",
};
static const char* const _globalFields_[] = {
	"names",
};
static const char* const _nonlocalFields_[] = {
	"names",
};
static const char* const _exprFields_[] = {
	"value",
};
static const char* const _exprAttributes_[] = {
	"lineno",
	"col_offset",
	"end_lineno",
	"end_col_offset",
};
static AlifObject* ast2obj_expr(ASTState*, Validator*, void*); // 553
static const char* const _boolOpFields_[] = {
	"op",
	"values",
};
static const char* const _namedExprFields_[] = {
	"target",
	"value",
};
static const char* const _binOpFields_[] = {
	"left",
	"op",
	"right",
};
static const char* const _unaryOpFields_[] = {
	"op",
	"operand",
};
static const char* const _lambdaFields_[] = {
	"args",
	"body",
};
static const char* const _ifExpFields_[] = {
	"test",
	"body",
	"orelse",
};
static const char* const _dictFields_[] = {
	"keys",
	"values",
};
static const char* const _setFields_[] = {
	"elts",
};
static const char* const _listCompFields_[] = {
	"elt",
	"generators",
};
static const char* const _setCompFields_[] = {
	"elt",
	"generators",
};
static const char* const _dictCompFields_[] = {
	"key",
	"value",
	"generators",
};
static const char* const _generatorExpFields_[] = {
	"elt",
	"generators",
};
static const char* const _awaitFields_[] = {
	"value",
};
static const char* const _yieldFields_[] = {
	"value",
};
static const char* const _yieldFromFields_[] = {
	"value",
};
static const char* const _compareFields_[] = {
	"left",
	"ops",
	"comparators",
};
static const char* const _callFields_[] = {
	"func",
	"args",
	"keywords",
};
static const char* const _formattedValueFields_[] = {
	"value",
	"conversion",
	"format_spec",
};
static const char* const _joinedStrFields_[] = {
	"values",
};
static const char* const _constantFields_[] = {
	"value",
	"kind",
};
static const char* const _attributeFields_[] = {
	"value",
	"attr",
	"ctx",
};
static const char* const _subScriptFields_[] = {
	"value",
	"slice",
	"ctx",
};
static const char* const _starredFields_[] = {
	"value",
	"ctx",
};
static const char* const _nameFields_[] = {
	"id",
	"ctx",
};
static const char* const _listFields_[] = {
	"elts",
	"ctx",
};
static const char* const _tupleFields_[] = {
	"elts",
	"ctx",
};
static const char* const _sliceFields_[] = { // 662
	"lower",
	"upper",
	"step",
};
static AlifObject* ast2obj_exprContext(ASTState*, Validator*, ExprContext_); // 667
static AlifObject* ast2obj_boolOp(ASTState*, Validator*, BoolOp_);
static AlifObject* ast2obj_operator(ASTState*, Validator*, Operator_);
static AlifObject* ast2obj_unaryOp(ASTState*, Validator*, UnaryOp_); // 673
static AlifObject* ast2obj_cmpOp(ASTState*, Validator*, CmpOp_); // 675
static AlifObject* ast2obj_comprehension(ASTState*, Validator*, void*);; // 677
static const char* const _comprehensionFields_[] = {
	"target",
	"iter",
	"ifs",
	"isAsync",
};
static const char* const _excepthandlerAttributes_[] = {
	"lineno",
	"col_offset",
	"end_lineno",
	"end_col_offset",
};
static AlifObject* ast2obj_exceptHandler(ASTState*, Validator*, void*); // 691
static const char* const _exceptHandlerFields_[] = {
	"type",
	"name",
	"body",
};
static AlifObject* ast2obj_arguments(ASTState*, Validator*, void*); // 698
static const char* const _argumentsFields_[] = {
	"posonlyargs",
	"args",
	"vararg",
	"kwonlyargs",
	"kwDefaults",
	"kwarg",
	"defaults",
};
static AlifObject* ast2obj_arg(ASTState*, Validator*, void*); // 709
static const char* const _argAttributes_[] = {
	"lineno",
	"col_offset",
	"end_lineno",
	"end_col_offset",
};
static const char* const _argFields_[] = {
	"arg",
	"annotation",
	"typeComment",
};
static AlifObject* ast2obj_keyword(ASTState*, Validator*, void*); // 722
static const char* const _keywordAttributes_[] = {
	"lineno",
	"col_offset",
	"end_lineno",
	"end_col_offset",
};
static const char* const _keywordFields_[] = {
	"arg",
	"value",
};
static AlifObject* ast2obj_alias(ASTState*, Validator*, void*); // 734
static const char* const _aliasAttributes_[] = {
	"lineno",
	"col_offset",
	"end_lineno",
	"end_col_offset",
};
static const char* const _aliasFields_[] = {
	"name",
	"asname",
};
static AlifObject* ast2obj_withItem(ASTState*, Validator*, void*); // 746
static const char* const _withItemFields_[] = {
	"context_expr",
	"optional_vars",
};
//static AlifObject* ast2obj_matchCase(ASTState*, Validator*, void*);
static const char* const _matchCaseFields_[] = {
	"pattern",
	"guard",
	"body",
};
static const char* const _patternAttributes_[] = {
	"lineno",
	"col_offset",
	"end_lineno",
	"end_col_offset",
};
//static AlifObject* ast2obj_pattern(ASTState*, Validator*, void*);
static const char* const _MatchValueFields_[] = {
	"value",
};
static const char* const _matchSingletonFields_[] = {
	"value",
};
static const char* const _MatchSequenceFields_[] = {
	"patterns",
};
static const char* const _matchMappingFields_[] = {
	"keys",
	"patterns",
	"rest",
};
static const char* const _matchClassFields_[] = {
	"cls",
	"patterns",
	"kwd_attrs",
	"kwd_patterns",
};
static const char* const _matchStarFields_[] = {
	"name",
};
static const char* const _matchAsFields_[] = {
	"pattern",
	"name",
};
static const char* const _matchOrFields_[] = { // 794
	"patterns",
};
//static AlifObject* ast2obj_typeIgnore(ASTState*, Validator*, void*);
static const char* const _typeIgnoreFields_[] = {
	"lineno",
	"tag",
};
static const char* const _typeParamAttributes_[] = {
	"lineno",
	"col_offset",
	"end_lineno",
	"end_col_offset",
};
static AlifObject* ast2obj_typeParam(ASTState*, Validator*, void*); // 809
static const char* const _typeVarFields_[] = {
	"name",
	"bound",
	"default_value",
};
static const char* const _paramSpecFields_[] = {
	"name",
	"default_value",
};
static const char* const _typeVarTupleFields_[] = { // 820
	"name",
	"default_value",
};
















class ASTObject { // 5035
public:
	ALIFOBJECT_HEAD;
	AlifObject* dict;
};



static AlifIntT ast_typeInit(AlifObject* self, AlifObject* args, AlifObject* kw) { // 5075
	ASTState* state = get_astState();
	if (state == nullptr) {
		return -1;
	}

	AlifSizeT i{}, numfields = 0;
	AlifIntT res = -1;
	AlifObject* key{}, * value{}, * fields{}, * attributes = nullptr, * remaining_fields = nullptr;
	if (alifObject_getOptionalAttr((AlifObject*)ALIF_TYPE(self), state->_fields, &fields) < 0) {
		goto cleanup;
	}
	if (fields) {
		numfields = alifSequence_size(fields);
		if (numfields == -1) {
			goto cleanup;
		}
		remaining_fields = alifSet_new(fields);
	}
	else {
		remaining_fields = alifSet_new(nullptr);
	}
	if (remaining_fields == nullptr) {
		goto cleanup;
	}

	res = 0; /* if no error occurs, this stays 0 to the end */
	if (numfields < ALIFTUPLE_GET_SIZE(args)) {
		alifErr_format(_alifExcTypeError_, "%.400s constructor takes at most "
			"%zd positional argument%s",
			_alifType_name(ALIF_TYPE(self)),
			numfields, numfields == 1 ? "" : "s");
		res = -1;
		goto cleanup;
	}
	for (i = 0; i < ALIFTUPLE_GET_SIZE(args); i++) {
		/* cannot be reached when fields is nullptr */
		AlifObject* name = alifSequence_getItem(fields, i);
		if (!name) {
			res = -1;
			goto cleanup;
		}
		res = alifObject_setAttr(self, name, ALIFTUPLE_GET_ITEM(args, i));
		if (alifSet_discard(remaining_fields, name) < 0) {
			res = -1;
			ALIF_DECREF(name);
			goto cleanup;
		}
		ALIF_DECREF(name);
		if (res < 0) {
			goto cleanup;
		}
	}
	if (kw) {
		i = 0;  /* needed by alifDict_next */
		while (alifDict_next(kw, &i, &key, &value)) {
			AlifIntT contains = alifSequence_contains(fields, key);
			if (contains == -1) {
				res = -1;
				goto cleanup;
			}
			else if (contains == 1) {
				AlifIntT p = alifSet_discard(remaining_fields, key);
				if (p == -1) {
					res = -1;
					goto cleanup;
				}
				if (p == 0) {
					alifErr_format(_alifExcTypeError_,
						"%.400s got multiple values for argument '%U'",
						ALIF_TYPE(self)->name, key);
					res = -1;
					goto cleanup;
				}
			}
			else {
				// Lazily initialize "attributes"
				if (attributes == nullptr) {
					attributes = alifObject_getAttr((AlifObject*)ALIF_TYPE(self), state->_attributes);
					if (attributes == nullptr) {
						res = -1;
						goto cleanup;
					}
				}
				AlifIntT contains = alifSequence_contains(attributes, key);
				if (contains == -1) {
					res = -1;
					goto cleanup;
				}
				else if (contains == 0) {
					//if (alifErr_warnFormat(
					//	_alifExcDeprecationWarning_, 1,
					//	"%.400s.__init__ got an unexpected keyword argument '%U'. "
					//	"Support for arbitrary keyword arguments is deprecated "
					//	"and will be removed in Alif 5.0.",
					//	ALIF_TYPE(self)->name, key
					//) < 0) {
					//	res = -1;
					//	goto cleanup;
					//}
				}
			}
			res = alifObject_setAttr(self, key, value);
			if (res < 0) {
				goto cleanup;
			}
		}
	}
	AlifSizeT size; size = alifSet_size(remaining_fields);
	AlifObject* field_types; field_types = nullptr;
	AlifObject* remaining_list; remaining_list = nullptr;
	if (size > 0) {
		if (alifObject_getOptionalAttr((AlifObject*)ALIF_TYPE(self), &ALIF_ID(_fieldTypes),
			&field_types) < 0) {
			res = -1;
			goto cleanup;
		}
		if (field_types == nullptr) {
			goto cleanup;
		}
		remaining_list = alifSequence_list(remaining_fields);
		if (!remaining_list) {
			goto set_remaining_cleanup;
		}
		for (AlifSizeT i = 0; i < size; i++) {
			AlifObject* name = ALIFLIST_GET_ITEM(remaining_list, i);
			AlifObject* type = alifDict_getItemWithError(field_types, name);
			if (!type) {
				if (alifErr_occurred()) {
					goto set_remaining_cleanup;
				}
				else {
					//if (alifErr_warnFormat(
					//	_alifExcDeprecationWarning_, 1,
					//	"Field '%U' is missing from %.400s._field_types. "
					//	"This will become an error in Alif 5.0.",
					//	name, ALIF_TYPE(self)->name
					//) < 0) {
					//	goto set_remaining_cleanup;
					//}
				}
			}
			//else if (ALIFUNION_CHECK(type)) {
			//	// optional field
			//	// do nothing, we'll have set a None default on the class
			//}
			else if (ALIF_IS_TYPE(type, &_alifGenericAliasType_)) {
				// list field
				AlifObject* empty = alifList_new(0);
				if (!empty) {
					goto set_remaining_cleanup;
				}
				res = alifObject_setAttr(self, name, empty);
				ALIF_DECREF(empty);
				if (res < 0) {
					goto set_remaining_cleanup;
				}
			}
			else if (type == state->expr_context_type) {
				// special case for expr_context: default to Load()
				res = alifObject_setAttr(self, name, state->LoadSingleton);
				if (res < 0) {
					goto set_remaining_cleanup;
				}
			}
			else {
				// simple field (e.g., identifier)
				//if (alifErr_warnFormat(
				//	_alifExcDeprecationWarning_, 1,
				//	"%.400s.__init__ missing 1 required positional argument: '%U'. "
				//	"This will become an error in Alif 5.0.",
				//	ALIF_TYPE(self)->name, name
				//) < 0) {
				//	goto set_remaining_cleanup;
				//}
			}
		}
		ALIF_DECREF(remaining_list);
		ALIF_DECREF(field_types);
	}
cleanup:
	ALIF_XDECREF(attributes);
	ALIF_XDECREF(fields);
	ALIF_XDECREF(remaining_fields);
	return res;
set_remaining_cleanup:
	ALIF_XDECREF(remaining_list);
	ALIF_XDECREF(field_types);
	res = -1;
	goto cleanup;
}



static AlifTypeSlot _astTypeSlots_[] = { // 5846
	//{ALIF_TP_DEALLOC, ast_dealloc},
	//{ALIF_TP_REPR, ast_repr},
	{ALIF_TP_GETATTRO, (void*)alifObject_genericGetAttr},
	//{ALIF_TP_SETATTRO, alifObject_genericSetAttr},
	//{ALIF_TP_TRAVERSE, ast_traverse},
	//{ALIF_TP_CLEAR, ast_clear},
	//{ALIF_TP_MEMBERS, _astTypeMembers_},
	//{ALIF_TP_METHODS, _astTypeMethods_},
	//{ALIF_TP_GETSET, _astTypeGetSets_},
	{ALIF_TP_INIT, (void*)ast_typeInit},
	//{ALIF_TP_ALLOC, alifType_genericAlloc},
	{ALIF_TP_NEW, (void*)alifType_genericNew},
	//{ALIF_TP_FREE, alifObject_gcDel},
	{0, 0},
};

static AlifTypeSpec _astTypeSpec_ = { // 5863
	"ast.AST",
	sizeof(ASTObject),
	0,
	ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC,
	_astTypeSlots_
};

static AlifObject* make_type(ASTState* _state, const char* _type, AlifObject* _base,
	const char* const* _fields, AlifIntT _numFields, const char* _doc) { // 5885
	AlifObject* fnames{}, * result{};
	AlifIntT i{};
	fnames = alifTuple_new(_numFields);
	if (!fnames) return nullptr;
	for (i = 0; i < _numFields; i++) {
		AlifObject* field = alifUStr_internFromString(_fields[i]);
		if (!field) {
			ALIF_DECREF(fnames);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(fnames, i, field);
	}
	result = alifObject_callFunction((AlifObject*)&_alifTypeType_, "s(O){OOOOOOOs}",
		_type, _base,
		_state->_fields, fnames,
		_state->__match_args__, fnames,
		_state->__module__,
		_state->ast,
		_state->__doc__, _doc);
	ALIF_DECREF(fnames);
	return result;
}

static AlifIntT add_attributes(ASTState* _state, AlifObject* _type,
	const char* const* _attrs, AlifIntT _numFields) { // 5912
	AlifIntT i{}, result{};
	AlifObject* s, * l = alifTuple_new(_numFields);
	if (!l)
		return -1;
	for (i = 0; i < _numFields; i++) {
		s = alifUStr_internFromString(_attrs[i]);
		if (!s) {
			ALIF_DECREF(l);
			return -1;
		}
		ALIFTUPLE_SET_ITEM(l, i, s);
	}
	result = alifObject_setAttr(_type, _state->_attributes, l);
	ALIF_DECREF(l);
	return result;
}

static AlifObject* ast2obj_list(ASTState* state, Validator* vstate, ASDLSeq* seq,
	AlifObject* (*func)(ASTState* state, Validator* vstate, void*)) { // 5934
	AlifSizeT i{}, n = ASDL_SEQ_LEN(seq);
	AlifObject* result = alifList_new(n);
	AlifObject* value;
	if (!result)
		return nullptr;
	for (i = 0; i < n; i++) {
		value = func(state, vstate, ASDL_SEQ_GETUNTYPED(seq, i));
		if (!value) {
			ALIF_DECREF(result);
			return nullptr;
		}
		ALIFLIST_SET_ITEM(result, i, value);
	}
	return result;
}

static AlifObject* ast2obj_object(ASTState* ALIF_UNUSED(state),
	Validator* ALIF_UNUSED(vstate), void* _o) { // 5953
	AlifObject* op = (AlifObject*)_o;
	if (!op) {
		op = ALIF_NONE;
	}
	return ALIF_NEWREF(op);
}
#define AST2OBJ_CONSTANT ast2obj_object
#define AST2OBJ_IDENTIFIER ast2obj_object
#define AST2OBJ_STRING ast2obj_object // 5963

static AlifObject* ast2obj_int(ASTState* ALIF_UNUSED(state),
	Validator* ALIF_UNUSED(vstate), long b) { // 5965
	return alifLong_fromLong(b);
}

static AlifIntT add_astFields(ASTState* state) { // 6008
	AlifObject* emptyTuple{};
	emptyTuple = alifTuple_new(0);
	if (!emptyTuple or
		alifObject_setAttrString(state->astType, "_fields", emptyTuple) < 0 or
		alifObject_setAttrString(state->astType, "__match_args__", emptyTuple) < 0 or
		alifObject_setAttrString(state->astType, "_attributes", emptyTuple) < 0) {
		ALIF_XDECREF(emptyTuple);
		return -1;
	}
	ALIF_DECREF(emptyTuple);
	return 0;
}

static AlifIntT init_types(void* arg) { // 6049
	ASTState* state = (ASTState*)arg;
	if (init_identifiers(state) < 0) {
		return -1;
	}
	state->astType = alifType_fromSpec(&_astTypeSpec_);
	if (!state->astType) {
		return -1;
	}
	if (add_astFields(state) < 0) {
		return -1;
	}
	state->mod_type = make_type(state, "mod", state->astType, nullptr, 0,
		"mod = Module(stmt* body, type_ignore* type_ignores)\n"
		"    | Interactive(stmt* body)\n"
		"    | Expression(expr body)\n"
		"    | FunctionType(expr* argtypes, expr returns)");
	if (!state->mod_type) return -1;
	if (add_attributes(state, state->mod_type, nullptr, 0) < 0) return -1;
	state->Module_type = make_type(state, "Module", state->mod_type,
		_moduleFields_, 2,
		"Module(stmt* body, type_ignore* type_ignores)");
	if (!state->Module_type) return -1;
	state->Interactive_type = make_type(state, "Interactive", state->mod_type,
		_interactiveFields_, 1,
		"Interactive(stmt* body)");
	if (!state->Interactive_type) return -1;
	state->Expression_type = make_type(state, "Expression", state->mod_type,
		_expressionFields_, 1,
		"Expression(expr body)");
	if (!state->Expression_type) return -1;
	state->FunctionType_type = make_type(state, "FunctionType",
		state->mod_type, _functionTypeFields_,
		2,
		"FunctionType(expr* argtypes, expr returns)");
	if (!state->FunctionType_type) return -1;
	state->stmt_type = make_type(state, "stmt", state->astType, nullptr, 0,
		"stmt = FunctionDef(identifier name, arguments args, stmt* body, expr* decorator_list, expr? returns, string? typeComment, type_param* type_params)\n"
		"     | AsyncFunctionDef(identifier name, arguments args, stmt* body, expr* decorator_list, expr? returns, string? typeComment, type_param* type_params)\n"
		"     | ClassDef(identifier name, expr* bases, keyword* keywords, stmt* body, expr* decorator_list, type_param* type_params)\n"
		"     | Return(expr? value)\n"
		"     | Delete(expr* targets)\n"
		"     | Assign(expr* targets, expr value, string? typeComment)\n"
		"     | TypeAlias(expr name, type_param* type_params, expr value)\n"
		"     | AugAssign(expr target, operator op, expr value)\n"
		"     | AnnAssign(expr target, expr annotation, expr? value, AlifIntT simple)\n"
		"     | For(expr target, expr iter, stmt* body, stmt* orelse, string? typeComment)\n"
		"     | AsyncFor(expr target, expr iter, stmt* body, stmt* orelse, string? typeComment)\n"
		"     | While(expr test, stmt* body, stmt* orelse)\n"
		"     | If(expr test, stmt* body, stmt* orelse)\n"
		"     | With(withitem* items, stmt* body, string? typeComment)\n"
		"     | AsyncWith(withitem* items, stmt* body, string? typeComment)\n"
		"     | Match(expr subject, match_case* cases)\n"
		"     | Raise(expr? exc, expr? cause)\n"
		"     | Try(stmt* body, excepthandler* handlers, stmt* orelse, stmt* finalbody)\n"
		"     | TryStar(stmt* body, excepthandler* handlers, stmt* orelse, stmt* finalbody)\n"
		"     | Assert(expr test, expr? msg)\n"
		"     | Import(alias* names)\n"
		"     | ImportFrom(identifier? module, alias* names, AlifIntT? level)\n"
		"     | Global(identifier* names)\n"
		"     | Nonlocal(identifier* names)\n"
		"     | Expr(expr value)\n"
		"     | Pass\n"
		"     | Break\n"
		"     | Continue");
	if (!state->stmt_type) return -1;
	if (add_attributes(state, state->stmt_type, _stmtAttributes_, 4) < 0) return
		-1;
	if (alifObject_setAttr(state->stmt_type, state->endLineNo, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->stmt_type, state->endColOffset, ALIF_NONE) ==
		-1)
		return -1;
	state->FunctionDefType = make_type(state, "FunctionDef", state->stmt_type,
		_functionDefFields_, 7,
		"FunctionDef(identifier name, arguments args, stmt* body, expr* decorator_list, expr? returns, string? typeComment, type_param* type_params)");
	if (!state->FunctionDefType) return -1;
	if (alifObject_setAttr(state->FunctionDefType, state->returns, ALIF_NONE) ==
		-1)
		return -1;
	if (alifObject_setAttr(state->FunctionDefType, state->typeComment, ALIF_NONE)
		== -1)
		return -1;
	state->AsyncFunctionDef_type = make_type(state, "AsyncFunctionDef",
		state->stmt_type,
		_asyncFunctionDefFields_, 7,
		"AsyncFunctionDef(identifier name, arguments args, stmt* body, expr* decorator_list, expr? returns, string? typeComment, type_param* type_params)");
	if (!state->AsyncFunctionDef_type) return -1;
	if (alifObject_setAttr(state->AsyncFunctionDef_type, state->returns, ALIF_NONE)
		== -1)
		return -1;
	if (alifObject_setAttr(state->AsyncFunctionDef_type, state->typeComment,
		ALIF_NONE) == -1)
		return -1;
	state->ClassDef_type = make_type(state, "ClassDef", state->stmt_type,
		_classDefFields_, 6,
		"ClassDef(identifier name, expr* bases, keyword* keywords, stmt* body, expr* decorator_list, type_param* type_params)");
	if (!state->ClassDef_type) return -1;
	state->ReturnType = make_type(state, "Return", state->stmt_type,
		_returnFields_, 1,
		"Return(expr? value)");
	if (!state->ReturnType) return -1;
	if (alifObject_setAttr(state->ReturnType, state->value, ALIF_NONE) == -1)
		return -1;
	state->DeleteType = make_type(state, "Delete", state->stmt_type,
		_deleteFields_, 1,
		"Delete(expr* targets)");
	if (!state->DeleteType) return -1;
	state->AssignType = make_type(state, "Assign", state->stmt_type,
		_assignFields_, 3,
		"Assign(expr* targets, expr value, string? typeComment)");
	if (!state->AssignType) return -1;
	if (alifObject_setAttr(state->AssignType, state->typeComment, ALIF_NONE) ==
		-1)
		return -1;
	state->TypeAlias_type = make_type(state, "TypeAlias", state->stmt_type,
		_typeAliasFields_, 3,
		"TypeAlias(expr name, type_param* type_params, expr value)");
	if (!state->TypeAlias_type) return -1;
	state->AugAssign_type = make_type(state, "AugAssign", state->stmt_type,
		_augAssignFields_, 3,
		"AugAssign(expr target, operator op, expr value)");
	if (!state->AugAssign_type) return -1;
	state->AnnAssign_type = make_type(state, "AnnAssign", state->stmt_type,
		_annAssignFields_, 4,
		"AnnAssign(expr target, expr annotation, expr? value, AlifIntT simple)");
	if (!state->AnnAssign_type) return -1;
	if (alifObject_setAttr(state->AnnAssign_type, state->value, ALIF_NONE) == -1)
		return -1;
	state->For_type = make_type(state, "For", state->stmt_type, _forFields_, 5,
		"For(expr target, expr iter, stmt* body, stmt* orelse, string? typeComment)");
	if (!state->For_type) return -1;
	if (alifObject_setAttr(state->For_type, state->typeComment, ALIF_NONE) == -1)
		return -1;
	state->AsyncFor_type = make_type(state, "AsyncFor", state->stmt_type,
		_asyncForFields_, 5,
		"AsyncFor(expr target, expr iter, stmt* body, stmt* orelse, string? typeComment)");
	if (!state->AsyncFor_type) return -1;
	if (alifObject_setAttr(state->AsyncFor_type, state->typeComment, ALIF_NONE) ==
		-1)
		return -1;
	state->While_type = make_type(state, "While", state->stmt_type,
		_whileFields_, 3,
		"While(expr test, stmt* body, stmt* orelse)");
	if (!state->While_type) return -1;
	state->If_type = make_type(state, "If", state->stmt_type, _ifFields_, 3,
		"If(expr test, stmt* body, stmt* orelse)");
	if (!state->If_type) return -1;
	state->With_type = make_type(state, "With", state->stmt_type, _withFields_,
		3,
		"With(withitem* items, stmt* body, string? typeComment)");
	if (!state->With_type) return -1;
	if (alifObject_setAttr(state->With_type, state->typeComment, ALIF_NONE) == -1)
		return -1;
	state->AsyncWith_type = make_type(state, "AsyncWith", state->stmt_type,
		_asyncWithFields_, 3,
		"AsyncWith(withitem* items, stmt* body, string? typeComment)");
	if (!state->AsyncWith_type) return -1;
	if (alifObject_setAttr(state->AsyncWith_type, state->typeComment, ALIF_NONE)
		== -1)
		return -1;
	state->Match_type = make_type(state, "Match", state->stmt_type,
		_matchFields_, 2,
		"Match(expr subject, match_case* cases)");
	if (!state->Match_type) return -1;
	state->Raise_type = make_type(state, "Raise", state->stmt_type,
		_raiseFields_, 2,
		"Raise(expr? exc, expr? cause)");
	if (!state->Raise_type) return -1;
	if (alifObject_setAttr(state->Raise_type, state->exc, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->Raise_type, state->cause, ALIF_NONE) == -1)
		return -1;
	state->Try_type = make_type(state, "Try", state->stmt_type, _tryFields_, 4,
		"Try(stmt* body, excepthandler* handlers, stmt* orelse, stmt* finalbody)");
	if (!state->Try_type) return -1;
	state->TryStar_type = make_type(state, "TryStar", state->stmt_type,
		_tryStarFields_, 4,
		"TryStar(stmt* body, excepthandler* handlers, stmt* orelse, stmt* finalbody)");
	if (!state->TryStar_type) return -1;
	state->Assert_type = make_type(state, "Assert", state->stmt_type,
		_assertFields_, 2,
		"Assert(expr test, expr? msg)");
	if (!state->Assert_type) return -1;
	if (alifObject_setAttr(state->Assert_type, state->msg, ALIF_NONE) == -1)
		return -1;
	state->Import_type = make_type(state, "Import", state->stmt_type,
		_importFields_, 1,
		"Import(alias* names)");
	if (!state->Import_type) return -1;
	state->ImportFrom_type = make_type(state, "ImportFrom", state->stmt_type,
		_importFromFields_, 3,
		"ImportFrom(identifier? module, alias* names, AlifIntT? level)");
	if (!state->ImportFrom_type) return -1;
	if (alifObject_setAttr(state->ImportFrom_type, state->module, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->ImportFrom_type, state->level, ALIF_NONE) == -1)
		return -1;
	state->Global_type = make_type(state, "Global", state->stmt_type,
		_globalFields_, 1,
		"Global(identifier* names)");
	if (!state->Global_type) return -1;
	state->Nonlocal_type = make_type(state, "Nonlocal", state->stmt_type,
		_nonlocalFields_, 1,
		"Nonlocal(identifier* names)");
	if (!state->Nonlocal_type) return -1;
	state->ExprType = make_type(state, "Expr", state->stmt_type, _exprFields_,
		1,
		"Expr(expr value)");
	if (!state->ExprType) return -1;
	state->Pass_type = make_type(state, "Pass", state->stmt_type, nullptr, 0,
		"Pass");
	if (!state->Pass_type) return -1;
	state->Break_type = make_type(state, "Break", state->stmt_type, nullptr, 0,
		"Break");
	if (!state->Break_type) return -1;
	state->Continue_type = make_type(state, "Continue", state->stmt_type, nullptr,
		0,
		"Continue");
	if (!state->Continue_type) return -1;
	state->expr_type = make_type(state, "expr", state->astType, nullptr, 0,
		"expr = BoolOp(boolop op, expr* values)\n"
		"     | NamedExpr(expr target, expr value)\n"
		"     | BinOp(expr left, operator op, expr right)\n"
		"     | UnaryOp(unaryop op, expr operand)\n"
		"     | Lambda(arguments args, expr body)\n"
		"     | IfExp(expr test, expr body, expr orelse)\n"
		"     | Dict(expr* keys, expr* values)\n"
		"     | Set(expr* elts)\n"
		"     | ListComp(expr elt, comprehension* generators)\n"
		"     | SetComp(expr elt, comprehension* generators)\n"
		"     | DictComp(expr key, expr value, comprehension* generators)\n"
		"     | GeneratorExp(expr elt, comprehension* generators)\n"
		"     | Await(expr value)\n"
		"     | Yield(expr? value)\n"
		"     | YieldFrom(expr value)\n"
		"     | Compare(expr left, cmpop* ops, expr* comparators)\n"
		"     | Call(expr func, expr* args, keyword* keywords)\n"
		"     | FormattedValue(expr value, AlifIntT conversion, expr? format_spec)\n"
		"     | JoinedStr(expr* values)\n"
		"     | Constant(constant value, string? kind)\n"
		"     | Attribute(expr value, identifier attr, expr_context ctx)\n"
		"     | Subscript(expr value, expr slice, expr_context ctx)\n"
		"     | Starred(expr value, expr_context ctx)\n"
		"     | Name(identifier id, expr_context ctx)\n"
		"     | List(expr* elts, expr_context ctx)\n"
		"     | Tuple(expr* elts, expr_context ctx)\n"
		"     | Slice(expr? lower, expr? upper, expr? step)");
	if (!state->expr_type) return -1;
	if (add_attributes(state, state->expr_type, _exprAttributes_, 4) < 0) return
		-1;
	if (alifObject_setAttr(state->expr_type, state->endLineNo, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->expr_type, state->endColOffset, ALIF_NONE) ==
		-1)
		return -1;
	state->BoolOp_type = make_type(state, "BoolOp", state->expr_type,
		_boolOpFields_, 2,
		"BoolOp(boolop op, expr* values)");
	if (!state->BoolOp_type) return -1;
	state->NamedExpr_type = make_type(state, "NamedExpr", state->expr_type,
		_namedExprFields_, 2,
		"NamedExpr(expr target, expr value)");
	if (!state->NamedExpr_type) return -1;
	state->BinOp_type = make_type(state, "BinOp", state->expr_type,
		_binOpFields_, 3,
		"BinOp(expr left, operator op, expr right)");
	if (!state->BinOp_type) return -1;
	state->UnaryOp_type = make_type(state, "UnaryOp", state->expr_type,
		_unaryOpFields_, 2,
		"UnaryOp(unaryop op, expr operand)");
	if (!state->UnaryOp_type) return -1;
	state->Lambda_type = make_type(state, "Lambda", state->expr_type,
		_lambdaFields_, 2,
		"Lambda(arguments args, expr body)");
	if (!state->Lambda_type) return -1;
	state->IfExp_type = make_type(state, "IfExp", state->expr_type,
		_ifExpFields_, 3,
		"IfExp(expr test, expr body, expr orelse)");
	if (!state->IfExp_type) return -1;
	state->Dict_type = make_type(state, "Dict", state->expr_type, _dictFields_,
		2,
		"Dict(expr* keys, expr* values)");
	if (!state->Dict_type) return -1;
	state->Set_type = make_type(state, "Set", state->expr_type, _setFields_, 1,
		"Set(expr* elts)");
	if (!state->Set_type) return -1;
	state->ListComp_type = make_type(state, "ListComp", state->expr_type,
		_listCompFields_, 2,
		"ListComp(expr elt, comprehension* generators)");
	if (!state->ListComp_type) return -1;
	state->SetComp_type = make_type(state, "SetComp", state->expr_type,
		_setCompFields_, 2,
		"SetComp(expr elt, comprehension* generators)");
	if (!state->SetComp_type) return -1;
	state->DictComp_type = make_type(state, "DictComp", state->expr_type,
		_dictCompFields_, 3,
		"DictComp(expr key, expr value, comprehension* generators)");
	if (!state->DictComp_type) return -1;
	state->GeneratorExp_type = make_type(state, "GeneratorExp",
		state->expr_type, _generatorExpFields_,
		2,
		"GeneratorExp(expr elt, comprehension* generators)");
	if (!state->GeneratorExp_type) return -1;
	state->Await_type = make_type(state, "Await", state->expr_type,
		_awaitFields_, 1,
		"Await(expr value)");
	if (!state->Await_type) return -1;
	state->Yield_type = make_type(state, "Yield", state->expr_type,
		_yieldFields_, 1,
		"Yield(expr? value)");
	if (!state->Yield_type) return -1;
	if (alifObject_setAttr(state->Yield_type, state->value, ALIF_NONE) == -1)
		return -1;
	state->YieldFrom_type = make_type(state, "YieldFrom", state->expr_type,
		_yieldFromFields_, 1,
		"YieldFrom(expr value)");
	if (!state->YieldFrom_type) return -1;
	state->Compare_type = make_type(state, "Compare", state->expr_type,
		_compareFields_, 3,
		"Compare(expr left, cmpop* ops, expr* comparators)");
	if (!state->Compare_type) return -1;
	state->Call_type = make_type(state, "Call", state->expr_type, _callFields_,
		3,
		"Call(expr func, expr* args, keyword* keywords)");
	if (!state->Call_type) return -1;
	state->FormattedValue_type = make_type(state, "FormattedValue",
		state->expr_type,
		_formattedValueFields_, 3,
		"FormattedValue(expr value, AlifIntT conversion, expr? format_spec)");
	if (!state->FormattedValue_type) return -1;
	if (alifObject_setAttr(state->FormattedValue_type, state->format_spec,
		ALIF_NONE) == -1)
		return -1;
	state->JoinedStr_type = make_type(state, "JoinedStr", state->expr_type,
		_joinedStrFields_, 1,
		"JoinedStr(expr* values)");
	if (!state->JoinedStr_type) return -1;
	state->Constant_type = make_type(state, "Constant", state->expr_type,
		_constantFields_, 2,
		"Constant(constant value, string? kind)");
	if (!state->Constant_type) return -1;
	if (alifObject_setAttr(state->Constant_type, state->kind, ALIF_NONE) == -1)
		return -1;
	state->Attribute_type = make_type(state, "Attribute", state->expr_type,
		_attributeFields_, 3,
		"Attribute(expr value, identifier attr, expr_context ctx)");
	if (!state->Attribute_type) return -1;
	state->Subscript_type = make_type(state, "Subscript", state->expr_type,
		_subScriptFields_, 3,
		"Subscript(expr value, expr slice, expr_context ctx)");
	if (!state->Subscript_type) return -1;
	state->Starred_type = make_type(state, "Starred", state->expr_type,
		_starredFields_, 2,
		"Starred(expr value, expr_context ctx)");
	if (!state->Starred_type) return -1;
	state->Name_type = make_type(state, "Name", state->expr_type, _nameFields_,
		2,
		"Name(identifier id, expr_context ctx)");
	if (!state->Name_type) return -1;
	state->List_type = make_type(state, "List", state->expr_type, _listFields_,
		2,
		"List(expr* elts, expr_context ctx)");
	if (!state->List_type) return -1;
	state->Tuple_type = make_type(state, "Tuple", state->expr_type,
		_tupleFields_, 2,
		"Tuple(expr* elts, expr_context ctx)");
	if (!state->Tuple_type) return -1;
	state->Slice_type = make_type(state, "Slice", state->expr_type,
		_sliceFields_, 3,
		"Slice(expr? lower, expr? upper, expr? step)");
	if (!state->Slice_type) return -1;
	if (alifObject_setAttr(state->Slice_type, state->lower, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->Slice_type, state->upper, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->Slice_type, state->step, ALIF_NONE) == -1)
		return -1;
	state->expr_context_type = make_type(state, "expr_context",
		state->astType, nullptr, 0,
		"expr_context = Load | Store | Del");
	if (!state->expr_context_type) return -1;
	if (add_attributes(state, state->expr_context_type, nullptr, 0) < 0) return -1;
	state->Load_type = make_type(state, "Load", state->expr_context_type, nullptr,
		0,
		"Load");
	if (!state->Load_type) return -1;
	state->LoadSingleton = alifType_genericNew((AlifTypeObject*)state->Load_type,
		nullptr, nullptr);
	if (!state->LoadSingleton) return -1;
	state->Store_type = make_type(state, "Store", state->expr_context_type,
		nullptr, 0,
		"Store");
	if (!state->Store_type) return -1;
	state->StoreSingleton = alifType_genericNew((AlifTypeObject
		*)state->Store_type, nullptr, nullptr);
	if (!state->StoreSingleton) return -1;
	state->Del_type = make_type(state, "Del", state->expr_context_type, nullptr, 0,
		"Del");
	if (!state->Del_type) return -1;
	state->DelSingleton = alifType_genericNew((AlifTypeObject*)state->Del_type,
		nullptr, nullptr);
	if (!state->DelSingleton) return -1;
	state->boolop_type = make_type(state, "boolop", state->astType, nullptr, 0,
		"boolop = And | Or");
	if (!state->boolop_type) return -1;
	if (add_attributes(state, state->boolop_type, nullptr, 0) < 0) return -1;
	state->And_type = make_type(state, "And", state->boolop_type, nullptr, 0,
		"And");
	if (!state->And_type) return -1;
	state->And_singleton = alifType_genericNew((AlifTypeObject*)state->And_type,
		nullptr, nullptr);
	if (!state->And_singleton) return -1;
	state->Or_type = make_type(state, "Or", state->boolop_type, nullptr, 0,
		"Or");
	if (!state->Or_type) return -1;
	state->Or_singleton = alifType_genericNew((AlifTypeObject*)state->Or_type,
		nullptr, nullptr);
	if (!state->Or_singleton) return -1;
	state->operator_type = make_type(state, "operator", state->astType, nullptr,
		0,
		"operator = Add | Sub | Mult | MatMult | Div | Mod | Pow | LShift | RShift | BitOr | BitXor | BitAnd | FloorDiv");
	if (!state->operator_type) return -1;
	if (add_attributes(state, state->operator_type, nullptr, 0) < 0) return -1;
	state->Add_type = make_type(state, "Add", state->operator_type, nullptr, 0,
		"Add");
	if (!state->Add_type) return -1;
	state->AddSingleton = alifType_genericNew((AlifTypeObject*)state->Add_type,
		nullptr, nullptr);
	if (!state->AddSingleton) return -1;
	state->Sub_type = make_type(state, "Sub", state->operator_type, nullptr, 0,
		"Sub");
	if (!state->Sub_type) return -1;
	state->SubSingleton = alifType_genericNew((AlifTypeObject*)state->Sub_type,
		nullptr, nullptr);
	if (!state->SubSingleton) return -1;
	state->Mult_type = make_type(state, "Mult", state->operator_type, nullptr, 0,
		"Mult");
	if (!state->Mult_type) return -1;
	state->MultSingleton = alifType_genericNew((AlifTypeObject*)state->Mult_type,
		nullptr, nullptr);
	if (!state->MultSingleton) return -1;
	state->MatMult_type = make_type(state, "MatMult", state->operator_type,
		nullptr, 0,
		"MatMult");
	if (!state->MatMult_type) return -1;
	state->MatMultSingleton = alifType_genericNew((AlifTypeObject
		*)state->MatMult_type, nullptr,
		nullptr);
	if (!state->MatMultSingleton) return -1;
	state->Div_type = make_type(state, "Div", state->operator_type, nullptr, 0,
		"Div");
	if (!state->Div_type) return -1;
	state->DivSingleton = alifType_genericNew((AlifTypeObject*)state->Div_type,
		nullptr, nullptr);
	if (!state->DivSingleton) return -1;
	state->Mod_type = make_type(state, "Mod", state->operator_type, nullptr, 0,
		"Mod");
	if (!state->Mod_type) return -1;
	state->ModSingleton = alifType_genericNew((AlifTypeObject*)state->Mod_type,
		nullptr, nullptr);
	if (!state->ModSingleton) return -1;
	state->Pow_type = make_type(state, "Pow", state->operator_type, nullptr, 0,
		"Pow");
	if (!state->Pow_type) return -1;
	state->PowSingleton = alifType_genericNew((AlifTypeObject*)state->Pow_type,
		nullptr, nullptr);
	if (!state->PowSingleton) return -1;
	state->LShift_type = make_type(state, "LShift", state->operator_type, nullptr,
		0,
		"LShift");
	if (!state->LShift_type) return -1;
	state->LShiftSingleton = alifType_genericNew((AlifTypeObject
		*)state->LShift_type, nullptr,
		nullptr);
	if (!state->LShiftSingleton) return -1;
	state->RShift_type = make_type(state, "RShift", state->operator_type, nullptr,
		0,
		"RShift");
	if (!state->RShift_type) return -1;
	state->RShiftSingleton = alifType_genericNew((AlifTypeObject
		*)state->RShift_type, nullptr,
		nullptr);
	if (!state->RShiftSingleton) return -1;
	state->BitOr_type = make_type(state, "BitOr", state->operator_type, nullptr, 0,
		"BitOr");
	if (!state->BitOr_type) return -1;
	state->BitOrSingleton = alifType_genericNew((AlifTypeObject
		*)state->BitOr_type, nullptr, nullptr);
	if (!state->BitOrSingleton) return -1;
	state->BitXor_type = make_type(state, "BitXor", state->operator_type, nullptr,
		0,
		"BitXor");
	if (!state->BitXor_type) return -1;
	state->BitXorSingleton = alifType_genericNew((AlifTypeObject
		*)state->BitXor_type, nullptr,
		nullptr);
	if (!state->BitXorSingleton) return -1;
	state->BitAnd_type = make_type(state, "BitAnd", state->operator_type, nullptr,
		0,
		"BitAnd");
	if (!state->BitAnd_type) return -1;
	state->BitAndSingleton = alifType_genericNew((AlifTypeObject
		*)state->BitAnd_type, nullptr,
		nullptr);
	if (!state->BitAndSingleton) return -1;
	state->FloorDiv_type = make_type(state, "FloorDiv", state->operator_type,
		nullptr, 0,
		"FloorDiv");
	if (!state->FloorDiv_type) return -1;
	state->FloorDivSingleton = alifType_genericNew((AlifTypeObject
		*)state->FloorDiv_type, nullptr,
		nullptr);
	if (!state->FloorDivSingleton) return -1;
	state->unaryop_type = make_type(state, "unaryop", state->astType, nullptr, 0,
		"unaryop = Invert | Not | UAdd | USub");
	if (!state->unaryop_type) return -1;
	if (add_attributes(state, state->unaryop_type, nullptr, 0) < 0) return -1;
	state->Invert_type = make_type(state, "Invert", state->unaryop_type, nullptr,
		0,
		"Invert");
	if (!state->Invert_type) return -1;
	state->InvertSingleton = alifType_genericNew((AlifTypeObject
		*)state->Invert_type, nullptr,
		nullptr);
	if (!state->InvertSingleton) return -1;
	state->Not_type = make_type(state, "Not", state->unaryop_type, nullptr, 0,
		"Not");
	if (!state->Not_type) return -1;
	state->NotSingleton = alifType_genericNew((AlifTypeObject*)state->Not_type,
		nullptr, nullptr);
	if (!state->NotSingleton) return -1;
	state->UAdd_type = make_type(state, "UAdd", state->unaryop_type, nullptr, 0,
		"UAdd");
	if (!state->UAdd_type) return -1;
	state->UAddSingleton = alifType_genericNew((AlifTypeObject*)state->UAdd_type,
		nullptr, nullptr);
	if (!state->UAddSingleton) return -1;
	state->USub_type = make_type(state, "USub", state->unaryop_type, nullptr, 0,
		"USub");
	if (!state->USub_type) return -1;
	state->USubSingleton = alifType_genericNew((AlifTypeObject*)state->USub_type,
		nullptr, nullptr);
	if (!state->USubSingleton) return -1;
	state->cmpop_type = make_type(state, "cmpop", state->astType, nullptr, 0,
		"cmpop = Eq | NotEq | Lt | LtE | Gt | GtE | Is | IsNot | In | NotIn");
	if (!state->cmpop_type) return -1;
	if (add_attributes(state, state->cmpop_type, nullptr, 0) < 0) return -1;
	state->Eq_type = make_type(state, "Eq", state->cmpop_type, nullptr, 0,
		"Eq");
	if (!state->Eq_type) return -1;
	state->EqSingleton = alifType_genericNew((AlifTypeObject*)state->Eq_type,
		nullptr, nullptr);
	if (!state->EqSingleton) return -1;
	state->NotEq_type = make_type(state, "NotEq", state->cmpop_type, nullptr, 0,
		"NotEq");
	if (!state->NotEq_type) return -1;
	state->NotEqSingleton = alifType_genericNew((AlifTypeObject
		*)state->NotEq_type, nullptr, nullptr);
	if (!state->NotEqSingleton) return -1;
	state->Lt_type = make_type(state, "Lt", state->cmpop_type, nullptr, 0,
		"Lt");
	if (!state->Lt_type) return -1;
	state->LtSingleton = alifType_genericNew((AlifTypeObject*)state->Lt_type,
		nullptr, nullptr);
	if (!state->LtSingleton) return -1;
	state->LtE_type = make_type(state, "LtE", state->cmpop_type, nullptr, 0,
		"LtE");
	if (!state->LtE_type) return -1;
	state->LtESingleton = alifType_genericNew((AlifTypeObject*)state->LtE_type,
		nullptr, nullptr);
	if (!state->LtESingleton) return -1;
	state->Gt_type = make_type(state, "Gt", state->cmpop_type, nullptr, 0,
		"Gt");
	if (!state->Gt_type) return -1;
	state->GtSingleton = alifType_genericNew((AlifTypeObject*)state->Gt_type,
		nullptr, nullptr);
	if (!state->GtSingleton) return -1;
	state->GtE_type = make_type(state, "GtE", state->cmpop_type, nullptr, 0,
		"GtE");
	if (!state->GtE_type) return -1;
	state->GtESingleton = alifType_genericNew((AlifTypeObject*)state->GtE_type,
		nullptr, nullptr);
	if (!state->GtESingleton) return -1;
	state->Is_type = make_type(state, "Is", state->cmpop_type, nullptr, 0,
		"Is");
	if (!state->Is_type) return -1;
	state->IsSingleton = alifType_genericNew((AlifTypeObject*)state->Is_type,
		nullptr, nullptr);
	if (!state->IsSingleton) return -1;
	state->IsNot_type = make_type(state, "IsNot", state->cmpop_type, nullptr, 0,
		"IsNot");
	if (!state->IsNot_type) return -1;
	state->IsNotSingleton = alifType_genericNew((AlifTypeObject
		*)state->IsNot_type, nullptr, nullptr);
	if (!state->IsNotSingleton) return -1;
	state->In_type = make_type(state, "In", state->cmpop_type, nullptr, 0,
		"In");
	if (!state->In_type) return -1;
	state->InSingleton = alifType_genericNew((AlifTypeObject*)state->In_type,
		nullptr, nullptr);
	if (!state->InSingleton) return -1;
	state->NotIn_type = make_type(state, "NotIn", state->cmpop_type, nullptr, 0,
		"NotIn");
	if (!state->NotIn_type) return -1;
	state->NotInSingleton = alifType_genericNew((AlifTypeObject
		*)state->NotIn_type, nullptr, nullptr);
	if (!state->NotInSingleton) return -1;
	state->comprehension_type = make_type(state, "comprehension",
		state->astType,
		_comprehensionFields_, 4,
		"comprehension(expr target, expr iter, expr* ifs, AlifIntT isAsync)");
	if (!state->comprehension_type) return -1;
	if (add_attributes(state, state->comprehension_type, nullptr, 0) < 0) return
		-1;
	state->excepthandler_type = make_type(state, "excepthandler",
		state->astType, nullptr, 0,
		"excepthandler = ExceptHandler(expr? type, identifier? name, stmt* body)");
	if (!state->excepthandler_type) return -1;
	if (add_attributes(state, state->excepthandler_type,
		_excepthandlerAttributes_, 4) < 0) return -1;
	if (alifObject_setAttr(state->excepthandler_type, state->endLineNo, ALIF_NONE)
		== -1)
		return -1;
	if (alifObject_setAttr(state->excepthandler_type, state->endColOffset,
		ALIF_NONE) == -1)
		return -1;
	state->ExceptHandler_type = make_type(state, "ExceptHandler",
		state->excepthandler_type,
		_exceptHandlerFields_, 3,
		"ExceptHandler(expr? type, identifier? name, stmt* body)");
	if (!state->ExceptHandler_type) return -1;
	if (alifObject_setAttr(state->ExceptHandler_type, state->type, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->ExceptHandler_type, state->name, ALIF_NONE) == -1)
		return -1;
	state->arguments_type = make_type(state, "arguments", state->astType,
		_argumentsFields_, 7,
		"arguments(arg* posonlyargs, arg* args, arg? vararg, arg* kwonlyargs, expr* kwDefaults, arg? kwarg, expr* defaults)");
	if (!state->arguments_type) return -1;
	if (add_attributes(state, state->arguments_type, nullptr, 0) < 0) return -1;
	if (alifObject_setAttr(state->arguments_type, state->vararg, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->arguments_type, state->kwarg, ALIF_NONE) == -1)
		return -1;
	state->arg_type = make_type(state, "arg", state->astType, _argFields_, 3,
		"arg(identifier arg, expr? annotation, string? typeComment)");
	if (!state->arg_type) return -1;
	if (add_attributes(state, state->arg_type, _argAttributes_, 4) < 0) return
		-1;
	if (alifObject_setAttr(state->arg_type, state->annotation, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->arg_type, state->typeComment, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->arg_type, state->endLineNo, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->arg_type, state->endColOffset, ALIF_NONE) == -1)
		return -1;
	state->keywordType = make_type(state, "keyword", state->astType,
		_keywordFields_, 2,
		"keyword(identifier? arg, expr value)");
	if (!state->keywordType) return -1;
	if (add_attributes(state, state->keywordType, _keywordAttributes_, 4) < 0)
		return -1;
	if (alifObject_setAttr(state->keywordType, state->arg, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->keywordType, state->endLineNo, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->keywordType, state->endColOffset, ALIF_NONE)
		== -1)
		return -1;
	state->alias_type = make_type(state, "alias", state->astType,
		_aliasFields_, 2,
		"alias(identifier name, identifier? asname)");
	if (!state->alias_type) return -1;
	if (add_attributes(state, state->alias_type, _aliasAttributes_, 4) < 0)
		return -1;
	if (alifObject_setAttr(state->alias_type, state->asname, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->alias_type, state->endLineNo, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->alias_type, state->endColOffset, ALIF_NONE) ==
		-1)
		return -1;
	state->withitem_type = make_type(state, "withitem", state->astType,
		_withItemFields_, 2,
		"withitem(expr context_expr, expr? optional_vars)");
	if (!state->withitem_type) return -1;
	if (add_attributes(state, state->withitem_type, nullptr, 0) < 0) return -1;
	if (alifObject_setAttr(state->withitem_type, state->optional_vars, ALIF_NONE)
		== -1)
		return -1;
	state->match_case_type = make_type(state, "match_case", state->astType,
		_matchCaseFields_, 3,
		"match_case(pattern pattern, expr? guard, stmt* body)");
	if (!state->match_case_type) return -1;
	if (add_attributes(state, state->match_case_type, nullptr, 0) < 0) return -1;
	if (alifObject_setAttr(state->match_case_type, state->guard, ALIF_NONE) == -1)
		return -1;
	state->pattern_type = make_type(state, "pattern", state->astType, nullptr, 0,
		"pattern = MatchValue(expr value)\n"
		"        | MatchSingleton(constant value)\n"
		"        | MatchSequence(pattern* patterns)\n"
		"        | MatchMapping(expr* keys, pattern* patterns, identifier? rest)\n"
		"        | MatchClass(expr cls, pattern* patterns, identifier* kwd_attrs, pattern* kwd_patterns)\n"
		"        | MatchStar(identifier? name)\n"
		"        | MatchAs(pattern? pattern, identifier? name)\n"
		"        | MatchOr(pattern* patterns)");
	if (!state->pattern_type) return -1;
	if (add_attributes(state, state->pattern_type, _patternAttributes_, 4) < 0)
		return -1;
	state->MatchValue_type = make_type(state, "MatchValue",
		state->pattern_type, _MatchValueFields_,
		1,
		"MatchValue(expr value)");
	if (!state->MatchValue_type) return -1;
	state->MatchSingleton_type = make_type(state, "MatchSingleton",
		state->pattern_type,
		_matchSingletonFields_, 1,
		"MatchSingleton(constant value)");
	if (!state->MatchSingleton_type) return -1;
	state->MatchSequence_type = make_type(state, "MatchSequence",
		state->pattern_type,
		_MatchSequenceFields_, 1,
		"MatchSequence(pattern* patterns)");
	if (!state->MatchSequence_type) return -1;
	state->MatchMapping_type = make_type(state, "MatchMapping",
		state->pattern_type,
		_matchMappingFields_, 3,
		"MatchMapping(expr* keys, pattern* patterns, identifier? rest)");
	if (!state->MatchMapping_type) return -1;
	if (alifObject_setAttr(state->MatchMapping_type, state->rest, ALIF_NONE) == -1)
		return -1;
	state->MatchClass_type = make_type(state, "MatchClass",
		state->pattern_type, _matchClassFields_,
		4,
		"MatchClass(expr cls, pattern* patterns, identifier* kwd_attrs, pattern* kwd_patterns)");
	if (!state->MatchClass_type) return -1;
	state->MatchStar_type = make_type(state, "MatchStar", state->pattern_type,
		_matchStarFields_, 1,
		"MatchStar(identifier? name)");
	if (!state->MatchStar_type) return -1;
	if (alifObject_setAttr(state->MatchStar_type, state->name, ALIF_NONE) == -1)
		return -1;
	state->MatchAs_type = make_type(state, "MatchAs", state->pattern_type,
		_matchAsFields_, 2,
		"MatchAs(pattern? pattern, identifier? name)");
	if (!state->MatchAs_type) return -1;
	if (alifObject_setAttr(state->MatchAs_type, state->pattern, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->MatchAs_type, state->name, ALIF_NONE) == -1)
		return -1;
	state->MatchOr_type = make_type(state, "MatchOr", state->pattern_type,
		_matchOrFields_, 1,
		"MatchOr(pattern* patterns)");
	if (!state->MatchOr_type) return -1;
	state->type_ignore_type = make_type(state, "type_ignore", state->astType,
		nullptr, 0,
		"type_ignore = TypeIgnore(AlifIntT lineno, string tag)");
	if (!state->type_ignore_type) return -1;
	if (add_attributes(state, state->type_ignore_type, nullptr, 0) < 0) return -1;
	state->TypeIgnore_type = make_type(state, "TypeIgnore",
		state->type_ignore_type,
		_typeIgnoreFields_, 2,
		"TypeIgnore(AlifIntT lineno, string tag)");
	if (!state->TypeIgnore_type) return -1;
	state->type_param_type = make_type(state, "type_param", state->astType,
		nullptr, 0,
		"type_param = TypeVar(identifier name, expr? bound, expr? default_value)\n"
		"           | ParamSpec(identifier name, expr? default_value)\n"
		"           | TypeVarTuple(identifier name, expr? default_value)");
	if (!state->type_param_type) return -1;
	if (add_attributes(state, state->type_param_type, _typeParamAttributes_, 4)
		< 0) return -1;
	state->TypeVar_type = make_type(state, "TypeVar", state->type_param_type,
		_typeVarFields_, 3,
		"TypeVar(identifier name, expr? bound, expr? default_value)");
	if (!state->TypeVar_type) return -1;
	if (alifObject_setAttr(state->TypeVar_type, state->bound, ALIF_NONE) == -1)
		return -1;
	if (alifObject_setAttr(state->TypeVar_type, state->default_value, ALIF_NONE) ==
		-1)
		return -1;
	state->ParamSpecType = make_type(state, "ParamSpec",
		state->type_param_type, _paramSpecFields_,
		2,
		"ParamSpec(identifier name, expr? default_value)");
	if (!state->ParamSpecType) return -1;
	if (alifObject_setAttr(state->ParamSpecType, state->default_value, ALIF_NONE)
		== -1)
		return -1;
	state->TypeVarTuple_type = make_type(state, "TypeVarTuple",
		state->type_param_type,
		_typeVarTupleFields_, 2,
		"TypeVarTuple(identifier name, expr? default_value)");
	if (!state->TypeVarTuple_type) return -1;
	if (alifObject_setAttr(state->TypeVarTuple_type, state->default_value,
		ALIF_NONE) == -1)
		return -1;

	//if (!add_ast_annotations(state)) {
	//	return -1;
	//}
	return 0;
}


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
		alifErr_setString(_alifExcValueError_,
			"الاسناد يحتاج الى قيمة");
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

	p_->type = StmtK_::AsyncFunctionDefK;
	p_->V.functionDef.name = _name;
	p_->V.functionDef.args = _args;
	p_->V.functionDef.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_functionDef(AlifObject* _name, Arguments* _args,
	ASDLStmtSeq* _body, AlifIntT _lineNo, AlifIntT _colOffset,
	AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) { // 6734

	StmtTy p_{};
	if (!_name) {
		//alifErr_setString(_alifExcValueError_,
		//	"field 'name' is required for FunctionDef");
		return nullptr;
	}
	if (!_args) {
		//alifErr_setString(_alifExcValueError_,
		//	"field 'args' is required for FunctionDef");
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK_::FunctionDefK;
	p_->V.asyncFunctionDef.name = _name;
	p_->V.asyncFunctionDef.args = _args;
	p_->V.asyncFunctionDef.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_return(ExprTy _val, AlifIntT _lineNo,
	AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 6835

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
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 6852

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK_::DeleteK;
	p_->V.delete_.targets = _targets;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_classDef(AlifObject* _name, ASDLExprSeq* _bases,
	ASDLKeywordSeq* _keywords, ASDLStmtSeq* _body, AlifIntT _lineNo,
	AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 6806

	StmtTy p_{};
	if (!_name) {
		//alifErr_setString(_alifExcValueError_,
		//	"field 'name' is required for ClassDef");
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK_::ClassDefK;
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
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 6924

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

	p_->type = StmtK_::AugAssignK;
	p_->V.augAssign.target = _target;
	p_->V.augAssign.op = _op;
	p_->V.augAssign.val = _val;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_for(ExprTy _target, ExprTy _iter,
	ASDLStmtSeq* _body, AlifIntT _lineNo, AlifIntT _colOffset,
	AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) { // 6989

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

	p_->type = StmtK_::ForK;
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
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 7021

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

	p_->type = StmtK_::ForK;
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
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 7053

	StmtTy p_{};
	if (!_condetion) {
		// error
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK_::WhileK;
	p_->V.while_.condition = _condetion;
	p_->V.while_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

ExcepthandlerTy alifAST_exceptHandler(ExprTy _type, Identifier _name,
	ASDLStmtSeq* _body, AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) { // 8318

	ExcepthandlerTy p{};
	p = (ExcepthandlerTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p)
		return nullptr;
	p->type = ExceptHandlerK;
	p->V.exceptHandler.type = _type;
	p->V.exceptHandler.name = _name;
	p->V.exceptHandler.body = _body;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

StmtTy alifAST_if(ExprTy _condition, ASDLStmtSeq* _body, ASDLStmtSeq* _else,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	if (!_condition) {
		// error
		return nullptr;
	}
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->type = StmtK_::IfK;
	p_->V.if_.condition = _condition;
	p_->V.if_.body = _body;
	p_->V.if_.else_ = _else;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_try(ASDLStmtSeq* _body, ASDLExcepthandlerSeq* _handlers,
	ASDLStmtSeq* _else, ASDLStmtSeq* _finalBody, AlifIntT _lineNo, AlifIntT
	_colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) { // 7407
	StmtTy p{};
	p = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p)
		return nullptr;
	p->type = TryK;
	p->V.try_.body = _body;
	p->V.try_.handlers = _handlers;
	p->V.try_.else_ = _else;
	p->V.try_.finalBody = _finalBody;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

StmtTy alifAST_tryStar(ASDLStmtSeq* _body, ASDLExcepthandlerSeq* _handlers, ASDLStmtSeq* _else,
	ASDLStmtSeq* _finalBody, AlifIntT _lineNo, AlifIntT _colOffset,
	AlifIntT _endLineNo, AlifIntT _endColOffset, AlifASTMem* _astMem) { // 7428
	StmtTy p{};
	p = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p));
	if (!p)
		return nullptr;
	p->type = TryStarK;
	p->V.tryStar.body = _body;
	p->V.tryStar.handlers = _handlers;
	p->V.tryStar.else_ = _else;
	p->V.tryStar.finalBody = _finalBody;
	p->lineNo = _lineNo;
	p->colOffset = _colOffset;
	p->endLineNo = _endLineNo;
	p->endColOffset = _endColOffset;
	return p;
}

StmtTy alifAST_with(ASDLWithItemSeq* _items, ASDLStmtSeq* _body,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK_::WithK;
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

	p_->type = StmtK_::AsyncWithK;
	p_->V.with_.items = _items;
	p_->V.with_.body = _body;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_import(ASDLAliasSeq* _names,
	AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK_::ImportK;
	p_->V.import.names = _names;
	p_->lineNo = _lineNo;
	p_->colOffset = _colOffset;
	p_->endLineNo = _endLineNo;
	p_->endColOffset = _endColOffset;

	return p_;
}

StmtTy alifAST_importFrom(AlifObject* _module, ASDLAliasSeq* _names,
	AlifIntT _level, AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo,
	AlifIntT _endColOffset, AlifASTMem* _astMem) {

	StmtTy p_{};
	p_ = (StmtTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_)
		return nullptr;

	p_->type = StmtK_::ImportFromK;
	p_->V.importFrom.module = _module;
	p_->V.importFrom.names = _names;
	p_->V.importFrom.level = _level;
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

	p_->type = StmtK_::GlobalK;
	p_->V.global.names = _names;
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

	p_->type = StmtK_::NonlocalK;
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

	p_->type = StmtK_::PassK;
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

	p_->type = StmtK_::BreakK;
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

	p_->type = StmtK_::ContinueK;
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

	p_->type = ExprK_::BoolOpK;
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

	p->type = ExprK_::IfExprK;
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
	p->V.listComp.generators = _generators;
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
	p->V.dictComp.generators = _generators;
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
	p->type = ExprK_::CompareK;
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
	p->V.formattedValue.val = _val;
	p->V.formattedValue.conversion = _conversion;
	p->V.formattedValue.formatSpec = _formatSpec;
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

WithItemTy alifAST_withItem(ExprTy _exprCTX, ExprTy _optVars, AlifASTMem* _astMem) {

	WithItemTy p_{};
	if (!_exprCTX) {
		// error
		return nullptr;
	}

	p_ = (WithItemTy)alifASTMem_malloc(_astMem, sizeof(*p_));
	if (!p_) return nullptr;

	p_->contextExpr = _exprCTX;
	p_->optionalVars = _optVars;

	return p_;
}




AlifObject* alifAST_getDocString(ASDLStmtSeq* _body) {
	if (!ASDL_SEQ_LEN(_body)) return nullptr;

	StmtTy stmt = ASDL_SEQ_GET(_body, 0);
	if (stmt->type != StmtK_::ExprK) return nullptr;

	ExprTy expr = stmt->V.expression.val;
	if (expr->type == ExprK_::ConstantK and ALIFUSTR_CHECKEXACT(expr->V.constant.val)) {
		return expr->V.constant.val;
	}

	return nullptr;
}





AlifObject* ast2obj_mod(ASTState* state, Validator* vstate, void* _o) { // 8712
	ModuleTy o = (ModuleTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp;
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	switch (o->type) {
	case ModK_::ModuleK:
		tp = (AlifTypeObject*)state->Module_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.module.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_list(state, vstate,
		//	(ASDLSeq*)o->V.module.typeIgnores,
		//	ast2obj_typeIgnore);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->type_ignores, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
	case ModK_::InteractiveK:
		tp = (AlifTypeObject*)state->Interactive_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.interactive.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ModK_::ExpressionK:
		tp = (AlifTypeObject*)state->Expression_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.expression.body);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ModK_::FunctionK:
		tp = (AlifTypeObject*)state->FunctionType_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate,
			(ASDLSeq*)o->V.function.argTypes,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->argtypes, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.function.returns);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->returns, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	}
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}

AlifObject* ast2obj_stmt(ASTState* state, Validator* vstate, void* _o) { // 8793
	StmtTy o = (StmtTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp{};
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	switch (o->type) {
	case StmtK_::FunctionDefK:
		tp = (AlifTypeObject*)state->FunctionDefType;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.functionDef.name);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->name, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_arguments(state, vstate, o->V.functionDef.args);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->args, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.functionDef.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_list(state, vstate,
		//	(ASDLSeq*)o->V.functionDef.decoratorList,
		//	ast2obj_expr);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->decorator_list, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.functionDef.returns);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->returns, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = AST2OBJ_STRING(state, vstate, o->V.functionDef.typeComment);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->typeComment, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		value = ast2obj_list(state, vstate,
			(ASDLSeq*)o->V.functionDef.typeParams,
			ast2obj_typeParam);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->typeParams, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::AsyncFunctionDefK:
		tp = (AlifTypeObject*)state->AsyncFunctionDef_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.asyncFunctionDef.name);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->name, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_arguments(state, vstate, o->V.asyncFunctionDef.args);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->args, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate,
			(ASDLSeq*)o->V.asyncFunctionDef.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_list(state, vstate,
		//	(ASDLSeq*)o->V.asyncFunctionDef.decoratorList,
		//	ast2obj_expr);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->decorator_list, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		//value = ast2obj_expr(state, vstate, o->V.asyncFunctionDef.returns);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->returns, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		//value = AST2OBJ_STRING(state, vstate, o->V.asyncFunctionDef.typeComment);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->typeComment, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		value = ast2obj_list(state, vstate,
			(ASDLSeq*)o->V.asyncFunctionDef.typeParams,
			ast2obj_typeParam);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->typeParams, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::ClassDefK:
		tp = (AlifTypeObject*)state->ClassDef_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.classDef.name);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->name, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.classDef.bases,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->bases, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.classDef.keywords,
			ast2obj_keyword);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->keywords, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.classDef.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_list(state, vstate,
		//	(ASDLSeq*)o->V.classDef.decoratorlist,
		//	ast2obj_expr);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->decorator_list, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		value = ast2obj_list(state, vstate,
			(ASDLSeq*)o->V.classDef.typeParams,
			ast2obj_typeParam);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->typeParams, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::ReturnK:
		tp = (AlifTypeObject*)state->ReturnType;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.return_.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::DeleteK:
		tp = (AlifTypeObject*)state->DeleteType;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.delete_.targets,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->targets, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::AssignK:
		tp = (AlifTypeObject*)state->AssignType;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.assign.targets,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->targets, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.assign.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = AST2OBJ_STRING(state, vstate, o->V.assign.typeComment);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->typeComment, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
		//case StmtK_::TypeAliasK:
		//	tp = (AlifTypeObject*)state->TypeAlias_type;
		//	result = alifType_genericNew(tp, nullptr, nullptr);
		//	if (!result) goto failed;
		//	value = ast2obj_expr(state, vstate, o->V.typeAlias.name);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->name, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_list(state, vstate,
		//		(ASDLSeq*)o->V.typeAlias.type_params,
		//		ast2obj_type_param);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->typeParams, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_expr(state, vstate, o->V.typeAlias.value);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->value, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	break;
	case StmtK_::AugAssignK:
		tp = (AlifTypeObject*)state->AugAssign_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.augAssign.target);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->target, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_operator(state, vstate, o->V.augAssign.op);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->op, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.augAssign.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
		//case StmtK_::AnnAssignK:
		//	tp = (AlifTypeObject*)state->AnnAssign_type;
		//	result = alifType_genericNew(tp, nullptr, nullptr);
		//	if (!result) goto failed;
		//	value = ast2obj_expr(state, vstate, o->V.annAssign.target);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->target, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_expr(state, vstate, o->V.annAssign.annotation);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->annotation, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_expr(state, vstate, o->V.annAssign.value);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->value, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_int(state, vstate, o->V.annAssign.simple);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->simple, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	break;
	case StmtK_::ForK:
		tp = (AlifTypeObject*)state->For_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.for_.target);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->target, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.for_.iter);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->iter, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.for_.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.for_.else_,
		//	ast2obj_stmt);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->else_, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		//value = AST2OBJ_STRING(state, vstate, o->V.for_.typeComment);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->typeComment, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
	case StmtK_::AsyncForK:
		tp = (AlifTypeObject*)state->AsyncFor_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.asyncFor.target);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->target, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.asyncFor.iter);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->iter, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.asyncFor.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.asyncFor.else_,
		//	ast2obj_stmt);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->else_, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		//value = AST2OBJ_STRING(state, vstate, o->V.asyncFor.typeComment);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->typeComment, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
	case StmtK_::WhileK:
		tp = (AlifTypeObject*)state->While_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.while_.condition);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->test, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.while_.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.while_.else_,
		//	ast2obj_stmt);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->else_, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
	case StmtK_::IfK:
		tp = (AlifTypeObject*)state->If_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.if_.condition);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->test, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.if_.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.if_.else_,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->else_, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::WithK:
		tp = (AlifTypeObject*)state->With_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.with_.items,
			ast2obj_withItem);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->items, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.with_.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = AST2OBJ_STRING(state, vstate, o->V.with_.typeComment);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->typeComment, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
	case StmtK_::AsyncWithK:
		tp = (AlifTypeObject*)state->AsyncWith_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.asyncWith.items,
			ast2obj_withItem);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->items, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.asyncWith.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = AST2OBJ_STRING(state, vstate, o->V.asyncWith.typeComment);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->typeComment, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
		//case StmtK_::MatchK:
		//	tp = (AlifTypeObject*)state->Match_type;
		//	result = alifType_genericNew(tp, nullptr, nullptr);
		//	if (!result) goto failed;
		//	value = ast2obj_expr(state, vstate, o->V.match.subject);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->subject, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.match.cases,
		//		ast2obj_matchCase);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->cases, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	break;
		//case StmtK_::RaiseK:
		//	tp = (AlifTypeObject*)state->Raise_type;
		//	result = alifType_genericNew(tp, nullptr, nullptr);
		//	if (!result) goto failed;
		//	value = ast2obj_expr(state, vstate, o->V.raise.exc);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->exc, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_expr(state, vstate, o->V.raise.cause);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->cause, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	break;
	case StmtK_::TryK:
		tp = (AlifTypeObject*)state->Try_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.try_.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.try_.handlers,
			ast2obj_exceptHandler);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->handlers, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.try_.else_,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->else_, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.try_.finalBody,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->finalbody, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::TryStarK:
		tp = (AlifTypeObject*)state->TryStar_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.tryStar.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.tryStar.handlers,
			ast2obj_exceptHandler);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->handlers, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.tryStar.else_,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->else_, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.tryStar.finalBody,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->finalbody, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
		//case StmtK_::AssertK:
		//	tp = (AlifTypeObject*)state->Assert_type;
		//	result = alifType_genericNew(tp, nullptr, nullptr);
		//	if (!result) goto failed;
		//	value = ast2obj_expr(state, vstate, o->V.assert.test);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->test, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_expr(state, vstate, o->V.assert.msg);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->msg, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	break;
	case StmtK_::ImportK:
		tp = (AlifTypeObject*)state->Import_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.import.names,
			ast2obj_alias);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->names, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::ImportFromK:
		tp = (AlifTypeObject*)state->ImportFrom_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.importFrom.module);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->module, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.importFrom.names,
			ast2obj_alias);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->names, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_int(state, vstate, o->V.importFrom.level);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->level, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::GlobalK:
		tp = (AlifTypeObject*)state->Global_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.global.names,
			AST2OBJ_IDENTIFIER);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->names, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
		//case StmtK_::NonlocalK:
		//	tp = (AlifTypeObject*)state->Nonlocal_type;
		//	result = alifType_genericNew(tp, nullptr, nullptr);
		//	if (!result) goto failed;
		//	value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.nonlocal.names,
		//		AST2OBJ_IDENTIFIER);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->names, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	break;
	case StmtK_::ExprK:
		tp = (AlifTypeObject*)state->ExprType;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.expression.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case StmtK_::PassK:
		tp = (AlifTypeObject*)state->Pass_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		break;
	case StmtK_::BreakK:
		tp = (AlifTypeObject*)state->Break_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		break;
	case StmtK_::ContinueK:
		tp = (AlifTypeObject*)state->Continue_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		break;
	}
	value = ast2obj_int(state, vstate, o->lineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->lineno, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->colOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->colOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endLineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endLineNo, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endColOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endColOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}

AlifObject* ast2obj_expr(ASTState* state, Validator* vstate, void* _o) { // 9415
	ExprTy o = (ExprTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp;
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	switch (o->type) {
	case ExprK_::BoolOpK:
		tp = (AlifTypeObject*)state->BoolOp_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_boolOp(state, vstate, o->V.boolOp.op);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->op, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.boolOp.vals,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->values, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::NamedExprK:
		tp = (AlifTypeObject*)state->NamedExpr_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.namedExpr.target);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->target, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.namedExpr.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::BinOpK:
		tp = (AlifTypeObject*)state->BinOp_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.binOp.left);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->left, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_operator(state, vstate, o->V.binOp.op);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->op, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.binOp.right);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->right, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::UnaryOpK:
		tp = (AlifTypeObject*)state->UnaryOp_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_unaryOp(state, vstate, o->V.unaryOp.op);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->op, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.unaryOp.operand);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->operand, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
		//case ExprK_::LambdaK:
		//	tp = (AlifTypeObject*)state->Lambda_type;
		//	result = alifType_genericNew(tp, nullptr, nullptr);
		//	if (!result) goto failed;
		//	value = ast2obj_arguments(state, vstate, o->V.lambda.args);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->args, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_expr(state, vstate, o->V.lambda.body);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->body, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	break;
	case ExprK_::IfExprK:
		tp = (AlifTypeObject*)state->IfExp_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.ifExpr.condition);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->test, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.ifExpr.body);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.ifExpr.else_);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->else_, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::DictK:
		tp = (AlifTypeObject*)state->Dict_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.dict.keys,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->keys, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.dict.vals,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->values, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::SetK:
		tp = (AlifTypeObject*)state->Set_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.set.elts,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->elts, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::ListCompK:
		tp = (AlifTypeObject*)state->ListComp_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.listComp.elt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->elt, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate,
			(ASDLSeq*)o->V.listComp.generators,
			ast2obj_comprehension);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->generators, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::SetCompK:
		tp = (AlifTypeObject*)state->SetComp_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.setComp.elts);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->elt, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.setComp.generators,
			ast2obj_comprehension);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->generators, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::DictCompK:
		tp = (AlifTypeObject*)state->DictComp_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.dictComp.key);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->key, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.dictComp.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate,
			(ASDLSeq*)o->V.dictComp.generators,
			ast2obj_comprehension);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->generators, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
		//case ExprK_::GeneratorExprK:
		//	tp = (AlifTypeObject*)state->GeneratorExp_type;
		//	result = alifType_genericNew(tp, nullptr, nullptr);
		//	if (!result) goto failed;
		//	value = ast2obj_expr(state, vstate, o->V.generatorExp.elt);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->elt, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	value = ast2obj_list(state, vstate,
		//		(ASDLSeq*)o->V.generatorExp.generators,
		//		ast2obj_comprehension);
		//	if (!value) goto failed;
		//	if (alifObject_setAttr(result, state->generators, value) == -1)
		//		goto failed;
		//	ALIF_DECREF(value);
		//	break;
	case ExprK_::AwaitK:
		tp = (AlifTypeObject*)state->Await_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.await.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::YieldK:
		tp = (AlifTypeObject*)state->Yield_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.yield.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::YieldFromK:
		tp = (AlifTypeObject*)state->YieldFrom_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.yieldFrom.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::CompareK:
		tp = (AlifTypeObject*)state->Compare_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.compare.left);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->left, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		{
			AlifSizeT i, n = ASDL_SEQ_LEN(o->V.compare.ops);
			value = alifList_new(n);
			if (!value) goto failed;
			for (i = 0; i < n; i++)
				ALIFLIST_SET_ITEM(value, i, ast2obj_cmpOp(state, vstate, (CmpOp_)ASDL_SEQ_GET(o->V.compare.ops, i)));
		}
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->ops, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate,
			(ASDLSeq*)o->V.compare.comparators, ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->comparators, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::CallK:
		tp = (AlifTypeObject*)state->Call_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.call.func);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->func, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.call.args,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->args, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.call.keywords,
			ast2obj_keyword);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->keywords, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::FormattedValK:
		tp = (AlifTypeObject*)state->FormattedValue_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.formattedValue.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_int(state, vstate, o->V.formattedValue.conversion);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->conversion, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.formattedValue.formatSpec);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->format_spec, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::JoinStrK:
		tp = (AlifTypeObject*)state->JoinedStr_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.joinStr.vals,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->values, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::ConstantK:
		tp = (AlifTypeObject*)state->Constant_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_CONSTANT(state, vstate, o->V.constant.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = AST2OBJ_STRING(state, vstate, o->V.constant.type);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->kind, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::AttributeK:
		tp = (AlifTypeObject*)state->Attribute_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.attribute.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.attribute.attr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->attr, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_exprContext(state, vstate, o->V.attribute.ctx);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->ctx, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::SubScriptK:
		tp = (AlifTypeObject*)state->Subscript_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.subScript.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.subScript.slice);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->slice, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_exprContext(state, vstate, o->V.subScript.ctx);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->ctx, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::StarK:
		tp = (AlifTypeObject*)state->Starred_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.star.val);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->value, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_exprContext(state, vstate, o->V.star.ctx);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->ctx, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::NameK:
		tp = (AlifTypeObject*)state->Name_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.name.name);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->id, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_exprContext(state, vstate, o->V.name.ctx);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->ctx, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::ListK:
		tp = (AlifTypeObject*)state->List_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.list.elts,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->elts, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_exprContext(state, vstate, o->V.list.ctx);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->ctx, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::TupleK:
		tp = (AlifTypeObject*)state->Tuple_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.tuple.elts,
			ast2obj_expr);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->elts, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_exprContext(state, vstate, o->V.tuple.ctx);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->ctx, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	case ExprK_::SliceK:
		tp = (AlifTypeObject*)state->Slice_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.slice.lower);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->lower, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.slice.upper);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->upper, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.slice.step);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->step, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	}
	value = ast2obj_int(state, vstate, o->lineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->lineno, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->colOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->colOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endLineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endLineNo, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endColOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endColOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}

AlifObject* ast2obj_exprContext(ASTState* _state, Validator
	* _vstate, ExprContext_ _o) { // 9908
	switch (_o) {
	case ExprContext_::Load:
		return ALIF_NEWREF(_state->LoadSingleton);
	case ExprContext_::Store:
		return ALIF_NEWREF(_state->StoreSingleton);
	case ExprContext_::Del:
		return ALIF_NEWREF(_state->DelSingleton);
	}
	ALIF_UNREACHABLE();
}
AlifObject* ast2obj_boolOp(ASTState* _state, Validator* _vstate,
	BoolOp_ _o) { // 9921
	switch (_o) {
	case And:
		return ALIF_NEWREF(_state->And_singleton);
	case Or:
		return ALIF_NEWREF(_state->Or_singleton);
	}
	ALIF_UNREACHABLE();
}
AlifObject* ast2obj_operator(ASTState* _state, Validator* _vstate,
	Operator_ _o) { // 9921
	switch (_o) {
	case Operator_::Add:
		return ALIF_NEWREF(_state->AddSingleton);
	case Operator_::Sub:
		return ALIF_NEWREF(_state->SubSingleton);
	case Operator_::Mult:
		return ALIF_NEWREF(_state->MultSingleton);
		//case Operator_::MatMult:
		//	return ALIF_NEWREF(_state->MatMultSingleton);
	case Operator_::Div:
		return ALIF_NEWREF(_state->DivSingleton);
	case Operator_::Mod:
		return ALIF_NEWREF(_state->ModSingleton);
	case Operator_::Pow:
		return ALIF_NEWREF(_state->PowSingleton);
	case Operator_::LShift:
		return ALIF_NEWREF(_state->LShiftSingleton);
	case Operator_::RShift:
		return ALIF_NEWREF(_state->RShiftSingleton);
	case Operator_::BitOr:
		return ALIF_NEWREF(_state->BitOrSingleton);
	case Operator_::BitXor:
		return ALIF_NEWREF(_state->BitXorSingleton);
	case Operator_::BitAnd:
		return ALIF_NEWREF(_state->BitAndSingleton);
	case Operator_::FloorDiv:
		return ALIF_NEWREF(_state->FloorDivSingleton);
	}
	ALIF_UNREACHABLE();
}
AlifObject* ast2obj_unaryOp(ASTState* _state, Validator* _vstate,
	UnaryOp_ _o) { // 9965
	switch (_o) {
	case UnaryOp_::Invert:
		return ALIF_NEWREF(_state->InvertSingleton);
	case UnaryOp_::Not:
		return ALIF_NEWREF(_state->NotSingleton);
	case UnaryOp_::UAdd:
		return ALIF_NEWREF(_state->UAddSingleton);
	case UnaryOp_::USub:
		return ALIF_NEWREF(_state->USubSingleton);
	}
	ALIF_UNREACHABLE();
}
AlifObject* ast2obj_cmpOp(ASTState* _state, Validator* _vstate,
	CmpOp_ _o) { // 9980
	switch (_o) {
	case CmpOp_::Equal:
		return ALIF_NEWREF(_state->EqSingleton);
	case CmpOp_::NotEq:
		return ALIF_NEWREF(_state->NotEqSingleton);
	case CmpOp_::LessThan:
		return ALIF_NEWREF(_state->LtSingleton);
	case CmpOp_::LessThanEq:
		return ALIF_NEWREF(_state->LtESingleton);
	case CmpOp_::GreaterThan:
		return ALIF_NEWREF(_state->GtSingleton);
	case CmpOp_::GreaterThanEq:
		return ALIF_NEWREF(_state->GtESingleton);
	case CmpOp_::Is:
		return ALIF_NEWREF(_state->IsSingleton);
	case CmpOp_::IsNot:
		return ALIF_NEWREF(_state->IsNotSingleton);
	case CmpOp_::In:
		return ALIF_NEWREF(_state->InSingleton);
	case CmpOp_::NotIn:
		return ALIF_NEWREF(_state->NotInSingleton);
	}
	ALIF_UNREACHABLE();
}
AlifObject* ast2obj_comprehension(ASTState* state,
	Validator* vstate, void* _o) { // 10007
	ComprehensionTy o = (ComprehensionTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp;
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	tp = (AlifTypeObject*)state->comprehension_type;
	result = alifType_genericNew(tp, nullptr, nullptr);
	if (!result) return nullptr;
	value = ast2obj_expr(state, vstate, o->target);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->target, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_expr(state, vstate, o->iter);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->iter, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_list(state, vstate, (ASDLSeq*)o->ifs, ast2obj_expr);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->ifs, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->isAsync);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->isAsync, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}
AlifObject* ast2obj_exceptHandler(ASTState* state,
	Validator* vstate, void* _o) { // 10054
	ExcepthandlerTy o = (ExcepthandlerTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp;
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	switch (o->type) {
	case ExcepthandlerK_::ExceptHandlerK:
		tp = (AlifTypeObject*)state->ExceptHandler_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = ast2obj_expr(state, vstate, o->V.exceptHandler.type);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->type, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.exceptHandler.name);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->name, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_list(state, vstate, (ASDLSeq*)o->V.exceptHandler.body,
			ast2obj_stmt);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->body, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		break;
	}
	value = ast2obj_int(state, vstate, o->lineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->lineno, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->colOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->colOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endLineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endLineNo, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endColOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endColOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}


AlifObject* ast2obj_arguments(ASTState* state, Validator* vstate, void* _o) { // 10121
	ArgumentsTy o = (ArgumentsTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp{};
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	tp = (AlifTypeObject*)state->arguments_type;
	result = alifType_genericNew(tp, nullptr, nullptr);
	if (!result) return nullptr;
	value = ast2obj_list(state, vstate, (ASDLSeq*)o->posOnlyArgs, ast2obj_arg);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->posonlyargs, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_list(state, vstate, (ASDLSeq*)o->args, ast2obj_arg);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->args, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_arg(state, vstate, o->varArg);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->vararg, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_list(state, vstate, (ASDLSeq*)o->kwOnlyArgs, ast2obj_arg);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->kwonlyargs, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_list(state, vstate, (ASDLSeq*)o->kwDefaults,
		ast2obj_expr);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->kwDefaults, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_arg(state, vstate, o->kwArg);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->kwarg, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_list(state, vstate, (ASDLSeq*)o->defaults, ast2obj_expr);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->defaults, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}
AlifObject* ast2obj_arg(ASTState* state, Validator* vstate, void* _o) { // 10183
	ArgTy o = (ArgTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp;
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	tp = (AlifTypeObject*)state->arg_type;
	result = alifType_genericNew(tp, nullptr, nullptr);
	if (!result) return nullptr;
	value = AST2OBJ_IDENTIFIER(state, vstate, o->arg);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->arg, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	//value = ast2obj_expr(state, vstate, o->annotation);
	//if (!value) goto failed;
	//if (alifObject_setAttr(result, state->annotation, value) == -1)
	//	goto failed;
	//ALIF_DECREF(value);
	//value = AST2OBJ_STRING(state, vstate, o->typeComment);
	//if (!value) goto failed;
	//if (alifObject_setAttr(result, state->typeComment, value) == -1)
	//	goto failed;
	//ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->lineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->lineno, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->colOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->colOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endLineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endLineNo, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endColOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endColOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}


AlifObject* ast2obj_keyword(ASTState* _state,
	Validator* _vstate, void* _o) { // 10244
	KeywordTy o = (KeywordTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp{};
	if (!o) {
		return ALIF_NONE;
	}
	if (++_vstate->recursionDepth > _vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	tp = (AlifTypeObject*)_state->keywordType;
	result = alifType_genericNew(tp, nullptr, nullptr);
	if (!result) return nullptr;
	value = AST2OBJ_IDENTIFIER(_state, _vstate, o->arg);
	if (!value) goto failed;
	if (alifObject_setAttr(result, _state->arg, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_expr(_state, _vstate, o->val);
	if (!value) goto failed;
	if (alifObject_setAttr(result, _state->value, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(_state, _vstate, o->lineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, _state->lineno, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(_state, _vstate, o->colOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, _state->colOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(_state, _vstate, o->endLineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, _state->endLineNo, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(_state, _vstate, o->endColOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, _state->endColOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	_vstate->recursionDepth--;
	return result;
failed:
	_vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}
AlifObject* ast2obj_alias(ASTState* state, Validator* vstate, void* _o) { // 10300
	AliasTy o = (AliasTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp;
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	tp = (AlifTypeObject*)state->alias_type;
	result = alifType_genericNew(tp, nullptr, nullptr);
	if (!result) return nullptr;
	value = AST2OBJ_IDENTIFIER(state, vstate, o->name);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->name, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = AST2OBJ_IDENTIFIER(state, vstate, o->asName);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->asname, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->lineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->lineno, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->colOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->colOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endLineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endLineNo, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endColOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endColOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}
AlifObject* ast2obj_withItem(ASTState* state, Validator* vstate, void* _o) { // 10356
	WithItemTy o = (WithItemTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp;
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	tp = (AlifTypeObject*)state->withitem_type;
	result = alifType_genericNew(tp, nullptr, nullptr);
	if (!result) return nullptr;
	value = ast2obj_expr(state, vstate, o->contextExpr);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->context_expr, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_expr(state, vstate, o->optionalVars);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->optional_vars, value) == -1)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}

AlifObject* ast2obj_typeParam(ASTState* state, Validator* vstate, void* _o) { // 10640
	TypeParamTy o = (TypeParamTy)_o;
	AlifObject* result = nullptr, * value = nullptr;
	AlifTypeObject* tp;
	if (!o) {
		return ALIF_NONE;
	}
	if (++vstate->recursionDepth > vstate->recursionLimit) {
		//alifErr_setString(_alifExcRecursionError_,
		//	"maximum recursion depth exceeded during ast construction");
		return nullptr;
	}
	switch (o->type) {
	case TypeParamK::TypeVarK:
		tp = (AlifTypeObject*)state->TypeVar_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.typeVar.name);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->name, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		value = ast2obj_expr(state, vstate, o->V.typeVar.bound);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->bound, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_expr(state, vstate, o->V.typeVar.defaultValue);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->default_value, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
	case TypeParamK::ParamSpecK:
		tp = (AlifTypeObject*)state->ParamSpecType;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.paramSpec.name);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->name, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_expr(state, vstate, o->V.paramSpec.defaultValue);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->default_value, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
	case TypeParamK::TypeVarTupleK:
		tp = (AlifTypeObject*)state->TypeVarTuple_type;
		result = alifType_genericNew(tp, nullptr, nullptr);
		if (!result) goto failed;
		value = AST2OBJ_IDENTIFIER(state, vstate, o->V.typeVarTuple.name);
		if (!value) goto failed;
		if (alifObject_setAttr(result, state->name, value) == -1)
			goto failed;
		ALIF_DECREF(value);
		//value = ast2obj_expr(state, vstate, o->V.typeVarTuple.defaultValue);
		//if (!value) goto failed;
		//if (alifObject_setAttr(result, state->default_value, value) == -1)
		//	goto failed;
		//ALIF_DECREF(value);
		break;
	}
	value = ast2obj_int(state, vstate, o->lineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->lineno, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->colOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->colOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endLineNo);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endLineNo, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	value = ast2obj_int(state, vstate, o->endColOffset);
	if (!value) goto failed;
	if (alifObject_setAttr(result, state->endColOffset, value) < 0)
		goto failed;
	ALIF_DECREF(value);
	vstate->recursionDepth--;
	return result;
failed:
	vstate->recursionDepth--;
	ALIF_XDECREF(value);
	ALIF_XDECREF(result);
	return nullptr;
}





AlifObject* alifAST_mod2obj(ModuleTy _t) { // 18130
	class ASTState* state = get_astState();
	if (state == nullptr) {
		return nullptr;
	}

	AlifIntT startingRecursionDepth{};
	/* Be careful here to prevent overflow. */
	AlifThread* tstate = _alifThread_get();
	if (!tstate) {
		return nullptr;
	}
	class Validator vstate {};
	vstate.recursionLimit = ALIFCPP_RECURSION_LIMIT;
	AlifIntT recursionDepth = ALIFCPP_RECURSION_LIMIT - tstate->cppRecursionRemaining;
	startingRecursionDepth = recursionDepth;
	vstate.recursionDepth = startingRecursionDepth;

	AlifObject* result = ast2obj_mod(state, &vstate, _t);

	/* Check that the recursion depth counting balanced correctly */
	if (result and vstate.recursionDepth != startingRecursionDepth) {
		alifErr_format(_alifExcSystemError_,
			"AST constructor recursion depth mismatch (before=%d, after=%d)",
			startingRecursionDepth, vstate.recursionDepth);
		return nullptr;
	}
	return result;
}
