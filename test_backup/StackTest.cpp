#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <map>
#include <chrono>

//using namespace std;
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
    std::map<wcstr*, int*> names;
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

    void create_name(wcstr* _name, int* _value)
    {
        currentScope->names.insert({ _name,_value });
    }

    bool assign_name(wcstr* _name, int* _value)
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

    int* get_object(wcstr* _name) {
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


int main()
{
    bool outWText = _setmode(_fileno(stdout), _O_WTEXT);
    bool inWText = _setmode(_fileno(stdin), _O_WTEXT);



    AlifNamesTable* ant = new AlifNamesTable();

    wcstr* a = new wcstr(*L"اسم");

    int* b[9] = {new int(1),new int(2),new int(3),new int(4),new int(5),new int(6),new int(7),new int(8),new int(9) };
    ant->create_name(a, b[0]);
    ant->assign_name(a, b[1]);

    ant->exit_scope();

    ant->create_scope(L"هلا");
    ant->enter_scope(L"هلا");

    ant->create_name(L"اسم", nullptr);
    ant->assign_name(L"اسم", b[3]);

    int* c = ant->get_object(L"اسم");


    PRINT_(*c);





    int d;
    std::wcin >> d;
    return 0;
}
