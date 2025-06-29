















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
