#pragma once

























































class AlifObjectArenaAllocator {
public:
	void* ctx;


	void* (*alloc) (void* ctx, size_t size);


	void (*free) (void* ctx, void* ptr, size_t size);
};
