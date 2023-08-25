#pragma once

























































class AlifObjectArenaAllocator {
public:
	void* ctx;

	/* allocate an arena of size bytes */
	void* (*alloc) (void* ctx, size_t size);

	/* free an arena */
	void (*free) (void* ctx, void* ptr, size_t size);
};
