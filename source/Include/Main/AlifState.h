#pragma once


class AlifThread {
public:
	AlifThread* prev{};
	AlifThread* next{};
	class AlifInterpreter* interpreter{};

};