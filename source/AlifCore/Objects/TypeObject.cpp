#include "alif.h"

#include "AlifCore_Object.h"









AlifObject* alifType_allocNoTrack(AlifTypeObject* _type, AlifSizeT _nitems) { // 2217
	AlifObject* obj_{};
	size_t size = alifObject_varSize(_type, _nitems + 1);

	const size_t preSize = alifType_preHeaderSize(_type);
	if (_type->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		size += alifInline_valuesSize(_type);
	}
	char* alloc = alifObject_mallocWithType(_type, size + preSize);
	if (alloc == nullptr) {
		//return alifErr_noMemory();
		return nullptr;
	}
	obj_ = (AlifObject*)(alloc + preSize);
	if (preSize) {
		((AlifObject**)alloc)[0] = nullptr;
		((AlifObject**)alloc)[1] = nullptr;
	}
	if (ALIFTYPE_IS_GC(_type)) {
		alifObject_gcLink(obj_);
	}
	memset(obj_, '\0', size);

	if (_type->itemSize == 0) {
		alifObject_init(obj_, _type);
	}
	else {
		alifObject_initVar((AlifVarObject*)obj_, _type, _nitems);
	}
	if (_type->flags & ALIF_TPFLAGS_INLINE_VALUES) {
		alifObject_initInlineValues(obj_, _type);
	}
	return obj_;
}





static void type_dealloc(AlifObject* self) { // 5911
	AlifTypeObject* type = (AlifTypeObject*)self;

	ALIFOBJECT_GC_UNTRACK(type);
	//type_deallocCommon(type);

	//alifObject_clearWeakRefs((AlifObject*)type);

	ALIF_XDECREF(type->base);
	//ALIF_XDECREF(type->dict);
	//ALIF_XDECREF(type->bases);
	//ALIF_XDECREF(type->mro);
	//ALIF_XDECREF(type->cache);
	//clear_tpSubClasses(type);	

	AlifHeapTypeObject* et = (AlifHeapTypeObject*)type;
	ALIF_XDECREF(et->name);
	ALIF_XDECREF(et->qualname);
	ALIF_XDECREF(et->slots);
	if (et->cachedKeys) {
		alifDictKeys_decRef(et->cachedKeys);
	}
	ALIF_XDECREF(et->module_);
	alifMem_objFree(et->name);

	alifType_releaseID(et);

	ALIF_TYPE(type)->free((AlifObject*)type);
}








AlifTypeObject _alifTypeType_ = { // 6195
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نوع",
	.basicSize = sizeof(AlifHeapTypeObject),
	.itemSize = sizeof(AlifMemberDef),
	.dealloc = type_dealloc,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_TYPE_SUBCLASS |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_ITEMS_AT_END,

	.base = 0,
};
