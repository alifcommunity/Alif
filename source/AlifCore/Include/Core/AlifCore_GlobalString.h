#pragma once


#define STRUCT_FOR_ASCII_STR(_litr) \
    class {							\
	public:							\
        AlifASCIIObject ascii{};	\
        uint8_t data[sizeof(_litr)]; \
    }
#define STRUCT_FOR_STR(_name, _litr) \
    STRUCT_FOR_ASCII_STR(_litr) alif ## _name;
#define STRUCT_FOR_ID(_name) \
    STRUCT_FOR_ASCII_STR(#_name) alif ## _name;


//* alif
// هذا القسم خاص بالنصوص ذات الترميز الحرفي الثنائي
// فقط "الاحرف العربية" والذي يحتاج الى عملية بحث عنه في القاموس
// ولكن يجب تحويله من 16 الى 8 قبل طباعته
// طول النص يساوي عدد احرف الكلمة
// النوع يجب ان يكون 2 لأنه يتم البحث عن الاحرف ك 2 ثمانية "بايت" لكل حرف بغض النظر عن النظام المستخدم
// هذا النظام يعطي كائن ألف مطابق لما ترجعه الدالة alifUStr_fromString("نص")
// جرب AlifUStrObject* str = (AlifUStrObject*)alifUStr_fromString("نص")
#define STRUCT_FOR_USTR_STR(_litr) \
    class {							\
	public:							\
        AlifASCIIObject ascii{};	\
        AlifSizeT utf8Length{};        \
        char* utf8{};        \
        const char16_t data[sizeof(u ## _litr)]{}; \
    }

#define STRUCT_FOR_USTR(_name, _litr) \
    STRUCT_FOR_USTR_STR(#_litr) alif ## _name;
//* alif






class AlifGlobalStrings {
public:
	class {
	public:
		STRUCT_FOR_USTR(_windowsConsoleIO, "طرفية_ويندوز")
		STRUCT_FOR_USTR(_findAndLoad, "_اوجد_وحمل")
		STRUCT_FOR_USTR(_initializing, "_تهيئة")
		STRUCT_FOR_USTR(_handleFromList, "_تعامل_من_القائمة")
		STRUCT_FOR_STR(AnonGenExpr, "<genexpr>")
		STRUCT_FOR_USTR(AnonLambda, "<خطية>")
		STRUCT_FOR_USTR(AnonListComp, "<مصفوفة_ضمنية>")
		STRUCT_FOR_USTR(AnonSetComp, "<مميزة_ضمنية>")
		STRUCT_FOR_USTR(AnonDictComp, "<فهرس_ضمني>")
		STRUCT_FOR_USTR(AnonModule, "<وحدة>") //* alif
		STRUCT_FOR_USTR(AnonString, "<نص>")
		STRUCT_FOR_STR(Builtins, "الضمنيات")
		STRUCT_FOR_STR(DblPercent, "%%")
		STRUCT_FOR_STR(Defaults, ".defaults")
		STRUCT_FOR_STR(DotLocals, ".<locals>")
		STRUCT_FOR_STR(Empty, "")
		STRUCT_FOR_STR(utf_8, "utf-8")
		STRUCT_FOR_USTR(Excepthook, "خطاف_الخلل")
		STRUCT_FOR_STR(Format, ".format")
		STRUCT_FOR_STR(GenericBase, ".generic_base")
		STRUCT_FOR_STR(IsAtty, "isatty") //* alif
		STRUCT_FOR_STR(KWDefaults, ".kwdefaults")
		STRUCT_FOR_STR(TypeParams, ".type_params")
		STRUCT_FOR_USTR(True, "صح") //* alif
		STRUCT_FOR_USTR(False, "خطأ") //* alif
		STRUCT_FOR_USTR(Flush, "مباشر") //* alif
		STRUCT_FOR_USTR(Globals, "اسماء_عامة") //* alif
		STRUCT_FOR_USTR(Locals, "اسماء_محلية") //* alif
		STRUCT_FOR_USTR(ListErr, "مؤشر المصفوفة خارج النطاق")
		STRUCT_FOR_USTR(lambda, "خطية") //* alif
		STRUCT_FOR_STR(Mode, "mode") //* alif
		STRUCT_FOR_USTR(MaxSplit, "التكرار")
		STRUCT_FOR_USTR(Name, "اسم") //* alif
		STRUCT_FOR_STR(Obj, "obj") //* alif
		STRUCT_FOR_USTR(Object, "كائن") //* alif
		STRUCT_FOR_USTR(Open, "افتح") //* alif
		STRUCT_FOR_USTR(ReadLine, "اقرا_سطر") //* alif
		STRUCT_FOR_USTR(Read, "اقرا") //* alif
		STRUCT_FOR_USTR(Close, "اغلق") //* alif
		STRUCT_FOR_USTR(Replace, "استبدل") //* alif
		STRUCT_FOR_STR(Raw, "raw") //* alif
		STRUCT_FOR_USTR(End, "النهاية") //* alif
		STRUCT_FOR_USTR(Sep, "الفاصل") //* alif
		STRUCT_FOR_USTR(Sort, "رتب") //* alif
		STRUCT_FOR_USTR(Stdout, "مخرجات") //* alif
		STRUCT_FOR_USTR(TextIOWrapper, "غلاف_النص") //* alif
		STRUCT_FOR_USTR(Write, "اكتب") //* alif
		STRUCT_FOR_USTR(FromList, "من_مصفوفة") //* alif
		STRUCT_FOR_USTR(Level, "مستوى") //* alif
		STRUCT_FOR_USTR(__abs__, "__مطلق__") //* alif
		STRUCT_FOR_USTR(__all__, "__كل__") //* alif
		STRUCT_FOR_USTR(__add__, "__اجمع__") //* alif
		STRUCT_FOR_USTR(__bytes__, "__ثمانيات__") //* alif
		STRUCT_FOR_USTR(__call__, "__استدعاء__") //* alif
		STRUCT_FOR_USTR(__format__, "__تنسيق__") //* alif
		STRUCT_FOR_USTR(__fspath__, "__مسار_ملف_نظام__") //* alif
		STRUCT_FOR_USTR(__getnewargs__, "__اجلب_وسيطات_جديدة__") //* alif
		STRUCT_FOR_USTR(__init__, "__تهيئة__") //* alif
		STRUCT_FOR_USTR(__import__, "__استورد__") //* alif
		STRUCT_FOR_USTR(__loader__, "__محمل__")
		STRUCT_FOR_USTR(__mul__, "__اضرب__") //* alif
		STRUCT_FOR_USTR(__module__, "__وحدة__")
		STRUCT_FOR_USTR(__new__, "__جديد__")
		STRUCT_FOR_USTR(__neg__, "__سالب__") //* alif
		STRUCT_FOR_USTR(__name__, "__اسم__")
		STRUCT_FOR_USTR(__pow__, "__اس__") //* alif
		STRUCT_FOR_USTR(__package__, "__حزمة__") //* alif
		STRUCT_FOR_USTR(__qualname__, "__اسم_مميز__")
		STRUCT_FOR_USTR(__rpow__, "__اس_ع__") //* alif
		STRUCT_FOR_USTR(__radd__, "__اجمع_ع__") //* alif
		STRUCT_FOR_USTR(__repr__, "__عرض__") //* alif
		STRUCT_FOR_USTR(__reversed__, "__معكوس__") //* alif
		STRUCT_FOR_USTR(__round__, "__قرب__") //* alif
		STRUCT_FOR_USTR(__rmul__, "__اضرب_ع__") //* alif
		STRUCT_FOR_USTR(__rsub__, "__اطرح_ع__") //* alif
		STRUCT_FOR_USTR(__sub__, "__اطرح__") //* alif
		STRUCT_FOR_USTR(__main__, "__الرئيسية__") //* alif
		STRUCT_FOR_USTR(__dict__, "__فهرس__")
		STRUCT_FOR_USTR(__spec__, "__خصائص__")
		STRUCT_FOR_USTR(__enter__, "__ادخل__")
		STRUCT_FOR_USTR(__exit__, "__اخرج__")
		STRUCT_FOR_USTR(__path__, "__مسار__")
		STRUCT_FOR_USTR(Path, "مسار")
		STRUCT_FOR_USTR(Parent, "الوالد")
		STRUCT_FOR_USTR(_io, "تبادل") //* alif
		STRUCT_FOR_USTR(nt, "واجهة_نظام_ويندوز")
		STRUCT_FOR_USTR(posix, "واجهة_نظام_يونكس")
	} literals;

	class {
	public:
		STRUCT_FOR_ID(CANCELLED)
		STRUCT_FOR_ID(AlifRepr)
		STRUCT_FOR_ID(__abstractMethods__)
		STRUCT_FOR_ID(__annotate__)
		STRUCT_FOR_ID(__bases__)
		STRUCT_FOR_ID(__buildClass__)
		STRUCT_FOR_ID(__builtins__)
		STRUCT_FOR_ID(__class__)
		STRUCT_FOR_ID(__classCell__)
		STRUCT_FOR_ID(__classDict__)
		STRUCT_FOR_ID(__classDictCell__)
		STRUCT_FOR_ID(__classGetItem__)
		STRUCT_FOR_ID(__complex__)
		STRUCT_FOR_ID(__dictOffset__)
		STRUCT_FOR_ID(__doc__)
		STRUCT_FOR_ID(__eq__)
		STRUCT_FOR_ID(__file__)
		STRUCT_FOR_ID(__firstLineno__)
		STRUCT_FOR_ID(__getAttr__)
		STRUCT_FOR_ID(__hash__)
		STRUCT_FOR_ID(__initSubclass__)
		STRUCT_FOR_ID(__instanceCheck__)
		STRUCT_FOR_ID(__IOBaseClosed)
		STRUCT_FOR_ID(__lengthHint__)
		STRUCT_FOR_ID(__mroEntries__)
		STRUCT_FOR_ID(__notes__)
		STRUCT_FOR_ID(__origClass__)
		STRUCT_FOR_ID(__prepare__)
		STRUCT_FOR_ID(__setName__)
		STRUCT_FOR_ID(__slots__)
		STRUCT_FOR_ID(__staticAttributes__)
		STRUCT_FOR_ID(__subClassCheck__)
		STRUCT_FOR_ID(__typeParams__)
		STRUCT_FOR_ID(__weakListOffset__)
		STRUCT_FOR_ID(__weakRef__)
		STRUCT_FOR_ID(_blksize)
		STRUCT_FOR_ID(_deallocWarn)
		STRUCT_FOR_ID(_fieldTypes)
		STRUCT_FOR_ID(_isTextEncoding)
		STRUCT_FOR_ID(_isAttyOpenOnly)
		STRUCT_FOR_ID(_WindowsConsoleIO)
		STRUCT_FOR_ID(Buffer)
		STRUCT_FOR_ID(Buffering)
		STRUCT_FOR_ID(BufferSize)
		STRUCT_FOR_ID(Closed)
		STRUCT_FOR_ID(CloseFD)
		STRUCT_FOR_ID(Closure)
		STRUCT_FOR_ID(Code)
		STRUCT_FOR_ID(Count)
		STRUCT_FOR_ID(Decode)
		STRUCT_FOR_ID(Decoder)
		STRUCT_FOR_ID(DictComp)
		STRUCT_FOR_ID(Doc)
		STRUCT_FOR_ID(Encode)
		STRUCT_FOR_ID(Encoding)
		STRUCT_FOR_ID(EndColOffset)
		STRUCT_FOR_ID(EndLineno)
		STRUCT_FOR_ID(EndOffset)
		STRUCT_FOR_ID(Errors)
		STRUCT_FOR_ID(File)
		STRUCT_FOR_ID(Filename)
		STRUCT_FOR_ID(Fileno)
		STRUCT_FOR_ID(GenExpr)
		STRUCT_FOR_ID(GetState)
		STRUCT_FOR_ID(HasLocation)
		STRUCT_FOR_ID(Join)
		STRUCT_FOR_ID(Key)
		STRUCT_FOR_ID(keys)
		STRUCT_FOR_ID(LastExc)
		STRUCT_FOR_ID(LastTraceback)
		STRUCT_FOR_ID(LastType)
		STRUCT_FOR_ID(LastValue)
		STRUCT_FOR_ID(LineBuffering)
		STRUCT_FOR_ID(Lineno)
		STRUCT_FOR_ID(ListComp)
		STRUCT_FOR_ID(MetaClass)
		STRUCT_FOR_ID(Mode)
		STRUCT_FOR_ID(Mro)
		STRUCT_FOR_ID(Msg)
		STRUCT_FOR_ID(Newline)
		STRUCT_FOR_ID(NFields)
		STRUCT_FOR_ID(NSequenceFields)
		STRUCT_FOR_ID(Offset)
		STRUCT_FOR_ID(Opener)
		STRUCT_FOR_ID(Origin)
		STRUCT_FOR_ID(Peek)
		STRUCT_FOR_ID(PrintFileAndLine)
		STRUCT_FOR_ID(Readable)
		STRUCT_FOR_ID(Read)
		STRUCT_FOR_ID(Read1)
		STRUCT_FOR_ID(ReadAll)
		STRUCT_FOR_ID(ReadInto)
		STRUCT_FOR_ID(Reset)
		STRUCT_FOR_ID(Reverse)
		STRUCT_FOR_ID(Seekable)
		STRUCT_FOR_ID(SetComp)
		STRUCT_FOR_ID(Start)
		STRUCT_FOR_ID(Stderr)
		STRUCT_FOR_ID(Stdin)
		STRUCT_FOR_ID(strict)
		STRUCT_FOR_ID(TBFrame)
		STRUCT_FOR_ID(TBLasti)
		STRUCT_FOR_ID(TBLineno)
		STRUCT_FOR_ID(TBNext)
		STRUCT_FOR_ID(Text)
		STRUCT_FOR_ID(Top)
		STRUCT_FOR_ID(Translate)
		STRUCT_FOR_ID(WithData)
		STRUCT_FOR_ID(Writable)
		STRUCT_FOR_ID(WriteThrough)
	} identifiers;

	class {
	public:
		AlifASCIIObject ascii;
		uint8_t data[2];
	} ascii[128];

	class {
	public:
		AlifCompactUStrObject latin1;
		uint8_t data[2];
	} latin1[128];
};


#define ALIF_ID(_name) \
     (ALIF_SINGLETON(strings.identifiers.alif ## _name.ascii.objBase))
#define ALIF_STR(_name) \
     (ALIF_SINGLETON(strings.literals.alif ## _name.ascii.objBase))
#define ALIF_LATIN1_CHR(_ch) \
    ((_ch) < 128 \
     ? (AlifObject*)&ALIF_SINGLETON(strings).ascii[(_ch)] \
     : (AlifObject*)&ALIF_SINGLETON(strings).latin1[(_ch) - 128])

#define ALIF_DECLARE_STR(name, str)
