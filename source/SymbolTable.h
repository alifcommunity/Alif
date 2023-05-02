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

    void create_scope(wcstr* _type)
    {
        Scope* newScope = new Scope();
        newScope->parent = currentScope;
        currentScope->scopes[_type] = newScope;
    }

    void enter_scope(wcstr* type) 
    {
        auto it = currentScope->scopes.begin();
        for (it; it != currentScope->scopes.end(); it++)
        {
            if (!wcscmp(it->first, type)) 
            {
                currentScope = it->second;
                break;
            }
        }

        //if (currentScope->scopes.count(type) == 0) {
        //    Scope* newScope = new Scope();
        //    newScope->parent = currentScope;
        //    currentScope->scopes[type] = newScope; // يعتبر الاسم الممرر ك عنوان وليس نص **يجب إصلاح المشكلة
        //}
    }

    void copy_scope(wcstr* a, wcstr* b)
    {
        std::map<wcstr*, AlifObject*>* symbols_{};
        std::map<wcstr*, Scope*>* scopes_{};
        auto it = currentScope->scopes.begin();
        for (it; it != currentScope->scopes.end(); it++)
        {
            if (!wcscmp(it->first, a))
            {
                symbols_ = &it->second->symbols;
                scopes_ = &it->second->scopes;
                break;
            }
        }
        it = currentScope->scopes.begin();
        for (it; it != currentScope->scopes.end(); it++)
        {
            if (!wcscmp(it->first, b))
            {
                it->second->symbols = *symbols_;
                it->second->scopes = *scopes_;
                break;
            }
        }
    }

    bool is_scope(wcstr* _type)
    {
        auto it = currentScope->scopes.begin();
        for (it; it != currentScope->scopes.end(); it++)
        {
            if (!wcscmp(it->first, _type))
            {
                return true;
            }
        }
        return false;
    }

    void exit_scope() {
        currentScope = currentScope->parent;
    }

    //void backup_scope()
    //{
    //    if (currentScope and currentScope->parent)
    //    {
    //        backupScope.push(currentScope);
    //        currentScope = currentScope->parent;
    //    }
    //}

    //void restore_scope()
    //{
    //    if (!backupScope.empty())
    //    {
    //        currentScope = backupScope.top();
    //        backupScope.pop();
    //    }
    //    else
    //    {
    //        currentScope = currentScope->parent;

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