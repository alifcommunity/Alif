#pragma once

#include <map>

#define PRINT_(a){std::wcout << a << std::endl;}
using double64_t = long double;

class Scope
{
public:
    std::map<double64_t, AlifObject> symbols;
    std::map<double64_t, Scope*> scopes;
    Scope* parent;
};

class SymbolTable {

    Scope* currentScope;
    Scope* globalScope;

public:
    SymbolTable() {
        globalScope = new Scope();
        globalScope->parent = nullptr;
        currentScope = globalScope;
    }

    void add_symbol(double64_t type, AlifObject value) {
        currentScope->symbols[type] = value;
    }

    //void add_value(double64_t _key, double64_t _value) {
    //    currentScope->symbols[_key].V.NumberObj.numberValue = _value;
    //}

    void enter_scope(double64_t type) {
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

    AlifObject get_data(double64_t type) {
        Scope* scope = currentScope;
        int level = 1;
        while (scope != nullptr)
        {
            if (scope->symbols.count(type) != 0)
            {
                return scope->symbols[type];
            }
            if (level == 0) {
                break;
            }
            scope = scope->parent;
            level--;
        }
        PRINT_(L"المتغير المستدعى غير معرف");
        exit(-1);
    }

};