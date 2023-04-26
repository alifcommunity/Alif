#pragma once

typedef void(*Instructions_func)();

void none_();

void get_data();
void set_data();

void plus_num();
void minus_num();
void add_num();
void sub_num();
void mul_num();
void div_num();
void rem_num();
void pow_num();
void augAdd_num();
void augSub_num();
void augMul_num();
void augDiv_num();
void augRem_num();
void augPow_num();

void equal_equal();
void not_equal();
void gr_than_num();
void gr_than_eq_num();
void ls_than_num();
void ls_than_eq_num();

void not_logic();
void and_logic();
void or_logic();

void add_str();
void mul_str();

void expr_op();

void store_name();

void list_make();


void jump_to();
void jump_if();
void jump_for();

void get_scope();
void call_name();
void return_expr();




void print_func();

const Instructions_func instr_funcs[] = {
	none_,

	get_data,
	set_data,
	
	plus_num,
	minus_num,
	add_num,
	sub_num,
	mul_num,
	div_num,
	rem_num,
	pow_num,
	augAdd_num,
	augSub_num,
	augMul_num,
	augDiv_num,
	augRem_num,
	augPow_num,

	equal_equal,
	not_equal,
	gr_than_num,
	gr_than_eq_num,
	ls_than_num,
	ls_than_eq_num,

	not_logic,
	and_logic,
	or_logic,

	add_str,
	mul_str,

	expr_op,

	store_name,

	list_make,

	jump_to,
	jump_if,
	jump_for,

	get_scope,
	call_name,
	return_expr,




	// الدوال الضمنية
	print_func,

};







// دوال الطباعة
typedef void(*print_funcs)();

void num_print();

const print_funcs print_types[] = {
	num_print,

};