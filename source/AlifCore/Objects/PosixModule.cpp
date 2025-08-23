#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Object.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Memory.h"





#ifdef _WINDOWS // 4595





AlifIntT alifOS_getFullPathName(const wchar_t* _path, wchar_t** _absPathP) { // 4875
	wchar_t woutbuf[MAX_PATH], * woutbufp = woutbuf;
	DWORD result{};

	result = GetFullPathNameW(_path,
		ALIF_ARRAY_LENGTH(woutbuf), woutbuf, nullptr);
	if (!result) {
		return -1;
	}

	if (result >= ALIF_ARRAY_LENGTH(woutbuf)) {
		if ((AlifUSizeT)result <= (AlifUSizeT)ALIF_SIZET_MAX / sizeof(wchar_t)) {
			woutbufp = (wchar_t*)alifMem_dataAlloc((AlifUSizeT)result * sizeof(wchar_t));
		}
		else {
			woutbufp = nullptr;
		}
		if (!woutbufp) {
			*_absPathP = nullptr;
			return 0;
		}

		result = GetFullPathNameW(_path, result, woutbufp, nullptr);
		if (!result) {
			alifMem_dataFree(woutbufp);
			return -1;
		}
	}

	if (woutbufp != woutbuf) {
		*_absPathP = woutbufp;
		return 0;
	}

	*_absPathP = alifMem_wcsDup(woutbufp);
	return 0;
}



#endif // 5494

























AlifObject* alifOS_fsPath(AlifObject* path) { // 16477
	AlifObject* func = nullptr;
	AlifObject* pathRepr = nullptr;

	if (ALIFUSTR_CHECK(path) or ALIFBYTES_CHECK(path)) {
		return ALIF_NEWREF(path);
	}

	//func = alifObject_lookupSpecial(path, &ALIF_ID(__fspath__));
	//if ((func == nullptr) or (func == ALIF_NONE)) {
	//	return alifErr_format(_alifExcTypeError_,
	//		"expected str, bytes or os.PathLike object, "
	//		"not %.200s",
	//		_alifType_name(ALIF_TYPE(path)));
	//}

	//pathRepr = _alifObject_callNoArgs(func);
	//ALIF_DECREF(func);
	//if (nullptr == pathRepr) {
	//	return nullptr;
	//}

	if (!(ALIFUSTR_CHECK(pathRepr) or ALIFBYTES_CHECK(pathRepr))) {
		alifErr_format(_alifExcTypeError_,
			"expected %.200s.__fspath__() to return str or bytes, "
			"not %.200s", _alifType_name(ALIF_TYPE(path)),
			_alifType_name(ALIF_TYPE(pathRepr)));
		ALIF_DECREF(pathRepr);
		return nullptr;
	}

	return pathRepr;
}
