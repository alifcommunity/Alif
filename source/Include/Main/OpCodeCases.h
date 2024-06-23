#pragma once



TARGET(RESUME) {
	_frame->instrPtr = nextInstr;
	nextInstr += 1;
	AlifCodeUnit* thisInstr = nextInstr - 1;
	if (_thread->tracing == 0) {
		AlifCodeObject* code = alifFrame_getCode(_frame);
		if (thisInstr->op.code == RESUME) {
#if ENABLE_SPECIALIZATION
			thisInstr->op.code = RESUME_CHECK;
#endif  /* ENABLE_SPECIALIZATION */
		}
	}
	//if ((opArg & RESUME_OPARG_LOCATION_MASK) < RESUME_AFTER_YIELD_FROM) {
	//	CHECK_EVAL_BREAKER();
	//}
	DISPATCH();
}
