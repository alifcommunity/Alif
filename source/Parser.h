#pragma once

#define Next_Is(t) (this->tokens->at(this->tokenIndex + 1).type_ == t ? true : false )

#include "Tokens.h"
#include "Node.h"
#include "SymbolTable.h"

SymbolTable symTable; // تم تعريفه ك متغير عام لمنع حذف المتغيرات عند استخدام الطرفية بعد الانتقال الى سطر جديد

class Parser {
public:
    std::vector<Token>* tokens;
    int tokenIndex = -1;
    Token currentToken;
    wstr fileName;
    wstr input_;

    /// <اعلام>

    bool lastParam = false;
    bool returnFlag = false;

    /// </اعلام>

    uint16_t exprlevel = 4000;
    uint16_t stmtslevel = 1000;

    ExprNode* exprNode = (ExprNode*)malloc(exprlevel * sizeof(struct ExprNode));
    StmtsNode* stmtsNode = (StmtsNode*)malloc(stmtslevel * sizeof(struct StmtsNode));

    Parser(std::vector<Token>* tokens, wstr _fileName, wstr _input);

    void advance();

    void parse_file();

    void parse_terminal();

    wstr lst;
    void list_print(AlifObject _obj);

    //////////////////////////////

    std::vector<ExprNode*>* arguments();

    std::vector<ExprNode*>* parameters();

    ExprNode* atom();

    ExprNode* list_expr();

    ExprNode* primary();

    ExprNode* power();

    ExprNode* factor();

    ExprNode* term();

    ExprNode* sum();

    ExprNode* comparesion();

    ExprNode* inversion();

    ExprNode* conjuction();

    ExprNode* disjuction();

    ExprNode* expression();

    ExprNode* expressions();

    ExprNode* assignment();

    StmtsNode* return_statement();

    StmtsNode* function_def();

    StmtsNode* class_def();

    StmtsNode* while_statement();

    StmtsNode* for_statement();

    StmtsNode* else_if();

    StmtsNode* else_();

    StmtsNode* if_statement();

    StmtsNode* block_();

    //void import_from();

    //void import_name();

    //void import_statement();

    //void delete_statement();

    StmtsNode* compound_statement();

    ExprNode* simple_statement();

    StmtsNode* statement();

    StmtsNode* statements();
};