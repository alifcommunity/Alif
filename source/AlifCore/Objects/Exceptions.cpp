#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Exceptions.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Errors.h"

#include "OSDefs.h"



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

	self->args = alifTuple_fromArray(args, ALIFVECTORCALL_NARGS(nargsf));
	if (!self->args) {
		ALIF_DECREF(self);
		return nullptr;
	}

	return (AlifObject*)self;
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
		alifErr_format(nullptr /*_alifExcTypeError_*/,
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
		alifErr_setString(nullptr /*_alifExcTypeError_*/, "Cannot add note: __notes__ is not a list");
		return nullptr;
	}
	if (alifList_append(notes, note) < 0) {
		ALIF_DECREF(notes);
		return nullptr;
	}
	ALIF_DECREF(notes);
	return ALIF_NONE;
}


static AlifIntT baseException_setTB(AlifBaseExceptionObject* self, AlifObject* tb, void* ALIF_UNUSED(ignored)) { // 319
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

#define MIDDLINGEXTENDSEXCEPTION(EXCBASE, EXCNAME, EXCSTORE, EXCDOC) \
    static MIDDLINGEXTENDSEXCEPTIONEX( \
        EXCBASE, EXCNAME, EXCNAME, EXCSTORE, EXCDOC); \
    AlifObject *_alifExc ## EXCNAME ## _ = (AlifObject *)&_exc ## EXCNAME ## _








AlifIntT _alifException_addNote(AlifObject* _exc, AlifObject* _note) { // 3866
	if (!ALIFEXCEPTIONINSTANCE_CHECK(_exc)) {
		alifErr_format(nullptr /*_alifExcTypeError_*/,
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
	.name = "استثناء_قاعدة",
	.basicSize = sizeof(AlifBaseExceptionObject),
	//.dealloc = (Destructor)baseException_dealloc,
	//.repr = (ReprFunc)baseException_repr,
	//.str = (ReprFunc)baseException_str,
	.getAttro = alifObject_genericGetAttr,
	.setAttro = alifObject_genericSetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASE_EXC_SUBCLASS,
	//.traverse = (TraverseProc)baseException_traverse,
	//(Inquiry)baseException_clear,
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
#define SIMPLEEXTENDSEXCEPTION(EXCBASE, EXCNAME, EXCDOC) \
static AlifTypeObject _exc ## EXCNAME ## _ = { \
    .objBase = ALIFVAROBJECT_HEAD_INIT(nullptr, 0), \
    .name = # EXCNAME, \
    .basicSize = sizeof(AlifBaseExceptionObject), \
    /*.dealloc = (Destructor)baseException_dealloc,*/ \
    .flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC, \
    /*.traverse = (TraverseProc)baseException_traverse,*/ \
    /*.clear = (Inquiry)baseException_clear,*/	\
	.base = &EXCBASE, \
    .dictOffset = offsetof(AlifBaseExceptionObject, dict), \
    .init = (InitProc)baseException_init,	\
	.new_ = baseException_new,	\
}; \
AlifObject* _alifExc ## EXCNAME ## _ = (AlifObject *)&_exc ## EXCNAME ## _


 // 567
#define COMPLEXEXTENDSEXCEPTION(EXCBASE, EXCNAME, NAME, EXCSTORE, EXCNEW, \
                                EXCMETHODS, EXCMEMBERS, EXCGETSET, \
                                EXCSTR, EXCDOC) \
static AlifTypeObject _exc ## EXCNAME ## _ = { \
    .objBase = ALIFVAROBJECT_HEAD_INIT(nullptr, 0), \
    .name = # NAME, \
    .basicSize = sizeof(Alif ## EXCNAME ## Object), \
    /*.dealloc = (Destructor)EXCSTORE ## _dealloc,*/ \
    .repr = (ReprFunc)EXCSTR, \
    .flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC, \
    /*.traverse = (TraverseProc)EXCSTORE ## _traverse,*/ \
    /*.clear = (Inquiry)EXCSTORE ## _clear,*/	\
	.methods = EXCMETHODS, \
    .members = EXCMEMBERS,	\
	.getSet = EXCGETSET,	\
	.base = &EXCBASE, \
    .dictOffset = offsetof(Alif ## EXCNAME ## Object, dict), \
    .init = (InitProc)EXCSTORE ## _init,	\
	.new_ = EXCNEW,	\
}; \
AlifObject* _alifExc ## EXCNAME ## _ = (AlifObject*)&_exc ## EXCNAME ## _



 // 586
SIMPLEEXTENDSEXCEPTION(_excBaseException_, Exception,
	"Common base class for all non-exit exceptions.");

// 596
SIMPLEEXTENDSEXCEPTION(_excException_, TypeError, 
	"Inappropriate argument type.");


// 603
SIMPLEEXTENDSEXCEPTION(_excException_, StopAsyncIteration,
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
COMPLEXEXTENDSEXCEPTION(_excException_, StopIteration, خطأ_تكرار_توقف, stopIteration,
	0, 0, nullptr/*_stopIterationMembers_*/, 0, 0,
	"Signal the end from iterator.__next__().");



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


COMPLEXEXTENDSEXCEPTION(_excBaseException_, BaseExceptionGroup, خطأ_اساس_مجموعة,
	baseExceptionGroup, nullptr/*baseExceptionGroup_new*/ /* new */,
	nullptr/*_baseExceptionGroupMethods_*/, nullptr/*_baseExceptionGroupMembers_*/,
	0 /* getset */, nullptr/*baseExceptionGroup_str*/,
	"A combination of multiple unrelated exceptions.");





AlifObject* _alifExc_createExceptionGroup(const char* _msgStr, AlifObject* _excs) { // 849
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
	//* todo
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
COMPLEXEXTENDSEXCEPTION(_excException_, ImportError,
	خطأ_استيراد, importError, 0 /* new */,
	nullptr /*_importErrorMethods_*/, nullptr /*_importErrorMembers_*/,
	0 /* getset */, importError_str,
	"لا يمكن إيجاد وحدة الاستيراد, او لا يمكن إيجاد الاسم في الوحدة ");

 // 1710
MIDDLINGEXTENDSEXCEPTION(_excImportError_, ModuleNotFoundError, ImportError,
	"Module not found.");


// 1717
#ifdef _WINDOWS
#include "errmap.h"
#endif




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
		result = alifUStr_fromFormat("%S (%U, line %ld)",
			self->msg ? self->msg : ALIF_NONE,
			filename,
			alifLong_asLongAndOverflow(self->lineno, &overflow));
	else if (filename)
		result = alifUStr_fromFormat("%S (%U)",
			self->msg ? self->msg : ALIF_NONE,
			filename);
	else /* only have_lineno */
		result = alifUStr_fromFormat("%S (line %ld)",
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
	{"printFileAndLine", ALIF_T_OBJECT,
		offsetof(AlifSyntaxErrorObject, printFileAndLine), 0},
	{nullptr}  /* Sentinel */
};

COMPLEXEXTENDSEXCEPTION(_excException_, SyntaxError, خطأ_نسق, syntaxError,
	0, 0, _syntaxErrorMembers_, 0,
	syntaxError_str, "خطأ في النسق"); // 2594




SIMPLEEXTENDSEXCEPTION(_excException_, LookupError, خطأ_بحث
	"Base class for lookup errors."); // 2621

SIMPLEEXTENDSEXCEPTION(_excLookupError_, IndexError, خطأ_مؤشر
	"Sequence index out of range."); // 2628





SIMPLEEXTENDSEXCEPTION(_excException_, ValueError, خطأ_قيمة
	"Inappropriate argument value (of correct type)."); // 2660




 // 3310
SIMPLEEXTENDSEXCEPTION(_excException_, SystemError,
	"Internal error in the Alif interpreter.\n");




class StaticException { // 3610
public:
	AlifTypeObject* exc{};
	const char* name{};
};

static StaticException _staticExceptions_[] = { // 3615
#define ITEM(_name) {&_exc##_name##_, #_name}
	// Level 1
	ITEM(BaseException),

	// Level 2: BaseException subclasses
	ITEM(BaseExceptionGroup),
	ITEM(Exception),
	//ITEM(GeneratorExit),
	//ITEM(KeyboardInterrupt),
	//ITEM(SystemExit),

	// Level 3: Exception(BaseException) subclasses
	//ITEM(ArithmeticError),
	//ITEM(AssertionError),
	//ITEM(AttributeError),
	//ITEM(BufferError),
	//ITEM(EOFError),
	//ITEM(ExceptionGroup),
	ITEM(ImportError),
	ITEM(LookupError),
	//ITEM(MemoryError),
	//ITEM(NameError),
	//ITEM(OSError),
	//ITEM(ReferenceError),
	//ITEM(RuntimeError),
	ITEM(StopAsyncIteration),
	ITEM(StopIteration),
	ITEM(SyntaxError),
	ITEM(SystemError),
	ITEM(TypeError),
	//ITEM(ValueError),
	//ITEM(Warning),

	// Level 4: ArithmeticError(Exception) subclasses
	//ITEM(FloatingPointError),
	//ITEM(OverflowError),
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
	//ITEM(BlockingIOError),
	//ITEM(ChildProcessError),
	//ITEM(ConnectionError),
	//ITEM(FileExistsError),
	//ITEM(FileNotFoundError),
	//ITEM(InterruptedError),
	//ITEM(IsADirectoryError),
	//ITEM(NotADirectoryError),
	//ITEM(PermissionError),
	//ITEM(ProcessLookupError),
	//ITEM(TimeoutError),

	// Level 4: Other subclasses
	//ITEM(IndentationError), // base: SyntaxError(Exception)
	//{&_alifExcIncompleteInputError_, "_IncompleteInputError"}, // base: SyntaxError(Exception)
	ITEM(IndexError),  // base: LookupError(Exception)
	//ITEM(KeyError),  // base: LookupError(Exception)
	//ITEM(ModuleNotFoundError), // base: ImportError(Exception)
	//ITEM(NotImplementedError),  // base: RuntimeError(Exception)
	//ITEM(AlifFinalizationError),  // base: RuntimeError(Exception)
	//ITEM(RecursionError),  // base: RuntimeError(Exception)
	//ITEM(UnboundLocalError), // base: NameError(Exception)
	//ITEM(UnicodeError),  // base: ValueError(Exception)

	// Level 5: ConnectionError(OSError) subclasses
	//ITEM(BrokenPipeError),
	//ITEM(ConnectionAbortedError),
	//ITEM(ConnectionRefusedError),
	//ITEM(ConnectionResetError),

	// Level 5: IndentationError(SyntaxError) subclasses
	//ITEM(TabError),  // base: IndentationError

	// Level 5: UnicodeError(ValueError) subclasses
	//ITEM(UnicodeDecodeError),
	//ITEM(UnicodeEncodeError),
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
			and exc->init == (InitProc)baseException_init)
		{
			exc->vectorCall = baseException_vectorCall;
		}
	}
	return 0;
}
