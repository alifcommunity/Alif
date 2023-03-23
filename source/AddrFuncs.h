#pragma once

typedef void(*Instructions_func)();

void none_();

void get_data();
void set_data();

void add_num();
void minus_num();
void mul_num();
void div_num();
void rem_num();
void pow_num();

void equal_equal();
void not_equal();
void gr_than_num();
void gr_than_eq_num();
void ls_than_num();
void ls_than_eq_num();

void add_str();
void mul_str();

const Instructions_func instr_funcs[] = {
	none_,

	get_data,
	set_data,
	
	add_num,
	minus_num,
	mul_num,
	div_num,
	rem_num,
	pow_num,

	equal_equal,
	not_equal,
	gr_than_num,
	gr_than_eq_num,
	ls_than_num,
	ls_than_eq_num,

	add_str,
	mul_str,
};