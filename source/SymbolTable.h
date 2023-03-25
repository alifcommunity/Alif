#pragma once

#include <map>

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

public:
    SymbolTable() 
    {
        globalScope = new Scope();
        globalScope->parent = nullptr;
        currentScope = globalScope;
    }

    void add_symbol(wcstr* _type, AlifObject* _value) {
        currentScope->symbols[_type] = _value;
    }

    void enter_scope(wcstr* type) {
        if (currentScope->scopes.count(type) == 0) {
            Scope* newScope = new Scope();
            newScope->parent = currentScope;
            currentScope->scopes[type] = newScope;
        }
        currentScope = currentScope->scopes[type];
    }

    void exit_scope() {
        currentScope = currentScope->parent;
    }

    AlifObject* get_data(wcstr* type) {
        Scope* scope = currentScope;
        int level = 1;
        while (scope != nullptr)
        {
            if (scope->symbols.count(type) != 0)
            {
                return scope->symbols[type];
            }
            if (level == 0) {
                PRINT_(L"المتغير المستدعى غير معرف");
                exit(-1);
            }
            scope = scope->parent;
            level--;
        }
    }
};