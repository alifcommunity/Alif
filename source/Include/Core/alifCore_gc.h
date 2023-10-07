#pragma once










class AlifGCHead {
public:

	uintptr_t gCNext;



	uintptr_t gCPrev;
};

#define ALIFGC_GEAD_UNUSED AlifGCHead






















































































#define NUM_GENERATIONS 3









































class GCGeneration {
public:
	AlifGCHead head;
	int threshold; 
	int count;
};


class GCGenerationStats {
public:
	AlifSizeT collections;

	AlifSizeT collected;

	AlifSizeT uncollectable;
};

class GCRuntimeState {
public:

	AlifObject* trashDeleteLater;

	int trashDeleteNesting;


	int enabled;
	int debug;

	class GCGeneration generations[NUM_GENERATIONS];
	AlifGCHead* generation0;

	class GCGeneration permanentGeneration;
	class GCGenerationStats generationStats[NUM_GENERATIONS];

	int collecting;

	AlifObject* garbage;

	AlifObject* callbacks;






	AlifSizeT longLivedTotal;



	AlifSizeT longLivedPending;
};


extern void alifGC_initState(class GCRuntimeState*);
