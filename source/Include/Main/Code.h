#pragma once






class AlifBackoffCounter {
	union {
		class {
		public:
			uint16_t backoff : 4;
			uint16_t value : 12;
		};
		uint16_t asCounter;
	};
};

union AlifCodeUnit {
	class {
	public:
		uint8_t code{};
		uint8_t arg{};
	}op;
	AlifBackoffCounter counter{};
};


class AlifCodeObject {  // 192
public:
	ALIFOBJECT_VAR_HEAD;
	AlifObject* consts{};
	AlifObject* names{};

	AlifIntT flags;

	AlifIntT args{};
	AlifIntT posOnlyArgCount;
	AlifIntT kwOnlyArgCount;
	AlifIntT stackSize{};
	AlifIntT firstLineNo{};
	AlifIntT nLocalsPlus{};
	AlifIntT frameSize{};
	AlifIntT nLocals{};
	AlifIntT version{};

	AlifObject* localsPlusNames{};
	AlifObject* localsPlusKinds{};

	AlifObject* fileName{};
	AlifObject* name{};
	AlifObject* qualName{};
	AlifObject* lineTable{};

	AlifIntT firstTraceable{};

	wchar_t codeAdaptive[1]; // changed to wchar_t
};

/* Masks for flags above */
#define CO_OPTIMIZED    0x0001
#define CO_NEWLOCALS    0x0002
#define CO_VARARGS      0x0004
#define CO_VARKEYWORDS  0x0008
#define CO_NESTED       0x0010
#define CO_GENERATOR    0x0020

#define CO_MAXBLOCKS 21 /* Max static block nesting within a function */ // 229

extern AlifTypeObject _alifCodeType_;


#define ALIFCODE_CODE(_co) ALIF_RVALUE((AlifCodeUnit*)(_co)->codeAdaptive)
