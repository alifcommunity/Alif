#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Object.h"







static AlifObject* tryComplex_specialMethod(AlifObject* _op) { // 328
	AlifObject* f_{};

	f_ = alifObject_lookupSpecial(_op, &ALIF_ID(__complex__));
	if (f_) {
		//AlifObject* res_ = _alifObject_callNoArgs(f_);
		//ALIF_DECREF(f_);
		//if (!res_ or ALIFCOMPLEX_CHECKEXACT(res_)) {
		//	return res_;
		//}
		//if (!ALIFCOMPLEX_CHECK(res_)) {
		//	//alifErr_format(_alifExcTypeError_,
		//		//"__complex__ returned non-complex (type %.200s)",
		//		//ALIF_TYPE(res)->name);
		//	ALIF_DECREF(res_);
		//	return nullptr;
		//}
		
		//if (alifErr_warnFormat(_alifExcDeprecationWarning_, 1,
			//"__complex__ returned non-complex (type %.200s).  "
			//"The ability to return an instance of a strict subclass of complex "
			//"is deprecated, and may be removed in a future version of Python.",
			//ALIF_TYPE(res)->name)) {
			//ALIF_DECREF(res);
			//return nullptr;
		//}
		
		//return res_;
	}
	return nullptr;
}

AlifComplex alifComplex_asCComplex(AlifObject* _op) { // 361
	AlifComplex cv_{};
	AlifObject* newOp = nullptr;

	if (ALIFCOMPLEX_CHECK(_op)) {
		return ((AlifComplexObject*)_op)->cVal;
	}
	
	cv_.real = -1.;
	cv_.imag = 0.;

	newOp = tryComplex_specialMethod(_op);

	if (newOp) {
		cv_ = ((AlifComplexObject*)newOp)->cVal;
		ALIF_DECREF(newOp);
		return cv_;
	}
	//else if (alifErr_occurred()) {
		//return cv;
	//}
	else {
		cv_.real = alifFloat_asDouble(_op);
		return cv_;
	}
}
