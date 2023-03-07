#pragma once

#include <iostream>
#include <vector>

#include "Types.h"
#include "Values.h"

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

        class {
        public:
            ExprNode* nodeValue;
        }ExprNodeObj;

        class
        {
        public:
            BuildInFuncValue buildInFunc;
        }BuildInFuncObj;

    }V;

};



//struct AlifObj
//{
    //TokType type_;

    //union UObj
    //{
        //struct {

        //    KeywordType kind_;

        //}None;

//        struct Boolean_ {
//
//            KeywordType Kkind_;
//            NUM value_;
//
//            void or_(AlifObj* _other)
//            {
//                this->value_ = this->value_ or _other->A.Boolean.value_;
//            }
//
//            void and_(AlifObj* _other)
//            {
//                this->value_ = this->value_ and _other->A.Boolean.value_;
//            }
//
//            void not_()
//            {
//                this->value_ = not this->value_;
//            }
//
//        }Boolean;
//
//        struct : Boolean_ {
//
//            TokType Tkind_;
//
//            void add_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ + _other->A.Number.value_;
//                }
//                else {
//                    prnt(L"خطأ في عملية الجمع");
//                }
//            }
//
//            void sub_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ - _other->A.Number.value_;
//                }
//                else {
//                    prnt(L"خطأ في عملية الطرح");
//                }
//            }
//
//            void mul_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ * _other->A.Number.value_;
//                }
//                else {
//                    prnt(L"خطأ في عملية الضرب");
//                }
//            }
//
//            void div_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    if (_other->A.Number.value_ != 0)
//                    {
//                        this->value_ = this->value_ / _other->A.Number.value_;
//                    }
//                    else
//                    {
//                        prnt(L"لا يمكن التقسيم على صفر");
//                    }
//                }
//                else {
//                    prnt(L"خطأ في عملية التقسيم");
//                }
//            }
//
//            void rem_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber and _other->A.Number.Tkind_ == TTinteger)
//                {
//                    this->value_ = (int)this->value_ % (int)_other->A.Number.value_;
//                }
//                else {
//                    prnt(L"خطأ في عملية باقي القسمة");
//                }
//            }
//
//            void pow_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = std::pow(this->value_, _other->A.Number.value_);
//                }
//                else
//                {
//                    prnt(L"خطأ في عملية الاس");
//                }
//            }
//
//            void equalE_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ == _other->A.Number.value_;
//                }
//                else
//                {
//                    prnt(L"خطأ في عملية المقارنة التساوي");
//                }
//            }
//
//            void notE_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ != _other->A.Number.value_;
//                }
//                else
//                {
//                    prnt(L"خطأ في عملية المقارنة لا يساوي");
//                }
//            }
//
//            void greaterT_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ > _other->A.Number.value_;
//                }
//                else
//                {
//                    prnt(L"خطأ في عملية المقارنة اكبر من");
//                }
//            }
//
//            void lessT_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ < _other->A.Number.value_;
//                }
//                else
//                {
//                    prnt(L"خطأ في عملية المقارنة اصغر من");
//                }
//            }
//
//            void greaterTE_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ >= _other->A.Number.value_;
//                }
//                else
//                {
//                    prnt(L"خطأ في عملية المقارنة اكبر من او يساوي");
//                }
//            }
//
//            void lessTE_(AlifObj* _other)
//            {
//                if (_other->type_ == TTnumber)
//                {
//                    this->value_ = this->value_ <= _other->A.Number.value_;
//                }
//                else
//                {
//                    prnt(L"خطأ في عملية المقارنة اصغر من او يساوي");
//                }
//            }
//
//        }Number;
//
//        struct {
//            STR* value_;
//
//            void add_(AlifObj* _other)
//            {
//                if (_other->type_ == TTstring)
//                {
//                    *this->value_ = *this->value_ + *_other->A.String.value_;
//                }
//                else {
//                    prnt(L"خطأ في عملية جمع نص");
//                }
//            }
//        }String;
//
//        struct {
//            NUM name_;
//            //Context ctx_;
//        }Name;
//
//        struct {
//            ExprNode* node_;
//        }ExprNodes;
//
//        struct {
//            std::vector<ExprNode*>* list_;
//            std::vector<AlifObj>* objList;
//
//            void add_element(AlifObj _obj) {
//                objList->push_back(_obj);
//            }
//
//        }List;
//
//        struct
//        {
//            BuildInFuncType buildInFunc;
//        }BuildInFunc;
//
//    }A;
//};
