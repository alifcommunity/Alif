#pragma once

#include <map>
#include "AlifObject.h"

//#define PRINT_(a){std::wcout << a << std::endl;}
//using wcstr = const wchar_t;
//
//class Scope
//{
//public:
//    std::map<wcstr&, AlifObject*> names;
//    std::map<wcstr&, Scope*> scopes;
//    Scope* parent;
//};
//
//class AlifNamesTable {
//
//    Scope* currentScope;
//    Scope* globalScope;
//
//public:
//    AlifNamesTable()
//    {
//        globalScope = new Scope();
//        globalScope->parent = nullptr;
//        currentScope = globalScope;
//    }
//
//    void create_name(wcstr& _type, AlifObject* _value)
//    {
//        currentScope->names.insert({ _type,_value });
//    }
//
//    void assign_name(wcstr& _type, AlifObject* _value)
//    {
//        currentScope->names.at(_type) = _value;
//    }
//
//    void create_scope(wcstr& _type)
//    {
//        Scope* newScope = new Scope();
//        newScope->parent = currentScope;
//        currentScope->scopes.at(_type) = newScope;
//    }
//
//    void enter_scope(wcstr& _type) 
//    {
//        currentScope = currentScope->scopes.at(_type);
//    }
//
//    //void copy_scope(wcstr* a, wcstr* b)
//    //{
//    //    std::map<wcstr*, AlifObject*> symbols_{};
//    //    std::map<wcstr*, Scope*> scopes_{};
//    //    auto it = currentScope->scopes.begin();
//    //    for (it; it != currentScope->scopes.end(); it++)
//    //    {
//    //        if (!wcscmp(it->first, a))
//    //        {
//    //            symbols_ = it->second->symbols;
//    //            scopes_ = it->second->scopes;
//    //            break;
//    //        }
//    //    }
//    //    it = currentScope->scopes.begin();
//    //    for (it; it != currentScope->scopes.end(); it++)
//    //    {
//    //        if (!wcscmp(it->first, b))
//    //        {
//    //            it->second->symbols = symbols_;
//    //            it->second->scopes = scopes_;
//    //            for (std::pair<wcstr*, Scope*> a : it->second->scopes)
//    //            {
//    //                a.second->parent = it->second;
//    //            }
//    //            break;
//    //        }
//    //    }
//    //}
//
//    //bool is_scope(wcstr* _type)
//    //{
//    //    auto it = currentScope->scopes.begin();
//    //    for (it; it != currentScope->scopes.end(); it++)
//    //    {
//    //        if (!wcscmp(it->first, _type))
//    //        {
//    //            return true;
//    //        }
//    //    }
//    //    return false;
//    //}
//
//    void exit_scope() {
//        currentScope = currentScope->parent;
//    }
//
//    AlifObject* get_object(wcstr& _type) {
//        Scope* scope = currentScope;
//        int level = 1;
//        while (scope != nullptr)
//        {
//            AlifObject* a = scope->names.at(_type);
//            if (a)
//            {
//                return a;
//            }
//            if (level == 0) // والا إبحث في النطاق العام
//            {
//                scope = globalScope;
//                a = scope->names.at(_type);
//                if (a)
//                {
//                    return a;
//                }
//
//                PRINT_(L" المتغير المستدعى غير معرف");
//                PRINT_((const wchar_t*)_type);
//                exit(-1);
//            }
//            scope = scope->parent;
//            level--;
//        }
//    }
//};

using wcstr = const wchar_t;


#define PRINT_(a){std::wcout << a << std::endl;}
#define Is_Scope_Exist(name) (currentScope->scopes.count(name) != 0)
#define Is_Name_Exist(name) (currentScope->names.count(name) != 0)
#define Is_Global_Scope(scope) (scope->parent == nullptr)
#define Get_Value(scope,name) ((scope)->names.at(name))
#define Get_Scope(name) (currentScope->scopes.at(name))




class Scope
{
public:
    std::map<wcstr*, AlifObject*> names;
    std::map<wcstr*, Scope*> scopes;
    Scope* parent{ nullptr };
};

class AlifNamesTable {

    Scope* currentScope;
    Scope* globalScope;

public:
    AlifNamesTable() {
        globalScope = new Scope();
        globalScope->parent = nullptr;
        currentScope = globalScope;
    }

    void create_name(wcstr* _name, AlifObject* _value)
    {
        currentScope->names.insert({ _name,_value });
    }

    bool assign_name(wcstr* _name, AlifObject* _value)
    {
        if (Is_Name_Exist(_name)) { Get_Value(currentScope, _name) = _value; return true; }
        else { return false; }
    }

    void create_scope(wcstr* _name)
    {
        Scope* newScope = new Scope();
        newScope->parent = currentScope;
        currentScope->scopes.insert({ _name,newScope });
    }

    bool enter_scope(wcstr* _name)
    {
        if (Is_Scope_Exist(_name)) { currentScope = Get_Scope(_name); return true; } // اذا كان الاسم موجود -> قم بالدخول في نطاق الاسم
        else { return false; }
    }

    bool exit_scope() {
        if (!Is_Global_Scope(currentScope)) { currentScope = currentScope->parent; return true; } // اذا كان النطاق الاب فارغ إذاً نحن في النطاق العام ولا يمكن الخروج منه
        else { return false; }
    }

    AlifObject* get_object(wcstr* _name) {
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
};