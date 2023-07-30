#pragma once


/* ___________ AlifStatus ___________ */

class AlifStatus
{
public:
	int type; // 0 -> Ok , 1 -> Error ,  2 -> Exit
	const wchar_t* func;
	const wchar_t* mesError;
	int exitCode;
};

/* ___________ AlifWideStringList ___________ */

class AlifWideStringList {
public:
	/* If length is greater than zero, items must be non-NULL
	   and all items strings must be non-NULL */
	Alif_ssize_t length;
	wchar_t** items;
};


/* ___________ AlifConfig ___________ */

/* This structure is best documented in the Doc/c-api/init_config.rst file. */
class AlifConfig {
public:
	int _config_init;     /* _PyConfigInitEnum value */

	int isolated;
	int use_environment;
	int dev_mode;
	int install_signal_handlers;
	int use_hash_seed;
	unsigned long hash_seed;
	int faulthandler;
	int tracemalloc;
	int perf_profiling;
	int import_time;
	int code_debug_ranges;
	int show_ref_count;
	int dump_refs;
	wchar_t* dump_refs_file;
	int malloc_stats;
	wchar_t* filesystem_encoding;
	wchar_t* filesystem_errors;
	wchar_t* pycache_prefix;
	int parse_argv;
	AlifWideStringList orig_argv;
	AlifWideStringList argv;
	AlifWideStringList xoptions;
	AlifWideStringList warnoptions;
	int site_import;
	int bytes_warning;
	int warn_default_encoding;
	int inspect;
	int interactive;
	int optimization_level;
	int parser_debug;
	int write_bytecode;
	int verbose;
	int quiet;
	int user_site_directory;
	int configure_c_stdio;
	int buffered_stdio;
	wchar_t* stdio_encoding;
	wchar_t* stdio_errors;
#ifdef MS_WINDOWS
	int legacy_windows_stdio;
#endif
	wchar_t* check_hash_pycs_mode;
	int use_frozen_modules;
	int safe_path;
	int int_max_str_digits;

	/* --- Path configuration inputs ------------ */
	int pathconfig_warnings;
	wchar_t* program_name;
	wchar_t* pythonpath_env;
	wchar_t* home;
	wchar_t* platlibdir;

	/* --- Path configuration outputs ----------- */
	int module_search_paths_set;
	//PyWideStringList module_search_paths;
	wchar_t* stdlib_dir;
	wchar_t* executable;
	wchar_t* base_executable;
	wchar_t* prefix;
	wchar_t* base_prefix;
	wchar_t* exec_prefix;
	wchar_t* base_exec_prefix;

	/* --- Parameter only used by Py_Main() ---------- */
	int skip_source_first_line;
	wchar_t* run_command;
	wchar_t* run_module;
	wchar_t* run_filename;

	/* --- Private fields ---------------------------- */

	// Install importlib? If equals to 0, importlib is not initialized at all.
	// Needed by freeze_importlib.
	int _install_importlib;

	// If equal to 0, stop Python initialization before the "main" phase.
	int _init_main;

	// If non-zero, we believe we're running from a source tree.
	int _is_python_build;
};


AlifStatus alifConfig_setArgv(AlifConfig* config, Alif_ssize_t argc, wchar_t* const* argv);
