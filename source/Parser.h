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

    wstr lst_;
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

    //StmtsNode* class_def();

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




    bool is_keyword(const wchar_t*);
};