#pragma once

#include "AlifCore_CondVar.h"









struct GILRuntimeState { // 22
	AlifIntT enabled{};

	unsigned long interval{};
	AlifThread* lastHolder{};
	AlifIntT locked{};
	unsigned long switchNumber{};
	AlifCondT cond{};
	AlifMutexT mutex{};

	AlifCondT switchCond{};
	AlifMutexT switchMutex{};
};
