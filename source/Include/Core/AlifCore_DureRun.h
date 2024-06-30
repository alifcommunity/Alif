#pragma once

#include "AlifCore_Interpreter.h"
#include "AlifCore_AlifThread.h"
#include "AlifCore_Import.h"
#include "alifCore_UString.h"


class {
public:
	uint64_t filename;
	uint64_t name;
	uint64_t lineTable;
	uint64_t firstLineNo;
	uint64_t argCount;
	uint64_t localsPlusNames;
	uint64_t localsPlusKinds;
	uint64_t codeAdaptive;
} codeObject;

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

	class TypesRuntimeState types;

	AlifStaticObjects staticObjects{};

	AlifInterpreter mainInterpreter{};
};

extern AlifDureRun _alifDureRun_;

extern AlifIntT alifDureRunState_init(AlifDureRun*);

extern AlifIntT alifDureRun_initialize();
