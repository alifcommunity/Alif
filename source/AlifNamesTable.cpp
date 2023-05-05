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



wcstr* is_repeated(wcstr* _name) /* يقوم بالتحقق ما اذا كان نفس الاسم قد تم تعريفه مسبقاً
                                    ففي حال كان الاسم معرف يتم استخدام عنوان الاسم القديم
                                    وإلا يتم استخدام الاسم الجديد */
{
    uint32_t size = names_.size();
    for (uint32_t i = 0; i < size; i++)
    {
        wcstr* a = names_.get(i);
        if (_name == a) // اذا اول حرف من الاسم يساوي اول حرف من الاسم الاخر
        {
            if (!wcscmp(_name, a)) // اذا كان الاسمان متطابقان
            {
                delete[] _name;
                return a;
            }
        }
    }
    return _name;
}