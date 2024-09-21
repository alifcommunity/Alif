#include "alif.h"



AlifIntT alifDescr_isData(AlifObject* _ob) { // 1028
	return ALIF_TYPE(_ob)->descrSet != nullptr;
}
