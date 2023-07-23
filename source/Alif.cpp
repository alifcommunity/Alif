#include "Alif.h"


typedef class _AlifArgv { // تحت المراجعة
public:
	int argc;
	int use_bytes_argv;
	char* const* bytes_argv;
	wchar_t* const* wchar_argv;
} _AlifArgv;


static int alif_main(_AlifArgv* args)
{
	//PyStatus status = pymain_init(args);
	//if (_PyStatus_IS_EXIT(status)) {
	//	pymain_free();
	//	return status.exitcode;
	//}
	//if (_PyStatus_EXCEPTION(status)) {
	//	pymain_exit_error(status);
	//}

	//return Py_RunMain();
	return 1;
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
	wchar_t* argsv[] = { L"alif", L"example.alif" };
	return Alif_Main(2, argsv);
}
#else
int main(int argc, char** argv)
{
	char* argsv[] = { "alif5", "example.alif5" };
	return Alif_MainByte(2, argsv);
}
#endif


