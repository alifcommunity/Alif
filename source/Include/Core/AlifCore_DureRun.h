#pragma once

#include "AlifCore_Interpreter.h"
#include "AlifCore_AlifThread.h"
#include "AlifCore_Import.h"
#include "alifCore_UString.h"




class AlifDureRun {
public:
	AlifIntT selfInitialized{};

	//AlifIntT preInitializing{};
	//AlifIntT preInitialized{};

	AlifIntT coreInitialized{};

	AlifIntT initialized{};

	class AlifInterpreters {
	public:
		AlifInterpreter* head{};
		AlifInterpreter* main{};
		AlifIntT nextID{};
	} interpreters;

	AlifIntT mainThreadID{};
	AlifThread* mainThread{};

	AlifIntT autoTSSKey{};
	AlifIntT trashTSSKey{};

	AlifWStringList origArgv{};

	ImportDureRun imports;


	AlifInterpreter mainInterpreter{};
};

extern AlifDureRun _alifDureRun_;

extern AlifIntT alifDureRunState_init(AlifDureRun*);

extern AlifIntT alifDureRun_initialize();
