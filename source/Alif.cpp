#include "Alif.h"

typedef class _AlifArgv { // تحت المراجعة
public:
	int argc;
	int use_bytes_argv;
	char* const* bytes_argv;
	wchar_t* const* wchar_argv;
} _AlifArgv;


int Alif_RunMain(void)
{
	//int exitcode = 0;

	//alifmain_run_alif(&exitcode);

	//if (Alif_FinalizeEx() < 0) {
	//	/* Value unlikely to be confused with a non-error exit status or
	//	   other special meaning */
	//	exitcode = 120;
	//}

	//alifmain_free();

	//if (_ALifRuntime.signals.unhandled_keyboard_interrupt) {
	//	exitcode = exit_sigint();
	//}

	//return exitcode;
	return 1;
}


static int alif_main(_AlifArgv* args)
{
	//AlifStatus status = alifmain_init(args);
	//if (_AlifStatus_IS_EXIT(status)) {
	//	alifmain_free();
	//	return status.exitcode;
	//}
	//if (_ALifStatus_EXCEPTION(status)) {
	//	alifmain_exit_error(status);
	//}

	return Alif_RunMain();
}


int Alif_Main(int argc, wchar_t** argv)
{
	_AlifArgv args = {
		.argc = argc,
		.use_bytes_argv = 0,
		.bytes_argv = NULL,
		.wchar_argv = argv
	};

	return alif_main(&args);
}


int Alif_MainByte(int argc, char** argv)
{
	_AlifArgv args = {
		.argc = argc,
		.use_bytes_argv = 1,
		.bytes_argv = argv,
		.wchar_argv = NULL
	};

	return alif_main(&args);
}

#ifdef MS_WINDOWS
int wmain(int argc, wchar_t** argv)
{
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	return Alif_Main(2, argsv);
}
#else
int main(int argc, char** argv)
{
	char* argsv[] = { "alif5", "example.alif5" };
	return Alif_MainByte(2, argsv);
}
#endif


