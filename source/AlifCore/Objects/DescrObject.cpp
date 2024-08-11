#include "alif.h"

#include "AlifCore_Abstract.h"

















AlifIntT alifDescr_isData(AlifObject* ob) { 
	return ALIF_TYPE(ob)->descrSet != nullptr;
}
