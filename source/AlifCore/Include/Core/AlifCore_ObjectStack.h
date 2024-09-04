#pragma once

















#define ALIFOBJECT_STACK_CHUNK_SIZE 254 // 16

class AlifObjectStackChunk { // 18
public:
	AlifObjectStackChunk* prev{};
	AlifSizeT n_{};
	AlifObject* objs[ALIFOBJECT_STACK_CHUNK_SIZE];
};

class AlifObjectStack { // 24
public:
	AlifObjectStackChunk* head{};
};




static inline AlifIntT alifObjectStack_push(AlifObjectStack* _stack, AlifObject* _obj) { // 36
	AlifObjectStackChunk* buf = _stack->head;
	if (buf == nullptr or buf->n_ == ALIFOBJECT_STACK_CHUNK_SIZE) {
		buf = alifObjectStackChunk_new();
		if (buf == nullptr) {
			return -1;
		}
		buf->prev = _stack->head;
		buf->n_ = 0;
		_stack->head = buf;
	}

	buf->objs[buf->n_] = _obj;
	buf->n_++;
	return 0;
}
