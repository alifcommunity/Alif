#pragma once







class AlifCodeObject {  // 192
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
};



#define CO_MAXBLOCKS 21 /* Max static block nesting within a function */ // 229
