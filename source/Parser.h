#pragma once

#include "Error.h"
#include "Tokens.h"
#include "Node.h"
#include "AlifMemory.h"
#include "AlifArray.h"
#include <string>

#define Next_Is(t) (this->tokens_->at(this->tokenIndex + 1).type_ == t ? true : false )
#define PRINT_(a){std::wcout << a << std::endl;}


class Parser {
    std::vector<Token>* tokens_;
    int tokenIndex = -1;
    Token currentToken;
    wstr fileName;
    wstr* input_;
public:
    std::vector<StmtsNode*> statements_;
    AlifMemory* alifMemory;

    /// <اعلام>

    bool lastParam = false;
    bool returnFlag = false;

    /// </اعلام>

    Parser(std::vector<Token>* tokens, wstr _fileName, wstr* _input, AlifMemory* _alifMemory);
    
    void advance();

    void parse_file();

    void parse_terminal();

    //////////////////////////////

    std::vector<ExprNode*>* arguments();

    std::vector<ExprNode*>* parameters();

    ExprNode* atom_statement(); // يجب جعل جميع اسماء الحالات تنتهي ب statement

    ExprNode* list_statement();

    ExprNode* primary_statement();

    ExprNode* power_statement();

    ExprNode* factor_statement();

    ExprNode* term_statement();

    ExprNode* sum_statement();

    ExprNode* comparesion_statement();

    ExprNode* inversion_statement();

    ExprNode* conjuction_statement();

    ExprNode* disjuction_statement();

    ExprNode* expression_statement();

    ExprNode* expressions_statement();

    ExprNode* assignment_statement();

    ExprNode* import_statement();

    StmtsNode* return_statement();

    StmtsNode* function_statement();

    StmtsNode* class_statement();

    StmtsNode* while_statement();

    StmtsNode* for_statement();

    StmtsNode* else_if_statement();

    StmtsNode* else_statement();

    StmtsNode* if_statement();

    StmtsNode* block_statement();

    //void delete_statement();

    StmtsNode* compound_statement();

    ExprNode* simple_statement();

    StmtsNode* statement();

    StmtsNode* statements();




    bool is_keyword(const wchar_t*);
};