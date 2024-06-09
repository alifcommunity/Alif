#pragma once

AlifObject* alifSubUStr_copy(AlifObject* );

AlifObject* alifSubUStr_transformDecimalAndSpaceToASCII(AlifObject* );

size_t count_characters(const wchar_t* );

AlifObject* alifUStr_joinArray(AlifObject* , AlifObject* const*, int64_t );
