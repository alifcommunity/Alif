#include "AlifNamesTable.h"

AlifNamesTable::AlifNamesTable()
{
    globalScope = new Scope();
    globalScope->parent = nullptr;
    currentScope = globalScope;
}

void AlifNamesTable::create_name(wcstr* _name, AlifObject* _value)
{
    currentScope->names.insert({ _name,_value });
}

bool AlifNamesTable::assign_name(wcstr* _name, AlifObject* _value)
{
    if (Is_Name_Exist(_name)) { Get_Value(currentScope, _name) = _value; return true; }
    else { return false; }
}

void AlifNamesTable::create_scope(wcstr* _name)
{
    Scope* newScope = new Scope();
    newScope->parent = currentScope;
    currentScope->scopes.insert({ _name,newScope });
}

bool AlifNamesTable::enter_scope(wcstr* _name)
{
    if (Is_Scope_Exist(_name)) // اذا كان الاسم موجود -> قم بالدخول في نطاق الاسم
    { 
        currentScope = Get_Scope(_name);
        depth_recursion++; 
        return true; 
    } 
    else { return false; }
}

bool AlifNamesTable::exit_scope() 
{
    if (!Is_Global_Scope(currentScope)) // اذا كان النطاق الاب فارغ إذاً نحن في النطاق العام ولا يمكن الخروج منه
    {
        currentScope = currentScope->parent;
        depth_recursion--;
        return true; 
    } 
    else { return false; }
}

AlifObject* AlifNamesTable::get_object(wcstr* _name) {
    Scope* scope = currentScope;
    int level = 1;
    while (scope)
    {
        if (Is_Name_Exist(_name))
        {
            return Get_Value(scope, _name);
        }
        if (level == 0) // والا إبحث في النطاق العام
        {
            scope = globalScope;
            if (Is_Name_Exist(_name))
            {
                return Get_Value(scope, _name);
            }

            PRINT_(L" المتغير المستدعى غير معرف");
            PRINT_((const wchar_t*)_name);
            exit(-1);
        }
        scope = scope->parent;
        level--;
    }
    return nullptr;
}




void table_prepare(std::vector<StmtsNode*>* _statements, AlifNamesTable* _namesTable)
{
    for (StmtsNode* node : *_statements)
    {
        VISIT_(stmts, node, _namesTable);
    }
}


void visit_assign(ExprNode* _node, AlifNamesTable* _namesTable)
{
    for (AlifObject* name : *_node->U.NameAssign.name_)
    {
        _namesTable->create_name(name->V.NameObj.name_, nullptr);
    }
}
void visit_expr(ExprNode* _node, AlifNamesTable* _namesTable)
{

}

void visit_for_(StmtsNode* _node, AlifNamesTable* _namesTable)
{

}
void visit_while_(StmtsNode* _node, AlifNamesTable* _namesTable)
{

}
void visit_if_(StmtsNode* _node, AlifNamesTable* _namesTable)
{

}
void visit_function(StmtsNode* _node, AlifNamesTable* _namesTable)
{

}
void visit_class_(StmtsNode* _node, AlifNamesTable* _namesTable)
{

}


void visit_exprs(ExprNode* _node, AlifNamesTable* _namesTable)
{
    if (_node->type_ == VTAssign)
    {
        VISIT_(assign, _node, _namesTable);
    }
    //else if (_node->type_ == VTAccess) // يمكن ان تستخدم لمعرفة ما إذا كان الاسم تم استخدامه ام انه لم يستخدم وبالتالي يتم حذفه في النهاية لانه لا فائدة منه
    else if (_node->type_ == VTCondExpr)
    {
        VISIT_(expr, _node, _namesTable);
    }
}

void visit_stmts(StmtsNode* _node, AlifNamesTable* _namesTable)
{
    if (_node->type_ == VTExpr)
    {
        VISIT_(exprs, _node->U.Expr.expr_, _namesTable);
    }
    else if (_node->type_ == VTFor)
    {
        VISIT_(for_, _node, _namesTable);
    }
    else if (_node->type_ == VTWhile)
    {
        VISIT_(while_, _node, _namesTable);
    }
    else if (_node->type_ == VTIf)
    {
        VISIT_(if_, _node, _namesTable);
    }
    else if (_node->type_ == VTFunction)
    {
        VISIT_(function, _node, _namesTable);
    }
    else if (_node->type_ == VTClass)
    {
        VISIT_(class_, _node, _namesTable);
    }
    else if (_node->type_ == VTStmts)
    {
        for (StmtsNode* stmt : *_node->U.Stmts.stmts_)
        {
            VISIT_(stmts, stmt, _namesTable);
        }
    }
}





wcstr* is_repeated(wcstr* _name) /* يقوم بالتحقق ما اذا كان نفس الاسم قد تم تعريفه مسبقاً
                                    ففي حال كان الاسم معرف يتم استخدام عنوان الاسم القديم
                                    وإلا يتم استخدام الاسم الجديد */
{
    uint32_t size = names_.size();
    for (uint32_t i = 0; i < size; i++)
    {
        wcstr* a = names_.get(i);
        if (*_name == *a) // اذا اول حرف من الاسم يساوي اول حرف من الاسم الاخر
        {
            if (!wcscmp(_name, a)) // اذا كان الاسمان متطابقان
            {
                delete[] _name;
                return a;
            }
        }
    }
    names_.push_back(_name);
    return _name;
}