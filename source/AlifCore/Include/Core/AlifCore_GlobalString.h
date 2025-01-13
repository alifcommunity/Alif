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


// alif //
// هذا القسم خاص بالنصوص ذات الترميز الحرفي الثنائي فقط "الاحرف العربية"
// طول الكلمة يساوي عدد الاحرف زائد 1 لقاطع النص
// نوع البيانات يجب ان يكون const char16_t لأنه يتم البحث عن الاحرف ك 2 بايت لكل حرف بغض النظر عن النظام المستخدم
#define STRUCT_FOR_USTR(_litr) \
    class {							\
	public:							\
        AlifASCIIObject ascii{};	\
        const char16_t data[sizeof(u ## _litr) / 2]; \
    }
#define STRUCT_FOR_UID(_name, _litr) \
    STRUCT_FOR_USTR(#_litr) alif ## _name;
// alif //



class AlifGlobalStrings {
public:
    class {
	public:
		STRUCT_FOR_STR(AnonModule, "<module>")
        STRUCT_FOR_STR(DblPercent, "%%")
		STRUCT_FOR_STR(Defaults, ".defaults")
		STRUCT_FOR_STR(DotLocals, ".<locals>")
        STRUCT_FOR_STR(Empty, "")
		STRUCT_FOR_STR(Format, ".format")
		STRUCT_FOR_STR(GenericBase, ".generic_base")
		STRUCT_FOR_STR(KWDefaults, ".kwdefaults")
		STRUCT_FOR_STR(TypeParams, ".type_params")
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
		STRUCT_FOR_ID(__call__)
		STRUCT_FOR_ID(__class__)
		STRUCT_FOR_ID(__classCell__)
		STRUCT_FOR_ID(__classDict__)
		STRUCT_FOR_ID(__classDictCell__)
        STRUCT_FOR_ID(__classGetItem__)
		STRUCT_FOR_ID(__complex__)
		STRUCT_FOR_ID(__dict__)
		STRUCT_FOR_ID(__doc__)
		STRUCT_FOR_ID(__eq__)
		STRUCT_FOR_ID(__firstLineno__)
		STRUCT_FOR_ID(__format__)
		STRUCT_FOR_ID(__getAttr__)
		STRUCT_FOR_ID(__hash__)
		STRUCT_FOR_UID(__init__, _تهيئة_)
		STRUCT_FOR_ID(__initSubclass__)
		STRUCT_FOR_ID(__lengthHint__)
		STRUCT_FOR_ID(__loader__)
		STRUCT_FOR_ID(__module__)
		STRUCT_FOR_ID(__mroEntries__)
		STRUCT_FOR_ID(__name__)
		STRUCT_FOR_ID(__new__)
		STRUCT_FOR_ID(__notes__)
		STRUCT_FOR_ID(__origClass__)
		STRUCT_FOR_ID(__package__)
		STRUCT_FOR_ID(__prepare__)
		STRUCT_FOR_ID(__qualname__)
		STRUCT_FOR_ID(__setName__)
		STRUCT_FOR_ID(__slots__)
		STRUCT_FOR_ID(__spec__)
		STRUCT_FOR_ID(__staticAttributes__)
        STRUCT_FOR_ID(__subClassCheck__)
		STRUCT_FOR_ID(__typeParams__)
		STRUCT_FOR_ID(__weakRef__)
		STRUCT_FOR_ID(Builtins)
     	STRUCT_FOR_ID(Encoding)
     	STRUCT_FOR_ID(End)
     	STRUCT_FOR_ID(Errors)
     	STRUCT_FOR_ID(False)
     	STRUCT_FOR_ID(خطأ) //* alif
     	STRUCT_FOR_ID(File)
     	STRUCT_FOR_ID(Fileno)
     	STRUCT_FOR_ID(Flush)
     	STRUCT_FOR_ID(HasLocation)
     	STRUCT_FOR_ID(Join)
     	STRUCT_FOR_ID(keys)
     	STRUCT_FOR_ID(MetaClass)
     	STRUCT_FOR_ID(Mro)
     	STRUCT_FOR_ID(Origin)
     	STRUCT_FOR_ID(Sep)
     	STRUCT_FOR_ID(Stderr)
     	STRUCT_FOR_ID(Stdin)
     	STRUCT_FOR_ID(Stdout)
     	STRUCT_FOR_ID(Top)
     	STRUCT_FOR_ID(True)
     	STRUCT_FOR_ID(صح) //* alif
     	STRUCT_FOR_ID(Write)
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
#define ALIF_UID(_name) \
     (ALIF_SINGLETON(strings.identifiers.alif ## _name.unicode.base.base.objBase))
#define ALIF_STR(_name) \
     (ALIF_SINGLETON(strings.literals.alif ## _name.ascii.objBase))
#define ALIF_LATIN1_CHR(_ch) \
    ((_ch) < 128 \
     ? (AlifObject*)&ALIF_SINGLETON(strings).ascii[(_ch)] \
     : (AlifObject*)&ALIF_SINGLETON(strings).latin1[(_ch) - 128])

#define ALIF_DECLARE_STR(name, str)
