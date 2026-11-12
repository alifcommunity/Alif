
#include "AlifCore_BytesMethods.h"








static AlifObject* stringLib_lower(AlifObject* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 58
	AlifObject* newobj{};
	newobj = STRINGLIB_NEW(nullptr, STRINGLIB_LEN(self));
	if (!newobj)
		return nullptr;
	_alifBytes_lower(STRINGLIB_STR(newobj), STRINGLIB_STR(self),
		STRINGLIB_LEN(self));
	return newobj;
}
