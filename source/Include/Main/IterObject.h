#pragma once

class SeqIterObject {
public:
    AlifObject* object{};
    int64_t index{};
    AlifObject* iterSeq{};
};

extern AlifInitObject typeSeqIter;

AlifObject* alifNew_seqIter(AlifObject* );