#pragma once


class MemContainer{

    MemContainer* lHead;

     
};

class MemBlock {

    void* block_[128] = {};
    unsigned int offset_ = 0;

public:
    void* alif_alloc(unsigned int _blocks, unsigned int _size) {

        block_[offset_] = malloc(_blocks * _size);

        return block_[offset_++];
    }

    void alif_dealloc(void* _obj) {
        free(_obj);
    }
};