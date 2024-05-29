#pragma once


class AlifThread {
public:
	AlifThread* prev{};
	AlifThread* next{};
	class AlifInterpreter* interpreter{};




	AlifIntT cppRecursionRemaining{};
};




#define ALIFCPP_RECURSION_LIMIT 10000
