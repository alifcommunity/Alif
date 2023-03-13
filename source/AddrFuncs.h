#pragma once

typedef void(*Instructions_func)();

void none_();
void bring_name();
void send_name();
void add_op();
void minus_op();

const Instructions_func instr_funcs[] = {
	none_,
	bring_name,
	send_name,
	add_op,
	minus_op,
};