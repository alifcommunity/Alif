#include "AlifCore_Abstract.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"




 // 19
#define LIST_INSERT_METHODDEF    \
    {"ادرج", ALIF_CPPFUNCTION_CAST(list_insert), METHOD_FASTCALL /*, list_insert__doc__*/},

static AlifObject* list_insertImpl(AlifListObject*, AlifSizeT, AlifObject*); // 22

static AlifObject* list_insert(AlifListObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs) { // 25
	AlifObject* returnValue = nullptr;
	AlifSizeT index{};
	AlifObject* object{};

	if (!_ALIFARG_CHECKPOSITIONAL("ادرج", _nargs, 2, 2)) {
		goto exit;
	}
	{
		AlifSizeT ival = -1;
		AlifObject* iobj = _alifNumber_index(_args[0]);
		if (iobj != nullptr) {
			ival = alifLong_asSizeT(iobj);
			ALIF_DECREF(iobj);
		}
		if (ival == -1 and alifErr_occurred()) {
			goto exit;
		}
		index = ival;
	}
	object = _args[1];
	ALIF_BEGIN_CRITICAL_SECTION(_self);
	returnValue = list_insertImpl(_self, index, object);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}





 // 110
#define LIST_APPEND_METHODDEF    \
    {"اضف", (AlifCPPFunction)list_append, METHOD_O /*, list_append__doc__*/},

static AlifObject*
list_appendImpl(AlifListObject*, AlifObject*); // 113

static AlifObject* list_append(AlifListObject* _self, AlifObject* _object) { // 116
	AlifObject* return_value = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(_self);
	return_value = list_appendImpl(_self, _object);
	ALIF_END_CRITICAL_SECTION();

	return return_value;
}









 // 351
#define LIST_REMOVE_METHODDEF    \
    {"امسح", (AlifCPPFunction)list_remove, METHOD_O /*, list_remove__doc__*/},

static AlifObject* list_removeImpl(AlifListObject*, AlifObject*); // 354

static AlifObject* list_remove(AlifListObject* _self, AlifObject* _value) { // 357
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(_self);
	returnValue = list_removeImpl(_self, _value);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}
