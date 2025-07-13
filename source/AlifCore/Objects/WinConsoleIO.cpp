#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"

#ifdef HAVE_WINDOWS_CONSOLE_IO // 14


#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <stddef.h> /* For offsetof */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <fcntl.h>

#include "_IOModule.h"





#define SMALLBUF 4 // 49



char _get_consoleType(HANDLE handle) { // 51
	DWORD mode{}, peek_count{};

	if (handle == INVALID_HANDLE_VALUE)
		return '\0';

	if (!GetConsoleMode(handle, &mode))
		return '\0';

	/* Peek at the handle to see whether it is an input or output handle */
	if (GetNumberOfConsoleInputEvents(handle, &peek_count))
		return 'r';
	return 'w';
}


char _alifIO_getConsoleType(AlifObject* _pathOrFd) { // 66
	AlifIntT fd = alifLong_asLong(_pathOrFd);
	alifErr_clear();
	if (fd >= 0) {
		HANDLE handle = _alifGet_osfHandleNoRaise(fd);
		if (handle == INVALID_HANDLE_VALUE)
			return '\0';
		return _get_consoleType(handle);
	}

	AlifObject* decoded{};
	wchar_t* decodedWstr{};

	if (!alifUStr_fsDecoder(_pathOrFd, &decoded)) {
		alifErr_clear();
		return '\0';
	}
	decodedWstr = alifUStr_asWideCharString(decoded, nullptr);
	ALIF_CLEAR(decoded);
	if (!decodedWstr) {
		alifErr_clear();
		return '\0';
	}

	char m = '\0';
	if (!_wcsicmp(decodedWstr, L"CONIN$")) {
		m = 'r';
	}
	else if (!_wcsicmp(decodedWstr, L"CONOUT$")) {
		m = 'w';
	}
	else if (!_wcsicmp(decodedWstr, L"CON")) {
		m = 'x';
	}
	if (m) {
		alifMem_dataFree(decodedWstr);
		return m;
	}

	DWORD length{};
	wchar_t nameBuf[MAX_PATH], * pnameBuf = nameBuf;

	length = GetFullPathNameW(decodedWstr, MAX_PATH, pnameBuf, nullptr);
	if (length > MAX_PATH) {
		pnameBuf = (AlifUSizeT)(length) > ALIF_SIZET_MAX / sizeof(wchar_t) ? nullptr : \
			(wchar_t*)alifMem_dataAlloc((length) * sizeof(length));
		if (pnameBuf)
			length = GetFullPathNameW(decodedWstr, length, pnameBuf, nullptr);
		else
			length = 0;
	}
	alifMem_dataFree(decodedWstr);

	if (length) {
		wchar_t* name = pnameBuf;
		if (length >= 4 && name[3] == L'\\' &&
			(name[2] == L'.' || name[2] == L'?') &&
			name[1] == L'\\' && name[0] == L'\\') {
			name += 4;
		}
		if (!_wcsicmp(name, L"CONIN$")) {
			m = 'r';
		}
		else if (!_wcsicmp(name, L"CONOUT$")) {
			m = 'w';
		}
		else if (!_wcsicmp(name, L"CON")) {
			m = 'x';
		}
	}

	if (pnameBuf != nameBuf)
		alifMem_dataFree(pnameBuf);
	return m;
}















class WinConsoleIO { // 161
public:
	ALIFOBJECT_HEAD;
	AlifIntT fd{};
	AlifUIntT created : 1;
	AlifUIntT readable : 1;
	AlifUIntT writable : 1;
	AlifUIntT closefd : 1;
	char finalizing{};
	AlifUIntT blkSize{};
	AlifObject* weakRefList{};
	AlifObject* dict{};
	char buf[SMALLBUF]{};
	wchar_t wbuf{};
};



static AlifIntT internal_close(WinConsoleIO* _self) { // 184
	if (_self->fd != -1) {
		if (_self->closefd) {
			ALIF_BEGIN_SUPPRESS_IPH
			close(_self->fd);
			ALIF_END_SUPPRESS_IPH
		}
		_self->fd = -1;
	}
	return 0;
}




static AlifObject* winconsoleio_new(AlifTypeObject* _type, AlifObject* _args, AlifObject* _kwds) { // 237
	WinConsoleIO* self{};

	self = (WinConsoleIO*)_type->alloc(_type, 0);
	if (self != nullptr) {
		self->fd = -1;
		self->created = 0;
		self->readable = 0;
		self->writable = 0;
		self->closefd = 0;
		self->blkSize = 0;
		self->weakRefList = nullptr;
	}

	return (AlifObject*)self;
}




static AlifIntT _io_windowsConsoleIO__init__Impl(WinConsoleIO* self, AlifObject* nameobj,
	const char* mode, AlifIntT closefd, AlifObject* opener) { // 272
	const char* s{};
	wchar_t* name{};
	char console_type = '\0';
	AlifIntT ret = 0;
	AlifIntT rwa = 0;
	AlifIntT fd = -1;
	AlifIntT fd_is_own = 0;
	HANDLE handle{};

#ifndef NDEBUG
	AlifIOState* state = findIOState_byDef(ALIF_TYPE(self));
#endif
	if (self->fd >= 0) {
		if (self->closefd) {
			/* Have to close the existing file first. */
			if (internal_close(self) < 0)
				return -1;
		}
		else
			self->fd = -1;
	}

	if (ALIFBOOL_CHECK(nameobj)) {
		//if (alifErr_warnEx(_alifExcRuntimeWarning_,
		//	"bool is used as a file descriptor", 1))
		//{
		//	return -1;
		//}
	}
	fd = alifLong_asInt(nameobj);
	if (fd < 0) {
		if (!alifErr_occurred()) {
			alifErr_setString(_alifExcValueError_,
				"negative file descriptor");
			return -1;
		}
		alifErr_clear();
	}
	self->fd = fd;

	if (fd < 0) {
		AlifObject* decodedname{};

		AlifIntT d = alifUStr_fsDecoder(nameobj, (void*)&decodedname);
		if (!d)
			return -1;

		name = alifUStr_asWideCharString(decodedname, nullptr);
		console_type = _alifIO_getConsoleType(decodedname);
		ALIF_CLEAR(decodedname);
		if (name == nullptr)
			return -1;
	}

	s = mode;
	while (*s) {
		switch (*s++) {
		case '+':
		case 'a':
		case 'b':
		case 'x':
			break;
		case 'r':
			if (rwa)
				goto bad_mode;
			rwa = 1;
			self->readable = 1;
			if (console_type == 'x')
				console_type = 'r';
			break;
		case 'w':
			if (rwa)
				goto bad_mode;
			rwa = 1;
			self->writable = 1;
			if (console_type == 'x')
				console_type = 'w';
			break;
		default:
			alifErr_format(_alifExcValueError_,
				"invalid mode: %.200s", mode);
			goto error;
		}
	}

	if (!rwa)
		goto bad_mode;

	if (fd >= 0) {
		handle = _alifGet_osfHandleNoRaise(fd);
		self->closefd = 0;
	}
	else {
		DWORD access = GENERIC_READ;

		self->closefd = 1;
		if (!closefd) {
			alifErr_setString(_alifExcValueError_,
				"Cannot use closefd=False with file name");
			goto error;
		}

		if (self->writable)
			access = GENERIC_WRITE;

		ALIF_BEGIN_ALLOW_THREADS
			handle = CreateFileW(name, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (handle == INVALID_HANDLE_VALUE)
			handle = CreateFileW(name, access,
				FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		ALIF_END_ALLOW_THREADS

			if (handle == INVALID_HANDLE_VALUE) {
				alifErr_setExcFromWindowsErrWithFilenameObject(_alifExcOSError_, GetLastError(), nameobj);
				goto error;
			}

		if (self->writable)
			self->fd = _alifOpen_osfHandleNoRaise(handle, _O_WRONLY | _O_BINARY | _O_NOINHERIT);
		else
			self->fd = _alifOpen_osfHandleNoRaise(handle, _O_RDONLY | _O_BINARY | _O_NOINHERIT);
		if (self->fd < 0) {
			alifErr_setFromErrnoWithFilenameObject(_alifExcOSError_, nameobj);
			CloseHandle(handle);
			goto error;
		}
	}

	if (console_type == '\0')
		console_type = _get_consoleType(handle);

	if (self->writable and console_type != 'w') {
		alifErr_setString(_alifExcValueError_,
			"Cannot open console input buffer for writing");
		goto error;
	}
	if (self->readable and console_type != 'r') {
		alifErr_setString(_alifExcValueError_,
			"Cannot open console output buffer for reading");
		goto error;
	}

	self->blkSize = DEFAULT_BUFFER_SIZE;
	memset(self->buf, 0, 4);

	if (alifObject_setAttr((AlifObject*)self, &ALIF_STR(Name), nameobj) < 0)
		goto error;

	goto done;

bad_mode:
	alifErr_setString(_alifExcValueError_,
		"Must have exactly one of read or write mode");
error:
	ret = -1;
	internal_close(self);

done:
	if (name)
		alifMem_dataFree(name);
	return ret;
}




static AlifIntT winconsoleio_traverse(WinConsoleIO* self,
	VisitProc visit, void* arg) { // 446
	ALIF_VISIT(ALIF_TYPE(self));
	ALIF_VISIT(self->dict);
	return 0;
}


static AlifObject* err_closed(void) { // 476
	alifErr_setString(_alifExcValueError_, "I/O operation on closed file");
	return nullptr;
}



static AlifObject* _io_windowsConsoleIO_readableImpl(WinConsoleIO* self) { // 512
	if (self->fd == -1)
		return err_closed();
	return alifBool_fromLong((long)self->readable);
}



static AlifObject* _io_windowsConsoleIOIsAttyImpl(WinConsoleIO* _self) { // 1107
	if (_self->fd == -1)
		return err_closed();

	ALIF_RETURN_TRUE;
}


#include "clinic/WinConsoleIO.cpp.h" // 1118



static AlifMethodDef _winConsoleIOMethods_[] = { // 1121
	//_IO__WINDOWSCONSOLEIO_READ_METHODDEF
	//_IO__WINDOWSCONSOLEIO_READALL_METHODDEF
	//_IO__WINDOWSCONSOLEIO_READINTO_METHODDEF
	//_IO__WINDOWSCONSOLEIO_WRITE_METHODDEF
	//_IO__WINDOWSCONSOLEIO_CLOSE_METHODDEF
	_IO__WINDOWSCONSOLEIO_READABLE_METHODDEF
	//_IO__WINDOWSCONSOLEIO_WRITABLE_METHODDEF
	//_IO__WINDOWSCONSOLEIO_FILENO_METHODDEF
	//_IO__WINDOWSCONSOLEIO_ISATTY_METHODDEF
	{"_isAttyOpenOnly", (AlifCPPFunction)_io_windowsConsoleIOIsAtty, METHOD_NOARGS},
	{nullptr, nullptr}             /* sentinel */
};


static AlifMemberDef _winConsoleIOMembers_[] = { // 1163
	{"_blksize", ALIF_T_UINT, offsetof(WinConsoleIO, blkSize), 0},
	{"_finalizing", ALIF_T_BOOL, offsetof(WinConsoleIO, finalizing), 0},
	{"__weaklistoffset__", ALIF_T_ALIFSIZET, offsetof(WinConsoleIO, weakRefList), ALIF_READONLY},
	{"__dictoffset__", ALIF_T_ALIFSIZET, offsetof(WinConsoleIO, dict), ALIF_READONLY},
	{nullptr}
};


static AlifTypeSlot _winConsoleIOSlots_[] = { // 1171
	{ALIF_TP_GETATTRO, alifObject_genericGetAttr},
	{ALIF_TP_TRAVERSE, winconsoleio_traverse},
	{ALIF_TP_METHODS, _winConsoleIOMethods_},
	{ALIF_TP_MEMBERS, _winConsoleIOMembers_},
	{ALIF_TP_INIT, _io_windowsConsoleIO__init__},
	{ALIF_TP_NEW, winconsoleio_new},
	{0, nullptr},
};


AlifTypeSpec _winConsoleIOSpec_ = { // 1186
	.name = "تبادل.طرفية_ويندوز",
	.basicsize = sizeof(WinConsoleIO),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _winConsoleIOSlots_,
};


#endif /* HAVE_WINDOWS_CONSOLE_IO */
