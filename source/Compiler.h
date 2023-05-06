#pragma once

#include "Node.h"
#include "Types.h"
#include "AlifNamesTable.h"
#include "AlifMemory.h"
#include "Container.h"
#include "AlifArray.h"
#include "Parser.h"

#define VISIT_(func,node) (visit_ ## func(node)) // -> visit_func(arg) <<-->> VISIT_(func, node)
												 //			^                        ^
												 //			|                        |
												 //			|                        |
												 //    طريقة الاستدعاء           شكل الاستدعاء


//extern AlifNamesTable* symTable; // تم تعريفه ك متغير عام لمنع حذف المتغيرات عند استخدام الطرفية بعد الانتقال الى سطر جديد
//static bool returnFlag = false;

class Compiler {
public:
	std::vector<StmtsNode*>* statements_{};
	Container* dataContainer{};
	AlifArray<Container*> containers_{};
	AlifMemory* alifMemory;
	AlifNamesTable* namesTable{};

	Compiler(std::vector<StmtsNode*>* _statements, AlifMemory* _alifMemory, AlifNamesTable* _namesTable);

	void compile_file();

	AlifObject* visit_object(ExprNode*);
	AlifObject* visit_unaryOp(ExprNode*);
	AlifObject* visit_binOp(ExprNode*);
	void visit_assign(ExprNode*);
	void visit_augAssign(ExprNode*);
	AlifObject* visit_access(ExprNode*);
	void visit_expr(ExprNode*);
	void visit_list(ExprNode*);
	void visit_call(ExprNode*);
	AlifObject* visit_return_(ExprNode*);
	void visit_stop();
	void visit_continue_();

	void visit_for_(StmtsNode*);
	void visit_while_(StmtsNode*);
	void visit_if_(StmtsNode*);
	void visit_function(StmtsNode*);
	void visit_class_(StmtsNode*);

	AlifObject* visit_exprs(ExprNode* _node);
	AlifObject* visit_stmts(StmtsNode* _node);





	void visit_print();
};


