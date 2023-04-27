#pragma once

#include <map>
#include <stack>

#define PRINT_(a){std::wcout << a << std::endl;}
using wcstr = const wchar_t;

class Scope
{
public:
    std::map<wcstr*, AlifObject*> symbols;
    std::map<wcstr*, Scope*> scopes;
    Scope* parent;
};

class SymbolTable {

    Scope* currentScope;
    Scope* globalScope;
    std::stack<Scope*> backupScope;

public:
    SymbolTable() 
    {
        globalScope = new Scope();
        globalScope->parent = nullptr;
        currentScope = globalScope;
    }

    void add_symbol(wcstr* _type, AlifObject* _value)
    {
        auto itr = currentScope->symbols.begin();
        for (; itr != currentScope->symbols.end(); itr++)
        {
            if (!wcscmp(itr->first, _type))
            {
                itr->second = _value;
                return;
            }
        }

        currentScope->symbols.insert({ _type,_value });
    }

    void enter_scope(wcstr* type) 
    {
        if (!currentScope->scopes.size())
        {
            Scope* newScope = new Scope();
            newScope->parent = currentScope;
            currentScope->scopes[type] = newScope;
            goto enter;
        }
        else
        {
            auto it = currentScope->scopes.begin();
            for (it; it != currentScope->scopes.end(); it++)
            {
                if (!wcscmp(it->first, type)) 
                {
                    goto enter;
                }
            }
        }

        {
            Scope* newScope = new Scope();
            newScope->parent = currentScope;
            currentScope->scopes[type] = newScope;
        }
        //if (currentScope->scopes.count(type) == 0) {
        //    Scope* newScope = new Scope();
        //    newScope->parent = currentScope;
        //    currentScope->scopes[type] = newScope;
        //}
    enter:
        auto it = currentScope->scopes.begin();
        for (it; it != currentScope->scopes.end(); it++)
        {
            if (!wcscmp(it->first, type))
            {
                currentScope = it->second; // يعتبر الاسم الممرر ك عنوان وليس نص **يجب إصلاح المشكلة
                break;
            }
        }
    }

    void exit_scope() {
        currentScope = currentScope->parent;
    }

    void backup_scope()
    {
        if (currentScope and currentScope->parent)
        {
            backupScope.push(currentScope);
            currentScope = currentScope->parent;
        }
    }

    void restore_scope()
    {
        if (!backupScope.empty())
        {
            currentScope = backupScope.top();
            backupScope.pop();
        }
        else
        {
            currentScope = currentScope->parent;

        }
    }
    //void exit_scope_if() // تستخدم في استدعاء دالة في المفسر فقط
    //{ 
    //    if (currentScope->parent and !backupScope)
    //    {
    //        currentScope = currentScope->parent;
    //    }
    //    else if (backupScope)
    //    {
    //        currentScope = backupScope;
    //        backupScope = nullptr;
    //    }
    //    else
    //    {
    //        backupScope = currentScope;
    //    }
    //}

    AlifObject* get_data(wcstr* type) {
        Scope* scope = currentScope;
        int level = 1;
        while (scope != nullptr)
        {
            auto it = scope->symbols.begin();
            for (it; it != scope->symbols.end(); it++)
            {
                if (!wcscmp(it->first, type))
                {
                    return it->second;
                }
            }
            //if (scope->symbols.count(type) != 0)
            //{
            //    return scope->symbols[type];
            //}
            if (level == 0) {
                PRINT_(L"المتغير المستدعى غير معرف");
                exit(-1);
            }
            scope = scope->parent;
            level--;
        }
    }
};