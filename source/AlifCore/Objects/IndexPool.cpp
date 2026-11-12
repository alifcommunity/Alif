#include <stdbool.h>

#include "alif.h"

#include "AlifCore_IndexPool.h"
#include "AlifCore_Lock.h"




static inline void swap(int32_t *values,
	AlifSizeT i, AlifSizeT j) {
	int32_t tmp = values[i];
	values[i] = values[j];
	values[j] = tmp;
}

static bool heap_trySwap(AlifIndexHeap *heap,
	AlifSizeT i, AlifSizeT j) {
	if (i < 0 || i >= heap->size) {
		return 0;
	}
	if (j < 0 || j >= heap->size) {
		return 0;
	}
	if (i <= j) {
		if (heap->values[i] <= heap->values[j]) {
			return 0;
		}
	}
	else if (heap->values[j] <= heap->values[i]) {
		return 0;
	}
	swap(heap->values, i, j);
	return 1;
}

static inline AlifSizeT parent(AlifSizeT i) {
	return (i - 1) / 2;
}

static inline AlifSizeT left_child(AlifSizeT i) {
	return 2 * i + 1;
}

static inline AlifSizeT right_child(AlifSizeT i) {
	return 2 * i + 2;
}

static void heap_add(AlifIndexHeap* heap, int32_t val) {
	// Add val to end
	heap->values[heap->size] = val;
	heap->size++;
	// Sift up
	for (AlifSizeT cur = heap->size - 1; cur > 0; cur = parent(cur)) {
		if (!heap_trySwap(heap, cur, parent(cur))) {
			break;
		}
	}
}

static AlifSizeT heap_minChild(AlifIndexHeap* heap, AlifSizeT i) {
	if (left_child(i) < heap->size) {
		if (right_child(i) < heap->size) {
			AlifSizeT lval = heap->values[left_child(i)];
			AlifSizeT rval = heap->values[right_child(i)];
			return lval < rval ? left_child(i) : right_child(i);
		}
		return left_child(i);
	}
	else if (right_child(i) < heap->size) {
		return right_child(i);
	}
	return -1;
}

static int32_t heap_pop(AlifIndexHeap* heap) {
	// Pop smallest and replace with the last element
	int32_t result = heap->values[0];
	heap->values[0] = heap->values[heap->size - 1];
	heap->size--;
	// Sift down
	for (AlifSizeT cur = 0; cur < heap->size;) {
		AlifSizeT min_child = heap_minChild(heap, cur);
		if (min_child > -1 and heap_trySwap(heap, cur, min_child)) {
			cur = min_child;
		}
		else {
			break;
		}
	}
	return result;
}

static AlifIntT heap_ensureCapacity(AlifIndexHeap* heap,
	AlifSizeT limit) {
	if (heap->capacity > limit) {
		return 0;
	}
	AlifSizeT newCapacity = heap->capacity ? heap->capacity : 1024;
	while (newCapacity and newCapacity < limit) {
		newCapacity <<= 1;
	}
	if (!newCapacity) {
		return -1;
	}
	int32_t* newValues = (int32_t*)alifMem_dataAlloc(newCapacity * sizeof(int32_t));
	if (newValues == nullptr) {
		return -1;
	}
	if (heap->values != nullptr) {
		memcpy(newValues, heap->values, heap->capacity);
		alifMem_dataFree(heap->values);
	}
	heap->values = newValues;
	heap->capacity = newCapacity;
	return 0;
}

static void heap_fini(AlifIndexHeap* heap) {
	if (heap->values != nullptr) {
		alifMem_dataFree(heap->values);
		heap->values = nullptr;
	}
	heap->size = -1;
	heap->capacity = -1;
}

#define LOCK_POOL(pool) alifMutex_lockFlags(&pool->mutex, AlifLockFlags_::Alif_Lock_Dont_Detach)
#define UNLOCK_POOL(pool) alifMutex_unlock(&pool->mutex)

int32_t _alifIndexPool_allocIndex(AlifIndexPool* pool) {
	LOCK_POOL(pool);
	int32_t index{};
	AlifIndexHeap* free_indices = &pool->freeIndices;
	if (free_indices->size == 0) {
		if (heap_ensureCapacity(free_indices, pool->nextIndex + 1) < 0) {
			UNLOCK_POOL(pool);
			//alifErr_noMemory();
			return -1;
		}
		index = pool->nextIndex++;
	}
	else {
		index = heap_pop(free_indices);
	}
	UNLOCK_POOL(pool);
	return index;
}

void _alifIndexPool_freeIndex(AlifIndexPool* pool, int32_t index) {
	LOCK_POOL(pool);
	heap_add(&pool->freeIndices, index);
	UNLOCK_POOL(pool);
}

void _alifIndexPool_fini(AlifIndexPool* pool) {
	heap_fini(&pool->freeIndices);
}
