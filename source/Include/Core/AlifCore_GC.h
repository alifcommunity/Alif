#pragma once

class AlifGCHead {
public:
	AlifUSizeT gcNext{};
	AlifUSizeT gcPrev{};
};


static inline AlifGCHead* alif_asGC(AlifObject* _gc) {
	char* gc = ((char*)_gc) - sizeof(AlifGCHead);
	return (AlifGCHead*)gc;
}


#define NUM_GENERATIONS 3


class GCGeneration {
public:
	AlifGCHead head{};
	AlifIntT threshold{};
	AlifIntT count{};
};

class GCGenerationStats {
public:
	AlifSizeT collections{};
	AlifSizeT collected{};
	AlifSizeT uncollectable{};
};

class AlifGCDureRun {
public:
	AlifObject* trashDeleteLater{};
	AlifIntT trashDeleteNesting{};

	AlifIntT enabled{};

	GCGeneration young{};
	GCGeneration old[2];

	GCGeneration permanentGeneration{};
	GCGenerationStats generationStats[NUM_GENERATIONS]{};

	AlifIntT collecting{};
	AlifObject* garbage{};
	AlifObject* callbacks{};
	AlifSizeT heapSize{};
	AlifSizeT workToDo{};
	AlifIntT visitiedSpace{};
};