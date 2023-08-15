#pragma once















































class IS {
public:
	CevalState ceval;

	AlifInterpreterState* next;

	int64_t id;
	int64_t id_refcount;
	int requires_idref;
	PyThread_type_lock id_mutex;

	int _initialized;
	int finalizing;

	uint64_t monitoring_version;
	uint64_t last_restart_version;
	struct pythreads {
		uint64_t next_unique_id;
		PyThreadState* head;
		long count;

		size_t stacksize;
	} threads;

	pyruntimestate* runtime;

	_Py_atomic_address _finalizing;

	_gc_runtime_state gc;

	PyObject* sysdict;

	PyObject* builtins;

	_import_state imports;

	_gil_runtime_state _gil;


	PyObject* codec_search_path;
	PyObject* codec_search_cache;
	PyObject* codec_error_registry;
	int codecs_initialized;

	PyConfig config;
	unsigned long feature_flags;

	PyObject* dict; 

	PyObject* sysdict_copy;
	PyObject* builtins_copy;
	_PyFrameEvalFunction eval_frame;

	PyFunction_WatchCallback func_watchers[FUNC_MAX_WATCHERS];
	uint8_t active_func_watchers;

	Py_ssize_t co_extra_user_count;
	freefunc co_extra_freefuncs[MAX_CO_EXTRA_USERS];

#ifdef HAVE_FORK
	PyObject* before_forkers;
	PyObject* after_forkers_parent;
	PyObject* after_forkers_child;
#endif

	_warnings_runtime_state warnings;
	atexit_state atexit;

	_obmalloc_state obmalloc;

	PyObject* audit_hooks;
	PyType_WatchCallback type_watchers[TYPE_MAX_WATCHERS];
	PyCode_WatchCallback code_watchers[CODE_MAX_WATCHERS];
	uint8_t active_code_watchers;

	_py_object_state object_state;
	_Py_unicode_state unicode;
	_Py_float_state float_state;
	_Py_long_state long_state;
	_dtoa_state dtoa;
	_py_func_state func_state;

	PySliceObject* slice_cache;

	_Py_tuple_state tuple;
	_Py_list_state list;
	_Py_dict_state dict_state;
	_Py_async_gen_state async_gen;
	_Py_context_state context;
	_Py_exc_state exc_state;

	ast_state ast;
	types_state types;
	callable_cache callable_cache;
	_PyOptimizerObject* optimizer;
	uint16_t optimizer_resume_threshold;
	uint16_t optimizer_backedge_threshold;

	_Py_Monitors monitors;
	bool f_opcode_trace_set;
	bool sys_profile_initialized;
	bool sys_trace_initialized;
	Py_ssize_t sys_profiling_threads; 
	Py_ssize_t sys_tracing_threads;
	PyObject* monitoring_callables[PY_MONITORING_TOOL_IDS][_PY_MONITORING_EVENTS];
	PyObject* monitoring_tool_names[PY_MONITORING_TOOL_IDS];

	_Py_interp_cached_objects cached_objects;
	_Py_interp_static_objects static_objects;

	AlifThreadState _initial_thread;
};
