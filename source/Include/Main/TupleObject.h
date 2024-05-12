#pragma once

class AlifTupleObject {
public:
    AlifVarObject object{};

     AlifObject* items[1];
};

extern AlifInitObject typeTuple;

AlifObject* alifNew_tuple(SSIZE_T );
AlifObject* tuple_pack(size_t, ...);
