#pragma once

//#include "AlifCore_Interpreter.h"
//#include "AlifCore_AlifThread.h"
//#include "AlifCore_Import.h"
//#include "AlifCore_UString.h"


class AlifDureRun { // 159
public:
	AlifIntT selfInitialized{};

	//AlifIntT coreInitialized{};

	//AlifIntT initialized{};

	//class AlifInterpreters {
	//public:
	//	AlifInterpreter* head{};
	//	AlifInterpreter* main{};
	//	AlifIntT nextID{};
	//} interpreters;

	AlifIntT mainThreadID{}; // not working in macos it's return pthread_t
	//AlifThread* mainThread{};

	AlifTssT autoTSSKey{};
	AlifTssT trashTSSKey{};

	//AlifWStringList origArgv{};

	//ImportDureRun imports;

	//class TypesRuntimeState types;

	//AlifStaticObjects staticObjects{};

	//AlifInterpreter mainInterpreter{};
};

extern AlifDureRun _alifDureRun_; // 318

extern AlifIntT alifDureRunState_init(AlifDureRun*); // 320

extern AlifIntT alifDureRun_initialize(); // 329
