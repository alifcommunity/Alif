#include "AlifCore_Abstract.h"
#include "AlifCore_Long.h"
#include "AlifCore_ModSupport.h"




// 38
#define OS_STAT_METHODDEF    \
    {"حالة", ALIF_CPPFUNCTION_CAST(os_stat), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* os_statImpl(AlifObject*, PathT*, AlifIntT, AlifIntT);

static AlifObject* os_stat(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs, AlifObject* _kwnames) { // 44
	AlifObject* returnValue = nullptr;

#define KWTUPLE nullptr

	static const char* const _keywords[] = { "مسار", "مجلد_واصف_ملف", "روابط_رمزية", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "حالة",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[3];
	AlifSizeT noptargs = _nargs + (_kwnames ? ALIFTUPLE_GET_SIZE(_kwnames) : 0) - 1;
	PathT path = PATH_T_INITIALIZE_P("حالة", "مسار", 0, 0, 0, 1);
	AlifIntT dir_fd = DEFAULT_DIR_FD;
	AlifIntT followSymlinks = 1;

	_args = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &_parser,
		/*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	if (!path_converter(_args[0], &path)) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_kwonly;
	}
	if (_args[1]) {
		if (!FSTATAT_DIR_FD_CONVERTER(_args[1], &dir_fd)) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	followSymlinks = alifObject_isTrue(_args[2]);
	if (followSymlinks < 0) {
		goto exit;
	}
skip_optional_kwonly:
	returnValue = os_statImpl(_module, &path, dir_fd, followSymlinks);

exit:
	/* Cleanup for path */
	path_cleanup(&path);

	return returnValue;
}






















// 1408
#define OS_GETCWD_METHODDEF    \
    {"اجلب_مسار_العمل", (AlifCPPFunction)os_getcwd, METHOD_NOARGS},

static AlifObject* os_getcwdImpl(AlifObject*);

static AlifObject* os_getcwd(AlifObject* _module,
	AlifObject* ALIF_UNUSED(ignored)) { // 1414
	return os_getcwdImpl(_module);
}



// 1566
#define OS_LISTDIR_METHODDEF    \
    {"محتويات_المجلد", ALIF_CPPFUNCTION_CAST(os_listdir), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* os_listdirImpl(AlifObject*, PathT*);

static AlifObject* os_listdir(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs, AlifObject* _kwnames) { // 1572
	AlifObject* returnValue{};

	//* todo
#  define KWTUPLE nullptr

	static const char* const _keywords[] = { "مسار", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "محتويات_المجلد",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[1];
	AlifSizeT noptargs = _nargs + (_kwnames ? ALIFTUPLE_GET_SIZE(_kwnames) : 0) - 0;
	PathT path = PATH_T_INITIALIZE_P("محتويات_المجلد", "مسار", 1, 0, 0, PATH_HAVE_FDOPENDIR);

	_args = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &_parser,
		/*minpos*/ 0, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (!path_converter(_args[0], &path)) {
		goto exit;
	}
skip_optional_pos:
	returnValue = os_listdirImpl(_module, &path);

exit:
	/* Cleanup for path */
	path_cleanup(&path);

	return returnValue;
}





#if defined(_WINDOWS) // 1972

// 1980
#define OS__PATH_SPLITROOT_METHODDEF    \
    {"_المسار_افصل_الجذر", ALIF_CPPFUNCTION_CAST(os__pathSplitRoot), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* os__pathSplitRootImpl(AlifObject*, PathT*);

static AlifObject* os__pathSplitRoot(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs, AlifObject* _kwnames) { // 1986
	AlifObject* returnValue{};
#define KWTUPLE nullptr

	static const char* const _keywords[] = { "المسار", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "_المسار_افصل_الجذر",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[1]{};
	PathT path = PATH_T_INITIALIZE_P("_المسار_افصل_الجذر", "المسار", 0, 0, 0, 0);

	_args = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &_parser,
		/*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	if (!path_converter(_args[0], &path)) {
		goto exit;
	}
	returnValue = os__pathSplitRootImpl(_module, &path);

exit:
	/* Cleanup for path */
	path_cleanup(&path);

	return returnValue;
}

#endif // 2035





// 2595
#define OS_MKDIR_METHODDEF    \
    {"اوجد_مجلد", ALIF_CPPFUNCTION_CAST(os_mkdir), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* os_mkdirImpl(AlifObject*, PathT*, AlifIntT, AlifIntT);

static AlifObject* os_mkdir(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 2601
	AlifObject* return_value{};

	//* todo
#  define KWTUPLE nullptr

	static const char* const _keywords[] = { "مسار", "وضع", "مجلد_واصف_ملف", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "اوجد_مجلد",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[3];
	AlifSizeT noptargs = _nargs + (_kwnames ? ALIFTUPLE_GET_SIZE(_kwnames) : 0) - 1;
	PathT path = PATH_T_INITIALIZE_P("اوجد_مجلد", "مسار", 0, 0, 0, 0);
	AlifIntT mode = 511;
	AlifIntT dir_fd = DEFAULT_DIR_FD;

	_args = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &_parser,
		/*minpos*/ 1, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	if (!path_converter(_args[0], &path)) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (_args[1]) {
		mode = alifLong_asInt(_args[1]);
		if (mode == -1 and alifErr_occurred()) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
skip_optional_pos:
	if (!noptargs) {
		goto skip_optional_kwonly;
	}
	if (!MKDIRAT_DIR_FD_CONVERTER(_args[2], &dir_fd)) {
		goto exit;
	}
skip_optional_kwonly:
	return_value = os_mkdirImpl(_module, &path, mode, dir_fd);

exit:
	/* Cleanup for path */
	path_cleanup(&path);

	return return_value;
}








































// 11419
#define OS_CPU_COUNT_METHODDEF    \
    {"عدد_المعالجات", (AlifCPPFunction)os_cpuCount, METHOD_NOARGS},

static AlifObject* os_cpuCountImpl(AlifObject*);

static AlifObject* os_cpuCount(AlifObject* _module,
	AlifObject* ALIF_UNUSED(ignored)) { // 11425
	return os_cpuCountImpl(_module);
}





















// 12542
#ifndef OS__PATH_SPLITROOT_METHODDEF
    #define OS__PATH_SPLITROOT_METHODDEF
#endif /* !defined(OS__PATH_SPLITROOT_METHODDEF) */














