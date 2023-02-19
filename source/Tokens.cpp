#include "Tokens.h"


class Token {
public:
    TokType type_{};
    unsigned int tokStart{}, tokEnd{}, tokIndex{};
    //Position positionStart{}, positionEnd{};

    union Values
    {
        KeywordType keywordType;
        BuildInFuncType buildInFunc;
        STR* strVal;
        NUM numVal;
    }val{};

    Token() {}

    Token(Position _positionStart, Position _positionEnd, TokType _type) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
    }

    Token(Position _positionStart, Position _positionEnd, TokType _type, STR* _strVal) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
        this->val.strVal = _strVal;
    }

    Token(Position _positionStart, Position _positionEnd, TokType _type, NUM _numVal) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
        this->val.numVal = _numVal;
    }

    Token(Position _positionStart, Position _positionEnd, TokType _type, KeywordType _keywordType) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
        this->val.keywordType = _keywordType;
    }

    Token(Position _positionStart, Position _positionEnd, TokType _type, BuildInFuncType _buildInFunc) {
        this->positionStart = _positionStart;
        this->positionEnd = _positionEnd;
        this->type_ = _type;
        this->val.buildInFunc = _buildInFunc;
    }
};