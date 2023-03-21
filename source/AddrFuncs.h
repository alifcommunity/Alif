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

void add_str();

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

	add_str,
};