#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_SymTable.h"
#include "AlifCore_Memory.h"


// Forward Declaration
static int analyze_block(AlifSTEntryObject*, AlifObject*, AlifObject*, AlifObject*, AlifObject*, AlifSTEntryObject*);


#define LOCATION(_x) \
 (_x)->lineNo, (_x)->colOffset, (_x)->endLineNo, (_x)->endColOffset


#define VISIT_QUIT(_symTable, _x) return --(_symTable)->recursionDepth,(_x)

#define VISIT(_symTable, _type, _v) \
    if (!symTable_visit ## _type((_symTable), (_v))) VISIT_QUIT((_symTable), 0);

#define VISIT_SEQ(_st, _type, _fullType, _seq) { \
    AlifIntT i; \
    _type ## Seq *seq = _seq;  \
    for (i = 0; i < SEQ_LEN(seq); i++) { \
        _fullType* elt = (_fullType*)SEQ_GET(seq, i); \
        if (!symTable_visit ## _type((_st), elt)) \
            VISIT_QUIT((_st), 0);                 \
    } \
}

#define VISIT_SEQ_WITH_NULLPTR(_st, _type, _fullType, _seq) {     \
    AlifIntT i = 0; \
    _type ## Seq *seq = _seq;  \
    for (i = 0; i < SEQ_LEN(seq); i++) { \
        _fullType* elt = (_fullType*)SEQ_GET(seq, i); \
        if (!elt) continue;  \
        if (!symTable_visit ## _type((_st), elt)) \
            VISIT_QUIT((_st), 0);             \
    } \
}

AlifSTEntryObject* alifSymTable_lookup(AlifSymTable* _st, void* _key) { // 487
    AlifObject* k, * v;

    k = alifInteger_fromSizeT((unsigned long long)(uintptr_t)_key, true);
    if (k == NULL)
        return NULL;
    if (alifDict_getItemRef(_st->stBlocks, k, &v) == 0) {
        return nullptr; // should be show error 
    }
    ALIF_DECREF(k);

    return (AlifSTEntryObject*)v;
}

AlifIntT alifST_getSymbol(AlifSTEntryObject* _ste, AlifObject* _name) { // 505
    AlifObject* v = dict_getItem(_ste->steSymbols, _name); // alifDict_getItemWithError
    if (!v) return 0;

    return alifInteger_asLong(v); // ALIFLONG_AS_LONG
}

AlifIntT alifST_getScope(AlifSTEntryObject* _ste, AlifObject* _name) { // 515
    AlifIntT symbol = alifST_getSymbol(_ste, _name);
    return (symbol >> SCOPE_OFFSET) & SCOPE_MASK;
}

AlifIntT alifST_isFunctionLike(AlifSTEntryObject* _ste) { // 522
    return _ste->steType == FunctionBlock
        or _ste->steType == TypeVarBoundBlock
        or _ste->steType == TypeAliasBlock
        or _ste->steType == TypeParamBlock;
}

AlifObject* alif_mangle(AlifObject * _privateObj, AlifObject * _ident)
{
    if (_privateObj == nullptr or !(_privateObj->type_ == &_alifUStrType_) or
        ALIFUSTR_READ_WCHAR(_ident, 0) != L'_' or
        ALIFUSTR_READ_WCHAR(_ident, 1) != L'_') {
        return ALIF_NEWREF(_ident);
    }

    size_t nlen = ALIFUSTR_GET_LENGTH(_ident);
    size_t plen = ALIFUSTR_GET_LENGTH(_privateObj);

    if ((ALIFUSTR_READ_WCHAR(_ident, nlen - 1) == L'_'
        and
        ALIFUSTR_READ_WCHAR(_ident, nlen - 2) == L'_')
        //or
        //ALIFUNICODE_FINDWCHAR(ident, '.', 0, nlen, 1) != -1
        ) {
        return ALIF_NEWREF(_ident);
    }
    size_t ipriv = 0;
    while (ALIFUSTR_READ_WCHAR(_privateObj, ipriv) == '_') {
        ipriv++;
    }
    if (ipriv == plen) {
        return ALIF_NEWREF(_ident);
    }
    plen -= ipriv;

    if (plen + nlen >= LLONG_MAX - 1) {
        return nullptr;
    }

    AlifUCS4 maxchar = ALIFUSTR_MAX_CHAR_VALUE(_ident);
    if (ALIFUSTR_MAX_CHAR_VALUE(_privateObj) > maxchar) {
        maxchar = ALIFUSTR_MAX_CHAR_VALUE(_privateObj);
    }

    AlifObject* result = alifNew_uStr(1 + nlen + plen, maxchar);
    if (!result) {
        return nullptr;
    }
    ALIFUSTR_WRITE(ALIFUSTR_KIND(result), ((AlifUStrObject*)result)->UTF, 0, '_');
    if (alifUStr_copyCharacters(result, 1, _privateObj, ipriv, plen) < 0) {
        ALIF_DECREF(result);
        return nullptr;
    }
    if (alifUStr_copyCharacters(result, plen + 1, _ident, 0, nlen) < 0) {
        ALIF_DECREF(result);
        return nullptr;
    }
    return result;
}


static AlifIntT symTable_addDefHelper(AlifSymTable* _st, AlifObject* _name, AlifIntT _flag, class AlifSTEntryObject* _ste,
    AlifIntT _lineNo, AlifIntT _colOffset, AlifIntT _endLineNo, AlifIntT _endColOffset) { // 1381
    
    AlifObject* obj_{};
    AlifObject* dict_{};
    long val_{};
    AlifObject* mangled = alif_mangle(_st->stPrivate, _name);

    if (!mangled) return 0;

    dict_ = _ste->steSymbols;
    if ((obj_ = dict_getItem(dict_, mangled))) {
        val_ = alifInteger_asLong(obj_);
        if ((_flag & DEF_PARAM) and (val_ & DEF_PARAM)) goto error;
        if ((_flag & DEF_TYPE_PARAM) and (val_ & DEF_TYPE_PARAM)) goto error;

        val_ |= _flag;
    }
    else {
        val_ = _flag;
    }
    if (_ste->steCompIterTarget) {
        if (val_ & (DEF_GLOBAL | DEF_NONLOCAL)) goto error;

        val_ |= DEF_COMP_ITER;
    }
    obj_ = alifInteger_fromLongLong(val_);
    if (obj_ == nullptr) goto error;

    if (dict_setItem((AlifDictObject*)dict_, mangled, obj_) < 0) {
        ALIF_DECREF(obj_);
        goto error;
    }
    ALIF_DECREF(obj_);

    if (_flag & DEF_PARAM) {
        if (alifList_append(_ste->steVarNames, mangled) < 0) goto error;
    }
    else if (_flag & DEF_GLOBAL) {
        val_ = _flag;
        if ((obj_ = dict_getItem(_st->stGlobal, mangled))) {
            val_ |= alifInteger_asLong(obj_);
        }
        obj_ = alifInteger_fromLongLong(val_);
        if (obj_ == nullptr) goto error;

        if (dict_setItem((AlifDictObject*)_st->stGlobal, mangled, obj_) < 0) {
            ALIF_DECREF(obj_);
            goto error;
        }
        ALIF_DECREF(obj_);
    }
    ALIF_DECREF(mangled);
    return 1;

error:
    ALIF_DECREF(mangled);
    return 0;
}

static AlifIntT symTable_addDef(class AlifSymTable* _st, AlifObject* _name, AlifIntT _flag,
    AlifIntT lineno, AlifIntT col_offset, AlifIntT end_lineno, AlifIntT end_col_offset) { // 1474

    return symTable_addDefHelper(_st, _name, _flag, _st->stCur, lineno, col_offset, end_lineno, end_col_offset);
}


static AlifIntT symTable_visitExpr(AlifSymTable* _symTable, Expression* _expr) { // 2118
    if (++_symTable->recursionDepth > _symTable->recursionLimit) {
        VISIT_QUIT(_symTable, 0);
    }

    if (_expr->type == ExprType::BoolOpK) {
        VISIT_SEQ(_symTable, Expr, Expression, _expr->V.boolOp.vals);
    }
    else if (_expr->type == ExprType::BinOpK) {
        VISIT(_symTable, Expr, _expr->V.binOp.left);
        VISIT(_symTable, Expr, _expr->V.binOp.right);
    }
    else if (_expr->type == ExprType::UnaryOpK) {
        VISIT(_symTable, Expr, _expr->V.unaryOp.operand);
    }
    else if (_expr->type == ExprType::IfExprK) {
        VISIT(_symTable, Expr, _expr->V.ifExpr.condition_);
        VISIT(_symTable, Expr, _expr->V.ifExpr.body_);
        VISIT(_symTable, Expr, _expr->V.ifExpr.else_);
    }
    else if (_expr->type == ExprType::CompareK) {
        VISIT(_symTable, Expr, _expr->V.compare.left);
        VISIT_SEQ(_symTable, Expr, Expression, _expr->V.compare.comparators);
    }
    else if (_expr->type == ExprType::CallK) {
        VISIT(_symTable, Expr, _expr->V.call.func);
        VISIT_SEQ(_symTable, Expr, Expression, _expr->V.call.args);
        //VISIT_SEQ_WITH_NULLPTR(_symTable, Keyword, Keyword, _expr->V.call.keywords);
    }
    else if (_expr->type == ExprType::AttributeK) {
        VISIT(_symTable, Expr, _expr->V.attribute.val);
    }
    else if (_expr->type == ExprType::SubScriptK) {
        VISIT(_symTable, Expr, _expr->V.subScript.val);
        VISIT(_symTable, Expr, _expr->V.subScript.slice);
    }
    else if (_expr->type == ExprType::SliceK) {
        if (_expr->V.slice.lower)
            VISIT(_symTable, Expr, _expr->V.slice.lower)
            if (_expr->V.slice.upper)
                VISIT(_symTable, Expr, _expr->V.slice.upper)
                if (_expr->V.slice.step)
                    VISIT(_symTable, Expr, _expr->V.slice.step)
    }
    else if (_expr->type == ExprType::NamedExprK) {
        if (!symTable_addDef(_symTable, _expr->V.name.name, _expr->V.name.ctx == Load ? USE : DEF_LOCAL, LOCATION(_expr)))
            VISIT_QUIT(_symTable, 0);
        if (_expr->V.name.ctx == ExprCTX::Load 
            and
            alifST_isFunctionLike(_symTable->stCur)
            //and
            //unicode_EqualToASCIIString(_expr->V.name.name, L"super")
            ) {
            AlifObject* name = alifUStr_decodeStringToUTF8(L"__class__");
            if (!symTable_addDef(_symTable, name, USE, LOCATION(_expr)))
                VISIT_QUIT(_symTable, 0);
        }
    }

    VISIT_QUIT(_symTable, 1);
}


static AlifIntT symTable_visitStmt(class AlifSymTable* _symTable, Statement* _stmt) { // 1623
    if (++_symTable->recursionDepth > _symTable->recursionLimit) {
        VISIT_QUIT(_symTable, 0);
    }
    if (_stmt->type == ExprK) {
        VISIT(_symTable, Expr, _stmt->V.expression.val);
    }
    VISIT_QUIT(_symTable, 1);
}

static void ste_dealloc(AlifSTEntryObject* _ste)
{
    _ste->stETable = NULL;
    ALIF_XDECREF(_ste->steID);
    ALIF_XDECREF(_ste->steName);
    ALIF_XDECREF(_ste->steSymbols);
    ALIF_XDECREF(_ste->steVarNames);
    ALIF_XDECREF(_ste->steChildren);
    ALIF_XDECREF(_ste->steDirectives);
    alifMem_objFree(_ste);
}

AlifTypeObject _alifSTEntryType_ = {
    0,
    0,
    0,
    L"symtable entry",
    sizeof(AlifSTEntryObject),
    0,
    (Destructor)ste_dealloc,                /* tp_dealloc */
    0,                                      /* tp_vectorcall_offset */
    0,                                         /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    0,                         /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    0,                         /* tp_flags */
    0,                                          /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                             /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
};


static AlifSTEntryObject* symTableEntry_new(AlifSymTable* _symTable, AlifObject* _name, AlifBlockType _block,
    void* _key, AlifIntT _lineNo, AlifIntT _colOffset,
    AlifIntT _endLineNo, AlifIntT _endColOffset) { // 80

    AlifSTEntryObject* symTableEntry = nullptr;
    AlifObject* k_ = nullptr;

    k_ = alifInteger_fromSizeT((unsigned long long)(uintptr_t)_key, true);
    if (k_ == nullptr) goto fail;

    symTableEntry = ALIFOBJECT_NEW(AlifSTEntryObject, &_alifSTEntryType_);
    if (symTableEntry == nullptr) {
        ALIF_DECREF(k_);
        goto fail;
    }
    symTableEntry->stETable = _symTable;
    symTableEntry->steID = k_;

    symTableEntry->steName = ALIF_NEWREF(_name);

    symTableEntry->steSymbols = nullptr;
    symTableEntry->steVarNames = nullptr;
    symTableEntry->steChildren = nullptr;

    symTableEntry->steDirectives = nullptr;

    symTableEntry->steType = _block;
    symTableEntry->steNested = 0;
    symTableEntry->steFree = 0;
    symTableEntry->steVarArgs = 0;
    symTableEntry->steVarKeywords = 0;
    symTableEntry->steOptLineNo = 0;
    symTableEntry->steOptColOffset = 0;
    symTableEntry->steLineNo = _lineNo;
    symTableEntry->steColOffset = _colOffset;
    symTableEntry->steEndLineNo = _endLineNo;
    symTableEntry->steEndColOffset = _endColOffset;


    if (_symTable->stCur != nullptr 
        and
        (_symTable->stCur->steNested
        or alifST_isFunctionLike(_symTable->stCur)))
        symTableEntry->steNested = 1;
    symTableEntry->steChildFree = 0;
    symTableEntry->steGenerator = 0;
    symTableEntry->steCoroutine = 0;
    symTableEntry->stEComprehension = AlifComprehensionType::NoComprehension;
    symTableEntry->steReturnsValue = 0;
    symTableEntry->steNeedsClassClosure = 0;
    symTableEntry->steCompInlined = 0;
    symTableEntry->steCompIterTarget = 0;
    symTableEntry->steCanSeeClassScope = 0;
    symTableEntry->steCompIterExpr = 0;
    symTableEntry->steNeedsClassDict = 0;

    symTableEntry->steSymbols = alifNew_dict();
    symTableEntry->steVarNames = alifNew_list(0);
    symTableEntry->steChildren = alifNew_list(0);
    if (symTableEntry->steSymbols == nullptr
        or symTableEntry->steVarNames == nullptr
        or symTableEntry->steChildren == nullptr) goto fail;

    if (dict_setItem((AlifDictObject*)_symTable->stBlocks, symTableEntry->steID, (AlifObject*)symTableEntry) < 0)
        goto fail;

    return symTableEntry;
fail:
    ALIF_XDECREF(symTableEntry);
    return nullptr;
}

static int update_symbols(AlifObject* _symbols, AlifObject* _scopes,
    AlifObject* _bound, AlifObject* _free, AlifObject* _inlinedCells, int _classflag) { // 903
    AlifObject* name = nullptr, * itr = nullptr;
    AlifObject* v = nullptr, * v_scope = nullptr, * v_new = nullptr, * v_free = nullptr;
    int64_t pos = 0;

    while (alifDict_next(_symbols, &pos, &name, &v, nullptr)) {
        long scope, flags;
        flags = alifInteger_asLong(v);
        int contains = alifSet_contains(_inlinedCells, name);
        if (contains < 0) {
            return 0;
        }
        if (contains) {
            flags |= DEF_COMP_CELL;
        }
        v_scope = dict_getItem(_scopes, name);
        scope = alifInteger_asLong(v_scope);
        flags |= (scope << SCOPE_OFFSET);
        v_new = alifInteger_fromLongLong(flags);
        if (!v_new)
            return 0;
        _symbols = (AlifObject*)dict_setItem((AlifDictObject*)_symbols, name, v_new);
        if ((_symbols) == nullptr) {
            ALIF_DECREF(v_new);
            return 0;
        }
        ALIF_DECREF(v_new);
    }

    v_free = alifInteger_fromLongLong(FREE << SCOPE_OFFSET);
    if (!v_free)
        return 0;

    itr = alifObject_getIter(_free);
    if (itr == nullptr) {
        ALIF_DECREF(v_free);
        return 0;
    }

    while ((name = alifIter_next(itr))) {
        v = dict_getItem(_symbols, name);

        if (v) {

            if (_classflag) {
                long flags = alifInteger_asLong(v) | DEF_FREE_CLASS;
                v_new = alifInteger_fromLongLong(flags);
                if (!v_new) {
                    goto error;
                }
                _symbols = (AlifObject*)dict_setItem((AlifDictObject*)_symbols, name, v_new);
                if ((_symbols) == nullptr) {
                    ALIF_DECREF(v_new);
                    goto error;
                }
                ALIF_DECREF(v_new);
            }
            ALIF_DECREF(name);
            continue;
        }
        if (_bound) {
            int contains = alifSet_contains(_bound, name);
            if (contains < 0) {
                goto error;
            }
            if (!contains) {
                ALIF_DECREF(name);
                continue;
            }
        }
        _symbols = (AlifObject*)dict_setItem((AlifDictObject*)_symbols, name, v_free);
        if ((_symbols) == nullptr) {
            goto error;
        }
        ALIF_DECREF(name);
    }

    ALIF_DECREF(itr);
    ALIF_DECREF(v_free);
    return 1;
error:
    ALIF_XDECREF(v_free);
    ALIF_XDECREF(itr);
    ALIF_XDECREF(name);
    return 0;
}

static AlifIntT drop_classFree(AlifSTEntryObject* _ste, AlifObject* _free) { // 882
    int res;
    AlifObject* className = alifUStr_decodeStringToUTF8(L"__class__");
    res = alifSet_discard(_free, className);
    if (res < 0) return 0;
    if (res) _ste->steNeedsClassClosure = 1;

    AlifObject* className2 = alifUStr_decodeStringToUTF8(L"__classdict__");
    res = alifSet_discard(_free, className2);
    if (res < 0) return 0;
    if (res) _ste->steNeedsClassDict = 1;

    return 1;
}

static AlifIntT analyze_cells(AlifObject* _scopes, AlifObject* _free, AlifObject* _inlinedCells) { // 83/
    AlifObject* name, * v, * v_cell;
    AlifIntT success = 0;
    AlifSizeT pos = 0;

    v_cell = alifInteger_fromLongLong(CELL);
    if (!v_cell) return 0;

    while (alifDict_next(_scopes, &pos, &name, &v, nullptr)) {
        long scope;
        scope = alifInteger_asLong(v);
        if (scope != LOCAL)
            continue;
        int contains = alifSet_contains(_free, name);
        if (contains < 0) {
            goto error;
        }
        if (!contains) {
            contains = alifSet_contains(_inlinedCells, name);
            if (contains < 0) {
                goto error;
            }
            if (!contains) {
                continue;
            }
        }
        _scopes = (AlifObject*)dict_setItem((AlifDictObject*)_scopes, name, v_cell);
        if (_scopes == nullptr)
            goto error;
        if (alifSet_discard(_free, name) < 0)
            goto error;
    }
    success = 1;
error:
    ALIF_DECREF(v_cell);
    return success;
}

static AlifIntT analyze_childBlock(AlifSTEntryObject* _entry, AlifObject* _bound, AlifObject* _free,
    AlifObject* _global, AlifObject* _typeParams, AlifSTEntryObject* _classEntry, AlifObject** _childFree) { // 1227

    AlifObject* temp_bound = nullptr, * temp_global = nullptr, * temp_free = nullptr;
    AlifObject* temp_type_params = nullptr;
    temp_bound = alifNew_set(_bound);
    if (!temp_bound)
        goto error;
    temp_free = alifNew_set(_free);
    if (!temp_free)
        goto error;
    temp_global = alifNew_set(_global);
    if (!temp_global)
        goto error;
    temp_type_params = alifNew_set(_typeParams);
    if (!temp_type_params)
        goto error;

    if (!analyze_block(_entry, temp_bound, temp_free, temp_global,
        temp_type_params, _classEntry))
        goto error;
    *_childFree = temp_free;
    ALIF_DECREF(temp_bound);
    ALIF_DECREF(temp_global);
    ALIF_DECREF(temp_type_params);
    return 1;
error:
    ALIF_XDECREF(temp_bound);
    ALIF_XDECREF(temp_free);
    ALIF_XDECREF(temp_global);
    ALIF_XDECREF(temp_type_params);
    return 0;
}

static int analyze_block(AlifSTEntryObject* _ste, AlifObject* _bound, AlifObject* _free,
    AlifObject* _global, AlifObject* _typeParams, AlifSTEntryObject* _classEntry) { // 1035

    AlifObject* name, * v, * local = nullptr, * scopes = nullptr, * newbound = nullptr;
    AlifObject* newglobal = nullptr, * newfree = nullptr, * inlined_cells = nullptr;
    AlifObject* temp{};
    int success = 0;
    AlifSizeT i, pos = 0;

    local = alifNew_set(nullptr);
    if (!local)
        goto error;
    scopes = alifNew_dict();
    if (!scopes)
        goto error;
    newglobal = alifNew_set(nullptr);
    if (!newglobal)
        goto error;
    newfree = alifNew_set(nullptr);
    if (!newfree)
        goto error;
    newbound = alifNew_set(nullptr);
    if (!newbound)
        goto error;
    inlined_cells = alifNew_set(nullptr);
    if (!inlined_cells)
        goto error;

    if (_ste->steType == ClassBlock) {
        temp = alifInteger_inPlaceOr(newglobal, _global);
        if (!temp)
            goto error;
        ALIF_DECREF(temp);
        if (_bound) {
            temp = alifInteger_inPlaceOr(newbound, _bound);
            if (!temp)
                goto error;
            ALIF_DECREF(temp);
        }
    }

    while (alifDict_next(_ste->steSymbols, &pos, &name, &v, nullptr)) {
        long flags = alifInteger_asLong(v);
        //if (!analyze_name(ste, scopes, name, flags,
            //bound, local, free, global, type_params, class_entry))
            //goto error;
    }

    if (_ste->steType != ClassBlock) {
        if (alifST_isFunctionLike(_ste)) {
            temp = alifInteger_inPlaceOr(newbound, local);
            if (!temp)
                goto error;
            ALIF_DECREF(temp);
        }
        if (_bound) {
            temp = alifInteger_inPlaceOr(newbound, _bound);
            if (!temp)
                goto error;
            ALIF_DECREF(temp);
        }
        temp = alifInteger_inPlaceOr(newglobal, _global);
        if (!temp)
            goto error;
        ALIF_DECREF(temp);
    }
    else {
        AlifObject* className = alifUStr_decodeStringToUTF8(L"__class__");
        if (alifSet_add(newbound, className) < 0)
            goto error;

        AlifObject* className2 = alifUStr_decodeStringToUTF8(L"__classdict__");
        if (alifSet_add(newbound, className2) < 0)
            goto error;
    }

    for (i = 0; i < ((AlifVarObject*)_ste->steChildren)->size_; ++i) {
        AlifObject* child_free = nullptr;
        AlifObject* c = ((AlifListObject*)_ste->steChildren)->items[i];
        AlifSTEntryObject* entry;
        entry = (AlifSTEntryObject*)c;

        AlifSTEntryObject* new_class_entry = nullptr;
        if (entry->steCanSeeClassScope) {
            if (_ste->steType == ClassBlock) {
                new_class_entry = _ste;
            }
            else if (_classEntry) {
                new_class_entry = _classEntry;
            }
        }

        int inline_comp =
            entry->stEComprehension and
            !entry->steGenerator;

        if (!analyze_childBlock(entry, newbound, newfree, newglobal,
            _typeParams, new_class_entry, &child_free))
        {
            goto error;
        }
        //if (inline_comp) {
            //if (!inline_comprehension(ste, entry, scopes, child_free, inlined_cells)) {
                //ALIF_DECREF(child_free);
                //goto error;
            //}
            //entry->sTECompInlined = 1;
        //}
        temp = alifInteger_inPlaceOr(newfree, child_free);
        ALIF_DECREF(child_free);
        if (!temp)
            goto error;
        ALIF_DECREF(temp);
        if (entry->steFree || entry->steChildFree)
            _ste->steChildFree = 1;
    }

    for (i = ((AlifVarObject*)_ste->steChildren)->size_ - 1; i >= 0; --i) {
        AlifObject* c = ((AlifListObject*)_ste->steChildren)->items[i];
        AlifSTEntryObject* entry;
        entry = (AlifSTEntryObject*)c;
        if (entry->steCompInlined and
            list_setSlice(_ste->steChildren, i, i + 1,
                entry->steChildren) < 0)
        {
            goto error;
        }
    }

    if (alifST_isFunctionLike(_ste) and !analyze_cells(scopes, newfree, inlined_cells))
        goto error;
    else if (_ste->steType == ClassBlock and !drop_classFree(_ste, newfree))
        goto error;
    if (!update_symbols(_ste->steSymbols, scopes, _bound, newfree, inlined_cells,
        (_ste->steType == ClassBlock) || _ste->steCanSeeClassScope))
        goto error;

    temp = alifInteger_inPlaceOr(_free, newfree);
    if (!temp)
        goto error;
    ALIF_DECREF(temp);
    success = 1;
error:
    ALIF_XDECREF(scopes);
    ALIF_XDECREF(local);
    ALIF_XDECREF(newbound);
    ALIF_XDECREF(newglobal);
    ALIF_XDECREF(newfree);
    ALIF_XDECREF(inlined_cells);
    return success;
}

static AlifIntT symTable_analyze(class AlifSymTable* st) { // 1271
    AlifObject* free, * global, * typeParams;
    int r{};

    free = alifNew_set(nullptr);
    if (!free)
        return 0;
    global = alifNew_set(nullptr);
    if (!global) {
        ALIF_DECREF(free);
        return 0;
    }
    typeParams = alifNew_set(nullptr);
    if (!typeParams) {
        ALIF_DECREF(free);
        ALIF_DECREF(global);
        return 0;
    }

    r = analyze_block(st->stTop, nullptr, free, global, typeParams, nullptr);
    ALIF_DECREF(free);
    ALIF_DECREF(global);
    ALIF_DECREF(typeParams);
    return r;
}

static AlifIntT symTable_exitBlock(AlifSymTable* _st) { // 1303
    AlifSizeT size;

    _st->stCur = nullptr;
    size = ((AlifVarObject*)_st->stStack)->size_;
    if (size) {
        if (list_setSlice(_st->stStack, size - 1, size, nullptr) < 0) return 0;
        if (--size)
            _st->stCur = (AlifSTEntryObject*)((AlifListObject*)_st->stStack)->items[size - 1];
    }
    return 1;
}

static AlifIntT symTable_enterBlock(AlifSymTable* _symTable, AlifObject* _name,
    AlifBlockType _block, void* _ast, AlifIntT _lineNo, AlifIntT _colOffset,
    AlifIntT _endLineNo, AlifIntT _endColOffset) { // 1320

    AlifSTEntryObject* prev_ = nullptr;
    AlifSTEntryObject* symTableEnter;

    symTableEnter = symTableEntry_new(_symTable, _name, _block, _ast, _lineNo, _colOffset, _endLineNo, _endColOffset);
    if (symTableEntry_new == nullptr) return 0;

    if (alifList_append(_symTable->stStack, (AlifObject*)symTableEnter) < 0) {
        ALIF_DECREF(symTableEntry_new);
        return 0;
    }
    prev_ = _symTable->stCur;

    if (prev_) symTableEnter->steCompIterExpr = prev_->steCompIterExpr;
  
    ALIF_DECREF(symTableEnter);
    _symTable->stCur = symTableEnter;

    if (_block == ModuleBlock)
        _symTable->stGlobal = _symTable->stCur->steSymbols;

    if (prev_) {
        if (alifList_append(prev_->steChildren, (AlifObject*)symTableEnter) < 0) {
            return 0;
        }
    }
    return 1;
}


static class AlifSymTable* symTable_new() { // 363
    class AlifSymTable* symTable;

    symTable = (class AlifSymTable*)alifMem_objAlloc(sizeof(class AlifSymTable));
    if (symTable == nullptr) return nullptr;

    symTable->stFileName = nullptr;
    symTable->stBlocks = nullptr;

    if ((symTable->stStack = alifNew_list(0)) == nullptr) goto fail;
    if ((symTable->stBlocks = alifNew_dict()) == nullptr) goto fail;
    symTable->stCur = nullptr;
    symTable->stPrivate = nullptr;

    return symTable;
    
fail:
    //alifSymtable_free(symTable);
    return nullptr;
}


class AlifSymTable* alifSymTable_build(Module* _module, AlifObject* _fn) { // 390
    AlifSymTable* symTable = symTable_new();
    StmtSeq* seq_;
    AlifIntT i_;
    AlifThread* thread_;
    AlifIntT startingRecursionDepth{};

    if (symTable == nullptr) return nullptr;
    if (_fn == nullptr) {
        //alifSubSymtable_free(sT);
        return nullptr;
    }
    symTable->stFileName = ALIF_NEWREF(_fn);

    thread_ = alifThread_get();
    if (!thread_) {
        //symTable_free(symTable);
        return nullptr;
    }
    AlifIntT recursionDepth = ALIFCPP_RECURSION_LIMIT - thread_->cppRecursionRemaining;
    startingRecursionDepth = recursionDepth;
    symTable->recursionDepth = startingRecursionDepth;
    symTable->recursionLimit = ALIFCPP_RECURSION_LIMIT;


    AlifObject* className = alifUStr_decodeStringToUTF8(L"top");
    if (!symTable_enterBlock(symTable, className, ModuleBlock, (void*)_module, 0, 0, 0, 0)) {
        //alifSymTable_free(symTable);
        return nullptr;
    }

    symTable->stTop = symTable->stCur;
    switch (_module->type) {
    case ModuleK:
        seq_ = _module->V.module.body;
        for (i_ = 0; i_ < SEQ_LEN(seq_); i_++)
            if (!symTable_visitStmt(symTable, (Statement*)SEQ_GET(seq_, i_)))
                goto error;
        break;
    case ExpressionK:
        if (!symTable_visitExpr(symTable, _module->V.expression.body))
            goto error;
        break;
    case InteractiveK:
        seq_ = _module->V.interactive.body;
        for (i_ = 0; i_ < SEQ_LEN(seq_); i_++)
            if (!symTable_visitStmt(symTable,
                (Statement*)SEQ_GET(seq_, i_)))
                goto error;
        break;
    case FunctionK:
        goto error;
    }
    if (!symTable_exitBlock(symTable)) {
        //alifSymtable_free(symTable);
        return nullptr;
    }
    if (symTable->recursionDepth != startingRecursionDepth) {
        //alifSymtable_free(symTable);
        return nullptr;
    }
    if (symTable_analyze(symTable)) {
//#if ALIF_DUMP_SYMTABLE
//        dump_symTable(symTable->stTop);
//#endif
        return symTable;
    }
    //alifSymtable_free(symTable);
    return nullptr;
error:
    (void)symTable_exitBlock(symTable);
    //alifSymtable_free(symTable);
    return nullptr;
}