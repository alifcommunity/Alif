#pragma once

#include <iostream>
#include <vector>

#include "Types.h"

class ExprNode; 

using wstr = std::wstring;
using wcstr = const wchar_t;
using double64_t = long double;

class AlifObject {
public:
    ObjectType objType{};
    uint32_t posStart{}, posEnd{}, tokLine{}, posIndex{};

    union UObj
    {
        //class {
        //public:
        //    KeywordValue noneValue;
        //}NoneObj;

        class Number{
        public:
            TokensType numberType;
            double64_t numberValue;
        }NumberObj;

        class : Number{
        public:
            wcstr* boolType;
        }BoolObj;

        class {
        public:
            wcstr* strValue;
        }StringObj;

        class {
        public:
            wcstr* name_;
            StateType state_;
        }NameObj;

        class {
        public:
            std::vector<ExprNode*>* list_;
            std::vector<AlifObject*>* objList;
        }ListObj;

        //class {
        //public:
        //    ExprNode* nodeValue;
        //}ExprNodeObj;

        //class {
        //public:
        //    BuildInFuncValue buildInFunc;
        //}BuildInFuncObj;

    }V{};

};
