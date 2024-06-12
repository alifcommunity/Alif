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

	AlifIntT args{};
	AlifIntT stackSize{};
	AlifIntT firstLineNo{};
	AlifIntT frameSize{};
	AlifIntT nLocals{};
	AlifIntT version{};

	AlifObject* fileName{};
	AlifObject* name{};
	AlifObject* qualName{};
	AlifObject* lineTable{};



	char codeAdaptive[1];
};



#define CO_MAXBLOCKS 21 /* Max static block nesting within a function */ // 229

extern AlifTypeObject _alifCodeType_;
