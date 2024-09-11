#pragma once

#include "AlifCore_DureRun.h"


#define NON_SIZE_BITS 3 // 167

#define TAG_FROM_SIGN_AND_SIZE(_sign, _size) ((1 - (_sign)) | ((_size) << NON_SIZE_BITS)) // 262


#define ALIFLONG_FALSE_TAG TAG_FROM_SIGN_AND_SIZE(0, 0) // 300
#define ALIFLONG_TRUE_TAG TAG_FROM_SIGN_AND_SIZE(1, 1) // 301
