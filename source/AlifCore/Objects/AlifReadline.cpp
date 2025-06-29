#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_State.h"



extern AlifThread* _alifOSReadlineThread_; // 28
AlifThread* _alifOSReadlineThread_ = nullptr;

static AlifMutex _alifOSReadlineLock_;


AlifIntT (*_alifOSInputHook_)(void) = nullptr; // 33

static AlifIntT my_fgets(AlifThread* tstate,
	char* buf, AlifIntT len, FILE* fp) { // 38
#ifdef _WINDOWS
	HANDLE handle;
	ALIF_BEGIN_SUPPRESS_IPH
	handle = (HANDLE)_get_osfhandle(fileno(fp));
	ALIF_END_SUPPRESS_IPH

	if (handle == INVALID_HANDLE_VALUE) {
		return -1; /* EOF */
	}
#endif

	while (1) {
		if (_alifOSInputHook_ != nullptr &&
			alif_isMainInterpreter(tstate->interpreter))
		{
			(void)(_alifOSInputHook_)();
		}

		errno = 0;
		clearerr(fp);
		char* p = fgets(buf, len, fp);
		if (p != nullptr) {
			return 0; /* No error */
		}
		AlifIntT err = errno;

#ifdef _WINDOWS
		if (GetLastError() == ERROR_OPERATION_ABORTED) {
			HANDLE hInterruptEvent = _alifOS_sigintEvent();
			switch (WaitForSingleObjectEx(hInterruptEvent, 10, FALSE)) {
			case WAIT_OBJECT_0:
				ResetEvent(hInterruptEvent);
				return 1; /* Interrupt */
			case WAIT_FAILED:
				return -2; /* Error */
			}
		}
#endif /* _WINDOWS */

		if (feof(fp)) {
			clearerr(fp);
			return -1; /* EOF */
		}

#ifdef EINTR
		if (err == EINTR) {
			alifEval_restoreThread(tstate);
			//AlifIntT s = alifErr_checkSignals(); //* todo
			alifEval_saveThread();

			//if (s < 0) {
			//	return 1;
			//}
			/* try again */
			continue;
		}
#endif

		//if (_alifOS_interruptOccurred(tstate)) {
		//	return 1; /* Interrupt */
		//}
		return -2; /* Error */
	}
	/* NOTREACHED */
}


#ifdef HAVE_WINDOWS_CONSOLE_IO // 122
/* Readline implementation using ReadConsoleW */

extern char _get_consoleType(HANDLE); // 125

char* _alifOS_windowsConsoleReadline(AlifThread* tstate, HANDLE hStdIn) { // 127
	static wchar_t wbuf_local[1024 * 16]{};
	const DWORD chunk_size = 1024;

	DWORD n_read{}, total_read{}, wbuflen{}, u8len{};
	wchar_t* wbuf{};
	char* buf = nullptr;
	int err = 0;

	n_read = (DWORD)-1;
	total_read = 0;
	wbuf = wbuf_local;
	wbuflen = sizeof(wbuf_local) / sizeof(wbuf_local[0]) - 1;
	while (1) {
		if (_alifOSInputHook_ != nullptr and
			alif_isMainInterpreter(tstate->interpreter))
		{
			(void)(_alifOSInputHook_)();
		}
		if (!ReadConsoleW(hStdIn, &wbuf[total_read], wbuflen - total_read, &n_read, nullptr)) {
			err = GetLastError();
			goto exit;
		}
		if (n_read == (DWORD)-1 and (err = GetLastError()) == ERROR_OPERATION_ABORTED) {
			break;
		}
		if (n_read == 0) {
			AlifIntT s{};
			err = GetLastError();
			if (err != ERROR_OPERATION_ABORTED)
				goto exit;
			err = 0;
			HANDLE hInterruptEvent = _alifOS_sigintEvent();
			if (WaitForSingleObjectEx(hInterruptEvent, 100, FALSE)
				== WAIT_OBJECT_0) {
				ResetEvent(hInterruptEvent);
				alifEval_restoreThread(tstate);
				//s = alifErr_checkSignals();
				alifEval_saveThread();
				if (s < 0) {
					goto exit;
				}
			}
			break;
		}

		total_read += n_read;
		if (total_read == 0 || wbuf[total_read - 1] == L'\n') {
			break;
		}
		wbuflen += chunk_size;
		if (wbuf == wbuf_local) {
			wbuf[total_read] = '\0';
			wbuf = (wchar_t*)alifMem_dataAlloc(wbuflen * sizeof(wchar_t));
			if (wbuf) {
				wcscpy_s(wbuf, wbuflen, wbuf_local);
			}
			else {
				alifEval_restoreThread(tstate);
				//alifErr_noMemory();
				alifEval_saveThread();
				goto exit;
			}
		}
		else {
			wchar_t* tmp = (wchar_t*)alifMem_dataRealloc(wbuf, wbuflen * sizeof(wchar_t));
			if (tmp == nullptr) {
				alifEval_restoreThread(tstate);
				//alifErr_noMemory();
				alifEval_saveThread();
				goto exit;
			}
			wbuf = tmp;
		}
	}

	if (wbuf[0] == '\x1a') {
		buf = (char*)alifMem_dataAlloc(1);
		if (buf) {
			buf[0] = '\0';
		}
		else {
			alifEval_restoreThread(tstate);
			//alifErr_noMemory();
			alifEval_saveThread();
		}
		goto exit;
	}

	u8len = WideCharToMultiByte(CP_UTF8, 0,
		wbuf, total_read,
		nullptr, 0,
		nullptr, nullptr);
	buf = (char*)alifMem_dataAlloc(u8len + 1);
	if (buf == nullptr) {
		alifEval_restoreThread(tstate);
		//alifErr_noMemory();
		alifEval_saveThread();
		goto exit;
	}

	u8len = WideCharToMultiByte(CP_UTF8, 0,
		wbuf, total_read,
		buf, u8len,
		nullptr, nullptr);
	buf[u8len] = '\0';

exit:
	if (wbuf != wbuf_local) {
		alifMem_dataFree(wbuf);
	}

	if (err) {
		alifEval_restoreThread(tstate);
		//alifErr_setFromWindowsErr(err);
		alifEval_saveThread();
	}
	return buf;
}

#endif /* HAVE_WINDOWS_CONSOLE_IO */ // 250


char* alifOS_stdioReadline(FILE* sys_stdin,
	FILE* sys_stdout, const char* prompt) { // 255
	AlifUSizeT n{};
	char* p{}, * pr{};
	AlifThread* tstate = _alifOSReadlineThread_;

#ifdef HAVE_WINDOWS_CONSOLE_IO
	const AlifConfig* config = alifInterpreter_getConfig(tstate->interpreter);
	if (!config->legacyWindowsStdio and sys_stdin == stdin) {
		HANDLE hStdIn{}, hStdErr{};

		hStdIn = _alifGet_osfHandleNoRaise(fileno(sys_stdin));
		hStdErr = _alifGet_osfHandleNoRaise(fileno(stderr));

		if (_get_consoleType(hStdIn) == 'r') {
			fflush(sys_stdout);
			if (prompt) {
				if (_get_consoleType(hStdErr) == 'w') {
					wchar_t* wbuf{};
					AlifIntT wlen{};
					wlen = MultiByteToWideChar(CP_UTF8, 0, prompt, -1,
						nullptr, 0);
					if (wlen) {
						wbuf = (wchar_t*)alifMem_dataAlloc(wlen * sizeof(wchar_t));
						if (wbuf == nullptr) {
							alifEval_restoreThread(tstate);
							//alifErr_noMemory();
							alifEval_saveThread();
							return nullptr;
						}
						wlen = MultiByteToWideChar(CP_UTF8, 0, prompt, -1,
							wbuf, wlen);
						if (wlen) {
							DWORD n;
							fflush(stderr);
							/* wlen includes null terminator, so subtract 1 */
							WriteConsoleW(hStdErr, wbuf, wlen - 1, &n, nullptr);
						}
						alifMem_dataFree(wbuf);
					}
				}
				else {
					fprintf(stderr, "%s", prompt);
					fflush(stderr);
				}
			}
			clearerr(sys_stdin);
			return _alifOS_windowsConsoleReadline(tstate, hStdIn);
		}
	}
#endif

	fflush(sys_stdout);
	if (prompt) {
		fprintf(stderr, "%s", prompt);
	}
	fflush(stderr);

	n = 0;
	p = nullptr;
	do {
		size_t incr = (n > 0) ? n + 2 : 100;
		if (incr > INT_MAX) {
			alifMem_dataFree(p);
			alifEval_restoreThread(tstate);
			//alifErr_setString(_alifExcOverflowError_, "input line too long");
			alifEval_saveThread();
			return nullptr;
		}
		pr = (char*)alifMem_dataRealloc(p, n + incr);
		if (pr == nullptr) {
			alifMem_dataFree(p);
			alifEval_restoreThread(tstate);
			//alifErr_noMemory();
			alifEval_saveThread();
			return nullptr;
		}
		p = pr;
		AlifIntT err = my_fgets(tstate, p + n, (int)incr, sys_stdin);
		if (err == 1) {
			// Interrupt
			alifMem_dataFree(p);
			return nullptr;
		}
		else if (err != 0) {
			// EOF or error
			p[n] = '\0';
			break;
		}
		n += strlen(p + n);
	} while (p[n - 1] != '\n');

	pr = (char*)alifMem_dataRealloc(p, n + 1);
	if (pr == nullptr) {
		alifMem_dataFree(p);
		alifEval_restoreThread(tstate);
		//alifErr_noMemory();
		alifEval_saveThread();
		return nullptr;
	}
	return pr;
}



char* (*_alifOSReadlineFunctionPointer_)(FILE*, FILE*, const char*) = nullptr; // 364


char* alifOS_readline(FILE* sys_stdin,
	FILE* sys_stdout, const char* prompt) { // 369
	char* rv{}, * res{};
	AlifUSizeT len{};

	AlifThread* thread = _alifThread_get();
	if (alifAtomic_loadPtrRelaxed(&_alifOSReadlineThread_) == thread) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"can't re-enter readline");
		return nullptr;
	}

	ALIF_BEGIN_ALLOW_THREADS
	alifMutex_lock(&_alifOSReadlineLock_);
	alifAtomic_storePtrRelaxed(&_alifOSReadlineThread_, thread);
	if (_alifOSReadlineFunctionPointer_ == nullptr) {
		_alifOSReadlineFunctionPointer_ = alifOS_stdioReadline;
	}


	if (!isatty(fileno(sys_stdin)) or !isatty(fileno(sys_stdout)) ||
		!alif_isMainInterpreter(thread->interpreter))
	{
		rv = alifOS_stdioReadline(sys_stdin, sys_stdout, prompt);
	}
	else {
		rv = (*_alifOSReadlineFunctionPointer_)(sys_stdin, sys_stdout, prompt);
	}

	alifAtomic_storePtrRelaxed(&_alifOSReadlineThread_, nullptr);
	alifMutex_unlock(&_alifOSReadlineLock_);
	ALIF_END_ALLOW_THREADS

	if (rv == nullptr)
		return nullptr;

	len = strlen(rv) + 1;
	res = (char*)alifMem_dataAlloc(len);
	if (res != nullptr) {
		memcpy(res, rv, len);
	}
	else {
		//alifErr_noMemory();
	}
	alifMem_dataFree(rv);

	return res;
}
