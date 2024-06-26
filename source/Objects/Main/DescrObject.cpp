#include "alif.h"

#include "AlifCore_Abstract.h"

















AlifIntT alifDescr_isData(AlifObject* ob) { // 1027
	return ALIF_TYPE(ob)->descrSet != nullptr;
}
