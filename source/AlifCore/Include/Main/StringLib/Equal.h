#pragma once

ALIF_LOCAL_INLINE(int) uStr_eq(AlifObject * _a, AlifObject * _b)
{
	if (ALIFUSTR_GET_LENGTH(_a) != ALIFUSTR_GET_LENGTH(_b))
		return 0;
	if (ALIFUSTR_GET_LENGTH(_a) == 0)
		return 1;
	if (ALIFUSTR_KIND(_a) != ALIFUSTR_KIND(_b))
		return 0;
	return memcmp(ALIFUSTR_1BYTE_DATA(_a), ALIFUSTR_1BYTE_DATA(_b),
		ALIFUSTR_GET_LENGTH(_a) * ALIFUSTR_KIND(_a)) == 0;
}
