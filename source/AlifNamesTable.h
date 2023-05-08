#pragma once

#include "AlifObject.h"
#include "AlifArray.h"
#include "Node.h"
#include <map>

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
#define Is_Scope_Exist(scope, name) ((scope)->scopes.count(name) != 0)
#define Is_Name_Exist(scope, name) (scope->names->count(name) != 0)
#define Is_Global_Scope(scope) (scope->parent == nullptr)
#define Get_Value(scope,name) ((scope)->names->at(name))
#define Get_Scope(scope,name) ((scope)->scopes.at(name))

#define VISIT_(func,node,nT) (visit_ ## func(node, nT)) // -> visit_func(arg) <<-->> VISIT_(func, node)


class Scope
{
public:
    std::map<wcstr*, AlifObject*>* names; // فهرس يحتوي على الاسماء وقيمها
    std::map<wcstr*, Scope*> scopes; // فهرس يحتوي على الاسماء ونطاقاتها
    Scope* parent{ nullptr }; // كـ مصفوفة متصلة - حيث parent يشير الى النطاق التالي -
};

class AlifNamesTable 
{
    Scope* currentScope;
    Scope* globalScope;


public:
    uint16_t depth_recursion = 0;
    uint16_t max_recursion = 9;


    AlifNamesTable(); // يقوم بتهيئة النطاق العام

    void create_name(wcstr* _name, AlifObject* _value); // يقوم بإنشاء اسم جديد ويسند له قيمة
    bool assign_name(wcstr* _name, AlifObject* _value); // إذا كان الاسم موجود يقوم بإسناده
    void create_scope(wcstr* _name); // يقوم بإنشاء نطاق جديد
    bool enter_scope(wcstr* _name); // إذا كان الاسم موجود يدخل في نطاقه
    void copy_scope(wcstr* _name);
    bool exit_scope(); // يرجع خطوة الى الوراء في مجال النطاقات
    AlifObject* get_object(wcstr* _name); // إذا كان الاسم موجود يقوم بإرجاع قيمته
};



void table_names_prepare(std::vector<StmtsNode*>*, AlifNamesTable*);
void table_call_prepare(std::vector<StmtsNode*>*, AlifNamesTable*);

void visit_assign(ExprNode*, AlifNamesTable*);

void visit_for_(StmtsNode*, AlifNamesTable*);
void visit_while_(StmtsNode*, AlifNamesTable*);
void visit_if_(StmtsNode*, AlifNamesTable*);
void visit_function(StmtsNode*, AlifNamesTable*);
void visit_class_(StmtsNode*, AlifNamesTable*);

void visit_exprs(ExprNode* _node, AlifNamesTable*);
void visit_stmts(StmtsNode* _node, AlifNamesTable*);



void visit_call(ExprNode*, AlifNamesTable*);
void visit_function_call(StmtsNode*, AlifNamesTable*);
void visit_class_call(StmtsNode*, AlifNamesTable*);

void visit_exprs_call(ExprNode* _node, AlifNamesTable*);
void visit_stmts_call(StmtsNode* _node, AlifNamesTable*);



static AlifArray<wcstr*> names_{};
wcstr* is_repeated(wcstr*);