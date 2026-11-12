#pragma once



static inline AlifManagedBufferObject* mbuf_alloc(void) { // 71
	AlifManagedBufferObject* mbuf{};

	mbuf = (AlifManagedBufferObject*)
		ALIFOBJECT_GC_NEW(AlifManagedBufferObject, &_alifManagedBufferType_);
	if (mbuf == nullptr)
		return nullptr;
	mbuf->flags = 0;
	mbuf->exports = 0;
	mbuf->master.obj = nullptr;
	ALIFOBJECT_GC_TRACK(mbuf);

	return mbuf;
}

static void mbuf_release(AlifManagedBufferObject* _self) { // 106
	if (_self->flags & ALIF_MANAGED_BUFFER_RELEASED)
		return;

	_self->flags |= ALIF_MANAGED_BUFFER_RELEASED;

	ALIFOBJECT_GC_UNTRACK(_self);
	alifBuffer_release(&_self->master);
}



AlifTypeObject _alifManagedBufferType_ = { // 147
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "managedbuffer",
	.basicSize = sizeof(AlifManagedBufferObject),
	//.dealloc = mbuf_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = mbuf_traverse,
	//.clear = mbuf_clear,
};


#define HAVE_PTR(_suboffsets, _dim) (_suboffsets && _suboffsets[_dim] >= 0) // 227
// 229
#define ADJUST_PTR(_ptr, _suboffsets, _dim) \
    (HAVE_PTR(_suboffsets, _dim) ? *((char**)_ptr) + _suboffsets[_dim] : _ptr)


// 233
#define MV_C_CONTIGUOUS(flags) (flags&(ALIF_MEMORYVIEW_SCALAR|ALIF_MEMORYVIEW_C))
#define MV_F_CONTIGUOUS(flags) \
    (flags&(ALIF_MEMORYVIEW_SCALAR|ALIF_MEMORYVIEW_FORTRAN))
#define MV_ANY_CONTIGUOUS(flags) \
    (flags&(ALIF_MEMORYVIEW_SCALAR|ALIF_MEMORYVIEW_C|ALIF_MEMORYVIEW_FORTRAN))

#define MV_CONTIGUOUS_NDIM1(_view) \
    ((_view)->shape[0] == 1 or (_view)->strides[0] == (_view)->itemSize)

// 244
#define REQ_INDIRECT(flags) ((flags&ALIFBUF_INDIRECT) == ALIFBUF_INDIRECT)
#define REQ_C_CONTIGUOUS(flags) ((flags&ALIFBUF_C_CONTIGUOUS) == ALIFBUF_C_CONTIGUOUS)
#define REQ_F_CONTIGUOUS(flags) ((flags&ALIFBUF_F_CONTIGUOUS) == ALIFBUF_F_CONTIGUOUS)
#define REQ_ANY_CONTIGUOUS(flags) ((flags&ALIFBUF_ANY_CONTIGUOUS) == ALIFBUF_ANY_CONTIGUOUS)
#define REQ_STRIDES(flags) ((flags&ALIFBUF_STRIDES) == ALIFBUF_STRIDES)
#define REQ_SHAPE(flags) ((flags&ALIFBUF_ND) == ALIFBUF_ND)
#define REQ_WRITABLE(flags) (flags&ALIFBUF_WRITABLE)
#define REQ_FORMAT(flags) (flags&ALIFBUF_FORMAT)



#define HAVE_SUBOFFSETS_IN_LAST_DIM(_view) \
    (_view->subOffsets and _view->subOffsets[_view->nDim-1] >= 0)

static inline AlifIntT lastDim_isContiguous(const AlifBuffer* dest,
	const AlifBuffer* src) { // 271
	return (!HAVE_SUBOFFSETS_IN_LAST_DIM(dest) and
		!HAVE_SUBOFFSETS_IN_LAST_DIM(src) and
		dest->strides[dest->nDim - 1] == dest->itemSize and
		src->strides[src->nDim - 1] == src->itemSize);
}

static inline AlifIntT equiv_format(const AlifBuffer* dest,
	const AlifBuffer* src) { // 287
	const char* dfmt, * sfmt;

	dfmt = dest->format[0] == '@' ? dest->format + 1 : dest->format;
	sfmt = src->format[0] == '@' ? src->format + 1 : src->format;

	if (strcmp(dfmt, sfmt) != 0 or
		dest->itemSize != src->itemSize) {
		return 0;
	}

	return 1;
}

static inline AlifIntT equiv_shape(const AlifBuffer* dest,
	const AlifBuffer* src) { // 307
	AlifIntT i{};

	if (dest->nDim != src->nDim)
		return 0;

	for (i = 0; i < dest->nDim; i++) {
		if (dest->shape[i] != src->shape[i])
			return 0;
		if (dest->shape[i] == 0)
			break;
	}

	return 1;
}

static AlifIntT equiv_structure(const AlifBuffer* dest,
	const AlifBuffer* src) { // 327
	if (!equiv_format(dest, src) ||
		!equiv_shape(dest, src)) {
		alifErr_setString(_alifExcValueError_,
			"memoryview assignment: lvalue and rvalue have different "
			"structures");
		return 0;
	}

	return 1;
}


static void copy_base(const AlifSizeT* shape, AlifSizeT itemsize,
	char* dptr, const AlifSizeT* dstrides, const AlifSizeT* dsuboffsets,
	char* sptr, const AlifSizeT* sstrides, const AlifSizeT* ssuboffsets,
	char* mem) { // 344
	if (mem == NULL) { /* contiguous */
		AlifSizeT size = shape[0] * itemsize;
		if (dptr + size < sptr || sptr + size < dptr)
			memcpy(dptr, sptr, size); /* no overlapping */
		else
			memmove(dptr, sptr, size);
	}
	else {
		char* p;
		AlifSizeT i;
		for (i = 0, p = mem; i < shape[0]; p += itemsize, sptr += sstrides[0], i++) {
			char* xsptr = ADJUST_PTR(sptr, ssuboffsets, 0);
			memcpy(p, xsptr, itemsize);
		}
		for (i = 0, p = mem; i < shape[0]; p += itemsize, dptr += dstrides[0], i++) {
			char* xdptr = ADJUST_PTR(dptr, dsuboffsets, 0);
			memcpy(xdptr, p, itemsize);
		}
	}

}

static void copy_rec(const AlifSizeT* shape, AlifSizeT ndim, AlifSizeT itemsize,
	char* dptr, const AlifSizeT* dstrides, const AlifSizeT* dsuboffsets,
	char* sptr, const AlifSizeT* sstrides, const AlifSizeT* ssuboffsets,
	char* mem) { // 374
	AlifSizeT i;

	if (ndim == 1) {
		copy_base(shape, itemsize,
			dptr, dstrides, dsuboffsets,
			sptr, sstrides, ssuboffsets,
			mem);
		return;
	}

	for (i = 0; i < shape[0]; dptr += dstrides[0], sptr += sstrides[0], i++) {
		char* xdptr = ADJUST_PTR(dptr, dsuboffsets, 0);
		char* xsptr = ADJUST_PTR(sptr, ssuboffsets, 0);

		copy_rec(shape + 1, ndim - 1, itemsize,
			xdptr, dstrides + 1, dsuboffsets ? dsuboffsets + 1 : nullptr,
			xsptr, sstrides + 1, ssuboffsets ? ssuboffsets + 1 : nullptr,
			mem);
	}
}


static AlifIntT copy_buffer(const AlifBuffer* dest,
	const AlifBuffer* src) { // 437
	char* mem = nullptr;

	if (!equiv_structure(dest, src))
		return -1;

	if (!lastDim_isContiguous(dest, src)) {
		mem = (char*)alifMem_dataAlloc(dest->shape[dest->nDim - 1] * dest->itemSize);
		if (mem == nullptr) {
			//alifErr_noMemory();
			return -1;
		}
	}

	copy_rec(dest->shape, dest->nDim, dest->itemSize,
		(char*)dest->buf, dest->strides, dest->subOffsets,
		(char*)src->buf, src->strides, src->subOffsets,
		mem);

	if (mem)
		alifMem_dataFree(mem);

	return 0;
}


static inline void initStrides_fromShape(AlifBuffer* view) { // 467
	AlifSizeT i{};

	view->strides[view->nDim - 1] = view->itemSize;
	for (i = view->nDim - 2; i >= 0; i--)
		view->strides[i] = view->strides[i + 1] * view->shape[i + 1];
}



static inline void initFortranStrides_fromShape(AlifBuffer* view) { // 480
	AlifSizeT i{};

	view->strides[0] = view->itemSize;
	for (i = 1; i < view->nDim; i++)
		view->strides[i] = view->strides[i - 1] * view->shape[i - 1];
}


static AlifIntT buffer_toContiguous(char* mem,
	const AlifBuffer* src, char order) { // 496
	AlifBuffer dest{};
	AlifSizeT* strides{};
	AlifIntT ret{};

	strides = (AlifSizeT*)alifMem_dataAlloc(src->nDim * (sizeof * src->strides));
	if (strides == nullptr) {
		//alifErr_noMemory();
		return -1;
	}

	/* initialize dest */
	dest = *src;
	dest.buf = mem;

	dest.strides = strides;
	if (order == 'C' || order == 'A') {
		initStrides_fromShape(&dest);
	}
	else {
		initFortranStrides_fromShape(&dest);
	}

	dest.subOffsets = nullptr;

	ret = copy_buffer(&dest, src);

	alifMem_dataFree(strides);
	return ret;
}


static inline void init_sharedValues(AlifBuffer* dest, const AlifBuffer* src) { // 542
	dest->obj = src->obj;
	dest->buf = src->buf;
	dest->len = src->len;
	dest->itemSize = src->itemSize;
	dest->readonly = src->readonly;
	src->format ? dest->format = src->format : dest->format = (char*)"B";
	dest->internal = src->internal;
}

static void init_shapeStrides(AlifBuffer* dest, const AlifBuffer* src) { // 555
	AlifSizeT i{};

	if (src->nDim == 0) {
		dest->shape = nullptr;
		dest->strides = nullptr;
		return;
	}
	if (src->nDim == 1) {
		dest->shape[0] = src->shape ? src->shape[0] : src->len / src->itemSize;
		dest->strides[0] = src->strides ? src->strides[0] : src->itemSize;
		return;
	}

	for (i = 0; i < src->nDim; i++)
		dest->shape[i] = src->shape[i];
	if (src->strides) {
		for (i = 0; i < src->nDim; i++)
			dest->strides[i] = src->strides[i];
	}
	else {
		initStrides_fromShape(dest);
	}
}

static inline void init_subOffsets(AlifBuffer* dest, const AlifBuffer* src) { // 582
	AlifSizeT i{};

	if (src->subOffsets == nullptr) {
		dest->subOffsets = nullptr;
		return;
	}
	for (i = 0; i < src->nDim; i++)
		dest->subOffsets[i] = src->subOffsets[i];
}

static void init_flags(AlifMemoryViewObject* _mv) { // 610
	const AlifBuffer* view = &_mv->view;
	AlifIntT flags = 0;

	switch (view->nDim) {
	case 0:
		flags |= (ALIF_MEMORYVIEW_SCALAR | ALIF_MEMORYVIEW_C |
			ALIF_MEMORYVIEW_FORTRAN);
		break;
	case 1:
		if (MV_CONTIGUOUS_NDIM1(view))
			flags |= (ALIF_MEMORYVIEW_C | ALIF_MEMORYVIEW_FORTRAN);
		break;
	default:
		if (alifBuffer_isContiguous(view, 'C'))
			flags |= ALIF_MEMORYVIEW_C;
		if (alifBuffer_isContiguous(view, 'F'))
			flags |= ALIF_MEMORYVIEW_FORTRAN;
		break;
	}

	if (view->subOffsets) {
		flags |= ALIF_MEMORYVIEW_PIL;
		flags &= ~(ALIF_MEMORYVIEW_C | ALIF_MEMORYVIEW_FORTRAN);
	}

	_mv->flags = flags;
}

static inline AlifMemoryViewObject* memory_alloc(AlifIntT ndim) { // 643
	AlifMemoryViewObject* mv{};

	mv = (AlifMemoryViewObject*)
		ALIFOBJECT_GC_NEWVAR(AlifMemoryViewObject, &_alifMemoryViewType_, 3 * ndim);
	if (mv == nullptr)
		return nullptr;

	mv->mbuf = nullptr;
	mv->hash = -1;
	mv->flags = 0;
	mv->exports = 0;
	mv->view.nDim = ndim;
	mv->view.shape = mv->array;
	mv->view.strides = mv->array + ndim;
	mv->view.subOffsets = mv->array + 2 * ndim;
	mv->weakRefList = nullptr;

	ALIFOBJECT_GC_TRACK(mv);
	return mv;
}

static AlifObject* mbuf_addView(AlifManagedBufferObject* _mbuf, const AlifBuffer* _src) { // 675
	AlifMemoryViewObject* mv{};
	AlifBuffer* dest{};

	if (_src == nullptr)
		_src = &_mbuf->master;

	if (_src->nDim > ALIFBUF_MAX_NDIM) {
		alifErr_setString(_alifExcValueError_,
			"memoryview: number of dimensions must not exceed "
			ALIF_STRINGIFY(ALIFBUF_MAX_NDIM));
		return nullptr;
	}

	mv = memory_alloc(_src->nDim);
	if (mv == nullptr)
		return nullptr;

	dest = &mv->view;
	init_sharedValues(dest, _src);
	init_shapeStrides(dest, _src);
	init_subOffsets(dest, _src);
	init_flags(mv);

	mv->mbuf = (AlifManagedBufferObject*)ALIF_NEWREF(_mbuf);
	_mbuf->exports++;

	return (AlifObject*)mv;
}




AlifObject* alifMemoryView_fromMemory(char* _mem,
	AlifSizeT _size, AlifIntT _flags) { // 740
	AlifManagedBufferObject* mbuf{};
	AlifObject* mv{};
	AlifIntT readonly{};

	mbuf = mbuf_alloc();
	if (mbuf == nullptr)
		return nullptr;

	readonly = (_flags == ALIFBUF_WRITE) ? 0 : 1;
	(void)alifBuffer_fillInfo(&mbuf->master, nullptr, _mem, _size, readonly,
		ALIFBUF_FULL_RO);

	mv = mbuf_addView(mbuf, nullptr);
	ALIF_DECREF(mbuf);

	return mv;
}

AlifObject* alifMemoryView_fromBuffer(const AlifBuffer* _info) { // 769
	AlifManagedBufferObject* mbuf{};
	AlifObject* mv{};

	if (_info->buf == nullptr) {
		alifErr_setString(_alifExcValueError_,
			"alifMemoryView_fromBuffer(): info->buf يجب أن لا تكون فارغة");
		return nullptr;
	}

	mbuf = mbuf_alloc();
	if (mbuf == nullptr)
		return nullptr;

	mbuf->master = *_info;
	mbuf->master.obj = nullptr;

	mv = mbuf_addView(mbuf, nullptr);
	ALIF_DECREF(mbuf);

	return mv;
}





class AlifBufferFull { // 1043
public:
	AlifBuffer view{};
	AlifSizeT array[1]{};
};

AlifIntT alifBuffer_toContiguous(void* _buf,
	const AlifBuffer* _src, AlifSizeT _len, char _order) { // 1049
	AlifBufferFull* fb{};
	AlifIntT ret{};

	if (_len != _src->len) {
		alifErr_setString(_alifExcValueError_,
			"alifBuffer_toContiguous: len != view->len");
		return -1;
	}

	if (alifBuffer_isContiguous(_src, _order)) {
		memcpy((char*)_buf, _src->buf, _len);
		return 0;
	}

	fb = (AlifBufferFull*)alifMem_dataAlloc(sizeof * fb + 3 * _src->nDim * (sizeof * fb->array));
	if (fb == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	fb->view.nDim = _src->nDim;
	fb->view.shape = fb->array;
	fb->view.strides = fb->array + _src->nDim;
	fb->view.subOffsets = fb->array + 2 * _src->nDim;

	init_sharedValues(&fb->view, _src);
	init_shapeStrides(&fb->view, _src);
	init_subOffsets(&fb->view, _src);

	_src = &fb->view;

	ret = buffer_toContiguous((char*)_buf, _src, _order);
	alifMem_dataFree(fb);
	return ret;
}




static void _memory_release(AlifMemoryViewObject* _self) { // 1098
	if (_self->flags & ALIF_MEMORYVIEW_RELEASED)
		return;

	_self->flags |= ALIF_MEMORYVIEW_RELEASED;
	if (--_self->mbuf->exports == 0) {
		mbuf_release(_self->mbuf);
	}
}

static void memory_dealloc(AlifObject* _self) { // 1139
	AlifMemoryViewObject* self = (AlifMemoryViewObject*)_self;
	ALIFOBJECT_GC_UNTRACK(self);
	_memory_release(self);
	ALIF_CLEAR(self->mbuf);
	if (self->weakRefList != nullptr)
		alifObject_clearWeakRefs((AlifObject*)self);
	alifObject_gcDel(self);
}



static AlifIntT memory_getBuf(AlifObject* _self, AlifBuffer* _view, AlifIntT _flags) { // 1517
	AlifMemoryViewObject* self = (AlifMemoryViewObject*)_self;
	AlifBuffer* base = &self->view;
	AlifIntT baseflags = self->flags;

	//CHECK_RELEASED_INT(self);
	//CHECK_RESTRICTED_INT(self);

	*_view = *base;
	_view->obj = nullptr;

	if (REQ_WRITABLE(_flags) and base->readonly) {
		//alifErr_setString(_alifExcBufferError_,
		//	"memoryview: underlying buffer is not writable");
		return -1;
	}
	if (!REQ_FORMAT(_flags)) {
		_view->format = nullptr;
	}

	if (REQ_C_CONTIGUOUS(_flags) and !MV_C_CONTIGUOUS(baseflags)) {
		//alifErr_setString(_alifExcBufferError_,
		//	"memoryview: underlying buffer is not C-contiguous");
		return -1;
	}
	if (REQ_F_CONTIGUOUS(_flags) and !MV_F_CONTIGUOUS(baseflags)) {
		//alifErr_setString(_alifExcBufferError_,
		//	"memoryview: underlying buffer is not Fortran contiguous");
		return -1;
	}
	if (REQ_ANY_CONTIGUOUS(_flags) and !MV_ANY_CONTIGUOUS(baseflags)) {
		//alifErr_setString(_alifExcBufferError_,
		//	"memoryview: underlying buffer is not contiguous");
		return -1;
	}
	if (!REQ_INDIRECT(_flags) and (baseflags & ALIF_MEMORYVIEW_PIL)) {
		//alifErr_setString(_alifExcBufferError_,
		//	"memoryview: underlying buffer requires suboffsets");
		return -1;
	}
	if (!REQ_STRIDES(_flags)) {
		if (!MV_C_CONTIGUOUS(baseflags)) {
			//alifErr_setString(_alifExcBufferError_,
			//	"memoryview: underlying buffer is not C-contiguous");
			return -1;
		}
		_view->strides = nullptr;
	}
	if (!REQ_SHAPE(_flags)) {
		if (_view->format != nullptr) {
			//alifErr_format(_alifExcBufferError_,
			//	"memoryview: cannot cast to unsigned bytes if the format flag "
			//	"is present");
			return -1;
		}
		_view->nDim = 1;
		_view->shape = nullptr;
	}


	_view->obj = ALIF_NEWREF(self);
	self->exports++;

	return 0;
}

static void memory_releaseBuf(AlifObject* _self, AlifBuffer* _view) { // 1597
	AlifMemoryViewObject* self = (AlifMemoryViewObject*)_self;
	self->exports--;
	return;
}

static AlifBufferProcs _memoryAsBuffer_ = { // 1607
	.getBuffer = memory_getBuf,
	.releaseBuffer = memory_releaseBuf,
};




























AlifTypeObject _alifMemoryViewType_ = { // 3403
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مشهد_ذاكرة",
	.basicSize = offsetof(AlifMemoryViewObject, array),
	.itemSize = sizeof(AlifSizeT),
	.dealloc = memory_dealloc,
	//.repr = memory_repr,
	//.asSequence = &_memoryAsSequence_,
	//.asMapping = &_memoryAsMapping_,
	//.hash = memory_hash,
	.getAttro = alifObject_genericGetAttr,
	.asBuffer = &_memoryAsBuffer_,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	   ALIF_TPFLAGS_SEQUENCE,
	//.traverse = memory_traverse,
	//.clear = memory_clear,  
	//.richCompare = memory_richcompare,
	.weakListOffset = offsetof(AlifMemoryViewObject, weakRefList),
	//.iter = memory_iter,
	//.methods = _memoryMethods_,
	//.getSet = _memoryGetSetList_,
	//.new_ = memory_view,
};

