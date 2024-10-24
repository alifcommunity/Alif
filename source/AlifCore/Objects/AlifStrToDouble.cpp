#include "alif.h"

#include "AlifCore_DoubleToASCII.h"








//#if ALIF_SHORT_FLOAT_REPR == 1

static double _alifOS_asciiStrToDouble(const char* _nPtr, char** _endPtr) { // 92
	double result;
	//_ALIF_SET_53BIT_PRECISION_HEADER;

	errno = 0;

	//_ALIF_SET_53BIT_PRECISION_START;
	result = _alif_dgStrToDouble(_nPtr, _endPtr);
	//_ALIF_SET_53BIT_PRECISION_END;

	//if (*_endPtr == _nPtr)
		/* string might represent an inf or nan */
		//result = _alif_parseInfOrNan(_nPtr, _endPtr);

	return result;

}

//#else

 // func

//#endif












double alifOS_stringToDouble(const char* _s,
	char** _endPtr, AlifObject* _overflowException) { // 298
	double x{}, result = -1.0;
	char* failPos{};

	errno = 0;
	x = _alifOS_asciiStrToDouble(_s, &failPos);

	if (errno == ENOMEM) {
		//alifErr_noMemory();
		failPos = (char*)_s;
	}
	else if (!_endPtr and (failPos == _s or *failPos != '\0')) {
		//alifErr_format(_alifExcValueError_,
		//	"could not convert string to float: "
		//	"'%.200s'", _s);
	}
	else if (failPos == _s) {
		//alifErr_format(_alifExcValueError_,
		//	"could not convert string to float: "
		//	"'%.200s'", _s);
	}
	else if (errno == ERANGE and fabs(x) >= 1.0 && _overflowException) {
		//alifErr_format(_overflowException,
		//	"value too large to convert to float: "
		//	"'%.200s'", _s);
	}
	else result = x;

	if (_endPtr != nullptr) *_endPtr = failPos;
	return result;
}
