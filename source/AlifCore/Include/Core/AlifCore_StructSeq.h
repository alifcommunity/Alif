#pragma once







extern AlifIntT _alifStructSequence_initBuiltinWithFlags(AlifInterpreter*,
	AlifTypeObject*, AlifStructSequenceDesc*, unsigned long); // 19

static inline AlifIntT _alifStructSequence_initBuiltin(AlifInterpreter* interp,
	AlifTypeObject* type, AlifStructSequenceDesc* desc) { // 25
	return _alifStructSequence_initBuiltinWithFlags(interp, type, desc, 0);
}
