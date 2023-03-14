#pragma once

typedef void(*Instructions_func)();

void none_();
void get_data();
void set_data();
void num_add();
void num_minus();
void str_add();

const Instructions_func instr_funcs[] = {
	none_,
	get_data,
	set_data,
	num_add,
	num_minus,
	str_add,
};