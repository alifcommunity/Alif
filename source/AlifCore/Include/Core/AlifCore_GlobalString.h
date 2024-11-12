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


class AlifGlobalStrings {
public:
    class {
	public:
        STRUCT_FOR_STR(Empty, "")
		STRUCT_FOR_STR(format, ".format")
        //STRUCT_FOR_STR(Newline, "\n")
        //STRUCT_FOR_STR(OpenBr, "{")
        //STRUCT_FOR_STR(Percent, "%")
        //STRUCT_FOR_STR(UTF_8, "utf-8")
    } literals;

    class {
	public:
        STRUCT_FOR_ID(CANCELLED)
        STRUCT_FOR_ID(__classGetItem__)
		STRUCT_FOR_ID(__class__)
		STRUCT_FOR_ID(__classDict__)
		STRUCT_FOR_ID(__doc__)
		STRUCT_FOR_ID(__getAttr__)
		STRUCT_FOR_ID(__hash__)
		STRUCT_FOR_ID(__lengthHint__)
		STRUCT_FOR_ID(__loader__)
		STRUCT_FOR_ID(__name__)
		STRUCT_FOR_ID(__new__)
		STRUCT_FOR_ID(__origClass__)
		STRUCT_FOR_ID(__package__)
		STRUCT_FOR_ID(__spec__)
     	STRUCT_FOR_ID(hasLocation)
     	STRUCT_FOR_ID(mro)
     	STRUCT_FOR_ID(origin)
     	STRUCT_FOR_ID(top)
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
