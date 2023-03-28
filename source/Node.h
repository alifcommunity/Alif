#pragma once

#include "Object.h"

class ExprNode
{
public:
    VisitType type_{};

    union UExprNode
    {
        class {
        public:
            AlifObject* value_;
        }Object;

        class {
        public:
            ExprNode* left_;
            TokensType operator_;
            wcstr* keyword_;
            ExprNode* right_;
        }BinaryOp;

        class {
        public:
            ExprNode* right_;
            TokensType operator_;
            wcstr* keyword_;
        }UnaryOp;

        class {
        public:
            AlifObject paramName; // متغير خاص بدالة parameters()
            std::vector<AlifObject*>* name_;
            ExprNode* value_;
        }NameAssign;

        class {
        public:
            AlifObject* name_;
            TokensType operator_;
            ExprNode* value_;
        }AugNameAssign;

        class {
        public:
            AlifObject* name_;
        }NameAccess;

        class {
        public:
            ExprNode* func_;
            ExprNode* name_;
            std::vector<ExprNode*>* args_;
        }Call;

        class {
        public:
            ExprNode* expr_;
            ExprNode* condetion_;
            ExprNode* elseExpr;
        }Expr;

    }U{};
};

class StmtsNode {
public:
    VisitType type_{};

    union UStmtsNode
    {
        class {
        public:
            ExprNode* expr_;
        }Expr;

        class {
        public:
            ExprNode* condetion_;
            StmtsNode* block_;
            std::vector<StmtsNode*>* elseIf;
            StmtsNode* else_;
        }If;

        class {
        public:
            AlifObject* itrName;
            std::vector<ExprNode*>* args_;
            StmtsNode* block_;
            StmtsNode* else_;
        }For;

        class {
        public:
            ExprNode* condetion_;
            StmtsNode* block_;
            StmtsNode* else_;
        }While;

        class
        {
        public:
            AlifObject* name_;
            StmtsNode* body_;
            ExprNode* base_;
        }ClassDef;

        class
        {
        public:
            AlifObject* name_;
            std::vector<ExprNode*>* params_;
            StmtsNode* body_;
        }FunctionDef;

        class
        {
        public:
            ExprNode* returnExpr;
        }Return;

        class {
        public:
            std::vector<StmtsNode*>* stmts_;
        }Stmts;
    }U{};
};