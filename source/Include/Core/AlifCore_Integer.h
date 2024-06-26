#pragma once

#include "AlifCore_GlobalObjects.h"






# define MAX_LEN_DIGITS 18

AlifIntegerObject* alifNew_integer(size_t , bool);





#define SIGN_NEGATIVE true //



static inline bool alifInteger_isNegative(const AlifIntegerObject* _op)
{
	return (_op->sign_ == SIGN_NEGATIVE);
}
