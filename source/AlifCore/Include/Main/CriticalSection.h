#pragma once






class AlifCriticalSection { // 98
public:
	uintptr_t prev{};
	AlifMutex* mutex{};
};



class AlifCriticalSection2 { // 110
public:
	AlifCriticalSection base{};
	AlifMutex* mutex2{};
};
