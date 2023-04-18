#pragma once

#include "Types.h"

#include <iostream>
#include <vector>

class Container;
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

        class Number 
        {
        public:
            TokensType numberType;
            double64_t numberValue;
        }NumberObj;

        class : public Number
        {
        public:
            wcstr* boolType;
        }BoolObj;

        class 
        {
        public:
            wcstr* strValue;
        }StringObj;

        class 
        {
        public:
            wcstr* name_;
        }NameObj;

        class 
        {
        public:
            std::vector<ExprNode*>* list_;
            std::vector<AlifObject*>* objList;
        }ListObj;

        class
        {
        public:
            Container* container_;
        }ContainerObj;

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
