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

//198
#define LIST_SORT_METHODDEF    \
    {"رتب", ALIF_CPPFUNCTION_CAST(list_sort), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* list_sortImpl(AlifListObject* , AlifObject* , AlifIntT);

static AlifObject* list_sort(AlifListObject* _self, AlifObject* const* _args, AlifSizeT _nArgs, AlifObject* _kwNames) { // 205
	AlifObject* returnValue = nullptr;

	#define NUM_KEYWORDS 2
		static class {
		public:
			AlifGCHead thisIsNotUsed{};
			ALIFOBJECT_VAR_HEAD;
			AlifObject* item[NUM_KEYWORDS];
		} kwTuple = {
			.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
			.item = { &ALIF_ID(Key), &ALIF_ID(Reverse), },
		};
	#undef NUM_KEYWORDS
	#define KWTUPLE (&kwTuple.objBase.objBase)

	//#define KWTUPLE nullptr

	static const char* const _keywords[] = { "مفتاح", "معكوس", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "رتب",
		.kwTuple = KWTUPLE,
	};
	#undef KWTUPLE
	AlifObject* argsbuf[2];
	AlifSizeT noptargs = _nArgs + (_kwNames ? ALIFTUPLE_GET_SIZE(_kwNames) : 0) - 0;
	AlifObject* keyfunc = ALIF_NONE;
	AlifIntT reverse = 0;

	_args = _alifArg_unpackKeywords(_args, _nArgs, nullptr, _kwNames, &_parser, 0, 0, 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	if (!noptargs) {
		goto skipOptionalkwOnly;
	}
	if (_args[0]) {
		keyfunc = _args[0];
		if (!--noptargs) {
			goto skipOptionalkwOnly;
		}
	}
	reverse = alifObject_isTrue(_args[1]);
	if (reverse < 0) {
		goto exit;
	}
skipOptionalkwOnly:
	ALIF_BEGIN_CRITICAL_SECTION(_self);
	returnValue = list_sortImpl(_self, keyfunc, reverse);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
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
