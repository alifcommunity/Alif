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
