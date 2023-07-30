#include "Alif.h"
#include "alifcore_initConfig.h"    // _AlifArgv



//static int runtimeInitialized = 0;

//AlifStatus _alifruntime_initialize()
//{
//	if (runtimeInitialized) {
//		return _AlifStatus_OK();
//	}
//
//	runtimeInitialized = 1;
//
//	return _AlifRuncTimeState_init();
//}


static AlifStatus alifmain_init(const _AlifArgv* args) {

	AlifStatus status{};
	//AlifStatus status = _alifruntime_initialize();

	//if (status.type != 0) {
	//	return status;
	//}

	AlifConfig config;


	if (args->use_bytes_argv) {
		//status = PyConfig_SetBytesArgv(&config, args->argc, args->bytes_argv);
	}
	else {
		status = alifConfig_setArgv(&config, args->argc, args->wchar_argv);
	}

	return status;
}


static void alifmain_run(int* exitcode)
{
//	PyObject* main_importer_path = NULL;
//	PyInterpreterState* interp = _PyInterpreterState_GET();
//	/* pymain_run_stdin() modify the config */
//	PyConfig* config = (PyConfig*)_PyInterpreterState_GetConfig(interp);
//
//	/* ensure path config is written into global variables */
//	if (_PyStatus_EXCEPTION(_PyPathConfig_UpdateGlobal(config))) {
//		goto error;
//	}
//
//	if (config->run_filename != NULL) {
//		/* If filename is a package (ex: directory or ZIP file) which contains
//		   __main__.py, main_importer_path is set to filename and will be
//		   prepended to sys.path.
//
//		   Otherwise, main_importer_path is left unchanged. */
//		if (pymain_get_importer(config->run_filename, &main_importer_path,
//			exitcode)) {
//			return;
//		}
//	}
//
//	// import readline and rlcompleter before script dir is added to sys.path
//	pymain_import_readline(config);
//
//	if (main_importer_path != NULL) {
//		if (pymain_sys_path_add_path0(interp, main_importer_path) < 0) {
//			goto error;
//		}
//	}
//	else if (!config->safe_path) {
//		PyObject* path0 = NULL;
//		int res = _PyPathConfig_ComputeSysPath0(&config->argv, &path0);
//		if (res < 0) {
//			goto error;
//		}
//
//		if (res > 0) {
//			if (pymain_sys_path_add_path0(interp, path0) < 0) {
//				Py_DECREF(path0);
//				goto error;
//			}
//			Py_DECREF(path0);
//		}
//	}
//
//	pymain_header(config);
//
//	if (config->run_command) {
//		*exitcode = pymain_run_command(config->run_command);
//	}
//	else if (config->run_module) {
//		*exitcode = pymain_run_module(config->run_module, 1);
//	}
//	else if (main_importer_path != NULL) {
//		*exitcode = pymain_run_module(L"__main__", 0);
//	}
//	else if (config->run_filename != NULL) {
//		*exitcode = pymain_run_file(config);
//	}
//	else {
//		*exitcode = pymain_run_stdin(config);
//	}
//
//	pymain_repl(config, exitcode);
//	goto done;
//
//error:
//	*exitcode = pymain_exit_err_print();
//
//done:
//	Py_XDECREF(main_importer_path);
}


static int Alif_RunMain()
{
	int exitcode = 0;

	alifmain_run(&exitcode);

	return exitcode;
}


static int alifmain_main(_AlifArgv* args)
{
	AlifStatus status = alifmain_init(args);

	return Alif_RunMain();
}


int Alif_MainWchar(int argc, wchar_t** argv)
{
	_AlifArgv args = {
		.argc = argc,
		.use_bytes_argv = 0,
		.bytes_argv = nullptr,
		.wchar_argv = argv
	};

	return alifmain_main(&args);
}


int Alif_MainChar(int argc, char** argv)
{
	_AlifArgv args = {
		.argc = argc,
		.use_bytes_argv = 1,
		.bytes_argv = argv,
		.wchar_argv = nullptr
	};

	return alifmain_main(&args);
}

#ifdef MS_WINDOWS
int wmain(int argc, wchar_t** argv)
{
	wchar_t* argsv[] = { (wchar_t*)L"alif", (wchar_t*)L"example.alif" };
	return Alif_MainWchar(2, argsv);
}
#else
int main(int argc, char** argv)
{
	char* argsv[] = { "alif", "example.alif" };
	return Alif_MainChar(2, argsv);
}
#endif


