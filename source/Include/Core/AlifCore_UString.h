#pragma once

AlifObject* alifSubUnicode_copy(AlifObject* );

AlifObject* alifSubUnicode_transformDecimalAndSpaceToASCII(AlifObject* );

size_t count_characters(const wchar_t* );

AlifObject* alifUnicode_joinArray(AlifObject* , AlifObject* const*, int64_t );