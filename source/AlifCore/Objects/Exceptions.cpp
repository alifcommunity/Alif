#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Exceptions.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Errors.h"

#include "OSDefs.h"



/* Compatibility aliases */
AlifObject *_alifExcEnvironmentError_ = nullptr;  // borrowed ref
AlifObject *_alifExcIOError_ = nullptr;  // borrowed ref
#ifdef _WINDOWS
AlifObject *_alifExcWindowsError_ = nullptr;  // borrowed ref
#endif

static AlifExcState* get_excState(void) { // 28
	AlifInterpreter* interp = _alifInterpreter_get();
	return &interp->excState;
}


/*
 *    BaseException
 */
static AlifObject* baseException_new(AlifTypeObject* type,
	AlifObject* args, AlifObject* kwds) { // 40
	AlifBaseExceptionObject* self{};

	self = (AlifBaseExceptionObject*)type->alloc(type, 0);
	if (!self)
		return nullptr;
	self->dict = nullptr;
	self->notes = nullptr;
	self->traceback = self->cause = self->context = nullptr;
	self->suppressContext = 0;

	if (args) {
		self->args = ALIF_NEWREF(args);
		return (AlifObject*)self;
	}

	self->args = alifTuple_new(0);
	if (!self->args) {
		ALIF_DECREF(self);
		return nullptr;
	}

	return (AlifObject*)self;
}

static AlifIntT baseException_init(AlifBaseExceptionObject* self,
	AlifObject* args, AlifObject* kwds) { // 71
	if (!_ALIFARG_NOKEYWORDS(ALIF_TYPE(self)->name, kwds))
		return -1;

	ALIF_XSETREF(self->args, ALIF_NEWREF(args));
	return 0;
}


static AlifObject* baseException_vectorCall(AlifObject* type_obj, AlifObject* const* args,
	AlifUSizeT nargsf, AlifObject* kwnames) { // 82
	AlifTypeObject* type = ALIFTYPE_CAST(type_obj);
	if (!_ALIFARG_NOKWNAMES(type->name, kwnames)) {
		return nullptr;
	}

	AlifBaseExceptionObject* self{};
	self = (AlifBaseExceptionObject*)type->alloc(type, 0);
	if (!self) {
		return nullptr;
	}

	self->dict = nullptr;
	self->notes = nullptr;
	self->traceback = nullptr;
	self->cause = nullptr;
	self->context = nullptr;
	self->suppressContext = 0;

	self->args = _alifTuple_fromArray(args, ALIFVECTORCALL_NARGS(nargsf));
	if (!self->args) {
		ALIF_DECREF(self);
		return nullptr;
	}

	return (AlifObject*)self;
}

static AlifIntT baseException_clear(AlifBaseExceptionObject* _self) { // 115
	ALIF_CLEAR(_self->dict);
	ALIF_CLEAR(_self->args);
	ALIF_CLEAR(_self->notes);
	ALIF_CLEAR(_self->traceback);
	ALIF_CLEAR(_self->cause);
	ALIF_CLEAR(_self->context);
	return 0;
}


static void baseException_dealloc(AlifBaseExceptionObject* _self) { // 127
	alifObject_gcUnTrack(_self);
	ALIF_TRASHCAN_BEGIN(_self, baseException_dealloc)
		baseException_clear(_self);
	ALIF_TYPE(_self)->free((AlifObject*)_self);
	ALIF_TRASHCAN_END
}

static AlifObject* baseException_str(AlifBaseExceptionObject* _self) { // 152
	switch (ALIFTUPLE_GET_SIZE(_self->args)) {
	case 0:
		return alif_getConstant(ALIF_CONSTANT_EMPTY_STR);
	case 1:
		return alifObject_str(ALIFTUPLE_GET_ITEM(_self->args, 0));
	default:
		return alifObject_str(_self->args);
	}
}



static inline AlifBaseExceptionObject* _alifBaseExceptionObject_cast(AlifObject* _exc) { // 228
	return (AlifBaseExceptionObject*)_exc;
}


static AlifObject* baseException_addNote(AlifObject* self, AlifObject* note) { // 235
	if (!ALIFUSTR_CHECK(note)) {
		alifErr_format(_alifExcTypeError_,
			"note must be a str, not '%s'",
			ALIF_TYPE(note)->name);
		return nullptr;
	}

	AlifObject* notes{};
	if (alifObject_getOptionalAttr(self, &ALIF_ID(__notes__), &notes) < 0) {
		return nullptr;
	}
	if (notes == nullptr) {
		notes = alifList_new(0);
		if (notes == nullptr) {
			return nullptr;
		}
		if (alifObject_setAttr(self, &ALIF_ID(__notes__), notes) < 0) {
			ALIF_DECREF(notes);
			return nullptr;
		}
	}
	else if (!ALIFLIST_CHECK(notes)) {
		ALIF_DECREF(notes);
		alifErr_setString(_alifExcTypeError_, "Cannot add note: __notes__ is not a list");
		return nullptr;
	}
	if (alifList_append(notes, note) < 0) {
		ALIF_DECREF(notes);
		return nullptr;
	}
	ALIF_DECREF(notes);
	return ALIF_NONE;
}


static AlifIntT baseException_setTB(AlifBaseExceptionObject* self,
	AlifObject* tb, void* ALIF_UNUSED(ignored)) { // 319
	if (tb == nullptr) {
		alifErr_setString(_alifExcTypeError_, "__traceback__ may not be deleted");
		return -1;
	}
	if (ALIFTRACEBACK_CHECK(tb)) {
		ALIF_XSETREF(self->traceback, ALIF_NEWREF(tb));
	}
	else if (tb == ALIF_NONE) {
		ALIF_CLEAR(self->traceback);
	}
	else {
		alifErr_setString(_alifExcTypeError_,
			"__traceback__ must be a traceback or None");
		return -1;
	}
	return 0;
}


AlifObject* alifException_getTraceback(AlifObject* _self) { // 411
	AlifBaseExceptionObject* baseSelf = _alifBaseExceptionObject_cast(_self);
	return ALIF_XNEWREF(baseSelf->traceback);
}

AlifIntT alifException_setTraceback(AlifObject* self, AlifObject* tb) { // 419
	return baseException_setTB(_alifBaseExceptionObject_cast(self), tb, nullptr);
}

AlifObject* alifException_getCause(AlifObject* _self) { // 425
	AlifObject* cause = _alifBaseExceptionObject_cast(_self)->cause;
	return ALIF_XNEWREF(cause);
}

/* Steals a reference to cause */
void alifException_setCause(AlifObject* _self, AlifObject* _cause) { // 433
	AlifBaseExceptionObject* base_self = _alifBaseExceptionObject_cast(_self);
	base_self->suppressContext = 1;
	ALIF_XSETREF(base_self->cause, _cause);
}

AlifObject* alifException_getContext(AlifObject* _self) { // 441
	AlifObject* context = _alifBaseExceptionObject_cast(_self)->context;
	return ALIF_XNEWREF(context);
}


void alifException_setContext(AlifObject* _self, AlifObject* _context) { // 449
	ALIF_XSETREF(_alifBaseExceptionObject_cast(_self)->context, _context);
}



// 548
#define MIDDLINGEXTENDSEXCEPTIONEX(EXCBASE, EXCNAME, ALIFEXCNAME, EXCSTORE, EXCDOC) \
AlifTypeObject _exc ## EXCNAME ## _ = { \
    .objBase = ALIFVAROBJECT_HEAD_INIT(nullptr, 0), \
    .name = #ALIFEXCNAME, \
    .basicSize = sizeof(Alif ## EXCSTORE ## Object), \
    /*.dealloc = (destructor)EXCSTORE ## _dealloc,*/ \
    .flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC, \
    /*.traverse = (traverseproc)EXCSTORE ## _traverse,*/ \
    /*(Inquiry)EXCSTORE ## _clear,*/		\
	.base = &EXCBASE, \
    .dictOffset = offsetof(Alif ## EXCSTORE ## Object, dict), \
    /*.init = (InitProc)EXCSTORE ## _init,*/ \
};

#define MIDDLINGEXTENDSEXCEPTION(EXCBASE, EXCNAME, ALIFEXCNAME, EXCSTORE, EXCDOC) \
    static MIDDLINGEXTENDSEXCEPTIONEX( \
        EXCBASE, EXCNAME, ALIFEXCNAME, EXCSTORE, EXCDOC); \
    AlifObject *_alifExc ## EXCNAME ## _ = (AlifObject *)&_exc ## EXCNAME ## _








AlifIntT _alifException_addNote(AlifObject* _exc, AlifObject* _note) { // 3866
	if (!ALIFEXCEPTIONINSTANCE_CHECK(_exc)) {
		alifErr_format(_alifExcTypeError_,
			"exc must be an exception, not '%s'",
			ALIF_TYPE(_exc)->name);
		return -1;
	}
	AlifObject* r = baseException_addNote(_exc, _note);
	AlifIntT res{};
	r == nullptr ? res = -1 : res = 0;
	ALIF_XDECREF(r);
	return res;
}




static AlifObject* baseException_getArgs(AlifBaseExceptionObject* self,
	void* ALIF_UNUSED(ignored)) { // 286
	if (self->args == nullptr) {
		return ALIF_NONE;
	}
	return ALIF_NEWREF(self->args);
}

static AlifIntT baseException_setArgs(AlifBaseExceptionObject* self,
	AlifObject* val, void* ALIF_UNUSED(ignored)) { // 295
	AlifObject* seq{};
	if (val == nullptr) {
		alifErr_setString(_alifExcTypeError_, "args may not be deleted");
		return -1;
	}
	seq = alifSequence_tuple(val);
	if (!seq)
		return -1;
	ALIF_XSETREF(self->args, seq);
	return 0;
}


static AlifGetSetDef _baseExceptionGetSet_[] = { // 399
	{"args", (Getter)baseException_getArgs, (Setter)baseException_setArgs},
	{nullptr},
};


static AlifTypeObject _excBaseException_ = { // 483
	.objBase = ALIFVAROBJECT_HEAD_INIT(nullptr, 0),
	.name = "خطأ_اساس",
	.basicSize = sizeof(AlifBaseExceptionObject),
	.dealloc = (Destructor)baseException_dealloc,
	//.repr = (ReprFunc)baseException_repr,
	.str = (ReprFunc)baseException_str,
	.getAttro = alifObject_genericGetAttr,
	.setAttro = alifObject_genericSetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASE_EXC_SUBCLASS,
	//.traverse = (TraverseProc)baseException_traverse,
	.clear = (Inquiry)baseException_clear,
	//.methods = baseException_methods,
	//.members = baseException_members,
	.getSet = _baseExceptionGetSet_,
	.dictOffset = offsetof(AlifBaseExceptionObject, dict),
	.init = (InitProc)baseException_init,
	.new_ = baseException_new,
	.vectorCall = baseException_vectorCall,
};

AlifObject* _alifExcBaseException_ = (AlifObject*)&_excBaseException_; // 528

// 533
#define SIMPLEEXTENDSEXCEPTION(EXCBASE, EXCNAME, ALIFNAME, EXCDOC) \
static AlifTypeObject _exc ## EXCNAME ## _ = { \
    .objBase = ALIFVAROBJECT_HEAD_INIT(nullptr, 0), \
    .name = # ALIFNAME, \
    .basicSize = sizeof(AlifBaseExceptionObject), \
    .dealloc = (Destructor)baseException_dealloc, \
    .flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC, \
    /*.traverse = (TraverseProc)baseException_traverse,*/ \
    .clear = (Inquiry)baseException_clear,	\
	.base = &EXCBASE, \
    .dictOffset = offsetof(AlifBaseExceptionObject, dict), \
    .init = (InitProc)baseException_init,	\
	.new_ = baseException_new,	\
}; \
AlifObject* _alifExc ## EXCNAME ## _ = (AlifObject *)&_exc ## EXCNAME ## _


 // 567
#define COMPLEXEXTENDSEXCEPTION(EXCBASE, EXCNAME, OBJNAME, ALIFNAME, EXCSTORE, EXCNEW, \
                                EXCMETHODS, EXCMEMBERS, EXCGETSET, \
                                EXCSTR, EXCDOC) \
static AlifTypeObject _exc ## EXCNAME ## _ = { \
    .objBase = ALIFVAROBJECT_HEAD_INIT(nullptr, 0), \
    .name = # ALIFNAME, \
    .basicSize = sizeof(Alif ## OBJNAME ## Object), \
    /*.dealloc = (Destructor)EXCSTORE ## _dealloc,*/ \
    .repr = (ReprFunc)EXCSTR, \
    .flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC, \
    /*.traverse = (TraverseProc)EXCSTORE ## _traverse,*/ \
    /*.clear = (Inquiry)EXCSTORE ## _clear,*/	\
	.methods = EXCMETHODS, \
    .members = EXCMEMBERS,	\
	.getSet = EXCGETSET,	\
	.base = &EXCBASE, \
    .dictOffset = offsetof(Alif ## OBJNAME ## Object, dict), \
    .init = (InitProc)EXCSTORE ## _init,	\
	.new_ = EXCNEW,	\
}; \
AlifObject* _alifExc ## EXCNAME ## _ = (AlifObject*)&_exc ## EXCNAME ## _



 // 586
SIMPLEEXTENDSEXCEPTION(_excBaseException_, Exception, الخطأ,
	"Common base class for all non-exit exceptions.");

// 596
SIMPLEEXTENDSEXCEPTION(_excException_, TypeError, خطأ_نوع,
	"Inappropriate argument type.");


// 603
SIMPLEEXTENDSEXCEPTION(_excException_, StopAsyncIteration, خطأ_توقف_التكرار_المتزامن,
	"Signal the end from iterator.__anext__().");




static AlifIntT stopIteration_init(AlifStopIterationObject* self,
	AlifObject* args, AlifObject* kwds) { // 617
	AlifSizeT size = ALIFTUPLE_GET_SIZE(args);
	AlifObject* value{};

	if (baseException_init((AlifBaseExceptionObject*)self, args, kwds) == -1)
		return -1;
	ALIF_CLEAR(self->value);
	if (size > 0)
		value = ALIFTUPLE_GET_ITEM(args, 0);
	else
		value = ALIF_NONE;
	self->value = ALIF_NEWREF(value);
	return 0;
}

// 656
COMPLEXEXTENDSEXCEPTION(_excException_, StopIteration, StopIteration, خطأ_توقف_التكرار,
	stopIteration, 0, 0, nullptr/*_stopIterationMembers_*/, 0, 0,
	"Signal the end from iterator.__next__().");




static AlifIntT systemExit_init(AlifSystemExitObject* self,
	AlifObject* args, AlifObject* kwds) { // 672
	AlifSizeT size = ALIFTUPLE_GET_SIZE(args);

	if (baseException_init((AlifBaseExceptionObject*)self, args, kwds) == -1)
		return -1;

	if (size == 0)
		return 0;
	if (size == 1) {
		ALIF_XSETREF(self->code, ALIF_NEWREF(ALIFTUPLE_GET_ITEM(args, 0)));
	}
	else { /* size > 1 */
		ALIF_XSETREF(self->code, ALIF_NEWREF(args));
	}
	return 0;
}

// 719
COMPLEXEXTENDSEXCEPTION(_excBaseException_, SystemExit, SystemExit, خروج_نظام,
	systemExit, 0, 0, nullptr/*_systemExitMembers_*/, 0, 0,
	"Request to exit from the interpreter.");



static inline AlifBaseExceptionGroupObject* _alifBaseExceptionGroupObject_cast(AlifObject* _exc) { // 729
	return (AlifBaseExceptionGroupObject*)_exc;
}



static AlifIntT baseExceptionGroup_init(AlifBaseExceptionGroupObject* self,
	AlifObject* args, AlifObject* kwds) { // 866
	if (!_ALIFARG_NOKEYWORDS(ALIF_TYPE(self)->name, kwds)) {
		return -1;
	}
	if (baseException_init((AlifBaseExceptionObject*)self, args, kwds) == -1) {
		return -1;
	}
	return 0;
}


COMPLEXEXTENDSEXCEPTION(_excBaseException_, BaseExceptionGroup, BaseExceptionGroup, خطأ_اساس_مجموعة,
	baseExceptionGroup, nullptr/*baseExceptionGroup_new*/ /* new */,
	nullptr/*_baseExceptionGroupMethods_*/, nullptr/*_baseExceptionGroupMembers_*/,
	0 /* getset */, nullptr/*baseExceptionGroup_str*/,
	"A combination of multiple unrelated exceptions.");


static AlifObject* createException_groupClass(void) { // 1531
	AlifExcState *state = get_excState();

	AlifObject *bases = alifTuple_pack(
		2, _alifExcBaseExceptionGroup_, _alifExcException_);
	if (bases == nullptr) {
		return nullptr;
	}

	state->alifExcExceptionGroup = alifErr_newException(
		"الضمنيات.ExceptionGroup", bases, nullptr);

	ALIF_DECREF(bases);
	return state->alifExcExceptionGroup;
}


AlifObject* _alifExc_createExceptionGroup(const char* _msgStr,
	AlifObject* _excs) { // 849
	AlifObject* msg = alifUStr_fromString(_msgStr);
	if (!msg) {
		return nullptr;
	}
	AlifObject* args = alifTuple_pack(2, msg, _excs);
	ALIF_DECREF(msg);
	if (!args) {
		return nullptr;
	}
	AlifObject* result = alifObject_callObject(_alifExcBaseExceptionGroup_, args);
	ALIF_DECREF(args);
	return result;
}


static AlifIntT importError_init(AlifImportErrorObject* self,
	AlifObject* args, AlifObject* kwds) { // 1560
	static const char* kwlist[] = {"اسم", "مسار", "اسم_من", 0};
	AlifObject* emptyTuple{};
	AlifObject* msg = nullptr;
	AlifObject* name = nullptr;
	AlifObject* path = nullptr;
	AlifObject* nameFrom = nullptr;

	if (baseException_init((AlifBaseExceptionObject*)self, args, nullptr) == -1)
		return -1;

	emptyTuple = alifTuple_new(0);
	if (!emptyTuple)
		return -1;
	if (!alifArg_parseTupleAndKeywords(emptyTuple, kwds, "|$OOO:خطأ_استيراد", kwlist,
		&name, &path, &nameFrom)) {
		ALIF_DECREF(emptyTuple);
		return -1;
	}
	ALIF_DECREF(emptyTuple);

	ALIF_XSETREF(self->name, ALIF_XNEWREF(name));
	ALIF_XSETREF(self->path, ALIF_XNEWREF(path));
	ALIF_XSETREF(self->nameFrom, ALIF_XNEWREF(nameFrom));

	if (ALIFTUPLE_GET_SIZE(args) == 1) {
		msg = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(args, 0));
	}
	ALIF_XSETREF(self->msg, msg);

	return 0;
}


static AlifObject* importError_str(AlifImportErrorObject* _self) { // 1623
	if (_self->msg and ALIFUSTR_CHECKEXACT(_self->msg)) {
		return ALIF_NEWREF(_self->msg);
	}
	else {
		return baseException_str((AlifBaseExceptionObject*)_self);
	}
}

// 1699
COMPLEXEXTENDSEXCEPTION(_excException_, ImportError, ImportError,
	خطأ_استيراد, importError, 0 /* new */,
	nullptr /*_importErrorMethods_*/, nullptr /*_importErrorMembers_*/,
	0 /* getset */, importError_str,
	"لا يمكن إيجاد وحدة الاستيراد, او لا يمكن إيجاد الاسم في الوحدة ");

// 1710
MIDDLINGEXTENDSEXCEPTION(_excImportError_, ModuleNotFoundError, خطأ_مكتبة_غير_موجودة, ImportError,
	"Module not found.");


// 1717
#ifdef _WINDOWS
#include "errmap.h"
#endif






static AlifIntT osError_parseArgs(AlifObject** p_args,
	AlifObject** myerrno, AlifObject** strerror,
	AlifObject** filename, AlifObject** filename2
#ifdef _WINDOWS
	, AlifObject** winerror
#endif
) { // 1738
	AlifSizeT nargs{};
	AlifObject* args = *p_args;
#ifndef _WINDOWS
	/*
	 * ignored on non-Windows platforms,
	 * but parsed so OSError has a consistent signature
	 */
	AlifObject* _winerror = NULL;
	AlifObject** winerror = &_winerror;
#endif /* _WINDOWS */

	nargs = ALIFTUPLE_GET_SIZE(args);

	if (nargs >= 2 && nargs <= 5) {
		if (!alifArg_unpackTuple(args, "OSError", 2, 5,
			myerrno, strerror,
			filename, winerror, filename2))
			return -1;
	#ifdef _WINDOWS
		if (*winerror and ALIFLONG_CHECK(*winerror)) {
			long errcode{}, winerrcode{};
			AlifObject* newargs{};
			AlifSizeT i{};

			winerrcode = alifLong_asLong(*winerror);
			if (winerrcode == -1 and alifErr_occurred())
				return -1;
			errcode = winerror_to_errno(winerrcode);
			*myerrno = alifLong_fromLong(errcode);
			if (!*myerrno)
				return -1;
			newargs = alifTuple_new(nargs);
			if (!newargs)
				return -1;
			ALIFTUPLE_SET_ITEM(newargs, 0, *myerrno);
			for (i = 1; i < nargs; i++) {
				AlifObject* val = ALIFTUPLE_GET_ITEM(args, i);
				ALIFTUPLE_SET_ITEM(newargs, i, ALIF_NEWREF(val));
			}
			ALIF_DECREF(args);
			args = *p_args = newargs;
		}
	#endif /* _WINDOWS */
	}

	return 0;
}


static AlifIntT oserror_init(AlifOSErrorObject* self, AlifObject** p_args,
	AlifObject* myerrno, AlifObject* strerror,
	AlifObject* filename, AlifObject* filename2
#ifdef _WINDOWS
	, AlifObject* winerror
#endif
) { // 1795
	AlifObject* args = *p_args;
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);

	if (filename and filename != ALIF_NONE) {
		if (ALIF_IS_TYPE(self, (AlifTypeObject*)_alifExcBlockingIOError_) and
			alifNumber_check(filename)) {
			/* BlockingIOError's 3rd argument can be the number of
			 * characters written.
			 */
			self->written = alifNumber_asSizeT(filename, _alifExcValueError_);
			if (self->written == -1 and alifErr_occurred())
				return -1;
		}
		else {
			self->fileName = ALIF_NEWREF(filename);

			if (filename2 and filename2 != ALIF_NONE) {
				self->fileName2 = ALIF_NEWREF(filename2);
			}

			if (nargs >= 2 and nargs <= 5) {
				AlifObject* subslice = alifTuple_getSlice(args, 0, 2);
				if (!subslice)
					return -1;

				ALIF_DECREF(args);  /* replacing args */
				*p_args = args = subslice;
			}
		}
	}
	self->myErrno = ALIF_XNEWREF(myerrno);
	self->strError = ALIF_XNEWREF(strerror);
#ifdef _WINDOWS
	self->winError = ALIF_XNEWREF(winerror);
#endif

	/* Steals the reference to args */
	ALIF_XSETREF(self->args, args);
	*p_args = args = nullptr;

	return 0;
}


static AlifObject* osError_new(AlifTypeObject*, AlifObject*, AlifObject*);
static AlifIntT osError_init(AlifOSErrorObject*, AlifObject*, AlifObject*);

static AlifIntT osError_useInit(AlifTypeObject* type) { // 1855
	if (type->init != (InitProc)osError_init and
		type->new_ == (NewFunc)osError_new) {
		return 1;
	}
	return 0;
}


static AlifObject* osError_new(AlifTypeObject* type, AlifObject* args, AlifObject* kwds) { // 1876
	AlifOSErrorObject* self = nullptr;
	AlifObject* myerrno = nullptr, * strerror = nullptr;
	AlifObject* filename = NULL, * filename2 = nullptr;
#ifdef _WINDOWS
	AlifObject* winerror = nullptr;
#endif

	ALIF_INCREF(args);

	if (!osError_useInit(type)) {
		if (!_ALIFARG_NOKEYWORDS(type->name, kwds))
			goto error;

		if (osError_parseArgs(&args, &myerrno, &strerror,
			&filename, &filename2
		#ifdef _WINDOWS
			, &winerror
		#endif
		))
			goto error;

		AlifExcState* state = get_excState();
		if (myerrno and ALIFLONG_CHECK(myerrno) and
			state->errNoMap and (AlifObject*)type == _alifExcOSError_) {
			AlifObject* newtype{};
			newtype = alifDict_getItemWithError(state->errNoMap, myerrno);
			if (newtype) {
				type = ALIFTYPE_CAST(newtype);
			}
			else if (alifErr_occurred())
				goto error;
		}
	}

	self = (AlifOSErrorObject*)type->alloc(type, 0);
	if (!self)
		goto error;

	self->dict = nullptr;
	self->traceback = self->cause = self->context = nullptr;
	self->written = -1;

	if (!osError_useInit(type)) {
		if (oserror_init(self, &args, myerrno, strerror, filename, filename2
		#ifdef _WINDOWS
			, winerror
		#endif
		))
			goto error;
	}
	else {
		self->args = alifTuple_new(0);
		if (self->args == nullptr)
			goto error;
	}

	ALIF_XDECREF(args);
	return (AlifObject*)self;

error:
	ALIF_XDECREF(args);
	ALIF_XDECREF(self);
	return NULL;
}


static AlifIntT osError_init(AlifOSErrorObject* self, AlifObject* args, AlifObject* kwds) { // 1944
	AlifObject* myerrno = nullptr, * strerror = nullptr;
	AlifObject* filename = nullptr, * filename2 = nullptr;
#ifdef _WINDOWS
	AlifObject* winerror = nullptr;
#endif

	if (!osError_useInit(ALIF_TYPE(self)))
		return 0;

	if (!_ALIFARG_NOKEYWORDS(ALIF_TYPE(self)->name, kwds))
		return -1;

	ALIF_INCREF(args);
	if (osError_parseArgs(&args, &myerrno, &strerror, &filename, &filename2
	#ifdef _WINDOWS
		, &winerror
	#endif
	))
		goto error;

	if (oserror_init(self, &args, myerrno, strerror, filename, filename2
	#ifdef _WINDOWS
		, winerror
	#endif
	))
		goto error;

	return 0;

error:
	ALIF_DECREF(args);
	return -1;
}



// 2131
static AlifMemberDef _osErrorMembers_[] = {
	{"errno", ALIF_T_OBJECT, offsetof(AlifOSErrorObject, myErrno), 0,
	/*ALIFDOC_STR("POSIX exception code")*/},
{"strerror", ALIF_T_OBJECT, offsetof(AlifOSErrorObject, strError), 0,
/*ALIFDOC_STR("exception strerror")*/},
{"filename", ALIF_T_OBJECT, offsetof(AlifOSErrorObject, fileName), 0,
/*ALIFDOC_STR("exception filename")*/},
{"filename2", ALIF_T_OBJECT, offsetof(AlifOSErrorObject, fileName2), 0,
/*ALIFDOC_STR("second exception filename")*/},
#ifdef _WINDOWS
	{"winerror", ALIF_T_OBJECT, offsetof(AlifOSErrorObject, winError), 0,
	/*ALIFDOC_STR("Win32 exception code")*/},
#endif
	{nullptr}  /* Sentinel */
};

static AlifMethodDef _osErrorMethods_[] = {

	{nullptr}
};

static AlifGetSetDef _osErrorGetSet_[] = {

	{nullptr}
};

// 2159
COMPLEXEXTENDSEXCEPTION(_excException_, OSError, OSError, خطأ_نظام_تشغيل,
	osError, osError_new,
	_osErrorMethods_, _osErrorMembers_, _osErrorGetSet_,
	nullptr/*osError_str*/,
	"Base class for I/O related errors.");


MIDDLINGEXTENDSEXCEPTION(_excOSError_, BlockingIOError, خطأ_منع_التبادل, OSError,
	"I/O operation would block."); // 2169



MIDDLINGEXTENDSEXCEPTION(_excOSError_, FileNotFoundError, خطأ_ملف_غير_موجود, OSError,
	"لم يتم العثور على الملف.");

MIDDLINGEXTENDSEXCEPTION(_excOSError_, NotADirectoryError, خطأ_ليس_مجلد, OSError,
	"العمليات تعمل فقط على المجلدات.");

MIDDLINGEXTENDSEXCEPTION(_excOSError_, PermissionError, خطأ_صلاحيات, OSError,
	"لا يوجد صلاحيات.");




/*
*    RuntimeError extends Exception
*/
SIMPLEEXTENDSEXCEPTION(_excException_, RuntimeError, خطأ_اثناء_التشغيل,
	"خطأ غير محدد أثناء التشغيل."); // 2210


/*
*    NotImplementedError extends RuntimeError
*/
SIMPLEEXTENDSEXCEPTION(_excRuntimeError_, NotImplementedError, خطأ_غير_منتهي,
	"دالة او وظيفة لم يتم الإنتهاء من العمل عليها بعد."); // 2226



/*
*    NameError extends Exception
*/

static AlifIntT nameError_init(AlifNameErrorObject* _self,
	AlifObject* _args, AlifObject* _kwds) {
	static char* kwlist[] = { (char*)"اسم", nullptr };
	AlifObject* name = nullptr;

	if (baseException_init((AlifBaseExceptionObject*)_self, _args, nullptr) == -1) {
		return -1;
	}

	AlifObject* emptyTuple = alifTuple_new(0);
	if (!emptyTuple) {
		return -1;
	}
	if (!alifArg_parseTupleAndKeywords(emptyTuple, _kwds, "|$O:خطأ_اسم", kwlist,
		&name)) {
		ALIF_DECREF(emptyTuple);
		return -1;
	}
	ALIF_DECREF(emptyTuple);

	ALIF_XSETREF(_self->name, ALIF_XNEWREF(name));

	return 0;
}

static AlifMemberDef _nameErrorMembers_[] = {
	{"اسم", ALIF_T_OBJECT, offsetof(AlifNameErrorObject, name), 0},
	{nullptr}  /* Sentinel */
};

static AlifMethodDef _nameErrorMethods_[] = {
	{nullptr}  /* Sentinel */
};

COMPLEXEXTENDSEXCEPTION(_excException_, NameError, NameError, خطأ_اسم,
	nameError, 0,
	_nameErrorMethods_, _nameErrorMembers_,
	0, baseException_str, "لم يتم العثور على الاسم في اسماء_عامة.");






static AlifIntT attributeError_init(AlifAttributeErrorObject* _self,
	AlifObject* _args, AlifObject* _kwds) { // 2306
	static const char* kwlist[] = { "اسم", "كائن", nullptr };
	AlifObject* name = nullptr;
	AlifObject* obj = nullptr;

	if (baseException_init((AlifBaseExceptionObject*)_self, _args, nullptr) == -1) {
		return -1;
	}

	AlifObject* emptyTuple = alifTuple_new(0);
	if (!emptyTuple) {
		return -1;
	}
	if (!alifArg_parseTupleAndKeywords(emptyTuple, _kwds, "|$OO:خطأ_صفة", kwlist,
		&name, &obj)) {
		ALIF_DECREF(emptyTuple);
		return -1;
	}
	ALIF_DECREF(emptyTuple);

	ALIF_XSETREF(_self->name, ALIF_XNEWREF(name));
	ALIF_XSETREF(_self->obj, ALIF_XNEWREF(obj));

	return 0;
}

static AlifMemberDef _attributeErrorMembers_[] = { // 2400
	{"اسم", ALIF_T_OBJECT, offsetof(AlifAttributeErrorObject, name), 0/*, ALIFDOC_STR("attribute name")*/},
	{"كائن", ALIF_T_OBJECT, offsetof(AlifAttributeErrorObject, obj), 0/*, ALIFDOC_STR("object")*/},
	{nullptr}  /* Sentinel */
};

// 2412
COMPLEXEXTENDSEXCEPTION(_excException_, AttributeError, AttributeError,
	خطأ_صفة, attributeError, 0,
	/*_attributeErrorMethods_*/ nullptr, _attributeErrorMembers_,
	0, baseException_str, "لم يتم العثور على الصفة 'المتغير'.");


static AlifIntT syntaxError_init(AlifSyntaxErrorObject* self,
	AlifObject* args, AlifObject* kwds) { // 2421
	AlifObject* info = nullptr;
	AlifSizeT lenargs = ALIFTUPLE_GET_SIZE(args);

	if (baseException_init((AlifBaseExceptionObject*)self, args, kwds) == -1)
		return -1;

	if (lenargs >= 1) {
		ALIF_XSETREF(self->msg, ALIF_NEWREF(ALIFTUPLE_GET_ITEM(args, 0)));
	}
	if (lenargs == 2) {
		info = ALIFTUPLE_GET_ITEM(args, 1);
		info = alifSequence_tuple(info);
		if (!info) {
			return -1;
		}

		self->endLineno = nullptr;
		self->endOffset = nullptr;
		if (!alifArg_parseTuple(info, "OOOO|OO",
			&self->filename, &self->lineno,
			&self->offset, &self->text,
			&self->endLineno, &self->endOffset)) {
			ALIF_DECREF(info);
			return -1;
		}

		ALIF_INCREF(self->filename);
		ALIF_INCREF(self->lineno);
		ALIF_INCREF(self->offset);
		ALIF_INCREF(self->text);
		ALIF_XINCREF(self->endLineno);
		ALIF_XINCREF(self->endOffset);
		ALIF_DECREF(info);

		if (self->endLineno != nullptr and self->endOffset == nullptr) {
			alifErr_setString(_alifExcTypeError_, "endOffset must be provided when endLineno is provided");
			return -1;
		}
	}
	return 0;
}


static AlifObject* my_basename(AlifObject* name) { // 2505
	AlifSizeT i{}, size{}, offset{};
	AlifIntT kind{};
	const void* data{};

	kind = ALIFUSTR_KIND(name);
	data = ALIFUSTR_DATA(name);
	size = ALIFUSTR_GET_LENGTH(name);
	offset = 0;
	for (i = 0; i < size; i++) {
		if (ALIFUSTR_READ(kind, data, i) == SEP) {
			offset = i + 1;
		}
	}
	if (offset != 0) {
		return alifUStr_subString(name, offset, size);
	}
	else {
		return ALIF_NEWREF(name);
	}
}

static AlifObject* syntaxError_str(AlifSyntaxErrorObject* self) { // 2530
	AlifIntT have_lineno = 0;
	AlifObject* filename{};
	AlifObject* result{};
	AlifIntT overflow{};

	if (self->filename and ALIFUSTR_CHECK(self->filename)) {
		filename = my_basename(self->filename);
		if (filename == nullptr)
			return nullptr;
	}
	else {
		filename = nullptr;
	}
	have_lineno = (self->lineno != nullptr) and ALIFLONG_CHECKEXACT(self->lineno);

	if (!filename and !have_lineno)
		return alifObject_str(self->msg ? self->msg : ALIF_NONE);

	if (filename and have_lineno)
		result = alifUStr_fromFormat("%S (%U, السطر %ld)",
			self->msg ? self->msg : ALIF_NONE,
			filename,
			alifLong_asLongAndOverflow(self->lineno, &overflow));
	else if (filename)
		result = alifUStr_fromFormat("%S (%U)",
			self->msg ? self->msg : ALIF_NONE,
			filename);
	else /* only have_lineno */
		result = alifUStr_fromFormat("%S (السطر %ld)",
			self->msg ? self->msg : ALIF_NONE,
			alifLong_asLongAndOverflow(self->lineno, &overflow));
	ALIF_XDECREF(filename);
	return result;
}

static AlifMemberDef _syntaxErrorMembers_[] = { // 2573
	{"msg", ALIF_T_OBJECT, offsetof(AlifSyntaxErrorObject, msg), 0},
	{"Filename", ALIF_T_OBJECT, offsetof(AlifSyntaxErrorObject, filename), 0},
	{"lineno", ALIF_T_OBJECT, offsetof(AlifSyntaxErrorObject, lineno), 0},
	{"offset", ALIF_T_OBJECT, offsetof(AlifSyntaxErrorObject, offset), 0},
	{"text", ALIF_T_OBJECT, offsetof(AlifSyntaxErrorObject, text), 0},
	{"endLineno", ALIF_T_OBJECT, offsetof(AlifSyntaxErrorObject, endLineno), 0},
	{"endOffset", ALIF_T_OBJECT, offsetof(AlifSyntaxErrorObject, endOffset), 0},
	{"PrintFileAndLine", ALIF_T_OBJECT,
		offsetof(AlifSyntaxErrorObject, printFileAndLine), 0},
	{nullptr}  /* Sentinel */
};

COMPLEXEXTENDSEXCEPTION(_excException_, SyntaxError, SyntaxError, خطأ_نسق,
	syntaxError, 0, 0, _syntaxErrorMembers_, 0,
	syntaxError_str, "خطأ في النسق"); // 2594



MIDDLINGEXTENDSEXCEPTION(_excSyntaxError_, IndentationError, خطأ_مسافة_بادئة, SyntaxError,
	"مسافة طويلة 'tab' غير صحيحة"); // 2602

MIDDLINGEXTENDSEXCEPTION(_excIndentationError_, TabError, خطأ_مسافة_طويلة, SyntaxError,
	"Improper mixture of spaces and tabs."); // 2609


SIMPLEEXTENDSEXCEPTION(_excException_, LookupError, خطأ_بحث,
	"Base class for lookup errors."); // 2621

SIMPLEEXTENDSEXCEPTION(_excLookupError_, IndexError, خطأ_مؤشر,
	"Sequence index out of range."); // 2628



/*
*    KeyError extends LookupError
*/
static AlifObject* keyError_str(AlifBaseExceptionObject* _self) {
	if (ALIFTUPLE_GET_SIZE(_self->args) == 1) {
		return alifObject_repr(ALIFTUPLE_GET_ITEM(_self->args, 0));
	}
	return baseException_str(_self);
}

COMPLEXEXTENDSEXCEPTION(_excLookupError_, KeyError, BaseException, خطأ_مفتاح, baseException,
	0, 0, 0, 0, keyError_str, "مفتاح الفهرسة غير موجود.");




SIMPLEEXTENDSEXCEPTION(_excException_, ValueError, خطأ_قيمة,
	"Inappropriate argument value (of correct type)."); // 2660


SIMPLEEXTENDSEXCEPTION(_excException_, ArithmeticError, خطأ_حسابي,
	"Base class for arithmetic errors.");

// 3310
SIMPLEEXTENDSEXCEPTION(_excException_, SystemError, خطأ_نظام,
	"Internal error in the Alif interpreter.\n");



SIMPLEEXTENDSEXCEPTION(_excArithmeticError_, OverflowError, خطأ_فائض,
	"Result too large to be represented.");



/*
*    UnicodeError extends ValueError
*/

SIMPLEEXTENDSEXCEPTION(_excValueError_, UnicodeError, خطأ_ترميز,
	"Unicode related error."); // 2663


static AlifIntT set_unicodeFromString(AlifObject** _attr,
	const char* _value) { // 2701
	AlifObject* obj = alifUStr_fromString(_value);
	if (!obj)
		return -1;
	ALIF_XSETREF(*_attr, obj);
	return 0;
}


AlifIntT alifUnicodeEncodeError_setStart(AlifObject* _exc,
	AlifSizeT _start) { // 2785
	((AlifUnicodeErrorObject*)_exc)->start = _start;
	return 0;
}


AlifIntT alifUnicodeEncodeError_setEnd(AlifObject* _exc,
	AlifSizeT _end) { // 2853
	((AlifUnicodeErrorObject*)_exc)->end = _end;
	return 0;
}


AlifIntT alifUnicodeEncodeError_setReason(AlifObject* _exc,
	const char* _reason) { // 2897
	return set_unicodeFromString(&((AlifUnicodeErrorObject*)_exc)->reason,
		_reason);
}



static AlifIntT unicodeEncodeError_init(AlifObject* _self,
	AlifObject* _args, AlifObject* _kwds) { // 2966
	AlifUnicodeErrorObject* err{};

	if (baseException_init((AlifBaseExceptionObject*)_self, _args, _kwds) == -1)
		return -1;

	err = (AlifUnicodeErrorObject*)_self;

	ALIF_CLEAR(err->encoding);
	ALIF_CLEAR(err->object);
	ALIF_CLEAR(err->reason);

	if (!alifArg_parseTuple(_args, "UUnnU",
		&err->encoding, &err->object,
		&err->start, &err->end, &err->reason)) {
		err->encoding = err->object = err->reason = NULL;
		return -1;
	}

	ALIF_INCREF(err->encoding);
	ALIF_INCREF(err->object);
	ALIF_INCREF(err->reason);

	return 0;
}

static AlifTypeObject _excUnicodeEncodeError_ = { // 3054
	.objBase = ALIFVAROBJECT_HEAD_INIT(nullptr, 0),
	.name = "خطأ_ترميز_نص",
	.basicSize = sizeof(AlifUnicodeErrorObject),
	//.dealloc = (Destructor)unicodeError_dealloc,
	//.repr = (ReprFunc)unicodeEncodeError_str,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = (TraverseProc)unicodeError_traverse,
	//.clear = (Inquiry)unicodeError_clear,
	//.members = _unicodeErrorMembers_,
	.base = &_excUnicodeError_,
	.dictOffset = offsetof(AlifUnicodeErrorObject, dict),
	.init = (InitProc)unicodeEncodeError_init,
	.new_ = baseException_new,
};
AlifObject* _alifExcUnicodeEncodeError_ = (AlifObject*)&_excUnicodeEncodeError_;





class StaticException { // 3610
public:
	AlifTypeObject* exc{};
	const char* name{};
};

static StaticException _staticExceptions_[] = { // 3615
#define ITEM(_name, _alifName) {&_exc##_name##_, #_alifName}
	// Level 1
	ITEM(BaseException, خطأ_اساس),

	// Level 2: BaseException subclasses
	ITEM(BaseExceptionGroup, خطأ_اساس_مجموعة),
	ITEM(Exception, الخطأ),
	//ITEM(GeneratorExit),
	//ITEM(KeyboardInterrupt),
	ITEM(SystemExit, خروج_نظام),

	// Level 3: Exception(BaseException) subclasses
	ITEM(ArithmeticError, خطأ_حسابي),
	//ITEM(AssertionError),
	ITEM(AttributeError, خطأ_صفة),
	//ITEM(BufferError),
	//ITEM(EOFError),
	//ITEM(ExceptionGroup),
	ITEM(ImportError, خطأ_استيراد),
	ITEM(LookupError, خطأ_بحث),
	//ITEM(MemoryError),
	ITEM(NameError, خطأ_اسم),
	ITEM(OSError, خطأ_نظام_تشغيل),
	//ITEM(ReferenceError),
	ITEM(RuntimeError, خطأ_اثناء_التشغيل),
	ITEM(StopAsyncIteration, خطأ_توقف_التكرار_المتزامن),
	ITEM(StopIteration, خطأ_توقف_التكرار),
	ITEM(SyntaxError, خطأ_نسق),
	ITEM(SystemError, خطأ_نظام),
	ITEM(TypeError, خطأ_نوع),
	ITEM(ValueError, خطأ_قيمة),
	//ITEM(Warning),

	// Level 4: ArithmeticError(Exception) subclasses
	//ITEM(FloatingPointError),
	ITEM(OverflowError, خطأ_فائض),
	//ITEM(ZeroDivisionError),

	// Level 4: Warning(Exception) subclasses
	//ITEM(BytesWarning),
	//ITEM(DeprecationWarning),
	//ITEM(EncodingWarning),
	//ITEM(FutureWarning),
	//ITEM(ImportWarning),
	//ITEM(PendingDeprecationWarning),
	//ITEM(ResourceWarning),
	//ITEM(RuntimeWarning),
	//ITEM(SyntaxWarning),
	//ITEM(UnicodeWarning),
	//ITEM(UserWarning),

	// Level 4: OSError(Exception) subclasses
	ITEM(BlockingIOError, خطأ_منع_التبادل),
	//ITEM(ChildProcessError),
	//ITEM(ConnectionError),
	//ITEM(FileExistsError),
	ITEM(FileNotFoundError, خطأ_ملف_غير_موجود),
	//ITEM(InterruptedError),
	//ITEM(IsADirectoryError),
	ITEM(NotADirectoryError, خطأ_ليس_مجلد),
	ITEM(PermissionError, خطأ_صلاحيات),
	//ITEM(ProcessLookupError),
	//ITEM(TimeoutError),

	// Level 4: Other subclasses
	ITEM(IndentationError, خطأ_مسافة_بادئة), // base: SyntaxError(Exception)
	//{&_alifExcIncompleteInputError_, "_IncompleteInputError"}, // base: SyntaxError(Exception)
	ITEM(IndexError, خطأ_مؤشر),  // base: LookupError(Exception)
	ITEM(KeyError, خطأ_مفتاح),  // base: LookupError(Exception)
	ITEM(ModuleNotFoundError, خطأ_مكتبة_غير_موجودة), // base: ImportError(Exception)
	ITEM(NotImplementedError, خطأ_غير_منتهي),  // base: RuntimeError(Exception)
	//ITEM(AlifFinalizationError),  // base: RuntimeError(Exception)
	//ITEM(RecursionError),  // base: RuntimeError(Exception)
	//ITEM(UnboundLocalError), // base: NameError(Exception)
	ITEM(UnicodeError, خطأ_ترميز),  // base: ValueError(Exception)

	// Level 5: ConnectionError(OSError) subclasses
	//ITEM(BrokenPipeError),
	//ITEM(ConnectionAbortedError),
	//ITEM(ConnectionRefusedError),
	//ITEM(ConnectionResetError),

	// Level 5: IndentationError(SyntaxError) subclasses
	ITEM(TabError, خطأ_مسافة_طويلة),  // base: IndentationError

	// Level 5: UnicodeError(ValueError) subclasses
	//ITEM(UnicodeDecodeError),
	ITEM(UnicodeEncodeError, خطأ_ترميز_نص),
	//ITEM(UnicodeTranslateError),
#undef ITEM
};



AlifIntT _alifExc_initTypes(AlifInterpreter* _interp) { // 3709
	for (AlifUSizeT i = 0; i < ALIF_ARRAY_LENGTH(_staticExceptions_); i++) {
		AlifTypeObject* exc = _staticExceptions_[i].exc;
		if (alifStaticType_initBuiltin(_interp, exc) < 0) {
			return -1;
		}
		if (exc->new_ == baseException_new
			and exc->init == (InitProc)baseException_init) {
			exc->vectorCall = baseException_vectorCall;
		}
	}
	return 0;
}



/* Add exception types to the builtins module */
AlifIntT _alifBuiltins_addExceptions(AlifObject *_bltinMod) { // 3828
	AlifObject *mod_dict = alifModule_getDict(_bltinMod);
	if (mod_dict == nullptr) {
		return -1;
	}

	for (size_t i=0; i < ALIF_ARRAY_LENGTH(_staticExceptions_); i++) {
		StaticException item = _staticExceptions_[i];

		if (alifDict_setItemString(mod_dict, item.name, (AlifObject*)item.exc)) {
			return -1;
		}
	}

	AlifObject* alifExcExceptionGroup = createException_groupClass();
	if (!alifExcExceptionGroup) {
		return -1;
	}
	if (alifDict_setItemString(mod_dict, "ExceptionGroup", alifExcExceptionGroup)) {
		return -1;
	}

#define INIT_ALIAS(_name, _type) \
    do { \
        _alifExc ## _name ## _ = _alifExc ## _type ## _; \
        if (alifDict_setItemString(mod_dict, # _name, _alifExc ## _type ## _)) { \
            return -1; \
        } \
    } while (0)

	INIT_ALIAS(EnvironmentError, OSError);
	INIT_ALIAS(IOError, OSError);
#ifdef _WINDOWS
	INIT_ALIAS(WindowsError, OSError);
#endif

#undef INIT_ALIAS

	return 0;
}
