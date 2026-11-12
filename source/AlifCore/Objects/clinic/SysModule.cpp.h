















// 81
#define SYS_EXCEPTHOOK_METHODDEF    \
    {"خطاف_الخلل", ALIF_CPPFUNCTION_CAST(sys_excepthook), METHOD_FASTCALL}

static AlifObject* sys_excepthookImpl(AlifObject* module, AlifObject* exctype,
	AlifObject* value, AlifObject* traceback);

static AlifObject* sys_excepthook(AlifObject* module, AlifObject* const* args, AlifSizeT nargs) { // 88
	AlifObject* return_value = nullptr;
	AlifObject* exctype;
	AlifObject* value;
	AlifObject* traceback;

	if (!_ALIFARG_CHECKPOSITIONAL("خطاف_الخلل", nargs, 3, 3)) {
		goto exit;
	}
	exctype = args[0];
	value = args[1];
	traceback = args[2];
	return_value = sys_excepthookImpl(module, exctype, value, traceback);

exit:
	return return_value;
}








// 229
#define SYS_EXIT_METHODDEF    \
    {"اخرج", ALIF_CPPFUNCTION_CAST(sys_exit), METHOD_FASTCALL}

static AlifObject* sys_exitImpl(AlifObject*, AlifObject*);

static AlifObject* sys_exit(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs) { // 235
	AlifObject* returnValue{};
	AlifObject* status = ALIF_NONE;

	if (!_ALIFARG_CHECKPOSITIONAL("اخرج", _nargs, 0, 1)) {
		goto exit;
	}
	if (_nargs < 1) {
		goto skip_optional;
	}
	status = _args[0];
skip_optional:
	returnValue = sys_exitImpl(_module, status);

exit:
	return returnValue;
}
