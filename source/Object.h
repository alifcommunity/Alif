#pragma once

#include <iostream>
#include <vector>

#include "Types.h"
#include "Values.h"
#include "Node.h"

class ExprNode; 
// ^
// | لا يجب الإفصاح عن الاصناف يدويا لذلك يجب مراجعة هذا

using wstr = std::wstring;
using double64_t = long double;

class AlifObject {
public:
    ObjectType objType;

    union UObj
    {
        class {
        public:
            KeywordValue noneValue;
        }NoneObj;

        class Number{
        public:
            TokensType numberType;
            double64_t numberValue;
        }NumberObj;

        class : Number{
        public:
            KeywordValue boolValue;
        }BoolObj;

        class {
        public:
            wstr* strValue;
        }StringObj;

        class {
        public:
            double64_t name_;
            StateType state_;
        }NameObj;

        class {
        public:
            std::vector<ExprNode*>* list_;
            std::vector<AlifObject>* objList;
        }ListObj;

        //class {
        //public:
        //    ExprNode* nodeValue;
        //}ExprNodeObj;

        class {
        public:
            BuildInFuncValue buildInFunc;
        }BuildInFuncObj;

    }V;

};
