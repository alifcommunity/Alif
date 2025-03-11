//#pragma once

// stringlib/eq.h
ALIF_LOCAL_INLINE(AlifIntT) uStr_eq(AlifObject * _str1, AlifObject * _str2) { // 6
	AlifSizeT len = ALIFUSTR_GET_LENGTH(_str1);
	if (ALIFUSTR_GET_LENGTH(_str2) != len) {
		return 0;
	}

	AlifIntT kind = ALIFUSTR_KIND(_str1);
	if (ALIFUSTR_KIND(_str2) != kind) {
		return 0;
	}

	const void* data1 = ALIFUSTR_DATA(_str1);
	const void* data2 = ALIFUSTR_DATA(_str2);
	return (memcmp(data1, data2, len * kind) == 0);
}
